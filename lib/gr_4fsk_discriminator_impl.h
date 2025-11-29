/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_GR_4FSK_DISCRIMINATOR_IMPL_H
#define INCLUDED_QRADIOLINK_GR_4FSK_DISCRIMINATOR_IMPL_H

#include <gnuradio/qradiolink/gr_4fsk_discriminator.h>

namespace gr {
namespace qradiolink {

class gr_4fsk_discriminator_impl : public gr_4fsk_discriminator
{
public:
    gr_4fsk_discriminator_impl();
    ~gr_4fsk_discriminator_impl() override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_GR_4FSK_DISCRIMINATOR_IMPL_H */

