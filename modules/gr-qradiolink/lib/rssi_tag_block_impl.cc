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

#include <gnuradio/qradiolink/rssi_tag_block.h>
#include "rssi_tag_block_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tags.h>
#include <cmath>
#include <typeinfo>

namespace gr {
namespace qradiolink {

static const pmt::pmt_t RSSI_TAG = pmt::string_to_symbol("RSSI");

rssi_tag_block::~rssi_tag_block() {}

rssi_tag_block::sptr rssi_tag_block::make()
{
    return gnuradio::get_initial_sptr(new rssi_tag_block_impl());
}

rssi_tag_block_impl::rssi_tag_block_impl()
    : rssi_tag_block("rssi_tag_block",
                    gr::io_signature::make(1, 1, sizeof(gr_complex)),
                    gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_calibration_level(0.0f),
      d_nitems(0),
      d_sum(0.0f)
{
}

rssi_tag_block_impl::~rssi_tag_block_impl() {}

int rssi_tag_block_impl::work(int noutput_items,
                              gr_vector_const_void_star& input_items,
                              gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    gr_complex* out = (gr_complex*)(output_items[0]);
    float pwr = 0.0;

    for (int i = 0; i < noutput_items; i++) {
        pwr = in[i].real() * in[i].real() + in[i].imag() * in[i].imag();
        d_sum += pwr * pwr;
        d_nitems += 1;
        out[i] = in[i];
        if (d_nitems >= 300) {
            float level = sqrt(d_sum / (float)(d_nitems));
            float db = (float)10.0f * log10f(level + 1.0e-20) + d_calibration_level;
            add_rssi_tag(db, i);
            d_sum = 0;
            d_nitems = 0;
        }
    }

    return noutput_items;
}

void rssi_tag_block_impl::add_rssi_tag(float db, uint64_t sample)
{
    const pmt::pmt_t t_val = pmt::from_float(db);
    this->add_item_tag(0, nitems_written(0) + sample, RSSI_TAG, t_val);
}

void rssi_tag_block::calibrate_rssi(float level)
{
    // Base class implementation - should never be called
    // Actual implementation is in rssi_tag_block_impl
    (void)level; // Suppress unused parameter warning
}

void rssi_tag_block_impl::calibrate_rssi(float level)
{
    d_calibration_level = level;
}


// Force vtable and typeinfo generation for RTTI
namespace {
    void force_rtti_symbols() {
        const std::type_info& ti = typeid(gr::qradiolink::rssi_tag_block);
        (void)ti;
        auto make_func = &gr::qradiolink::rssi_tag_block::make;
        (void)make_func;
    }
    __attribute__((used)) static void (*force_init)() = force_rtti_symbols;
    
    const std::type_info& g_rssi_tag_block_typeinfo = typeid(gr::qradiolink::rssi_tag_block);
    __attribute__((used)) static const void* force_rssi_tag_block_typeinfo = &g_rssi_tag_block_typeinfo;
}

} // namespace qradiolink
} // namespace gr

