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

#include <gnuradio/qradiolink/demod_bpsk.h>
#include "demod_bpsk_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/window.h>
#include <cmath>

namespace gr {
namespace qradiolink {

demod_bpsk::sptr demod_bpsk::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(new demod_bpsk_impl(sps, samp_rate, carrier_freq, filter_width));
}

demod_bpsk_impl::demod_bpsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : demod_bpsk("demod_bpsk",
                 gr::io_signature::make(1, 1, sizeof(gr_complex)),
                 gr::io_signature::makev(4, 4, std::vector<int>{sizeof(gr_complex), sizeof(gr_complex), sizeof(char), sizeof(char)}))
{
    d_target_samp_rate = 20000;
    d_samples_per_symbol = sps;
    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;

    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);

    std::vector<float> taps = gr::filter::firdes::low_pass(
        1, d_samp_rate, d_target_samp_rate / 2, d_target_samp_rate / 2, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(1, 50, taps);
    d_resampler->set_thread_priority(99);
    d_agc = gr::analog::agc2_cc::make(1e-1, 1e-1, 1, 1);
    float gain_mu = 0.05;
    float gain_omega = 0.005;
    d_clock_recovery = gr::digital::clock_recovery_mm_cc::make(
        d_samples_per_symbol, gain_omega * gain_omega, 0.5, gain_mu, 0.001);
    d_costas_loop = gr::digital::costas_loop_cc::make(2 * M_PI / 200, 2);
    d_fll = gr::digital::fll_band_edge_cc::make(d_samples_per_symbol, 0.35, 32, 8 * M_PI / 100);
    d_shaping_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::root_raised_cosine(
            d_samples_per_symbol, d_samples_per_symbol, 1, 0.35, 15 * d_samples_per_symbol));
    d_complex_to_real = gr::blocks::complex_to_real::make();

    d_multiply_const_fec = gr::blocks::multiply_const_ff::make(64);
    d_float_to_uchar = gr::blocks::float_to_uchar::make();
    d_add_const_fec = gr::blocks::add_const_ff::make(128.0);

    gr::fec::code::cc_decoder::sptr decoder = gr::fec::code::cc_decoder::make(80, 7, 2, polys);
    gr::fec::code::cc_decoder::sptr decoder2 = gr::fec::code::cc_decoder::make(80, 7, 2, polys);
    d_cc_decoder = gr::fec::decoder::make(decoder, 1, 1);
    d_cc_decoder2 = gr::fec::decoder::make(decoder2, 1, 1);

    d_descrambler = gr::digital::descrambler_bb::make(0x8A, 0x7F, 7);
    d_delay = gr::blocks::delay::make(1, 1);
    d_descrambler2 = gr::digital::descrambler_bb::make(0x8A, 0x7F, 7);

    connect(self(), 0, d_resampler, 0);
    connect(d_resampler, 0, d_fll, 0);
    connect(d_fll, 0, d_shaping_filter, 0);
    connect(d_shaping_filter, 0, self(), 0);
    connect(d_shaping_filter, 0, d_agc, 0);
    connect(d_agc, 0, d_clock_recovery, 0);
    connect(d_clock_recovery, 0, d_costas_loop, 0);
    connect(d_costas_loop, 0, self(), 1);
    connect(d_costas_loop, 0, d_complex_to_real, 0);
    connect(d_complex_to_real, 0, d_multiply_const_fec, 0);
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

demod_bpsk_impl::~demod_bpsk_impl() {}

} // namespace qradiolink
} // namespace gr

