// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#ifndef _TESTS_UTIL_H_
#define _TESTS_UTIL_H_
#include <string.h>

#define ck_write(fd, buf, c) ({ \
		size_t _ck_count = (c); \
		ck_assert_int_eq(write((fd), (buf), _ck_count), _ck_count); \
	})

#define write_lines(fd, lines) \
	for (size_t i = 0; i < (sizeof(lines) / sizeof(lines[0])); i++) { \
		ck_write(fd, lines[i], strlen(lines[i])); \
		ck_write(fd, "\n", 1); \
	}

#define write_str(fd, str) \
	ck_write(fd, str, strlen(str));

#define write_ntimes(fd, str, n) \
	for (unsigned i = 0; i < (n); i++) \
		write_str(fd, str);

#endif
