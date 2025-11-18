/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_RSSI_TAG_BLOCK_IMPL_H
#define INCLUDED_QRADIOLINK_RSSI_TAG_BLOCK_IMPL_H

#include <gnuradio/qradiolink/rssi_tag_block.h>
#include <gnuradio/tags.h>

namespace gr {
namespace qradiolink {

class rssi_tag_block_impl : public rssi_tag_block
{
private:
    void add_rssi_tag(float db, uint64_t sample);
    float d_calibration_level;
    unsigned int d_nitems;
    float d_sum;

public:
    rssi_tag_block_impl();
    ~rssi_tag_block_impl() override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;

    void calibrate_rssi(float level) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_RSSI_TAG_BLOCK_IMPL_H */

