/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_FREEDV_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_FREEDV_IMPL_H

#include <gnuradio/qradiolink/mod_freedv.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/filter/fft_filter_ccc.h>
#include <gnuradio/analog/agc2_ff.h>
#include <gnuradio/analog/feedforward_agc_cc.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/vocoder/freedv_tx_ss.h>
#include <gnuradio/filter/firdes.h>

namespace gr {
namespace qradiolink {

class mod_freedv_impl : public mod_freedv
{
private:
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::filter::fft_filter_fff::sptr d_audio_filter;
    gr::analog::agc2_ff::sptr d_agc;
    gr::analog::feedforward_agc_cc::sptr d_feed_forward_agc;
    gr::filter::fft_filter_ccc::sptr d_filter;
    gr::blocks::float_to_complex::sptr d_float_to_complex;
    gr::vocoder::freedv_tx_ss::sptr d_freedv;
    gr::blocks::float_to_short::sptr d_float_to_short;
    gr::blocks::short_to_float::sptr d_short_to_float;
    gr::blocks::multiply_const_ff::sptr d_audio_gain;

    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;

public:
    mod_freedv_impl(int sps, int samp_rate, int carrier_freq, int filter_width, int low_cutoff, int mode, int sb);
    ~mod_freedv_impl();

    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_FREEDV_IMPL_H */

