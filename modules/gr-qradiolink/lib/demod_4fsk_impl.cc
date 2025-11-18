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

#include <gnuradio/qradiolink/demod_4fsk.h>
#include "demod_4fsk_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/digital/constellation.h>
#include <cmath>
#include <gnuradio/qradiolink/gr_4fsk_discriminator.h>

namespace gr {
namespace qradiolink {

demod_4fsk::sptr demod_4fsk::make(int sps, int samp_rate, int carrier_freq, int filter_width, bool fm)
{
    return gnuradio::get_initial_sptr(new demod_4fsk_impl(sps, samp_rate, carrier_freq, filter_width, fm));
}

demod_4fsk_impl::demod_4fsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width, bool fm)
    : demod_4fsk("demod_4fsk",
                 gr::io_signature::make(1, 1, sizeof(gr_complex)),
                 gr::io_signature::makev(3, 3, std::vector<int>{sizeof(gr_complex), sizeof(gr_complex), sizeof(char)}))
{
    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    d_fm = fm;

    int rs, bw, decimation, interpolation, nfilts;

    if (sps == 1) {
        d_target_samp_rate = 80000;
        d_samples_per_symbol = sps * 8;
        decimation = 25;
        interpolation = 2;
        rs = 10000;
        bw = 4000;
        nfilts = 32 * d_samples_per_symbol;
    }
    if (sps == 5) {
        d_target_samp_rate = 20000;
        d_samples_per_symbol = sps * 2;
        decimation = 50;
        interpolation = 1;
        rs = 2000;
        bw = 4000;
        nfilts = 25 * d_samples_per_symbol;
    }
    if (sps == 10) {
        d_target_samp_rate = 10000;
        d_samples_per_symbol = sps;
        decimation = 100;
        interpolation = 1;
        rs = 1000;
        bw = 2000;
        nfilts = 25 * d_samples_per_symbol;
    }
    if (sps == 2) {
        interpolation = 1;
        decimation = 2;
        d_samples_per_symbol = 5;
        d_target_samp_rate = 500000;
        nfilts = 50 * d_samples_per_symbol;
    }
    if ((nfilts % 2) == 0)
        nfilts += 1;

    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);
    std::vector<gr_complex> constellation_points;
    constellation_points.push_back(gr_complex(-1.5, 0));
    constellation_points.push_back(gr_complex(-0.5, 0));
    constellation_points.push_back(gr_complex(0.5, 0));
    constellation_points.push_back(gr_complex(1.5f, 0));

    std::vector<int> pre_diff;

    gr::digital::constellation_rect::sptr constellation_4fsk = gr::digital::constellation_rect::make(
        constellation_points, pre_diff, 2, 4, 1, 1.0, 1.0);

    int spacing = 1;

    std::vector<float> taps = gr::filter::firdes::low_pass(
        interpolation, interpolation * d_samp_rate, d_target_samp_rate / 2, d_target_samp_rate / 2,
        gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> symbol_filter_taps = gr::filter::firdes::low_pass(
        1.0, d_target_samp_rate, d_target_samp_rate / d_samples_per_symbol,
        d_target_samp_rate / d_samples_per_symbol / 20, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(interpolation, decimation, taps);
    d_resampler->set_thread_priority(99);
    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass(
            1, d_target_samp_rate, d_filter_width, d_filter_width / 2, gr::fft::window::WIN_BLACKMAN_HARRIS));
    gr::qradiolink::gr_4fsk_discriminator::sptr d_discriminator;
    if (!d_fm) {
        d_filter1 = gr::filter::fft_filter_ccc::make(
            1,
            gr::filter::firdes::complex_band_pass(
                1, d_target_samp_rate, -d_filter_width, -d_filter_width + rs, bw,
                gr::fft::window::WIN_BLACKMAN_HARRIS));
        d_filter2 = gr::filter::fft_filter_ccc::make(
            1,
            gr::filter::firdes::complex_band_pass(
                1, d_target_samp_rate, -d_filter_width + rs, 0, bw, gr::fft::window::WIN_BLACKMAN_HARRIS));
        d_filter3 = gr::filter::fft_filter_ccc::make(
            1,
            gr::filter::firdes::complex_band_pass(
                1, d_target_samp_rate, 0, d_filter_width - rs, bw, gr::fft::window::WIN_BLACKMAN_HARRIS));
        d_filter4 = gr::filter::fft_filter_ccc::make(
            1,
            gr::filter::firdes::complex_band_pass(
                1, d_target_samp_rate, d_filter_width - rs, d_filter_width, bw,
                gr::fft::window::WIN_BLACKMAN_HARRIS));
        d_mag1 = gr::blocks::complex_to_mag::make();
        d_mag2 = gr::blocks::complex_to_mag::make();
        d_mag3 = gr::blocks::complex_to_mag::make();
        d_mag4 = gr::blocks::complex_to_mag::make();
        d_discriminator = gr::qradiolink::gr_4fsk_discriminator::make();
    }

    d_phase_mod = gr::analog::phase_modulator_fc::make(M_PI / 2);
    d_symbol_filter = gr::filter::fft_filter_ccf::make(1, symbol_filter_taps);

    d_freq_demod = gr::analog::quadrature_demod_cf::make(d_samples_per_symbol / (spacing * M_PI));
    d_shaping_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::root_raised_cosine(
            1.5, d_target_samp_rate, d_target_samp_rate / d_samples_per_symbol, 0.2, nfilts));
    float sps_deviation = 0.05f;
    d_symbol_sync = gr::digital::symbol_sync_ff::make(
        gr::digital::TED_MOD_MUELLER_AND_MULLER, d_samples_per_symbol, 2 * M_PI / 200.0f, 1.0, 0.2869,
        sps_deviation, 1, constellation_4fsk);
    d_symbol_sync_complex = gr::digital::symbol_sync_cc::make(
        gr::digital::TED_MOD_MUELLER_AND_MULLER, d_samples_per_symbol, 2 * M_PI / 200.0f, 1.0, 0.2869,
        sps_deviation, 1, constellation_4fsk);
    d_float_to_complex = gr::blocks::float_to_complex::make();
    d_descrambler = gr::digital::descrambler_bb::make(0x8A, 0x7F, 7);

