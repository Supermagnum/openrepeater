/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */


#include <climits>
#include <cstdint>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gnuradio/qradiolink/demod_2fsk.h>
#include "demod_2fsk_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/digital/constellation.h>
#include <cmath>

namespace gr {
namespace qradiolink {

demod_2fsk::sptr demod_2fsk::make(int sps, int samp_rate, int carrier_freq, int filter_width, bool fm)
{
    std::vector<int> signature;
    signature.push_back(sizeof(gr_complex));
    signature.push_back(sizeof(gr_complex));
    signature.push_back(sizeof(char));
    signature.push_back(sizeof(char));
    return gnuradio::get_initial_sptr(
        new demod_2fsk_impl(sps, samp_rate, carrier_freq, filter_width, fm));
}

demod_2fsk_impl::demod_2fsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width, bool fm)
    : demod_2fsk("demod_2fsk",
                 gr::io_signature::make(1, 1, sizeof(gr_complex)),
                 gr::io_signature::makev(4, 4, [&]() {
                     std::vector<int> sig;
                     sig.push_back(sizeof(gr_complex));
                     sig.push_back(sizeof(gr_complex));
                     sig.push_back(sizeof(char));
                     sig.push_back(sizeof(char));
                     return sig;
                 }()))
{
    int decim, interp, nfilts;
    if (sps == 10) {
        d_target_samp_rate = 20000;
        d_samples_per_symbol = sps;
        decim = 50;
        interp = 1;
        nfilts = 35 * d_samples_per_symbol;
    } else if (sps >= 5) {
        d_target_samp_rate = 40000;
        d_samples_per_symbol = sps * 2;
        decim = 25;
        interp = 1;
        nfilts = 35 * d_samples_per_symbol;
    } else if (sps == 1) {
        d_target_samp_rate = 80000;
        d_samples_per_symbol = 4;
        decim = 25;
        interp = 2;
        nfilts = 125 * d_samples_per_symbol;
    }
    int spacing = 2;
    if (fm)
        spacing = 1;
    if ((nfilts % 2) == 0)
        nfilts += 1;

    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;

    std::vector<int> map;
    map.push_back(0);
    map.push_back(1);

    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);

    std::vector<float> taps = gr::filter::firdes::low_pass(
        interp, interp * d_samp_rate, d_target_samp_rate / 2, d_target_samp_rate / 2,
        gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> symbol_filter_taps = gr::filter::firdes::low_pass(
        1.0, d_target_samp_rate, d_target_samp_rate / d_samples_per_symbol,
        d_target_samp_rate / d_samples_per_symbol, gr::fft::window::WIN_HAMMING);
    d_resampler = gr::filter::rational_resampler_ccf::make(interp, decim, taps);
    d_fll = gr::digital::fll_band_edge_cc::make(d_samples_per_symbol, 0.1, 16, 24 * M_PI / 100);
    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass(
            1, d_target_samp_rate, d_filter_width, d_filter_width,
            gr::fft::window::WIN_BLACKMAN_HARRIS));

    d_upper_filter = gr::filter::fft_filter_ccc::make(
        1,
        gr::filter::firdes::complex_band_pass(
            1, d_target_samp_rate, -d_filter_width, 0, d_filter_width,
            gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_lower_filter = gr::filter::fft_filter_ccc::make(
        1,
        gr::filter::firdes::complex_band_pass(
            1, d_target_samp_rate, 0, d_filter_width, d_filter_width,
            gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_mag_lower = gr::blocks::complex_to_mag::make();
    d_mag_upper = gr::blocks::complex_to_mag::make();
    d_divide = gr::blocks::divide_ff::make();
    d_add = gr::blocks::add_const_ff::make(-1);
    d_rail = gr::analog::rail_ff::make(0, 2);
    d_float_to_complex = gr::blocks::float_to_complex::make();
    d_symbol_filter = gr::filter::fft_filter_fff::make(1, symbol_filter_taps);

    float symbol_rate = ((float)d_target_samp_rate / (float)d_samples_per_symbol);
    float sps_deviation = 200.0f / symbol_rate;
    d_symbol_sync = gr::digital::symbol_sync_ff::make(
        gr::digital::TED_MOD_MUELLER_AND_MULLER, d_samples_per_symbol,
        2 * M_PI / (symbol_rate / 10), 1.0, 0.2869, sps_deviation, 1,
        gr::digital::constellation_bpsk::make());

    d_freq_demod = gr::analog::quadrature_demod_cf::make(d_samples_per_symbol / (spacing * M_PI / 2));
    d_shaping_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::root_raised_cosine(
            1, d_target_samp_rate, d_target_samp_rate / d_samples_per_symbol, 0.2, nfilts));
    d_multiply_const_fec = gr::blocks::multiply_const_ff::make(128);
    d_float_to_uchar = gr::blocks::float_to_uchar::make();
    d_add_const_fec = gr::blocks::add_const_ff::make(128.0);

    gr::fec::code::cc_decoder::sptr decoder = gr::fec::code::cc_decoder::make(80, 7, 2, polys);
    gr::fec::code::cc_decoder::sptr decoder2 = gr::fec::code::cc_decoder::make(80, 7, 2, polys);
    d_cc_decoder = gr::fec::decoder::make(decoder, 1, 1);
    d_cc_decoder2 = gr::fec::decoder::make(decoder2, 1, 1);

    d_delay = gr::blocks::delay::make(1, 1);
    d_descrambler = gr::digital::descrambler_bb::make(0x8A, 0x7F, 7);
    d_descrambler2 = gr::digital::descrambler_bb::make(0x8A, 0x7F, 7);

    connect(self(), 0, d_resampler, 0);
    connect(d_resampler, 0, d_fll, 0);
    connect(d_fll, 0, d_filter, 0);
    connect(d_filter, 0, self(), 0);
    if (fm) {
        connect(d_filter, 0, d_freq_demod, 0);
        connect(d_freq_demod, 0, d_shaping_filter, 0);
        connect(d_shaping_filter, 0, d_symbol_sync, 0);
    } else {
        connect(d_filter, 0, d_lower_filter, 0);
        connect(d_filter, 0, d_upper_filter, 0);
        connect(d_lower_filter, 0, d_mag_lower, 0);
        connect(d_upper_filter, 0, d_mag_upper, 0);
        connect(d_mag_lower, 0, d_divide, 1);
        connect(d_mag_upper, 0, d_divide, 0);
        connect(d_divide, 0, d_rail, 0);
        connect(d_rail, 0, d_add, 0);
        connect(d_add, 0, d_symbol_filter, 0);
        connect(d_symbol_filter, 0, d_symbol_sync, 0);
    }
    connect(d_symbol_sync, 0, d_float_to_complex, 0);
    connect(d_float_to_complex, 0, self(), 1);
    connect(d_symbol_sync, 0, d_multiply_const_fec, 0);
    connect(d_multiply_const_fec, 0, d_add_const_fec, 0);
    connect(d_add_const_fec, 0, d_float_to_uchar, 0);
    connect(d_float_to_uchar, 0, d_cc_decoder, 0);
    connect(d_cc_decoder, 0, d_descrambler, 0);
    connect(d_descrambler, 0, self(), 2);
    connect(d_float_to_uchar, 0, d_delay, 0);
    connect(d_delay, 0, d_cc_decoder2, 0);
    connect(d_cc_decoder2, 0, d_descrambler2, 0);
    connect(d_descrambler2, 0, self(), 3);
}

demod_2fsk_impl::~demod_2fsk_impl() {}

} // namespace qradiolink
} // namespace gr

