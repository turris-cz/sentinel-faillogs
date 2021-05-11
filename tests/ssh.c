// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include <check.h>
#include "fixtures.h"
#include "../src/parser.h"

void unittests_add_suite(Suite*);


struct {
	const char *message;
	const struct data data;
} valid_lines_d[] = {
	{
		.message = "Failed password for root from 103.55.24.144 port 56689 ssh2",
		.data = {
			.user = "root",
			.source_ip = "103.55.24.144",
			.source_port = 56689
		}
	},
	{
		.message = "Failed password for invalid user guest from 202.53.174.136 port 42358 ssh2",
		.data = {
			.user = "guest",
			.source_ip = "202.53.174.136",
			.source_port = 42358
		}
	},
	{
		.message = "Failed password for invalid user busio from 117.80.212.113 port 51816 ssh2",
		.data = {
			.user = "busio",
			.source_ip = "117.80.212.113",
			.source_port = 51816
		}
	},
	{
		.message = "Failed password for invalid user asterisk from 103.55.24.144 port 38393 ssh2",
		.data = {
			.user = "asterisk",
			.source_ip = "103.55.24.144",
			.source_port = 38393
		}
	},
	{
		.message = "Failed password for root from 2a01:510:d5:138:a498:3f5a:2c1a:abbb port 42136 ssh2",
		.data = {
			.user = "root",
			.source_ip = "2a01:510:d5:138:a498:3f5a:2c1a:abbb",
			.source_port = 42136
		}
	},
	{
		.message = "Failed password for invalid user yuanqing from 2a01:510:d5:138:a498:3f5a:6c1a:abbb port 60118 ssh2",
		.data = {
			.user = "yuanqing",
			.source_ip = "2a01:510:d5:138:a498:3f5a:6c1a:abbb",
			.source_port = 60118
		}
	},

};

START_TEST(valid_lines) {
	struct data d;
	ck_assert(parse(&d, "sshd", valid_lines_d[_i].message));
	// Note: we do not check timestamp as it can be different even over current time)
	ck_assert_str_eq(d.service, "ssh");
	ck_assert_str_eq(d.user, valid_lines_d[_i].data.user);
	ck_assert_str_eq(d.source_ip, valid_lines_d[_i].data.source_ip);
	ck_assert_int_eq(d.source_port, valid_lines_d[_i].data.source_port);
}
END_TEST


const char *invalid_lines_d[] = {
	"Disconnected from invalid user eric 220.196.1.142 port 54613 [preauth]",
	"Received disconnect from 220.196.1.142 port 54613:11: Bye Bye [preauth]",
	"error: Could not get shadow information for NOUSER",
};

START_TEST(invalid_lines) {
	struct data d;
	ck_assert(!parse(&d, "sshd", invalid_lines_d[_i]));
}
END_TEST


__attribute__((constructor))
static void suite() {
	Suite *suite = suite_create("ssh");

	TCase *parse_case = tcase_create("parse");
	tcase_add_checked_fixture(parse_case, f_parser_setup, f_parser_teardown);
	tcase_add_loop_test(parse_case, valid_lines,
			0, sizeof(valid_lines_d) / sizeof(valid_lines_d[0]));
	tcase_add_loop_test(parse_case, invalid_lines,
			0, sizeof(invalid_lines_d) / sizeof(invalid_lines_d[0]));
	suite_add_tcase(suite, parse_case);

	unittests_add_suite(suite);
}
