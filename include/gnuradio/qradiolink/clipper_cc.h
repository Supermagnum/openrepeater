/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_CLIPPER_CC_H
#define INCLUDED_QRADIOLINK_CLIPPER_CC_H

#include <gnuradio/qradiolink/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace qradiolink {

/*!
 * \brief CESSB Clipper for complex signals
 * \ingroup qradiolink
 *
 * This block implements a magnitude clipper for complex signals,
 * used in CESSB (Clipped-Envelope Single Sideband) processing.
 */
class QRADIOLINK_API clipper_cc : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<clipper_cc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of qradiolink::clipper_cc.
     *
     * \param clip Clip level (default: 0.95)
     */
    static sptr make(float clip = 0.95f);
};

} // namespace qradiolink
} // namespace gr

#endif /* INCLUDED_QRADIOLINK_CLIPPER_CC_H */

