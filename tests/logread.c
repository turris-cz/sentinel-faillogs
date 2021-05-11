// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "fixtures.h"
#include "../src/logread.h"
#include "../src/log.h"
#include "util.h"

void unittests_add_suite(Suite*);


struct event_base *evb;

void f_logread_setup() {
	evb = event_base_new();
}

void f_logread_teardown() {
	event_base_free(evb);
}


size_t test_i;

struct parsed {
	const char *ident;
	const char *message;
};

const char *simple_lines[] = {
	// This is snapshot of Turris syslog
	"May 12 14:21:17 turris-home syncthing[18876]: [YJMCM] 16:21:17 INFO: \"Turris\" (turris): Failed to sync 9 items",
	"May 12 14:21:17 turris-home syncthing[18876]: [YJMCM] 16:21:17 INFO: Folder \"Turris\" (turris) isn't making sync progress - retrying in 1h4m0.979309166s.",
	"May 12 13:59:05 turris-home hostapd: wlan1: STA c8:3d:dc:95:44:fd IEEE 802.11: authenticated",
	"May 12 14:21:54 turris-home updater-supervisor: Running pkgupdate",
	"May 12 14:21:57 turris-home updater[14092]: repository.lua.lua:49 (Globals): Target Turris OS: 5.3.0",
	"May 12 14:21:58 turris-home netdata[19256]: BTRFS: failed to read '/sys/fs/btrfs/bbddad05-cb54-4b64-907f-f14ee3df35ea/allocation/data/total_bytes'",
	"May 12 14:22:01 turris-home crond[14122]: (root) CMD (/usr/bin/rainbow_button_sync.sh)",
	"May 12 14:22:08 turris-home updater[14092]: planner.lua:356 (pkg_plan): Requested package luci-i18n-sqm-cs that is missing, ignoring as requested.",
	"May 12 14:22:08 turris-home updater-supervisor: pkgupdate reported no errors",
	"May 12 14:22:20 turris-home sshd[14218]: Invalid user misp from 190.113.46.4 port 43098",
	"May 12 14:22:20 turris-home sshd[14218]: error: Could not get shadow information for NOUSER",
	"May 12 14:22:20 turris-home sshd[14218]: Failed password for invalid user misp from 190.113.46.4 port 43098 ssh2",
	"May 12 14:22:20 turris-home sshd[14218]: Received disconnect from 190.113.46.4 port 43098:11: Bye Bye [preauth]",
	"May 12 14:22:20 turris-home sshd[14218]: Disconnected from invalid user misp 190.113.46.4 port 43098 [preauth]",
};
struct parsed simple_parsed[] = {
	{"syncthing", "[YJMCM] 16:21:17 INFO: \"Turris\" (turris): Failed to sync 9 items"},
	{"syncthing", "[YJMCM] 16:21:17 INFO: Folder \"Turris\" (turris) isn't making sync progress - retrying in 1h4m0.979309166s."},
	{"hostapd", "wlan1: STA c8:3d:dc:95:44:fd IEEE 802.11: authenticated"},
	{"updater-supervisor", "Running pkgupdate"},
	{"updater", "repository.lua.lua:49 (Globals): Target Turris OS: 5.3.0"},
	{"netdata", "BTRFS: failed to read '/sys/fs/btrfs/bbddad05-cb54-4b64-907f-f14ee3df35ea/allocation/data/total_bytes'"},
	{"crond", "(root) CMD (/usr/bin/rainbow_button_sync.sh)"},
	{"updater", "planner.lua:356 (pkg_plan): Requested package luci-i18n-sqm-cs that is missing, ignoring as requested."},
	{"updater-supervisor", "pkgupdate reported no errors"},
	{"sshd", "Invalid user misp from 190.113.46.4 port 43098"},
	{"sshd", "error: Could not get shadow information for NOUSER"},
	{"sshd", "Failed password for invalid user misp from 190.113.46.4 port 43098 ssh2"},
	{"sshd", "Received disconnect from 190.113.46.4 port 43098:11: Bye Bye [preauth]"},
	{"sshd", "Disconnected from invalid user misp 190.113.46.4 port 43098 [preauth]"},
};

