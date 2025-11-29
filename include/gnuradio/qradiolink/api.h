/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_QRADIOLINK_API_H
#define INCLUDED_QRADIOLINK_API_H

#include <gnuradio/attributes.h>

#ifdef gnuradio_qradiolink_EXPORTS
#define QRADIOLINK_API __GR_ATTR_EXPORT
#else
#define QRADIOLINK_API __GR_ATTR_IMPORT
#endif

#endif /* INCLUDED_QRADIOLINK_API_H */

