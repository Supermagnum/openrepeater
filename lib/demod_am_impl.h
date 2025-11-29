/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_AM_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_AM_IMPL_H

#include <gnuradio/qradiolink/demod_am.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/analog/pwr_squelch_cc.h>
#include <gnuradio/filter/fft_filter_ccc.h>
#include <gnuradio/analog/agc2_ff.h>
#include <gnuradio/blocks/complex_to_mag.h>
#include <gnuradio/filter/iir_filter_ffd.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/fft_filter_fff.h>

namespace gr {
namespace qradiolink {

class demod_am_impl : public demod_am
{
private:
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::rational_resampler_fff::sptr d_audio_resampler;
    gr::analog::pwr_squelch_cc::sptr d_squelch;
    gr::filter::fft_filter_ccc::sptr d_filter;
    gr::analog::agc2_ff::sptr d_agc;
    gr::blocks::complex_to_mag::sptr d_complex_to_mag;
    gr::filter::iir_filter_ffd::sptr d_iir_filter;
    gr::blocks::multiply_const_ff::sptr d_audio_gain;
    gr::filter::fft_filter_fff::sptr d_audio_filter;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;

public:
    demod_am_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~demod_am_impl();

    void set_squelch(int value) override;
    void set_filter_width(int filter_width) override;
    void set_agc_attack(float value) override;
    void set_agc_decay(float value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_AM_IMPL_H */

