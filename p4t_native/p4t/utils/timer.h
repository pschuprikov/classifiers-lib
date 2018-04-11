#ifndef TIMER_H
#define TIMER_H

namespace p4t::utils {

struct Timer {
    Timer (string command) 
        : start_time_{std::clock()}, command_{std::move(command)} {
    }

    ~Timer() {
        log()->info(
            "execution of {} took {} ms", 
            command_, (std::clock() - start_time_) * 1000.0 / CLOCKS_PER_SEC
        );
    }
    
private:
    std::clock_t start_time_;
    string const command_;
};

}

#endif
