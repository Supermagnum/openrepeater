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

#include <gnuradio/qradiolink/zero_idle_bursts.h>
#include "zero_idle_bursts_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tags.h>
#include <algorithm>

namespace gr {
namespace qradiolink {

zero_idle_bursts::sptr zero_idle_bursts::make(unsigned int delay)
{
    return gnuradio::get_initial_sptr(new zero_idle_bursts_impl(delay));
}

zero_idle_bursts_impl::zero_idle_bursts_impl(unsigned int delay)
    : zero_idle_bursts("zero_idle_bursts",
                       gr::io_signature::make(1, 1, sizeof(gr_complex)),
                       gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_sample_counter = 0;
    d_delay = delay;
    if (delay > 0) {
        set_history(2 * 720); // SAMPLES_PER_SLOT from config_mmdvm.h
    }
}

zero_idle_bursts_impl::~zero_idle_bursts_impl() {}

int zero_idle_bursts_impl::work(int noutput_items,
                                 gr_vector_const_void_star& input_items,
                                 gr_vector_void_star& output_items)
{
    gr_complex* out = (gr_complex*)(output_items[0]);
    const gr_complex* in = (const gr_complex*)(input_items[0]);
    std::vector<gr::tag_t> tags;
    uint64_t nitems = nitems_written(0);

    static const pmt::pmt_t ZERO_TAG = pmt::string_to_symbol("zero_samples");

    get_tags_in_window(tags, 0, 0, noutput_items, ZERO_TAG);
    if (!tags.empty()) {
        std::sort(tags.begin(), tags.end(), gr::tag_t::offset_compare);
    }

    for (int i = 0; i < noutput_items; i++) {
        for (gr::tag_t& tag : tags) {
            if (tag.offset == nitems + (uint64_t)i + d_delay) {
                d_sample_counter = pmt::to_uint64(tag.value);
                break;
            }
        }
        if (d_sample_counter > 0) {
            out[i] = gr_complex(0, 0);
            d_sample_counter--;
        } else {
            out[i] = in[i];
        }
    }

    return noutput_items;
}

} // namespace qradiolink
} // namespace gr

