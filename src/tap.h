/* Copyright (c) 2024 Krypto-IT Jakub Juszczakiewicz
 * All rights reserved.
 */

#include <unistd.h>

int tap_create(const char * dev, uid_t user, gid_t group);
int tap_destroy(const char * dev);
