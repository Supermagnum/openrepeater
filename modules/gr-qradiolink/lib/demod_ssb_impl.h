/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_SSB_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_SSB_IMPL_H

#include <gnuradio/qradiolink/demod_ssb.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/analog/pwr_squelch_cc.h>
#include <gnuradio/filter/fft_filter_ccc.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/analog/agc2_cc.h>
#include <gnuradio/blocks/complex_to_real.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/qradiolink/clipper_cc.h>
#include <gnuradio/qradiolink/stretcher_cc.h>

namespace gr {
namespace qradiolink {

class demod_ssb_impl : public demod_ssb
{
private:
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::analog::pwr_squelch_cc::sptr d_squelch;
    gr::filter::fft_filter_ccc::sptr d_filter_usb;
    gr::filter::fft_filter_ccc::sptr d_filter_lsb;
    gr::filter::fft_filter_fff::sptr d_audio_filter;
    gr::analog::agc2_cc::sptr d_agc;
    gr::blocks::complex_to_real::sptr d_complex_to_real;
    gr::blocks::multiply_const_cc::sptr d_if_gain;
    gr::blocks::multiply_const_ff::sptr d_level_control;
    gr::qradiolink::clipper_cc::sptr d_clipper;
    gr::qradiolink::stretcher_cc::sptr d_stretcher;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_sps;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;
    int d_sb;

public:
    demod_ssb_impl(int sps, int samp_rate, int carrier_freq, int filter_width, int sb);
    ~demod_ssb_impl();

    void set_squelch(int value) override;
    void set_filter_width(int filter_width) override;
    void set_agc_attack(float value) override;
    void set_agc_decay(float value) override;
    void set_gain(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_SSB_IMPL_H */

