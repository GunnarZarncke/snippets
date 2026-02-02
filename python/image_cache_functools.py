#!/usr/bin/env python3
"""
Minimal image fetcher with LRU caching.
"""

import hashlib
import urllib.request
from functools import lru_cache
from pathlib import Path
from typing import Optional

class ImageCache:
    """Image cache with LRU eviction policy."""
    
    def __init__(self, cache_dir: str = ".image_cache", max_size: int = 10):
        self.cache_dir = Path(cache_dir)
        self.cache_dir.mkdir(exist_ok=True)
        self.max_size = max_size
        # Create cached method with dynamic maxsize
        self._get_cached = lru_cache(maxsize=max_size)(self._get_impl)
    
    def _url_to_filename(self, url: str) -> str:
        """Convert URL to cache filename."""
        url_hash = hashlib.md5(url.encode()).hexdigest()
        extension = Path(url).suffix or '.jpg'
        return f"{url_hash}{extension}"
    
    def _get_cache_path(self, url: str) -> Path:
        """Get cache file path for URL."""
        return self.cache_dir / self._url_to_filename(url)
    
    def _evict_oldest_file(self):
        """Evict oldest file from cache directory when max size exceeded."""
        files = [(f.stat().st_mtime, f) for f in self.cache_dir.glob("*") if f.is_file()]
        if len(files) > self.max_size:
            files.sort()
            oldest_file = files[0][1]
            oldest_file.unlink()
            print(f"Evicted LRU file: {oldest_file}")
    
    def is_cached(self, url: str) -> bool:
        """Check if image is already cached."""
        return self._get_cache_path(url).exists()
    
    def _get_impl(self, url: str) -> Optional[Path]:
        """Internal implementation for getting cached image."""
        cache_path = self._get_cache_path(url)
        
        if cache_path.exists():
            print(f"Using cached image: {cache_path}")
            return cache_path
        
        return self._fetch_impl(url)
    
    def _fetch_impl(self, url: str) -> Optional[Path]:
        """Internal implementation for fetching image."""
        cache_path = self._get_cache_path(url)
        
        # Evict oldest file if cache is full
        self._evict_oldest_file()
        
        try:
            print(f"Fetching image from: {url}")
            with urllib.request.urlopen(url, timeout=10) as response:
                if response.status == 200:
                    data = response.read()
                    cache_path.write_bytes(data)
                    print(f"Cached image: {cache_path} ({len(data)} bytes)")
                    return cache_path
                else:
                    print(f"Error: HTTP {response.status}")
                    return None
        except Exception as e:
            print(f"Error fetching image: {e}")
            return None
    
    def fetch(self, url: str, force_refresh: bool = False) -> Optional[Path]:
        """Fetch image from URL and cache it locally with LRU."""
        if force_refresh:
            # Clear from LRU cache if present
            self._get_cached.cache_clear()
            return self._fetch_impl(url)
        return self._get_cached(url)
    
    def get(self, url: str) -> Optional[Path]:
        """Get image path (from cache or fetch)."""
        return self._get_cached(url)
    
    def clear(self):
        """Clear all cached images."""
        for file in self.cache_dir.glob("*"):
            if file.is_file():
                file.unlink()
        self._get_cached.cache_clear()
        print(f"Cleared cache directory: {self.cache_dir}")

def main():
    """Example usage."""
    cache = ImageCache(max_size=3)
    
    test_urls = [
        "https://httpbin.org/image/jpeg",
        "https://httpbin.org/image/png",
        "https://httpbin.org/image/webp",
        "https://httpbin.org/image/svg",
    ]
    
    print("Fetching images (cache size: 3)...")
    for url in test_urls:
        image_path = cache.get(url)
        if image_path:
            print(f"  Image saved to: {image_path}\n")
    
    print("\nFetching first image again (should be in cache)...")
    image_path = cache.get(test_urls[0])
    if image_path:
        print(f"  Image from cache: {image_path}\n")
    
    cache_info = cache._get_cached.cache_info()
    print(f"LRU cache stats: {cache_info.hits} hits, {cache_info.misses} misses, {cache_info.currsize}/{cache.max_size} entries")

if __name__ == '__main__':
    main()
