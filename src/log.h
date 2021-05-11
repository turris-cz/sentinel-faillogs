// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#ifndef _SENTINEL_FAILLOGS_LOG_H_
#define _SENTINEL_FAILLOGS_LOG_H_

#define DEFLOG log_sentinel_faillogs // use our log as default one in logc macros
#include <logc.h>
#include <logc_util.h>
#include <logc_asserts.h>
extern log_t log_sentinel_faillogs;

#endif
