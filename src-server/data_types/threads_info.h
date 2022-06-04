#ifndef ROBOTS_THREADS_INFO_H
#define ROBOTS_THREADS_INFO_H

#include "definitions.h"

class ThreadsInfo {
    std::mutex mutex;
    std::condition_variable condition_variable;
    bool should_exit = false;
public:
    auto &get_mutex() {
        return mutex;
    }

    auto &get_condition_variable() {
        return condition_variable;
    }

    bool get_should_exit() const {
        return should_exit;
    }

    void set_should_exit() {
        should_exit = true;
        condition_variable.notify_all();
    }
};


#endif //ROBOTS_THREADS_INFO_H
