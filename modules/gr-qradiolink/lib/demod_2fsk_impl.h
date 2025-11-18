/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_2FSK_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_2FSK_IMPL_H

#include <gnuradio/qradiolink/demod_2fsk.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/digital/symbol_sync_ff.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/fft_filter_ccc.h>
#include <gnuradio/digital/fll_band_edge_cc.h>
#include <gnuradio/blocks/complex_to_mag.h>
#include <gnuradio/blocks/divide.h>
#include <gnuradio/analog/rail_ff.h>
#include <gnuradio/digital/binary_slicer_fb.h>
#include <gnuradio/digital/descrambler_bb.h>
#include <gnuradio/blocks/delay.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/blocks/float_to_uchar.h>
#include <gnuradio/fec/cc_decoder.h>
#include <gnuradio/fec/decoder.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/filter/firdes.h>

namespace gr {
namespace qradiolink {

class demod_2fsk_impl : public demod_2fsk
{
private:
    gr::blocks::multiply_const_cc::sptr d_multiply_symbols;
    gr::blocks::float_to_complex::sptr d_float_to_complex;
    gr::filter::fft_filter_fff::sptr d_symbol_filter;
    gr::digital::symbol_sync_ff::sptr d_symbol_sync;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::fft_filter_ccf::sptr d_filter;
    gr::digital::fll_band_edge_cc::sptr d_fll;
    gr::filter::fft_filter_ccc::sptr d_lower_filter;
    gr::filter::fft_filter_ccc::sptr d_upper_filter;
    gr::blocks::complex_to_mag::sptr d_mag_lower;
    gr::blocks::complex_to_mag::sptr d_mag_upper;
    gr::blocks::divide_ff::sptr d_divide;
    gr::analog::rail_ff::sptr d_rail;
    gr::digital::binary_slicer_fb::sptr d_binary_slicer;
    gr::digital::descrambler_bb::sptr d_descrambler;
    gr::digital::descrambler_bb::sptr d_descrambler2;
    gr::blocks::delay::sptr d_delay;
    gr::blocks::multiply_const_ff::sptr d_multiply_const_fec;
    gr::blocks::add_const_ff::sptr d_add;
    gr::blocks::float_to_uchar::sptr d_float_to_uchar;
    gr::blocks::add_const_ff::sptr d_add_const_fec;
    gr::fec::decoder::sptr d_cc_decoder;
    gr::fec::decoder::sptr d_cc_decoder2;
    gr::analog::quadrature_demod_cf::sptr d_freq_demod;
    gr::filter::fft_filter_fff::sptr d_shaping_filter;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;

public:
    demod_2fsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width, bool fm);
    ~demod_2fsk_impl();
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_2FSK_IMPL_H */

