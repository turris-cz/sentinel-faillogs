// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "log.h"

static regex_t r_sshd;

static void matchcpy(char *dest, size_t maxlen, const char *msg, regmatch_t *m) {
	size_t n = MIN(m->rm_eo - m->rm_so, maxlen - 1);
	memcpy(dest, msg + m->rm_so, n);
	dest[n] = '\0';
}

static unsigned portnum(const char *message, regmatch_t *m) {
	// TODO there is really no atoi like function that would accept not-null
	// terminated string?
	char *s = strndup(message + m->rm_so, m->rm_eo - m->rm_so);
	unsigned res = atoi(s);
	// Note: we do not care about atoi failing as it retunrs in such case 0 that
	// is just invalid port and thus for us unknown port.
	free(s);
	return res;
}

static bool parse_sshd(struct data *data, const char *msg) {
	trace("Parsing sshd log: %s", msg);
	regmatch_t m[4];
	if (regexec(&r_sshd, msg, sizeof(m) / sizeof(m[0]), m, 0) == REG_NOMATCH)
		return false;

	std_fatal(time(&data->ts));
	data->service = "ssh";
	matchcpy(data->user, MAXUSERLEN, msg, &m[1]);
	matchcpy(data->source_ip, ADDRSTRLEN, msg, &m[2]);
	data->source_port = portnum(msg, &m[3]);

	return true;
}

void parser_init() {
	// TODO we might want to have IP format verification
	assert_eq(regcomp(&r_sshd, "Failed.* ([^ ]+) from ([^ ]+) port ([0-9]+)", REG_EXTENDED), 0);
}

void parser_free() {
	regfree(&r_sshd);
}


enum identifier {
	I_SSHD,
};

#include "identifier.gperf.c"

bool parse(struct data *data, const char *ident, const char *message) {
	const struct gperf_identifier *id;
	id = gperf_identifier(ident, strlen(ident));
	if (id)
		switch (id->identifier) {
			case I_SSHD:
				return parse_sshd(data, message);
			default:
				trace("Unknown identifier: %s", ident);
		}
	return false;
}
