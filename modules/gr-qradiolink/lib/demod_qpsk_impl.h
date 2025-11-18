/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_QPSK_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_QPSK_IMPL_H

#include <gnuradio/qradiolink/demod_qpsk.h>
#include <gnuradio/digital/linear_equalizer.h>
#include <gnuradio/analog/agc2_cc.h>
#include <gnuradio/digital/fll_band_edge_cc.h>
#include <gnuradio/digital/pfb_clock_sync_ccf.h>
#include <gnuradio/digital/symbol_sync_cc.h>
#include <gnuradio/digital/costas_loop_cc.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/digital/constellation.h>
#include <gnuradio/digital/constellation_decoder_cb.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/digital/descrambler_bb.h>
#include <gnuradio/blocks/float_to_uchar.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/fec/decoder.h>
#include <gnuradio/fec/cc_decoder.h>
#include <gnuradio/digital/diff_phasor_cc.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/interleave.h>
#include <complex>

namespace gr {
namespace qradiolink {

class demod_qpsk_impl : public demod_qpsk
{
private:
    gr::digital::linear_equalizer::sptr d_equalizer;
    gr::analog::agc2_cc::sptr d_agc;
    gr::digital::fll_band_edge_cc::sptr d_fll;
    gr::digital::pfb_clock_sync_ccf::sptr d_clock_sync;
    gr::digital::symbol_sync_cc::sptr d_symbol_sync;
    gr::digital::costas_loop_cc::sptr d_costas_loop;
    gr::digital::costas_loop_cc::sptr d_costas_pll;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::filter::fft_filter_ccf::sptr d_shaping_filter;
    gr::filter::fft_filter_ccf::sptr d_filter;
    gr::digital::descrambler_bb::sptr d_descrambler;
    gr::fec::decoder::sptr d_decode_ccsds;
    gr::digital::diff_phasor_cc::sptr d_diff_phasor;
    gr::blocks::multiply_const_cc::sptr d_rotate_const;
    gr::blocks::multiply_const_ff::sptr d_multiply_const_fec;
    gr::blocks::complex_to_float::sptr d_complex_to_float;
    gr::blocks::interleave::sptr d_interleave;
    gr::blocks::float_to_uchar::sptr d_float_to_uchar;
    gr::blocks::add_const_ff::sptr d_add_const_fec;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;

public:
    demod_qpsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width);
    ~demod_qpsk_impl();
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_QPSK_IMPL_H */

