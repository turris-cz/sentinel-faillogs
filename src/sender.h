// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021, CZ.NIC z.s.p.o. (http://www.nic.cz/)
#ifndef _SENTINEL_FAILLOGS_SENDER_H_
#define _SENTINEL_FAILLOGS_SENDER_H_
#include "parser.h"

struct sender;
typedef struct sender* sender_t;

// Allocate and initialize new sender
// Returns sender_t instance or NULL in case of of invalid socket
sender_t sender_new(const char *socket, const char *topic) __attribute__((malloc));

// Send data with given sender
// Returns true on success and false otherwise
bool sender_send(sender_t, struct data*);

// Free sender instance resources
void sender_destroy(sender_t);

#endif