void process_line_simple(const char *ident, const char *message) {
	ck_assert_str_eq(ident, simple_parsed[test_i].ident);
	ck_assert_str_eq(message, simple_parsed[test_i].message);
	test_i++;
	if (test_i >= sizeof(simple_parsed)/sizeof(simple_parsed[0]))
		event_base_loopexit(evb, NULL);
}

START_TEST(simple) {
	test_i = 0;
	char *path = strdup("/tmp/sentinel-faillogs-logread-simple-XXXXXX");
	int fd = mkstemp(path);
	ck_assert_int_ge(fd, 0);

	struct logread lr;
	logread_init(&lr, evb, path, process_line_simple);

	// We write after init as logread starts from the end of line.
	write_lines(fd, simple_lines);
	close(fd);

	ck_assert_int_eq(event_base_dispatch(evb), 0);

	logread_free(&lr);
	unlink(path);
	free(path);
}
END_TEST


const char *invalid_lines[] = {
	"",
	"foo",
	"foo foo foo",
	"May 12 14:22:20 turris-home sshd[14218]"
	"turris-home sshd[14218]: Diconnected"
	"May 12 14:22:20 turris-home: Disconnected from invalid user misp 190.113.46.4 port 43098 [preauth]",
};

void process_line_invalid(const char *ident, const char *message) {
	ck_abort_msg("callback should not be called for invalid messages: %s: %s",
			ident, message);
}

START_TEST(invalid) {
	test_i = 0;
	char *path = strdup("/tmp/sentinel-faillogs-logread-invalid-XXXXXX");
	int fd = mkstemp(path);
	ck_assert_int_ge(fd, 0);

	struct logread lr;
	logread_init(&lr, evb, path, process_line_simple);

	write_lines(fd, simple_lines);
	close(fd);

	ck_assert_int_eq(event_base_dispatch(evb), 0);

	logread_free(&lr);
	unlink(path);
	free(path);
}
END_TEST


START_TEST(no_such_file) {
	struct logread lr;
	logread_init(&lr, evb, "/tmp/sentinel-faillogs-logread-not-exists", NULL);
	ck_assert_int_eq(lr.fd, -1);
	logread_free(&lr);
}
END_TEST


void process_line_longline(const char *ident, const char *message) {
	ck_assert_str_eq(ident, "foo");
	ck_assert_int_eq(strlen(message), BUFSIZ + 2);
	// Note: we do not check content of the message as length should be enough to
	// make sure  we got a whole message.
	test_i++;
	event_base_loopexit(evb, NULL);
}
void process_line_longline_short(const char *ident, const char *message) {
	ck_assert_str_eq(ident, "foo");
	ck_assert_str_eq(message, "foo");
	test_i++;
}

START_TEST(longline) {
	test_i = 0;
	char *path = strdup("/tmp/sentinel-faillogs-logread-longline-XXXXXX");
	int fd = mkstemp(path);
	ck_assert_int_ge(fd, 0);

	struct logread lr;
	logread_init(&lr, evb, path, process_line_longline);

	write_str(fd, "May 12 14:21:17 turris-home foo: f");
	write_ntimes(fd, "o", BUFSIZ);
	write_ntimes(fd, "o\n", 9);
	fdatasync(fd);

	ck_assert_int_eq(event_base_dispatch(evb), 0);
	ck_assert_int_eq(test_i, 1);
	ck_assert_int_eq(lr.siz, BUFSIZ * 2);

	write_ntimes(fd, "May 12 14:21:18 turris-home foo: foo\n", 9);
	close(fd);
	lr.process_line = process_line_longline_short;

	ck_assert_int_eq(event_base_loop(evb, EVLOOP_ONCE), 0);
	ck_assert_int_eq(test_i, 10);
	ck_assert_int_eq(lr.siz, BUFSIZ);

	logread_free(&lr);
	unlink(path);
	free(path);
}
END_TEST



