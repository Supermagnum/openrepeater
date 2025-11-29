/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_STRETCHER_CC_IMPL_H
#define INCLUDED_QRADIOLINK_STRETCHER_CC_IMPL_H

#include <gnuradio/qradiolink/stretcher_cc.h>
#include <volk/volk.h>

#define CHUNK_SIZE 1024

namespace gr {
namespace qradiolink {

class stretcher_cc_impl : public stretcher_cc
{
private:
    float d_env[CHUNK_SIZE + 4];
    float d_envhold[CHUNK_SIZE];
    float d_ones[CHUNK_SIZE];
    float d_real[CHUNK_SIZE];
    float d_imag[CHUNK_SIZE];

public:
    stretcher_cc_impl();
    ~stretcher_cc_impl();

    void forecast(int noutput_items, gr_vector_int& ninput_items_required) override;

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_STRETCHER_CC_IMPL_H */

