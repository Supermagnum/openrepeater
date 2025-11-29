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
#include <gnuradio/qradiolink/clipper_cc.h>
#include "clipper_cc_impl.h"
#include <gnuradio/io_signature.h>
#include <volk/volk.h>
#include <gnuradio/math.h>

namespace gr {
namespace qradiolink {

clipper_cc::sptr clipper_cc::make(float clip)
{
    return gnuradio::get_initial_sptr(new clipper_cc_impl(clip));
}

clipper_cc_impl::clipper_cc_impl(float clip)
    : gr::sync_block("clipper_cc",
                     gr::io_signature::make(1, 1, sizeof(gr_complex)),
                     gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    const int alignment_multiple = volk_get_alignment() / sizeof(float);
    set_alignment(std::max(1, alignment_multiple));
    d_clip = clip;
    for (int i = 0; i < CHUNK_SIZE; i++) {
        d_cliplevel[i] = clip;
    }
    set_output_multiple(CHUNK_SIZE);
}

clipper_cc_impl::~clipper_cc_impl() {}

int clipper_cc_impl::work(int noutput_items,
                          gr_vector_const_void_star& input_items,
                          gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    gr_complex* out = (gr_complex*)output_items[0];

    for (int i = 0; i < noutput_items; i += CHUNK_SIZE) {
        volk_32fc_magnitude_32f(d_magnitude, in, CHUNK_SIZE);

        for (int j = 0; j < CHUNK_SIZE; j++) {
            d_phase[j] = gr::fast_atan2f(in[j]);
        }

        volk_32f_x2_min_32f(d_clipped, d_magnitude, d_cliplevel, CHUNK_SIZE);

        volk_32f_cos_32f(d_phase_cos, d_phase, CHUNK_SIZE);
        volk_32f_sin_32f(d_phase_sin, d_phase, CHUNK_SIZE);
        volk_32f_x2_multiply_32f(d_phase_cos, d_phase_cos, d_clipped, CHUNK_SIZE);
        volk_32f_x2_multiply_32f(d_phase_sin, d_phase_sin, d_clipped, CHUNK_SIZE);
        volk_32f_x2_interleave_32fc(out, d_phase_cos, d_phase_sin, CHUNK_SIZE);
        in += CHUNK_SIZE;
        out += CHUNK_SIZE;
    }

    return noutput_items;
}

} // namespace qradiolink
} // namespace gr

