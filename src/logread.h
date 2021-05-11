// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#ifndef _SENTINEL_FAILLOGS_LOGREAD_H_
#define  _SENTINEL_FAILLOGS_LOGREAD_H_
#include <stdint.h>
#include <regex.h>
#include <event2/event.h>
#include "parser.h"

typedef void (*process_line_t)(const char *ident, const char *message);

struct logread {
	process_line_t process_line;
	int fd;
	char *logpath;
	int infd;
	struct event *event;
	struct event *timer;
	regex_t regex;
	char *buf;
	size_t off, siz;
	int sizcnt;
};

void logread_init(struct logread*, struct event_base*, const char *logpath, process_line_t);
void logread_free(struct logread*);

#endif
