/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_DSSS_DSSS_ENCODER_BB_IMPL_H
#define INCLUDED_DSSS_DSSS_ENCODER_BB_IMPL_H

#include <gnuradio/dsss/dsss_encoder_bb.h>
#include <vector>

namespace gr {
namespace dsss {

class dsss_encoder_bb_impl : public dsss_encoder_bb
{
private:
    std::vector<int> d_spreading_code;
    size_t d_code_length;

public:
    dsss_encoder_bb_impl(const std::vector<int>& spreading_code);
    ~dsss_encoder_bb_impl();

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace dsss
} // namespace gr

#endif /* INCLUDED_DSSS_DSSS_ENCODER_BB_IMPL_H */

