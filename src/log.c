// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include "log.h"
#include <event2/logc.h>
#include <czmq_logc.h>

APP_LOG(sentinel_faillogs);


__attribute__((constructor))
static void log_constructor() {
	logc_event_init();
	logc_czmq_init();
}

__attribute__((destructor))
static void log_destructor() {
	logc_event_cleanup();
	logc_czmq_cleanup();
}
