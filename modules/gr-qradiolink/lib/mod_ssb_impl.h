/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_MOD_SSB_IMPL_H
#define INCLUDED_QRADIOLINK_MOD_SSB_IMPL_H

#include <gnuradio/qradiolink/mod_ssb.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fft_filter_ccc.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/analog/agc2_ff.h>
#include <gnuradio/analog/feedforward_agc_cc.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/analog/rail_ff.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/qradiolink/clipper_cc.h>
#include <gnuradio/qradiolink/stretcher_cc.h>

// CESSB blocks - these are in src/gr/cessb
// For now, we'll reference them from the original location
// TODO: Migrate CESSB blocks to the module as well

namespace gr {
namespace qradiolink {

class mod_ssb_impl : public mod_ssb
{
private:
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::blocks::multiply_const_cc::sptr d_amplify;
    gr::blocks::multiply_const_cc::sptr d_bb_gain;
    gr::filter::fft_filter_fff::sptr d_audio_filter;
    gr::analog::agc2_ff::sptr d_agc;
    gr::analog::feedforward_agc_cc::sptr d_feed_forward_agc;
    gr::filter::fft_filter_ccc::sptr d_filter_usb;
    gr::filter::fft_filter_ccc::sptr d_filter_lsb;
    gr::blocks::float_to_complex::sptr d_float_to_complex;
    gr::analog::rail_ff::sptr d_rail;
    // CESSB blocks - migrated to qradiolink namespace
    gr::qradiolink::clipper_cc::sptr d_clipper;
    gr::qradiolink::stretcher_cc::sptr d_stretcher;

    int d_samp_rate;
    int d_sps;
    int d_carrier_freq;
    int d_filter_width;
    int d_sb; // 0 = USB, 1 = LSB

public:
    mod_ssb_impl(int sps, int samp_rate, int carrier_freq, int filter_width, int sb);
    ~mod_ssb_impl();

    void set_filter_width(int filter_width) override;
    void set_bb_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_MOD_SSB_IMPL_H */

