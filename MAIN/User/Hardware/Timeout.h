/**
  ******************************************************************************
  * @file    Timeout.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <stdint.h>

#define TIMEOUT_FLAGS_ACTIVE  0x01
#define TIMEOUT_FLAGS_ELAPSED 0x02

class Timeout
{
private:
    uint8_t flags;

    uint32_t delayTimeMillis;
	uint32_t startTimeMillis;

public:
    /// Instantiate a new timeout object.
	Timeout();

    /// Start a new timeout cycle that will elapse after the given amount of time.
    /// \param [in] duration_micros Microseconds until the timeout cycle should elapse.
	void start(uint32_t durationMillis);

    /// Test whether the current timeout cycle has elapsed. When called, this function will
    /// compare the system time to the calculated time that the timeout should expire, and
    /// if it has, the timer is marked as elapsed and not active.
    /// \return True if the timeout has elapsed.
	bool hasElapsed();

    /// \return True if the timeout is still running.
    bool isActive() const { return 0 != ( flags & TIMEOUT_FLAGS_ACTIVE ); }

    /// Stop the current timeout.
    void abort() { flags &= ~( TIMEOUT_FLAGS_ACTIVE ); }

    /// Clear the timeout so it can be used again
    void clear() { flags &= ~( TIMEOUT_FLAGS_ELAPSED ); }
};


#endif /* TIMEOUT_H */
