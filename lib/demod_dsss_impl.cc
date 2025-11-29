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

#include <gnuradio/qradiolink/demod_dsss.h>
#include "demod_dsss_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/window.h>
#include <cmath>
// DSSS decoder - Barker-13 spreading code correlation
#include "dsss_decoder_cc_impl.h"
using gr::dsss::dsss_decoder_cc;

namespace gr {
namespace qradiolink {

demod_dsss::sptr demod_dsss::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(new demod_dsss_impl(sps, samp_rate, carrier_freq, filter_width));
}

demod_dsss_impl::demod_dsss_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : demod_dsss("demod_dsss",
                 gr::io_signature::make(1, 1, sizeof(gr_complex)),
                 gr::io_signature::makev(4, 4, std::vector<int>{sizeof(gr_complex), sizeof(gr_complex), sizeof(char), sizeof(char)}))
{
    d_if_samp_rate = 20000;
    d_target_samp_rate = 5200;
    d_samples_per_symbol = sps;
    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    static const int barker_13[] = {1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1};
    std::vector<int> dsss_code(barker_13, barker_13 + sizeof(barker_13) / sizeof(barker_13[0]));

    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);
    float gain_omega, gain_mu, omega_rel_limit;
    gain_omega = 0.005;
    gain_mu = 0.05;
    omega_rel_limit = 0.005;

    std::vector<float> taps = gr::filter::firdes::low_pass(
        1, d_samp_rate, d_if_samp_rate / 2, d_if_samp_rate / 2, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(1, 50, taps);
    d_resampler->set_thread_priority(99);

    std::vector<float> taps_if = gr::filter::firdes::low_pass(
        1, d_if_samp_rate, d_target_samp_rate / 2, d_target_samp_rate / 2, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler_if = gr::filter::rational_resampler_ccf::make(13, 50, taps_if);

    d_agc = gr::analog::agc2_cc::make(1e-1, 1e-1, 1, 10);
    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass(
            1, d_target_samp_rate, d_filter_width, 1200, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_costas_loop = gr::digital::costas_loop_cc::make(2 * M_PI / 100, 2);
    d_costas_freq = gr::digital::costas_loop_cc::make(M_PI / 200, 2, true);
    // DSSS decoder - Barker-13 spreading code correlation
    gr::dsss::dsss_decoder_cc::sptr d_dsss_decoder = gr::dsss::dsss_decoder_cc::make(dsss_code, d_samples_per_symbol);
    d_complex_to_real = gr::blocks::complex_to_real::make();
    d_clock_recovery = gr::digital::clock_recovery_mm_cc::make(
        1, gain_omega * gain_omega, 0.5, gain_mu, omega_rel_limit);

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
    connect(d_resampler, 0, d_resampler_if, 0);
    connect(d_resampler_if, 0, d_costas_freq, 0);
    connect(d_costas_freq, 0, d_filter, 0);
    connect(d_filter, 0, d_agc, 0);
    connect(d_filter, 0, self(), 0);
    connect(d_agc, 0, d_dsss_decoder, 0);
    connect(d_dsss_decoder, 0, d_clock_recovery, 0);
    connect(d_clock_recovery, 0, d_costas_loop, 0);
    connect(d_costas_loop, 0, d_complex_to_real, 0);
    connect(d_costas_loop, 0, self(), 1);
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

demod_dsss_impl::~demod_dsss_impl() {}

} // namespace qradiolink
} // namespace gr

