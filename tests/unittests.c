// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include <check.h>
#include <stdlib.h>
#include <stdbool.h>

static Suite **suites = NULL;
static size_t suites_len = 0, suites_size = 1;

void unittests_add_suite(Suite *s) {
	if (suites == NULL || suites_len == suites_size)
		suites = realloc(suites, (suites_size *= 2) * sizeof *suites);
	suites[suites_len++] = s;
}


int main(void) {
	SRunner *runner = srunner_create(NULL);

	for (size_t i = 0; i < suites_len; i++)
		srunner_add_suite(runner, suites[i]);

	char *test_output_tap = getenv("TEST_OUTPUT_TAP");
	if (test_output_tap && *test_output_tap != '\0')
		srunner_set_tap(runner, test_output_tap);
	char *test_output_xml = getenv("TEST_OUTPUT_XML");
	if (test_output_xml && *test_output_xml != '\0')
		srunner_set_xml(runner, test_output_xml);
	//if (getenv("VALGRIND")) // Do not fork with valgrind
	srunner_set_fork_status(runner, CK_NOFORK);

	srunner_run_all(runner, CK_NORMAL);
	int failed = srunner_ntests_failed(runner);

	srunner_free(runner);
	return (bool)failed;
}
