#!/usr/bin/env python3
"""
Minimal image fetcher with LRU caching.
"""

import hashlib
import urllib.request
from collections import OrderedDict
from pathlib import Path
from typing import Optional

class ImageCache:
    """Image cache with LRU eviction policy."""
    
    def __init__(self, cache_dir: str = ".image_cache", max_size: int = 10):
        self.cache_dir = Path(cache_dir)
        self.cache_dir.mkdir(exist_ok=True)
        self.max_size = max_size
        self.lru_cache = OrderedDict()
    
    def _url_to_filename(self, url: str) -> str:
        """Convert URL to cache filename."""
        url_hash = hashlib.md5(url.encode()).hexdigest()
        extension = Path(url).suffix or '.jpg'
        return f"{url_hash}{extension}"
    
    def _get_cache_path(self, url: str) -> Path:
        """Get cache file path for URL."""
        return self.cache_dir / self._url_to_filename(url)
    
    def _evict_lru(self):
        """Evict least recently used item from cache."""
        if self.lru_cache:
            lru_url, _ = self.lru_cache.popitem(last=False)
            cache_path = self._get_cache_path(lru_url)
            if cache_path.exists():
                cache_path.unlink()
            print(f"Evicted LRU: {lru_url}")
    
    def _touch(self, url: str):
        """Move URL to end (most recently used)."""
        if url in self.lru_cache:
            self.lru_cache.move_to_end(url)
    
    def is_cached(self, url: str) -> bool:
        """Check if image is already cached."""
        return self._get_cache_path(url).exists()
    
    def fetch(self, url: str, force_refresh: bool = False) -> Optional[Path]:
        """Fetch image from URL and cache it locally with LRU."""
        cache_path = self._get_cache_path(url)
        
        if not force_refresh and cache_path.exists():
            self._touch(url)
            print(f"Using cached image: {cache_path}")
            return cache_path
        
        while len(self.lru_cache) >= self.max_size:
            self._evict_lru()
        
        try:
            print(f"Fetching image from: {url}")
            with urllib.request.urlopen(url, timeout=10) as response:
                if response.status == 200:
                    data = response.read()
                    cache_path.write_bytes(data)
                    self.lru_cache[url] = cache_path
                    self._touch(url)
                    print(f"Cached image: {cache_path} ({len(data)} bytes)")
                    return cache_path
                else:
                    print(f"Error: HTTP {response.status}")
                    return None
        except Exception as e:
            print(f"Error fetching image: {e}")
            return None
    
    def get(self, url: str) -> Optional[Path]:
        """Get image path (from cache or fetch)."""
        cache_path = self._get_cache_path(url)
        
        if cache_path.exists():
            # Add to LRU cache if not already tracked
            if url not in self.lru_cache:
                self.lru_cache[url] = cache_path
            self._touch(url)
            return cache_path
        
        return self.fetch(url)
    
    def clear(self):
        """Clear all cached images."""
        for file in self.cache_dir.glob("*"):
            if file.is_file():
                file.unlink()
        self.lru_cache.clear()
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
    
    print(f"Cache size: {len(cache.lru_cache)}/{cache.max_size}")

if __name__ == '__main__':
    main()
