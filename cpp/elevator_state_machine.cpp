#include <iostream>
#include <string>
#include <optional>

enum class State {
    IDLE,
    MOVING_UP,
    MOVING_DOWN,
    DOORS_OPEN
};

std::string state_to_string(State state) {
    switch (state) {
        case State::IDLE: return "idle";
        case State::MOVING_UP: return "moving_up";
        case State::MOVING_DOWN: return "moving_down";
        case State::DOORS_OPEN: return "doors_open";
        default: return "unknown";
    }
}

class Elevator {
private:
    int floors;
    int current_floor;
    State state;
    std::optional<int> target_floor;
    
public:
    Elevator(int floors = 10) 
        : floors(floors), current_floor(1), state(State::IDLE), target_floor(std::nullopt) {}
    
    bool request_floor(int floor) {
        if (floor < 1 || floor > floors) {
            return false;
        }
        
        if (state == State::DOORS_OPEN) {
            if (floor == current_floor) {
                return true;
            }
            state = State::IDLE;
        }
        
        target_floor = floor;
        
        if (floor > current_floor) {
            state = State::MOVING_UP;
        } else if (floor < current_floor) {
            state = State::MOVING_DOWN;
        } else {
            state = State::DOORS_OPEN;
        }
        
        return true;
    }
    
    void move() {
        if (state == State::MOVING_UP) {
            if (target_floor && current_floor < *target_floor) {
                current_floor++;
                if (current_floor == *target_floor) {
                    state = State::DOORS_OPEN;
                    target_floor = std::nullopt;
                }
            }
        } else if (state == State::MOVING_DOWN) {
            if (target_floor && current_floor > *target_floor) {
                current_floor--;
                if (current_floor == *target_floor) {
                    state = State::DOORS_OPEN;
                    target_floor = std::nullopt;
                }
            }
        }
    }
    
    void close_doors() {
        if (state == State::DOORS_OPEN) {
            state = State::IDLE;
        }
    }
    
    std::string get_status() const {
        std::string status = "Floor " + std::to_string(current_floor) + 
                           ", State: " + state_to_string(state);
        if (target_floor) {
            status += ", Target: " + std::to_string(*target_floor);
        }
        return status;
    }
    
    State get_state() const { return state; }
};

int main() {
    Elevator elevator(5);
    
    std::cout << "Initial state:" << std::endl;
    std::cout << "  " << elevator.get_status() << std::endl;
    
    std::cout << "\nRequesting floor 3:" << std::endl;
    elevator.request_floor(3);
    std::cout << "  " << elevator.get_status() << std::endl;
    
    std::cout << "\nMoving..." << std::endl;
    while (elevator.get_state() != State::DOORS_OPEN) {
        elevator.move();
        std::cout << "  " << elevator.get_status() << std::endl;
    }
    
    std::cout << "\nClosing doors:" << std::endl;
    elevator.close_doors();
    std::cout << "  " << elevator.get_status() << std::endl;
    
    std::cout << "\nRequesting floor 1:" << std::endl;
    elevator.request_floor(1);
    std::cout << "  " << elevator.get_status() << std::endl;
    
    std::cout << "\nMoving..." << std::endl;
    while (elevator.get_state() != State::DOORS_OPEN) {
        elevator.move();
        std::cout << "  " << elevator.get_status() << std::endl;
    }
    
    return 0;
}
