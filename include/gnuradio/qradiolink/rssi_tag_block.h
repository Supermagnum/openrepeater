/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_RSSI_TAG_BLOCK_H
#define INCLUDED_QRADIOLINK_RSSI_TAG_BLOCK_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief RSSI Tag Block
 * \ingroup qradiolink
 *
 * Adds RSSI (Received Signal Strength Indicator) tags to the signal stream.
 */
class QRADIOLINK_API rssi_tag_block : public gr::sync_block
{
public:
    typedef std::shared_ptr<rssi_tag_block> sptr;

    /*!
     * \brief Make an RSSI tag block
     */
    static sptr make();

    virtual void calibrate_rssi(float level);

protected:
    rssi_tag_block(const std::string& name,
                   gr::io_signature::sptr input_signature,
                   gr::io_signature::sptr output_signature)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_RSSI_TAG_BLOCK_H */

