/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Mock BurstTimer implementation for testing
 */

#ifndef TEST_BURSTTIMER_H
#define TEST_BURSTTIMER_H

#include "../src/bursttimer.h"
#include <cstdint>
#include <map>
#include <mutex>

/**
 * MockBurstTimer - Test implementation of BurstTimer for unit testing
 */
class MockBurstTimer : public BurstTimer {
private:
    std::map<int, bool> timing_initialized;
    std::map<int, uint64_t> timers;
    std::map<int, int> slot_numbers;
    std::mutex mtx;
    
public:
    MockBurstTimer() {
        // Initialize timing for all channels by default
        for (int i = 0; i < 7; i++) {
            timing_initialized[i] = true;
            timers[i] = 0;
            slot_numbers[i] = 0;
        }
    }
    
    virtual ~MockBurstTimer() = default;
    
    uint64_t allocate_slot(int slot_number, int64_t timing_correction, int channel) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (channel >= 0 && channel < 7) {
            slot_numbers[channel] = slot_number;
            return 1000000000ULL; // Return 1 second in nanoseconds
        }
        return 0;
    }
    
    int check_time(int channel) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (channel >= 0 && channel < 7 && timing_initialized[channel]) {
            return slot_numbers[channel];
        }
        return 0;
    }
    
    void set_timer(uint64_t nsec, int channel) override {
        std::lock_guard<std::mutex> lock(mtx);
        if (channel >= 0 && channel < 7) {
            timers[channel] = nsec;
            timing_initialized[channel] = true;
        }
    }
    
    bool get_timing_initialized(int channel) const override {
        if (channel >= 0 && channel < 7) {
            return timing_initialized.at(channel);
        }
        return false;
    }
    
    // Test helper methods
    void set_timing_initialized(int channel, bool initialized) {
        std::lock_guard<std::mutex> lock(mtx);
        if (channel >= 0 && channel < 7) {
            timing_initialized[channel] = initialized;
        }
    }
    
    void set_slot_number(int channel, int slot) {
        std::lock_guard<std::mutex> lock(mtx);
        if (channel >= 0 && channel < 7) {
            slot_numbers[channel] = slot;
        }
    }
};

#endif /* TEST_BURSTTIMER_H */

