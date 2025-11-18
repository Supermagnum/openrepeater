/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_NBFM_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_NBFM_IMPL_H

#include <gnuradio/qradiolink/demod_nbfm.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/filter/iir_filter_ffd.h>
#include <gnuradio/analog/pwr_squelch_cc.h>
#include <gnuradio/analog/ctcss_squelch_ff.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/rational_resampler.h>
#include <vector>

namespace gr {
namespace qradiolink {

class demod_nbfm_impl : public demod_nbfm
{
private:
    gr::analog::quadrature_demod_cf::sptr d_fm_demod;
    gr::filter::iir_filter_ffd::sptr d_de_emph_filter;
    gr::analog::pwr_squelch_cc::sptr d_squelch;
    gr::blocks::multiply_const_ff::sptr d_level_control;
    gr::analog::ctcss_squelch_ff::sptr d_ctcss;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::rational_resampler_fff::sptr d_audio_resampler;
    gr::filter::fft_filter_ccf::sptr d_filter;
    gr::filter::fft_filter_fff::sptr d_audio_filter;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;
    std::vector<double> d_btaps;
    std::vector<double> d_ataps;

public:
    demod_nbfm_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~demod_nbfm_impl();

    void set_squelch(int value) override;
    void set_ctcss(float value) override;
    void set_filter_width(int filter_width) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_NBFM_IMPL_H */

