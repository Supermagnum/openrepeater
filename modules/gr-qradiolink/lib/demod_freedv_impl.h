/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_FREEDV_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_FREEDV_IMPL_H

#include <gnuradio/qradiolink/demod_freedv.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/analog/agc2_ff.h>
#include <gnuradio/analog/feedforward_agc_cc.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/filter/fft_filter_ccc.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/blocks/complex_to_real.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/vocoder/freedv_rx_ss.h>
#include <vector>

namespace gr {
namespace qradiolink {

class demod_freedv_impl : public demod_freedv
{
private:
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::fft_filter_ccc::sptr d_filter;
    gr::analog::agc2_ff::sptr d_agc;
    gr::analog::feedforward_agc_cc::sptr d_feed_forward_agc;
    gr::blocks::complex_to_real::sptr d_complex_to_real;
    gr::blocks::float_to_short::sptr d_float_to_short;
    gr::blocks::short_to_float::sptr d_short_to_float;
    gr::blocks::multiply_const_ff::sptr d_audio_gain;
    gr::blocks::multiply_const_ff::sptr d_freedv_gain;
    gr::filter::fft_filter_fff::sptr d_audio_filter;
    gr::vocoder::freedv_rx_ss::sptr d_freedv;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;

public:
    demod_freedv_impl(int sps,
                     int samp_rate,
                     int carrier_freq,
                     int filter_width,
                     int low_cutoff,
                     int mode,
                     int sb);
    ~demod_freedv_impl();

    void set_agc_attack(float value) override;
    void set_agc_decay(float value) override;
    void set_squelch(int value) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_FREEDV_IMPL_H */

