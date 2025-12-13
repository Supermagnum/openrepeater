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

#include <gnuradio/qradiolink/demod_dmr.h>
#include "demod_dmr_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>
#include <typeinfo>

namespace gr {
namespace qradiolink {

demod_dmr::~demod_dmr() {}

demod_dmr::sptr demod_dmr::make(int sps, int samp_rate)
{
    return gnuradio::get_initial_sptr(new demod_dmr_impl(sps, samp_rate));
}

demod_dmr_impl::demod_dmr_impl(int sps, int samp_rate)
    : demod_dmr("demod_dmr",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::makev(4, 4, {sizeof(gr_complex), sizeof(gr_complex), sizeof(unsigned char), sizeof(float)})),
      d_sps(sps),
      d_samp_rate(samp_rate),
      d_target_samp_rate(24000)
{
    unsigned int samples_per_symbol = 5;
    std::vector<gr_complex> constellation_points;
    constellation_points.push_back(gr_complex(-1.5, 0));
    constellation_points.push_back(gr_complex(-0.5, 0));
    constellation_points.push_back(gr_complex(0.5, 0));
    constellation_points.push_back(gr_complex(1.5, 0));
    int ntaps = 25 * samples_per_symbol;

    std::vector<int> pre_diff;

    auto constellation_4fsk = gr::digital::constellation_rect::make(
        constellation_points, pre_diff, 2, 4, 1, 1.0, 1.0);

    d_filter_width = 5000.0f;

    std::vector<float> taps = gr::filter::firdes::low_pass_2(
        3, d_samp_rate * 3, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(3, 125, taps);

    d_phase_mod = gr::analog::phase_modulator_fc::make(M_PI / 2);
    std::vector<float> symbol_filter_taps = gr::filter::firdes::root_raised_cosine(
        1, d_target_samp_rate, d_target_samp_rate / samples_per_symbol, 0.2, ntaps);
    d_symbol_filter = gr::filter::fft_filter_fff::make(1, symbol_filter_taps);
    float symbol_rate = ((float)d_target_samp_rate / (float)samples_per_symbol);
    float sps_deviation = 0.06;
    d_symbol_sync = gr::digital::symbol_sync_ff::make(
        gr::digital::TED_MUELLER_AND_MULLER, samples_per_symbol, 2 * M_PI / 100.0f, 1.0, 0.2869, sps_deviation, 1, constellation_4fsk);
    d_fm_demod = gr::analog::quadrature_demod_cf::make(d_target_samp_rate / (M_PI / 2 * symbol_rate));
    d_level_control = gr::blocks::multiply_const_ff::make(0.9);
    d_complex_to_float = gr::blocks::complex_to_float::make();
    d_complex_to_float_corr = gr::blocks::complex_to_float::make();
    d_float_to_complex_corr = gr::blocks::float_to_complex::make();
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
    connect(d_resampler, 0, self(), 0);
    connect(d_resampler, 0, d_fm_demod, 0);
    connect(d_fm_demod, 0, d_symbol_filter, 0);

    connect(d_symbol_filter, 0, d_symbol_sync, 0);
    connect(d_symbol_filter, 0, self(), 3);
    connect(d_symbol_sync, 0, d_level_control, 0);
    connect(d_level_control, 0, d_phase_mod, 0);
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

demod_dmr_impl::~demod_dmr_impl() {}

// Force vtable and typeinfo generation for RTTI
namespace {
    const std::type_info& g_demod_dmr_typeinfo = typeid(gr::qradiolink::demod_dmr);
    __attribute__((used)) static const void* force_demod_dmr_typeinfo = &g_demod_dmr_typeinfo;
    
    void* force_vtable_generation() {
        return const_cast<void*>(static_cast<const void*>(&g_demod_dmr_typeinfo));
    }
    __attribute__((used)) static void* vtable_force = force_vtable_generation();
}

} // namespace qradiolink
} // namespace gr

