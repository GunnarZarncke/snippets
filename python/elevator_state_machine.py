#!/usr/bin/env python3
"""
Minimal elevator state machine implementation.
"""

from enum import Enum
from typing import Optional

class State(Enum):
    """Elevator states."""
    IDLE = "idle"
    MOVING_UP = "moving_up"
    MOVING_DOWN = "moving_down"
    DOORS_OPEN = "doors_open"

class Elevator:
    """Elevator with state machine."""
    
    def __init__(self, floors: int = 10):
        self.floors = floors
        self.current_floor = 1
        self.state = State.IDLE
        self.target_floor: Optional[int] = None
    
    def request_floor(self, floor: int) -> bool:
        """Request elevator to go to a floor."""
        if not 1 <= floor <= self.floors:
            return False
        
        if self.state == State.DOORS_OPEN:
            if floor == self.current_floor:
                return True
            self.state = State.IDLE
        
        self.target_floor = floor
        
        if floor > self.current_floor:
            self.state = State.MOVING_UP
        elif floor < self.current_floor:
            self.state = State.MOVING_DOWN
        else:
            self.state = State.DOORS_OPEN
        
        return True
    
    def move(self):
        """Move elevator one floor based on current state."""
        if self.state == State.MOVING_UP:
            if self.current_floor < self.target_floor:
                self.current_floor += 1
                if self.current_floor == self.target_floor:
                    self.state = State.DOORS_OPEN
                    self.target_floor = None
        elif self.state == State.MOVING_DOWN:
            if self.current_floor > self.target_floor:
                self.current_floor -= 1
                if self.current_floor == self.target_floor:
                    self.state = State.DOORS_OPEN
                    self.target_floor = None
    
    def close_doors(self):
        """Close doors and return to idle."""
        if self.state == State.DOORS_OPEN:
            self.state = State.IDLE
    
    def get_status(self) -> str:
        """Get current status string."""
        status = f"Floor {self.current_floor}, State: {self.state.value}"
        if self.target_floor:
            status += f", Target: {self.target_floor}"
        return status

def main():
    """Example usage."""
    elevator = Elevator(floors=5)
    
    print("Initial state:")
    print(f"  {elevator.get_status()}")
    
    print("\nRequesting floor 3:")
    elevator.request_floor(3)
    print(f"  {elevator.get_status()}")
    
    print("\nMoving...")
    while elevator.state != State.DOORS_OPEN:
        elevator.move()
        print(f"  {elevator.get_status()}")
    
    print("\nClosing doors:")
    elevator.close_doors()
    print(f"  {elevator.get_status()}")
    
    print("\nRequesting floor 1:")
    elevator.request_floor(1)
    print(f"  {elevator.get_status()}")
    
    print("\nMoving...")
    while elevator.state != State.DOORS_OPEN:
        elevator.move()
        print(f"  {elevator.get_status()}")

if __name__ == '__main__':
    main()
