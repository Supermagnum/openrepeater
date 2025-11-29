/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_AM_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_AM_IMPL_H

#include <gnuradio/qradiolink/mod_am.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fft_filter_ccc.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/analog/agc2_ff.h>
#include <gnuradio/analog/feedforward_agc_cc.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/analog/rail_ff.h>
#include <gnuradio/fft/window.h>

namespace gr {
namespace qradiolink {

class mod_am_impl : public mod_am
{
private:
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::blocks::multiply_const_ff::sptr d_audio_amplify;
    gr::filter::fft_filter_fff::sptr d_audio_filter;
    gr::filter::fft_filter_ccc::sptr d_filter;
    gr::analog::agc2_ff::sptr d_agc;
    gr::analog::feedforward_agc_cc::sptr d_feed_forward_agc;
    gr::analog::sig_source_f::sptr d_signal_source;
    gr::blocks::add_ff::sptr d_add;
    gr::blocks::float_to_complex::sptr d_float_to_complex;
    gr::analog::rail_ff::sptr d_rail;

    int d_samp_rate;
    int d_sps;
    int d_carrier_freq;
    int d_filter_width;

public:
    mod_am_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~mod_am_impl();

    void set_filter_width(int filter_width) override;
    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_AM_IMPL_H */

