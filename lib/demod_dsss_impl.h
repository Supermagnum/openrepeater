/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_DSSS_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_DSSS_IMPL_H

#include <gnuradio/qradiolink/demod_dsss.h>
#include <gnuradio/blocks/complex_to_real.h>
#include <gnuradio/analog/agc2_cc.h>
#include <gnuradio/digital/clock_recovery_mm_cc.h>
#include <gnuradio/digital/costas_loop_cc.h>
#include <gnuradio/blocks/float_to_uchar.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/fec/decoder.h>
#include <gnuradio/fec/cc_decoder.h>
#include <gnuradio/blocks/delay.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/digital/descrambler_bb.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/dsss/dsss_decoder_cc.h>

namespace gr {
namespace qradiolink {

class demod_dsss_impl : public demod_dsss
{
private:
    gr::blocks::complex_to_real::sptr d_complex_to_real;
    gr::analog::agc2_cc::sptr d_agc;
    gr::dsss::dsss_decoder_cc::sptr d_dsss_decoder;
    gr::digital::clock_recovery_mm_cc::sptr d_clock_recovery;
    gr::digital::costas_loop_cc::sptr d_costas_freq;
    gr::digital::costas_loop_cc::sptr d_costas_loop;
    gr::blocks::float_to_uchar::sptr d_float_to_uchar;
    gr::blocks::add_const_ff::sptr d_add_const_fec;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::rational_resampler_ccf::sptr d_resampler_if;
    gr::filter::fft_filter_ccf::sptr d_filter;
    gr::digital::descrambler_bb::sptr d_descrambler;
    gr::digital::descrambler_bb::sptr d_descrambler2;
    gr::blocks::delay::sptr d_delay;
    gr::blocks::multiply_const_ff::sptr d_multiply_const_fec;
    gr::fec::decoder::sptr d_cc_decoder;
    gr::fec::decoder::sptr d_cc_decoder2;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;
    int d_if_samp_rate;

public:
    demod_dsss_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~demod_dsss_impl();
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_DSSS_IMPL_H */

