// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#ifndef _SENTINEL_FAILLOGS_PARSER_H_
#define _SENTINEL_FAILLOGS_PARSER_H_
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <sys/param.h>
#include <arpa/inet.h>

#define ADDRSTRLEN (MAX(INET6_ADDRSTRLEN, INET_ADDRSTRLEN) + 1)
// Note: we crop longer user names but that should not be an issue as most of the
// names are shorter
#define MAXUSERLEN 32

struct data {
	time_t ts; // timestamp
	const char *service; // service identifier
	char user[MAXUSERLEN]; // user name connection was attempted to
	char source_ip[ADDRSTRLEN]; // IP address of attemt source
	unsigned source_port; // Port of attempt source
};

// Initialize parser's structures
void parser_init();

void parser_free();

// Parse given message for process with provided identifier.
bool parse(struct data *data, const char *ident, const char *message)
	__attribute((nonnull));

#endif
