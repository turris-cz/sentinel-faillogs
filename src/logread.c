// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include "logread.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include "log.h"

// The time we check if we haven't get stuck because of inotify. There can be race
// condition where we can have open file but others are writing to new one. The
// easiest solution is to just try to read when there was no activity in log for
// some time.
static const struct timeval logtimeout = { .tv_sec = 300 };

// We implement here crude statistics for buffer size. We dynamically increase
// size once we can't fit log line to buffer but at some point we want to decrease
// it if we won't encounter such long line again. We have counter that is simply
// increased up to this limit when line exactly fits and decreases when buffer can
// be smaller. We decrease size of buffer once counter hits negative limit.
static const int sizcnt_limit = 16;


static int sizcmp(size_t siz, size_t lsiz) {
	siz -= BUFSIZ;
	return siz < lsiz ? 1 : siz > lsiz ? -1 : 0;
}

static void process_line(struct logread *lr, char *line) {
	debug("Read line from '%s': %s", lr->logpath, line);
	regmatch_t m[4];
	if (regexec(&lr->regex, line, sizeof(m) / sizeof(m[0]), m, 0) == REG_NOMATCH)
		return; // ignore any unmatched line
	line[m[1].rm_eo] = '\0';
	lr->process_line(line + m[1].rm_so, line + m[3].rm_so);
}

static void readlog(struct logread *lr) {
	if (lr->fd == -1)
		return;
	trace("Reading log: %s", lr->logpath);
	while (true) {
		if (lr->siz - lr->off < BUFSIZ) {
			lr->buf = realloc(lr->buf, (lr->siz += BUFSIZ) * sizeof *lr->buf);
			lr->sizcnt = 0;
		}
		size_t len = read(lr->fd, lr->buf + lr->off, BUFSIZ);
		switch (len) {
			case -1:
				error("Read from log '%s' failed", lr->logpath);
			case 0:
				return;
		}
		lr->off += len;

		char *line_s = lr->buf, *line_e;
		size_t off = lr->off;
		while ((line_e = memchr(line_s, '\n', off))) {
			lr->sizcnt += sizcmp(lr->siz, line_e - line_s);
			*line_e = '\0';
			process_line(lr, line_s);
			off -= line_e + 1 - line_s;
			line_s = line_e + 1;
		}
		memmove(lr->buf, lr->buf + lr->off - off, off);
		lr->off = off;

		if (lr->sizcnt <= -sizcnt_limit) {
			lr->buf = realloc(lr->buf, (lr->siz -= BUFSIZ) * sizeof *lr->buf);
			assert(lr->off < lr->siz);
			lr->sizcnt = 0;
		} else if (lr->sizcnt > sizcnt_limit)
			lr->sizcnt = sizcnt_limit;
	}
}

bool is_same_file(int fd, const char *path) {
	struct stat our, cur;
	if (stat(path, &cur) == 0) {
		assert_eq(fstat(fd, &our), 0);
		if (our.st_dev == cur.st_dev && our.st_ino == cur.st_ino)
			return true;
	}
	errno = 0;
	return false;
}

static void readlog_reopen(struct logread *lr) {
	readlog(lr); // make sure that we read all from previous log

	if (lr->fd != -1 && is_same_file(lr->fd, lr->logpath))
		return;

	int newfd = open(lr->logpath, O_RDONLY);
	if (newfd == -1) {
		error("Can't reopen log: %s", lr->logpath);
		return;
	}
	if (lr->fd != -1)
		close(lr->fd);
	lr->fd = newfd;
	std_fatal(inotify_add_watch(lr->infd, lr->logpath, IN_MODIFY|IN_DELETE_SELF));
	lr->off = 0;
	info("Reopened log file: %s", lr->logpath);

	readlog(lr); // read what we have in new log
}

static void inotify_callback(evutil_socket_t fd, short event, void *vlr) {
	struct logread *lr = vlr;
	const size_t bufsiz = sizeof(struct inotify_event) + NAME_MAX + 1;
	uint8_t *buf = malloc(bufsiz);
	ssize_t len = read(lr->infd, buf, bufsiz);
	size_t off = 0;
	while(off < len) {
		struct inotify_event *ie = (struct inotify_event*)(buf + off);
		debug("Received inotify event (%d) for log: %s", ie->mask, lr->logpath);
		switch (ie->mask) {
			case IN_MODIFY:
				readlog(lr);
				break;
			case IN_MOVE_SELF:
			case IN_DELETE_SELF:
				readlog_reopen(lr);
				break;
			default:
				critical("Unknown event received from inotify: %d", ie->mask);
		}
		off += sizeof(struct inotify_event) + ie->len;
	}
	free(buf);
	assert_eq(evtimer_add(lr->timer, &logtimeout), 0);
}

static void timer_callback(evutil_socket_t fd, short event, void *vlr) {
	struct logread *lr = vlr;
	debug("Timeout expired on log: %s", lr->logpath);
	readlog_reopen(lr);
	assert_eq(evtimer_add(lr->timer, &logtimeout), 0);
}


void logread_init(struct logread *lr, struct event_base *evb, const char *path, process_line_t pl) {
	lr->fd = open(path, O_RDONLY);
	if (lr->fd != -1) {
		info("Opened log file: %s", path);
		std_fatal(lseek(lr->fd, 0, SEEK_END));
	} else
		error("Can't open log: %s", path); // we  continue anyway as it might appear

	std_fatal(lr->infd = inotify_init());
	if (lr->fd != -1)
		std_fatal(inotify_add_watch(lr->infd, path, IN_MODIFY|IN_DELETE_SELF|IN_MOVE_SELF));
	lr->event = event_new(evb, lr->infd, EV_READ | EV_PERSIST, inotify_callback, lr);
	assert_eq(event_add(lr->event, NULL), 0);

	// Note: We do not want to 100% rely on inotify as it can be unreliable and
	// thus in case of long inactivity we just try to read ourself.
	lr->timer = evtimer_new(evb, timer_callback, lr);
	assert_eq(evtimer_add(lr->timer, &logtimeout), 0);

	lr->logpath = strdup(path); assert(lr->logpath);
	lr->process_line = pl;
	assert_eq(regcomp(&lr->regex, "([^[ ]+)(\\[[0-9]+\\])?: (.*)$", REG_EXTENDED), 0);
	lr->buf = NULL;
	lr->off = lr->siz = 0;
	lr->sizcnt = 0;
}

void logread_free(struct logread *lr) {
	free(lr->buf);
	regfree(&lr->regex);
	evtimer_del(lr->timer);
	event_del(lr->event);
	event_free(lr->timer);
	event_free(lr->event);
	free(lr->logpath);
	if (lr->fd != -1)
		close(lr->fd);
	close(lr->infd);
}
