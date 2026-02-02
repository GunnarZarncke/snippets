#!/usr/bin/env python3
"""
Minimal scheduler for managing time intervals and finding free slots.
"""

from dataclasses import dataclass
from typing import List, Tuple

@dataclass
class Interval:
    """Time interval with start and end times."""
    start: int
    end: int
    
    def overlaps(self, other: 'Interval') -> bool:
        """Check if this interval overlaps with another."""
        return self.start < other.end and self.end > other.start
    
    def __str__(self):
        return f"[{self.start:02d}:00-{self.end:02d}:00]"

class Scheduler:
    """Minimal scheduler for managing intervals."""
    
    def __init__(self, day_start: int = 9, day_end: int = 17):
        self.intervals: List[Interval] = []
        self.day_start = day_start
        self.day_end = day_end
    
    def add(self, start: int, end: int) -> bool:
        """Add interval if it doesn't overlap with existing ones."""
        new_interval = Interval(start, end)
        
        if new_interval.start >= new_interval.end:
            return False
        
        for existing in self.intervals:
            if new_interval.overlaps(existing):
                return False
        
        self.intervals.append(new_interval)
        self.intervals.sort(key=lambda x: x.start)
        return True
    
    def remove(self, start: int, end: int) -> bool:
        """Remove matching interval."""
        target = Interval(start, end)
        for i, interval in enumerate(self.intervals):
            if interval.start == target.start and interval.end == target.end:
                self.intervals.pop(i)
                return True
        return False
    
    def free_slots(self, min_duration: int = 1) -> List[Interval]:
        """Find free time slots between scheduled intervals."""
        free = []
        current = self.day_start
        
        for interval in sorted(self.intervals, key=lambda x: x.start):
            if current < interval.start:
                if interval.start - current >= min_duration:
                    free.append(Interval(current, interval.start))
            current = max(current, interval.end)
        
        if current < self.day_end:
            if self.day_end - current >= min_duration:
                free.append(Interval(current, self.day_end))
        
        return free
    
    def list_all(self) -> List[Interval]:
        """List all scheduled intervals."""
        return sorted(self.intervals, key=lambda x: x.start)
    
    def find_conflicts(self, start: int, end: int) -> List[Interval]:
        """Find intervals that conflict with given time."""
        candidate = Interval(start, end)
        return [iv for iv in self.intervals if candidate.overlaps(iv)]

def main():
    """Example usage."""
    scheduler = Scheduler(day_start=9, day_end=17)
    
    print("Adding meetings...")
    scheduler.add(10, 11)
    scheduler.add(13, 14)
    scheduler.add(15, 16)
    
    print("\nScheduled intervals:")
    for interval in scheduler.list_all():
        print(f"  {interval}")
    
    print("\nFree slots:")
    for slot in scheduler.free_slots():
        print(f"  {slot}")
    
    print("\nTrying to add overlapping meeting (10:30-11:30):")
    if scheduler.add(10, 11):
        print("  Added successfully")
    else:
        print("  Failed - conflicts with existing meeting")
    
    print("\nConflicts for 10:30-11:30:")
    conflicts = scheduler.find_conflicts(10, 11)
    for conflict in conflicts:
        print(f"  {conflict}")

if __name__ == '__main__':
    main()
