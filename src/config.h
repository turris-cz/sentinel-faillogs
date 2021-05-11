// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#ifndef _SENTINEL_FAILLOGS_CONFIG_H_
#define  _SENTINEL_FAILLOGS_CONFIG_H_
#include <stdbool.h>
#include <stdint.h>

struct config {
	const char *config_file;
	const char *socket;
	const char *topic;
};


// Parse arguments and load configuration file
// Returned pointer is to statically allocated (do not call free on it).
struct config *parse_args(int argc, char **argv);

#endif
