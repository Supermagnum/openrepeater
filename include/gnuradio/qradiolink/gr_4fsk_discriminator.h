/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_GR_4FSK_DISCRIMINATOR_H
#define INCLUDED_QRADIOLINK_GR_4FSK_DISCRIMINATOR_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief 4FSK Discriminator block
 * \ingroup qradiolink
 *
 * Takes 4 float inputs and outputs the maximum value as a complex constellation point.
 */
class QRADIOLINK_API gr_4fsk_discriminator : public gr::sync_block
{
public:
    typedef std::shared_ptr<gr_4fsk_discriminator> sptr;

    /*!
     * \brief Make a 4FSK discriminator block
     */
    static sptr make();

protected:
    gr_4fsk_discriminator(const std::string& name,
                          gr::io_signature::sptr input_signature,
                          gr::io_signature::sptr output_signature)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_GR_4FSK_DISCRIMINATOR_H */

