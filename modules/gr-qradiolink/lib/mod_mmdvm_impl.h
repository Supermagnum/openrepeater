/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_MMDVM_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_MMDVM_IMPL_H

#include <gnuradio/qradiolink/mod_mmdvm.h>
#include <gnuradio/analog/frequency_modulator_fc.h>
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/qradiolink/zero_idle_bursts.h>

namespace gr {
namespace qradiolink {

class mod_mmdvm_impl : public mod_mmdvm
{
private:
    gr::analog::frequency_modulator_fc::sptr d_fm_modulator;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::blocks::multiply_const_ff::sptr d_audio_amplify;
    gr::filter::fft_filter_ccf::sptr d_filter;
    gr::blocks::short_to_float::sptr d_short_to_float;
    gr::qradiolink::zero_idle_bursts::sptr d_zero_idle_bursts;

    int d_samp_rate;
    int d_sps;
    int d_carrier_freq;
    int d_filter_width;

public:
    mod_mmdvm_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~mod_mmdvm_impl();

    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_MMDVM_IMPL_H */