    d_complex_to_float = gr::blocks::complex_to_float::make();
    d_interleave = gr::blocks::interleave::make(4);
    d_multiply_const_fec = gr::blocks::multiply_const_ff::make(128);
    d_float_to_uchar = gr::blocks::float_to_uchar::make();
    d_add_const_fec = gr::blocks::add_const_ff::make(128.0);
    gr::fec::code::cc_decoder::sptr decoder = gr::fec::code::cc_decoder::make(80, 7, 2, polys);
    d_decode_ccsds = gr::fec::decoder::make(decoder, 1, 1);

    connect(self(), 0, d_resampler, 0);
    connect(d_resampler, 0, d_filter, 0);
    connect(d_filter, 0, self(), 0);
    if (d_fm) {
        connect(d_filter, 0, d_freq_demod, 0);
        connect(d_freq_demod, 0, d_shaping_filter, 0);
        connect(d_shaping_filter, 0, d_symbol_sync, 0);
        connect(d_symbol_sync, 0, d_phase_mod, 0);
        connect(d_phase_mod, 0, self(), 1);
        connect(d_phase_mod, 0, d_complex_to_float, 0);
    } else {
        // Non-FM path uses discriminator
        connect(d_filter, 0, d_filter1, 0);
        connect(d_filter, 0, d_filter2, 0);
        connect(d_filter, 0, d_filter3, 0);
        connect(d_filter, 0, d_filter4, 0);
        connect(d_filter1, 0, d_mag1, 0);
        connect(d_filter2, 0, d_mag2, 0);
        connect(d_filter3, 0, d_mag3, 0);
        connect(d_filter4, 0, d_mag4, 0);
        connect(d_mag1, 0, d_discriminator, 0);
        connect(d_mag2, 0, d_discriminator, 1);
        connect(d_mag3, 0, d_discriminator, 2);
        connect(d_mag4, 0, d_discriminator, 3);
        connect(d_discriminator, 0, d_symbol_filter, 0);
        connect(d_symbol_filter, 0, d_symbol_sync_complex, 0);
        connect(d_symbol_sync_complex, 0, self(), 1);
        connect(d_symbol_sync_complex, 0, d_complex_to_float, 0);
    }
    if (d_fm) {
        connect(d_complex_to_float, 0, d_interleave, 1);
        connect(d_complex_to_float, 1, d_interleave, 0);
    } else {
        connect(d_complex_to_float, 0, d_interleave, 0);
        connect(d_complex_to_float, 1, d_interleave, 1);
    }
    connect(d_interleave, 0, d_multiply_const_fec, 0);
    connect(d_multiply_const_fec, 0, d_add_const_fec, 0);
    connect(d_add_const_fec, 0, d_float_to_uchar, 0);
    connect(d_float_to_uchar, 0, d_decode_ccsds, 0);
    connect(d_decode_ccsds, 0, d_descrambler, 0);
    connect(d_descrambler, 0, self(), 2);
}

demod_4fsk_impl::~demod_4fsk_impl() {}

} // namespace qradiolink
} // namespace gr

