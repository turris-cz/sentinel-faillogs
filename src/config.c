// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#include "config.h"
#include <unistd.h>
#include <argp.h>
#include <logc_argp.h>
#include <libconfig.h>
#include "log.h"

static struct config conf = {
	.config_file = DEFAULT_CONFIG_FILE,
	.socket = DEFAULT_SOCKET_URI,
	.topic = DEFAULT_TOPIC,
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);

const static struct argp_option argp_options[] = {
	{"config", 'f', "path",  0, "Path to config file"},
	{"socket", 's', "socket", 0, "Local socket to push data to Sentinel Proxy", 0},
	{"topic", 't', "topic", 0, "Topic for communication with Sentinel Proxy", 0},
	{NULL}
};

const static struct argp argp_parser = {
	.options = argp_options,
	.parser = parse_opt,
	.doc = "Sentinel failed login attempts logs collector",
	.children = (struct argp_child[]){{&logc_argp_parser, 0, "Logging", 2}, {NULL}},
};


static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	switch (key) {
		case 'f':
			conf.config_file = arg;
			break;
		case 's':
			conf.socket = arg;
			break;
		case 't':
			conf.topic = arg;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static void load_config_file() {
	if (access(conf.config_file, R_OK)) {
		debug("Skipping config file load (%s)", conf.config_file);
		return;
	}
	config_t cfg;
	config_init(&cfg);
	if(!config_read_file(&cfg, conf.config_file))
		critical("Load of config file failed: %s: %s:%d - %s",
				conf.config_file,
				config_error_file(&cfg),
				config_error_line(&cfg),
				config_error_text(&cfg)
		);

	config_destroy(&cfg);
}


struct config *parse_args(int argc, char **argv) {
	logc_argp_log = log_sentinel_faillogs; // set our log to be configured by logc_argp
	argp_parse(&argp_parser, argc, argv, 0, NULL, NULL);
	load_config_file();
	return &conf;
}
