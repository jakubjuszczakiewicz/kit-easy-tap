/* Copyright (c) 2024 Krypto-IT Jakub Juszczakiewicz
 * All rights reserved.
 */

#include <unistd.h>
#include <sys/wait.h>

extern pid_t child;

int check_pattern(const char * pattern);
int is_dev_exists(const char * dev);
char * str_repall(const char * in, const char * pattern, const char * rep);
int in_list(char ** list, size_t idx);
int own_system(char * args[], uid_t user, gid_t group);
