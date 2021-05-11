// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include "fixtures.h"
#include "../src/parser.h"

void f_parser_setup() {
	parser_init();
}

void f_parser_teardown() {
	parser_free();
}
