/**
 * This code displays the timing issue in mutex locking as exhibited by GCC 4.8.
 * Parent/main function locks a mutex then starts the thread. This thread tries
 * to lock the same mutex for a specified duration (5 second in this example).
 * Once lock succeed (because main is expected to unlock mutex in 2 seconds),
 * thread waits for another 5 seconds on a conditional variable.
 *
 * Using gcc 4.6 behaviour is following showing right timeline of events.
 * 1410448076263 M mutex locked
 * 1410448076264 T lock try
 * 1410448078264 M mutex unlocked
 * 1410448078264 T lock success
 *
 * Using gcc 4.9 same code fails to acheive above results. Timed mutex lock
 * fails as soon as it is executed instead of waiting for provided time period.
 * errno is also zero when lock failed.
 * 1410448141287 M mutex locked
 * 1410448141288 T lock try
 * 1410448141288 T lock failed
 * 1410448143288 M mutex unlocked
 *
 * Following is how code was compiled.
 * g++-4.6 -std=c++0x -pthread -o timing.4.6 timing.cpp
 * g++-4.8 -std=c++11x -pthread -o timing.4.8 timing.cpp
 * */
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <unistd.h>

std::timed_mutex mutex;		/**< Mutex to lock */

/** Macro to print message and time in unix format */
#define LOG(Message_) std::cout \
		<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() \
		<< Message_ << std::endl

/**
 * Thread runs this function, exectes a timed lock for 5 seconds and reports
 * success or failure.
 * */
void f()
{
	LOG(" T lock try");
	std::unique_lock<std::timed_mutex> lk(mutex, std::chrono::seconds(5));
	if (!lk) LOG(" T lock failed");
	else LOG(" T lock success");
}

/**
 * main thread that locks the mutex, starts thread then unlocks the mutex after
 * 2 seconds. That allows child thread to lock the mutex.
 * */
int main()
{
	mutex.lock();
	LOG(" M mutex locked");

	std::thread t(f);
	sleep(2);
	mutex.unlock();
	LOG(" M mutex unlocked");
	sleep(2);
	t.join();
	return 0;
}

// vim:ts=8:sw=8:syntax=cpp
