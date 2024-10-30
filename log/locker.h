#ifndef LOCKER_H
#define LOCKER_H

#include <mutex>
#include <stdexcept>

class locker {
public:
    locker() = default;

    ~locker() = default;

    void lock() {
        m_mutex.lock();
    }

    void unlock() {
        m_mutex.unlock();
    }

private:
    std::mutex m_mutex;
};

#endif // LOCKER_H
