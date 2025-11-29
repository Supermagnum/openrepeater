/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_ZERO_IDLE_BURSTS_IMPL_H
#define INCLUDED_QRADIOLINK_ZERO_IDLE_BURSTS_IMPL_H

#include <gnuradio/qradiolink/zero_idle_bursts.h>

namespace gr {
namespace qradiolink {

class zero_idle_bursts_impl : public zero_idle_bursts
{
private:
    uint64_t d_sample_counter;
    unsigned int d_delay;

public:
    zero_idle_bursts_impl(unsigned int delay = 0);
    ~zero_idle_bursts_impl() override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_ZERO_IDLE_BURSTS_IMPL_H */

