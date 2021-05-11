// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include <stdlib.h>
#include <stdio.h>
#include <event2/event.h>
#include "log.h"
#include "config.h"
#include "sender.h"
#include "parser.h"
#include "logread.h"

static sender_t sender;

void process_messages_line(const char *ident, const char *message) {
	struct data data;
	if (parse(&data, ident, message))
		sender_send(sender, &data);
}

int main(int argc, char **argv) {
	struct config *conf = parse_args(argc, argv);

	sender = sender_new(conf->socket, conf->topic);
	if (!sender)
		critical("Unable to initialize ZMQ socket, probably invalid socket was provided");
	parser_init();

	struct event_base *evb = event_base_new();
	assert(evb);

	struct logread messages;
	logread_init(&messages, evb, "/var/log/messages", process_messages_line);

	event_base_dispatch(evb);

	logread_free(&messages);
	event_base_free(evb);
	parser_free();
	sender_destroy(sender);
	return 0;
}
