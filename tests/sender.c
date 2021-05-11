// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include <check.h>
#include <stdio.h>
#include <czmq.h>
#include <msgpack.h>
#include "fixtures.h"
#include "../src/sender.h"

void unittests_add_suite(Suite*);


static const char *topic = "faillogs";
static char *dirpath, *sockpath, *addr;
static zsock_t *ssock;

static void f_zmq_setup() {
	dirpath = mkdtemp(strdup("/tmp/sentinel-faillogs-sender-XXXXXX"));
	ck_assert_int_gt(asprintf(&sockpath, "%s/sock", dirpath), 0);
	ck_assert_int_gt(asprintf(&addr, "ipc://%s", sockpath), 0);
	ssock = zsock_new_pull(addr);
	ck_assert(ssock);
}
static void f_zmq_teardown() {
	zsock_destroy(&ssock);
	ck_assert_int_eq(unlink(sockpath), 0);
	ck_assert_int_eq(rmdir(dirpath), 0);
	free(addr);
	free(sockpath);
	free(dirpath);
}

#define ck_assert_msgpack_str(msgpack, string) do { \
		ck_assert_int_eq(msgpack.type, MSGPACK_OBJECT_STR); \
		ck_assert_int_eq(msgpack.via.str.size, strlen(string)); \
		ck_assert_mem_eq(msgpack.via.str.ptr, string, strlen(string)); \
	} while (false)
#define ck_assert_msgpack_uint(msgpack, num) do { \
		ck_assert_int_eq(msgpack.type, MSGPACK_OBJECT_POSITIVE_INTEGER); \
		ck_assert_int_eq(msgpack.via.u64, num); \
	} while (false)


START_TEST(submit) {
	sender_t sender = sender_new(addr, topic);

	zmsg_t *msg = zmsg_recv(ssock); // welcome message
	ck_assert(msg);
	ck_assert_int_eq(zmsg_size(msg), 1);
	zframe_t *topic_frame = zmsg_first(msg);
	ck_assert_int_eq(zframe_size(topic_frame), strlen(topic));
	ck_assert_mem_eq(zframe_data(topic_frame), topic, strlen(topic));
	zmsg_destroy(&msg);


	struct data d = {
		.service = "ssh",
		.user = "foo",
		.source_ip = "1234:1234:1234:1234:1234:1234:1234:1234",
		.source_port = 42
	};
	ck_assert_int_gt(time(&d.ts), 0);

	ck_assert(sender_send(sender, &d));

	msg = zmsg_recv(ssock);
	ck_assert(msg);
	ck_assert_int_eq(zmsg_size(msg), 2);
	topic_frame = zmsg_first(msg);
	ck_assert_int_eq(zframe_size(topic_frame), strlen(topic));
	ck_assert_mem_eq(zframe_data(topic_frame), topic, strlen(topic));

	zframe_t *payload_frame = zmsg_last(msg);
	size_t payload_size = zframe_size(payload_frame);
	msgpack_unpacked upkd;
	msgpack_unpacked_init(&upkd);
	ck_assert_int_eq(msgpack_unpack_next(&upkd, zframe_data(payload_frame), payload_size, NULL), MSGPACK_UNPACK_SUCCESS);
	msgpack_object r = upkd.data;
	ck_assert_int_eq(r.type, MSGPACK_OBJECT_MAP);
	ck_assert_int_eq(r.via.map.size, 5);
	ck_assert_msgpack_str(r.via.map.ptr[0].key, "ts");
	ck_assert_msgpack_uint(r.via.map.ptr[0].val, d.ts);
	ck_assert_msgpack_str(r.via.map.ptr[1].key, "service");
	ck_assert_msgpack_str(r.via.map.ptr[1].val, d.service);
	ck_assert_msgpack_str(r.via.map.ptr[2].key, "user");
	ck_assert_msgpack_str(r.via.map.ptr[2].val, d.user);
	ck_assert_msgpack_str(r.via.map.ptr[3].key, "ip");
	ck_assert_msgpack_str(r.via.map.ptr[3].val, d.source_ip);
	ck_assert_msgpack_str(r.via.map.ptr[4].key, "port");
	ck_assert_msgpack_uint(r.via.map.ptr[4].val, d.source_port);
	msgpack_unpacked_destroy(&upkd);

	zmsg_destroy(&msg);

	sender_destroy(sender);
}
END_TEST


__attribute__((constructor))
static void suite() {
	Suite *suite = suite_create("sender");

	TCase *send_case = tcase_create("send");
	tcase_add_checked_fixture(send_case, f_zmq_setup, f_zmq_teardown);
	tcase_add_test(send_case, submit);
	suite_add_tcase(suite, send_case);

	unittests_add_suite(suite);
}
