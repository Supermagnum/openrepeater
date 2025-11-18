/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_DEMOD_4FSK_IMPL_H
#define INCLUDED_QRADIOLINK_DEMOD_4FSK_IMPL_H

#include <gnuradio/qradiolink/demod_4fsk.h>
#include <gnuradio/blocks/unpack_k_bits_bb.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/complex_to_mag.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/digital/constellation.h>
#include <gnuradio/digital/symbol_sync_ff.h>
#include <gnuradio/digital/symbol_sync_cc.h>
#include <gnuradio/digital/constellation_decoder_cb.h>
#include <gnuradio/filter/fft_filter_ccf.h>
#include <gnuradio/filter/fft_filter_ccc.h>
#include <gnuradio/filter/fft_filter_fff.h>
#include <gnuradio/digital/descrambler_bb.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/analog/phase_modulator_fc.h>
#include <gnuradio/blocks/float_to_uchar.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/analog/rail_ff.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/interleave.h>
#include <gnuradio/fec/decoder.h>
#include <gnuradio/fec/cc_decoder.h>
#include <gnuradio/qradiolink/gr_4fsk_discriminator.h>

namespace gr {
namespace qradiolink {

class demod_4fsk_impl : public demod_4fsk
{
private:
    gr::blocks::unpack_k_bits_bb::sptr d_unpack;
    gr::filter::fft_filter_ccc::sptr d_filter1;
    gr::filter::fft_filter_ccc::sptr d_filter2;
    gr::filter::fft_filter_ccc::sptr d_filter3;
    gr::filter::fft_filter_ccc::sptr d_filter4;
    gr::blocks::complex_to_mag::sptr d_mag1;
    gr::blocks::complex_to_mag::sptr d_mag2;
    gr::blocks::complex_to_mag::sptr d_mag3;
    gr::blocks::complex_to_mag::sptr d_mag4;
    gr::qradiolink::gr_4fsk_discriminator::sptr d_discriminator;
    gr::blocks::multiply_const_cc::sptr d_multiply_symbols;
    gr::analog::quadrature_demod_cf::sptr d_freq_demod;
    gr::blocks::float_to_complex::sptr d_float_to_complex;
    gr::filter::fft_filter_ccf::sptr d_symbol_filter;
    gr::filter::rational_resampler_ccf::sptr d_resampler;
    gr::digital::constellation_decoder_cb::sptr d_constellation_receiver;
    gr::digital::symbol_sync_ff::sptr d_symbol_sync;
    gr::digital::symbol_sync_cc::sptr d_symbol_sync_complex;
    gr::filter::fft_filter_ccf::sptr d_filter;
    gr::digital::descrambler_bb::sptr d_descrambler;
    gr::blocks::multiply_const_ff::sptr d_multiply_const_fec;
    gr::blocks::complex_to_float::sptr d_complex_to_float;
    gr::blocks::interleave::sptr d_interleave;
    gr::blocks::float_to_uchar::sptr d_float_to_uchar;
    gr::blocks::add_const_ff::sptr d_add_const_fec;
    gr::fec::decoder::sptr d_decode_ccsds;
    gr::filter::fft_filter_fff::sptr d_shaping_filter;
    gr::analog::phase_modulator_fc::sptr d_phase_mod;

    int d_samples_per_symbol;
    int d_samp_rate;
    int d_carrier_freq;
    int d_filter_width;
    int d_target_samp_rate;
    bool d_fm;

public:
    demod_4fsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width, bool fm);
    ~demod_4fsk_impl();
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_DEMOD_4FSK_IMPL_H */

