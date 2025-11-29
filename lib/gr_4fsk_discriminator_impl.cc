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

#include <gnuradio/qradiolink/gr_4fsk_discriminator.h>
#include "gr_4fsk_discriminator_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace qradiolink {

gr_4fsk_discriminator::sptr gr_4fsk_discriminator::make()
{
    return gnuradio::get_initial_sptr(new gr_4fsk_discriminator_impl());
}

gr_4fsk_discriminator_impl::gr_4fsk_discriminator_impl()
    : gr_4fsk_discriminator("gr_4fsk_discriminator",
                           gr::io_signature::make(4, 4, sizeof(float)),
                           gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
}

gr_4fsk_discriminator_impl::~gr_4fsk_discriminator_impl() {}

int gr_4fsk_discriminator_impl::work(int noutput_items,
                                     gr_vector_const_void_star& input_items,
                                     gr_vector_void_star& output_items)
{
    float* in1 = (float*)(input_items[0]);
    float* in2 = (float*)(input_items[1]);
    float* in3 = (float*)(input_items[2]);
    float* in4 = (float*)(input_items[3]);

    gr_complex* out = (gr_complex*)(output_items[0]);

    for (int i = 0; i < noutput_items; i++) {
        if ((in1[i] > in2[i]) && (in1[i] > in3[i]) && (in1[i] > in4[i]))
            out[i] = gr_complex(-0.707107, -0.707107);
        else if ((in2[i] > in1[i]) && (in2[i] > in3[i]) && (in2[i] > in4[i]))
            out[i] = gr_complex(-0.707107, 0.707107);
        else if ((in3[i] > in2[i]) && (in3[i] > in1[i]) && (in3[i] > in4[i]))
            out[i] = gr_complex(0.707107, 0.707107);
        else if ((in4[i] > in2[i]) && (in4[i] > in1[i]) && (in4[i] > in3[i]))
            out[i] = gr_complex(0.707107, -0.707107);
        else
            out[i] = gr_complex(0, 0);
    }
    return noutput_items;
}

} // namespace qradiolink
} // namespace gr

