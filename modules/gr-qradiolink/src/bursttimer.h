/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * BurstTimer class for MMDVM TDMA timing coordination
 * This is a minimal interface definition for the application-level BurstTimer class
 */

#ifndef BURSTTIMER_H
#define BURSTTIMER_H

#include <cstdint>

/**
 * BurstTimer - Application-level class for TDMA timing coordination
 * 
 * This class manages TDMA slot timing for MMDVM (Multi-Mode Digital Voice Modem).
 * It coordinates transmission timing across multiple channels and slots.
 */
class BurstTimer {
public:
    /**
     * Allocate a TDMA slot for transmission
     * 
     * @param slot_number Slot number (1 or 2)
     * @param timing_correction Timing correction value
     * @param channel Channel number
     * @return Timestamp in nanoseconds, or 0 if allocation failed
     */
    virtual uint64_t allocate_slot(int slot_number, int64_t timing_correction, int channel) = 0;
    
    /**
     * Check if it's time to transmit on a channel
     * 
     * @param channel Channel number
     * @return Slot number if it's time to transmit, 0 otherwise
     */
    virtual int check_time(int channel) = 0;
    
    /**
     * Set the timer for a channel
     * 
     * @param nsec Timestamp in nanoseconds
     * @param channel Channel number
     */
    virtual void set_timer(uint64_t nsec, int channel) = 0;
    
    /**
     * Check if timing has been initialized for a channel
     * 
     * @param channel Channel number
     * @return true if timing is initialized, false otherwise
     */
    virtual bool get_timing_initialized(int channel) const = 0;
    
    virtual ~BurstTimer() = default;
};

#endif /* BURSTTIMER_H */

