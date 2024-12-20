/* Copyright (c) 2024 Krypto-IT Jakub Juszczakiewicz
 * All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "tap.h"
#include "bridge.h"
#include "tools.h"
#include "version.h"

#define MAX_TAPS 9

void usage(const char * argv0)
{
  fprintf(stderr,
      "kit-easy-tap %s - tool for easy normal user TAP device usage.\n",
      version_str);
  fprintf(stderr,
      "Usage: %s tap_pattern[:bridge][,tap2_pattern[:bridge][,...]] ", argv0);
  fprintf(stderr, "tap_name_file [-d] command arg arg2 ...\n\n");
  fprintf(stderr, "tap_pattern   is tap device name pattern printf like "
      "tap%%04u. It counld be only %%u or %%0Xu. Tool with select first free "
      "tap device name with this pattern or fail.\n\n");
  fprintf(stderr, "bridge is network bridge name for attach new tap ");
  fprintf(stderr, "interface.\n\n");
  fprintf(stderr, "tap_name_file  is path for create file with selected tap "
      "device name.\n\n");
  fprintf(stderr, "-d is switch for daemonize.\n\n");
  fprintf(stderr, "command arg1 arg2...  is command executed after prepare_cmd "
      " command with the seme args rules. This will be executed $USER\n\n");
  exit(1);
}

void file_write(const char * path, char ** text, size_t count, uid_t uid,
    gid_t gid)
{
  FILE * f = fopen(path, "w");
  if (!f)
    return;
  for (size_t i = 1; i < count; i++)
    fprintf(f, "%s,", text[i - 1]);
  fprintf(f, "%s", text[count - 1]);
  fchown(fileno(f), uid, gid);
  fclose(f);
}

void forward_signal(int signum)
{
  if (child)
    kill(child, signum);
}

int main(int argc, char * argv[])
{
  if (argc < 4)
    usage(argv[0]);

  size_t taps = 1;
  char * p = argv[1], * q;
  while ((q = strchr(p, ','))) {
    *q = 0;
    int count = check_pattern(argv[1]);
    if ((count < 0) || (count > 1))
      usage(argv[0]);
    p = q + 1;
    taps++;
  }

  int count = check_pattern(p);
  if ((count < 0) || (count > 1))
    usage(argv[0]);

  char * tap_names[MAX_TAPS];
  char * tap_bridges[MAX_TAPS];

  p = argv[1];
  for (size_t i = 0; i < taps; i++, p += 1 + strlen(p)) {
    char * brptr = strchr(p, ':');
    if (brptr == NULL) {
      tap_bridges[i] = NULL;
    } else {
      size_t len = strlen(brptr);
      tap_bridges[i] = alloca(len);
      memcpy(tap_bridges[i], brptr + 1, len);
      *brptr = 0;
    }
    if (strchr(p, '%') == NULL) {
      tap_names[i] = p;
    } else {
      size_t len = strlen(p) + 10;
      tap_names[i] = alloca(len + 1);
      snprintf(tap_names[i], len, p, 0);
      for (unsigned int j = 1; (is_dev_exists(tap_names[i]) == 0) ||
          (in_list(tap_names, i)); j++)
        snprintf(tap_names[i], len, p, j);
    }
    if (brptr != NULL) {
      p = brptr + 1;
    }
  }

  char ** args = alloca(sizeof(char *) * (argc - 2));
  for (size_t i = 3, j = 0; i < argc; i++, j++) {
    args[j] = strdup(argv[i]);
    if (strstr(argv[i], "<TAP") != NULL) {
      if (strstr(argv[i], "<TAP>") != NULL) {
        char * tmp = args[j];
        args[j] = str_repall(args[j], "<TAP>", tap_names[0]);
        free(tmp);
      }
      for (size_t k = 1; k < taps; k++) {
        char str[] = "<TAPx>";
        str[4] = '0' + k;
        if (strstr(argv[i], str) != NULL) {
          char * tmp = args[j];
          args[j] = str_repall(args[j], str, tap_names[k]);
          free(tmp);
        }
      }
    }
  }

  args[argc - 3] = NULL;

  for (size_t j = 0; j < taps; j++) {
    if (tap_create(tap_names[j], getuid(), getgid())) {
      for (size_t i = 0; i < argc - 3; i++) {
        free(args[i]);
      }
      for (size_t i = 0; i < j; i++) {
        if (tap_bridges[i]) {
          bridge_del_interface(tap_bridges[i], tap_names[i]);
        }
        tap_destroy(tap_names[i]);
      }
      puts(tap_names[j]);
      usage(argv[0]);
    }
    if (tap_bridges[j]) {
      bridge_add_interface(tap_bridges[j], tap_names[j]);
    }
  }

  signal(SIGHUP, forward_signal);
  signal(SIGINT, forward_signal);
  signal(SIGTERM, forward_signal);
  signal(SIGUSR1, forward_signal);
  signal(SIGUSR2, forward_signal);

  file_write(argv[2], tap_names, taps, getuid(), getgid());

  if (strcmp(argv[3], "-d") == 0) {
    daemon(1, 0);
    own_system(args + 1, getuid(), getgid());
  } else {
    own_system(args, getuid(), getgid());
  }

  unlink(argv[2]);

  for (size_t i = 0; i < taps; i++) {
    if (tap_bridges[i]) {
      bridge_del_interface(tap_bridges[i], tap_names[i]);
    }
    tap_destroy(tap_names[i]);
  }
  for (size_t i = 0; i < argc - 3; i++)
    free(args[i]);

  return 0;
}