const char *move_lines_1[] = {
	"May 12 14:21:17 turris-home syncthing[18876]: [YJMCM] 16:21:17 INFO: Folder \"Turris\" (turris) isn't making sync progress - retrying in 1h4m0.979309166s.",
	"May 12 13:59:05 turris-home hostapd: wlan1: STA c8:3d:dc:95:44:fd IEEE 802.11: authenticated",
	"May 12 14:21:54 turris-home updater-supervisor: Running pkgupdate",
	"May 12 14:21:57 turris-home updater[14092]: repository.lua.lua:49 (Globals): Target Turris OS: 5.3.0",
};
const char *move_lines_2[] = {
	"May 12 14:22:01 turris-home crond[14122]: (root) CMD (/usr/bin/rainbow_button_sync.sh)",
	"May 12 14:22:08 turris-home updater-supervisor: pkgupdate reported no errors",
	"May 12 14:22:20 turris-home sshd[14218]: Invalid user misp from 190.113.46.4 port 43098",
};
struct parsed move_parsed[] = {
	{"syncthing", "[YJMCM] 16:21:17 INFO: Folder \"Turris\" (turris) isn't making sync progress - retrying in 1h4m0.979309166s."},
	{"hostapd", "wlan1: STA c8:3d:dc:95:44:fd IEEE 802.11: authenticated"},
	{"updater-supervisor", "Running pkgupdate"},
	{"updater", "repository.lua.lua:49 (Globals): Target Turris OS: 5.3.0"},
	{"crond", "(root) CMD (/usr/bin/rainbow_button_sync.sh)"},
	{"updater-supervisor", "pkgupdate reported no errors"},
	{"sshd", "Invalid user misp from 190.113.46.4 port 43098"},
};

void process_line_move(const char *ident, const char *message) {
	ck_assert_str_eq(ident, move_parsed[test_i].ident);
	ck_assert_str_eq(message, move_parsed[test_i].message);
	test_i++;
	if (test_i >= sizeof(move_parsed)/sizeof(move_parsed[0]))
		event_base_loopexit(evb, NULL);
}

START_TEST(move) {
	test_i = 0;
	char *path = strdup("/tmp/sentinel-faillogs-logread-invalid-XXXXXX");
	int fd = mkstemp(path);
	ck_assert_int_ge(fd, 0);

	struct logread lr;
	logread_init(&lr, evb, path, process_line_move);

	char *oldpath;
	ck_assert_int_gt(asprintf(&oldpath, "%s.prev", path), 0);
	write_lines(fd, move_lines_1);
	close(fd);
	ck_assert_int_eq(rename(path, oldpath), 0);

	int newfd = open(path, O_RDWR|O_CREAT|O_EXCL, 0600);
	ck_assert_int_ge(newfd, 0);
	write_lines(newfd, move_lines_2);
	close(newfd);

	ck_assert_int_eq(event_base_dispatch(evb), 0);

	logread_free(&lr);
	unlink(path);
	unlink(oldpath);
	free(path);
	free(oldpath);
}
END_TEST


START_TEST(timer) {
	test_i = 0;
	char *path = strdup("/tmp/sentinel-faillogs-logread-timer-XXXXXX");
	int fd = mkstemp(path);
	close(fd);

	struct logread lr;
	logread_init(&lr, evb, path, process_line_move);

	evtimer_add(lr.timer, &(struct timeval){.tv_sec = 0});
	ck_assert_int_eq(event_base_loop(evb, EVLOOP_ONCE), 0);

	logread_free(&lr);
	unlink(path);
	free(path);
}
END_TEST



__attribute__((constructor))
static void suite() {
	Suite *suite = suite_create("logread");

	TCase *read_case = tcase_create("read");
	tcase_add_checked_fixture(read_case, f_logread_setup, f_logread_teardown);
	tcase_add_test(read_case, simple);
	tcase_add_test(read_case, invalid);
	tcase_add_test(read_case, no_such_file);
	tcase_add_test(read_case, longline);
	suite_add_tcase(suite, read_case);

	TCase *reopen_case = tcase_create("reopen");
	tcase_add_checked_fixture(reopen_case, f_logread_setup, f_logread_teardown);
	tcase_add_test(reopen_case, move);
	tcase_add_test(reopen_case, timer);
	suite_add_tcase(suite, reopen_case);

	unittests_add_suite(suite);
}
