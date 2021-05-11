// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include "sender.h"
#include <czmq.h>
#include <msgpack.h>
#include "log.h"

struct sender {
	char *topic;
	zsock_t *sock;
	msgpack_sbuffer sbuf;
};


sender_t sender_new(const char *socket, const char *topic) {
	zsock_t *sock = zsock_new_push(socket);
	if (sock == NULL)
		return NULL;

	sender_t sender = malloc(sizeof *sender);
	*sender = (struct sender){
		.topic = strdup(topic),
		.sock = sock
	};
	msgpack_sbuffer_init(&sender->sbuf);

	// We send initial empty welcome message to identify ourself to proxy
	zmsg_t *msg = zmsg_new();
	if (zmsg_addstr(msg, topic) || zmsg_send(&msg, sender->sock)) {
		error("Submit to ZMQ failed for welcome message");
		zmsg_destroy(&msg);
		return NULL;
	}

	return sender;
}

#define PACK_STR(packer, str) do { \
	msgpack_pack_str(packer, strlen(str));\
	msgpack_pack_str_body(packer, str, strlen(str)); \
	} while(0)

bool sender_send(sender_t sender, struct data *data) {
	msgpack_sbuffer_clear(&sender->sbuf);
	msgpack_packer pk;
	msgpack_packer_init(&pk, &sender->sbuf, msgpack_sbuffer_write);

	msgpack_pack_map(&pk, 5);
	PACK_STR(&pk, "ts");
	msgpack_pack_long_long(&pk, data->ts);
	PACK_STR(&pk, "service");
	PACK_STR(&pk, data->service);
	PACK_STR(&pk, "user");
	PACK_STR(&pk, data->user);
	PACK_STR(&pk, "ip");
	PACK_STR(&pk, data->source_ip);
	PACK_STR(&pk, "port");
	msgpack_pack_unsigned_int(&pk, data->source_port);

	zmsg_t *msg = zmsg_new();
	if (zmsg_addstr(msg, sender->topic) ||
			zmsg_addmem(msg, sender->sbuf.data, sender->sbuf.size) ||
			zmsg_send(&msg, sender->sock)) {
		error("Submit to ZMQ failed: ts=%ld service=%s user=%s ip=%s port=%d",
			data->ts, data->service, data->user, data->source_ip, data->source_port);
		zmsg_destroy(&msg);
		return false;
	}
	return true;
}

void sender_destroy(sender_t sender) {
	zsock_destroy(&sender->sock);
	msgpack_sbuffer_destroy(&sender->sbuf);
	free(sender->topic);
	free(sender);
}
