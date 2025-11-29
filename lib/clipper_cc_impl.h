/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_CLIPPER_CC_IMPL_H
#define INCLUDED_QRADIOLINK_CLIPPER_CC_IMPL_H

#include <gnuradio/qradiolink/clipper_cc.h>

#define CHUNK_SIZE 1024

namespace gr {
namespace qradiolink {

class clipper_cc_impl : public clipper_cc
{
private:
    float d_magnitude[CHUNK_SIZE];
    float d_phase[CHUNK_SIZE];
    float d_clipped[CHUNK_SIZE];
    float d_cliplevel[CHUNK_SIZE];
    float d_phase_cos[CHUNK_SIZE];
    float d_phase_sin[CHUNK_SIZE];
    float d_clip;

public:
    clipper_cc_impl(float clip);
    ~clipper_cc_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_CLIPPER_CC_IMPL_H */

