/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_ZERO_IDLE_BURSTS_H
#define INCLUDED_QRADIOLINK_ZERO_IDLE_BURSTS_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief Zero Idle Bursts block
 * \ingroup qradiolink
 *
 * This block zeros out samples based on GNU Radio tags marked with "zero_samples".
 * Used for MMDVM and DMR to handle idle bursts.
 */
class QRADIOLINK_API zero_idle_bursts : public gr::sync_block
{
public:
    typedef std::shared_ptr<zero_idle_bursts> sptr;

    /*!
     * \brief Make a zero idle bursts block
     *
     * \param delay Delay in samples before applying zero tags (default: 0)
     */
    static sptr make(unsigned int delay = 0);

protected:
    zero_idle_bursts(const std::string& name,
                     gr::io_signature::sptr input_signature,
                     gr::io_signature::sptr output_signature)
        : sync_block(name, input_signature, output_signature)
    {
    }
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_ZERO_IDLE_BURSTS_H */

