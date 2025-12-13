/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_DSSS_DSSS_DECODER_CC_IMPL_H
#define INCLUDED_DSSS_DSSS_DECODER_CC_IMPL_H

#include <gnuradio/dsss/dsss_decoder_cc.h>
#include <vector>

namespace gr {
namespace dsss {

class dsss_decoder_cc_impl : public dsss_decoder_cc
{
private:
    std::vector<int> d_spreading_code;
    std::vector<gr_complex> d_code_complex;
    size_t d_code_length;
    int d_samples_per_symbol;
    std::vector<gr_complex> d_correlation_buffer;

public:
    dsss_decoder_cc_impl(const std::vector<int>& spreading_code, int samples_per_symbol);
    ~dsss_decoder_cc_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace dsss
} // namespace gr

#endif /* INCLUDED_DSSS_DSSS_DECODER_CC_IMPL_H */

