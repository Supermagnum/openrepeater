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

#include <gnuradio/qradiolink/demod_m17.h>
#include "demod_m17_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/digital/constellation.h>
#include <cmath>

namespace gr {
namespace qradiolink {

demod_m17::sptr demod_m17::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(new demod_m17_impl(sps, samp_rate, carrier_freq, filter_width));
}

demod_m17_impl::demod_m17_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : demod_m17("demod_m17",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::makev(3, 3, std::vector<int>{sizeof(gr_complex), sizeof(gr_complex), sizeof(unsigned char)}))
{
    (void)sps;
    d_target_samp_rate = 24000;
    d_samples_per_symbol = 5;

    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    std::vector<gr_complex> constellation_points;
    constellation_points.push_back(gr_complex(-1.5, 0));
    constellation_points.push_back(gr_complex(-0.5, 0));
    constellation_points.push_back(gr_complex(0.5, 0));
    constellation_points.push_back(gr_complex(1.5f, 0));
    int ntaps = 50 * d_samples_per_symbol;

    std::vector<int> pre_diff;

    gr::digital::constellation_rect::sptr constellation_4fsk = gr::digital::constellation_rect::make(
        constellation_points, pre_diff, 2, 4, 1, 1.0, 1.0);

    std::vector<float> taps = gr::filter::firdes::low_pass(
        3, d_samp_rate * 3, d_target_samp_rate / 2, d_target_samp_rate / 2, gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_resampler = gr::filter::rational_resampler_ccf::make(3, 125, taps);

    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass(
            1, d_target_samp_rate, d_filter_width, d_filter_width, gr::fft::window::WIN_BLACKMAN_HARRIS));

    d_fm_demod = gr::analog::quadrature_demod_cf::make(d_samples_per_symbol / M_PI);
    std::vector<float> symbol_filter_taps = gr::filter::firdes::root_raised_cosine(
        1.5, d_target_samp_rate, d_target_samp_rate / d_samples_per_symbol, 0.5, ntaps);
    d_symbol_filter = gr::filter::fft_filter_fff::make(1, symbol_filter_taps);
    float symbol_rate((float)d_target_samp_rate / (float)d_samples_per_symbol);
    float sps_deviation = 500.0f / symbol_rate;
    d_symbol_sync = gr::digital::symbol_sync_ff::make(
        gr::digital::TED_MOD_MUELLER_AND_MULLER, d_samples_per_symbol, 2 * M_PI / (symbol_rate / 50), 1.0, 0.2869,
        sps_deviation, 1, constellation_4fsk);
    d_phase_mod = gr::analog::phase_modulator_fc::make(M_PI / 2);
    d_complex_to_float = gr::blocks::complex_to_float::make();
    d_interleave = gr::blocks::interleave::make(4);
    d_slicer = gr::digital::binary_slicer_fb::make();
    d_packer = gr::blocks::pack_k_bits_bb::make(2);
    d_unpacker = gr::blocks::unpack_k_bits_bb::make(2);
    std::vector<int> map;
    map.push_back(3);
    map.push_back(1);
    map.push_back(2);
    map.push_back(0);
    d_symbol_map = gr::digital::map_bb::make(map);

    connect(self(), 0, d_resampler, 0);
    connect(d_resampler, 0, d_filter, 0);
    connect(d_filter, 0, self(), 0);
    connect(d_filter, 0, d_fm_demod, 0);
    connect(d_fm_demod, 0, d_symbol_filter, 0);
    connect(d_symbol_filter, 0, d_symbol_sync, 0);
    connect(d_symbol_sync, 0, d_phase_mod, 0);
    connect(d_phase_mod, 0, self(), 1);
    connect(d_phase_mod, 0, d_complex_to_float, 0);
    connect(d_complex_to_float, 0, d_interleave, 0);
    connect(d_complex_to_float, 1, d_interleave, 1);
    connect(d_interleave, 0, d_slicer, 0);
    connect(d_slicer, 0, d_packer, 0);
    connect(d_packer, 0, d_symbol_map, 0);
    connect(d_symbol_map, 0, d_unpacker, 0);
    connect(d_unpacker, 0, self(), 2);
}

demod_m17_impl::~demod_m17_impl() {}

} // namespace qradiolink
} // namespace gr

