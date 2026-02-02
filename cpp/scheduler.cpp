#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <sstream>

struct Interval {
    int start;
    int end;
    
    Interval(int start, int end) : start(start), end(end) {}
    
    bool overlaps(const Interval& other) const {
        return start < other.end && end > other.start;
    }
    
    std::string to_string() const {
        std::ostringstream oss;
        oss << "[" << std::setfill('0') << std::setw(2) << start 
            << ":00-" << std::setw(2) << end << ":00]";
        return oss.str();
    }
};

class Scheduler {
private:
    std::vector<Interval> intervals;
    int day_start;
    int day_end;
    
public:
    Scheduler(int day_start = 9, int day_end = 17) 
        : day_start(day_start), day_end(day_end) {}
    
    bool add(int start, int end) {
        Interval new_interval(start, end);
        
        if (new_interval.start >= new_interval.end) {
            return false;
        }
        
        for (const auto& existing : intervals) {
            if (new_interval.overlaps(existing)) {
                return false;
            }
        }
        
        intervals.push_back(new_interval);
        std::sort(intervals.begin(), intervals.end(),
                  [](const Interval& a, const Interval& b) {
                      return a.start < b.start;
                  });
        return true;
    }
    
    bool remove(int start, int end) {
        Interval target(start, end);
        for (auto it = intervals.begin(); it != intervals.end(); ++it) {
            if (it->start == target.start && it->end == target.end) {
                intervals.erase(it);
                return true;
            }
        }
        return false;
    }
    
    std::vector<Interval> free_slots(int min_duration = 1) {
        std::vector<Interval> free;
        int current = day_start;
        
        std::vector<Interval> sorted_intervals = intervals;
        std::sort(sorted_intervals.begin(), sorted_intervals.end(),
                  [](const Interval& a, const Interval& b) {
                      return a.start < b.start;
                  });
        
        for (const auto& interval : sorted_intervals) {
            if (current < interval.start) {
                if (interval.start - current >= min_duration) {
                    free.push_back(Interval(current, interval.start));
                }
            }
            current = std::max(current, interval.end);
        }
        
        if (current < day_end) {
            if (day_end - current >= min_duration) {
                free.push_back(Interval(current, day_end));
            }
        }
        
        return free;
    }
    
    std::vector<Interval> list_all() {
        std::vector<Interval> sorted_intervals = intervals;
        std::sort(sorted_intervals.begin(), sorted_intervals.end(),
                  [](const Interval& a, const Interval& b) {
                      return a.start < b.start;
                  });
        return sorted_intervals;
    }
    
    std::vector<Interval> find_conflicts(int start, int end) {
        Interval candidate(start, end);
        std::vector<Interval> conflicts;
        
        for (const auto& iv : intervals) {
            if (candidate.overlaps(iv)) {
                conflicts.push_back(iv);
            }
        }
        
        return conflicts;
    }
};

int main() {
    Scheduler scheduler(9, 17);
    
    std::cout << "Adding meetings..." << std::endl;
    scheduler.add(10, 11);
    scheduler.add(13, 14);
    scheduler.add(15, 16);
    
    std::cout << "\nScheduled intervals:" << std::endl;
    for (const auto& interval : scheduler.list_all()) {
        std::cout << "  " << interval.to_string() << std::endl;
    }
    
    std::cout << "\nFree slots:" << std::endl;
    for (const auto& slot : scheduler.free_slots()) {
        std::cout << "  " << slot.to_string() << std::endl;
    }
    
    std::cout << "\nTrying to add overlapping meeting (10:30-11:30):" << std::endl;
    if (scheduler.add(10, 11)) {
        std::cout << "  Added successfully" << std::endl;
    } else {
        std::cout << "  Failed - conflicts with existing meeting" << std::endl;
    }
    
    std::cout << "\nConflicts for 10:30-11:30:" << std::endl;
    auto conflicts = scheduler.find_conflicts(10, 11);
    for (const auto& conflict : conflicts) {
        std::cout << "  " << conflict.to_string() << std::endl;
    }
    
    return 0;
}
