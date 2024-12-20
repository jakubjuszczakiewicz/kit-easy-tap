/* Copyright (c) 2024 Krypto-IT Jakub Juszczakiewicz
 * All rights reserved.
 */

#include "tools.h"

#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <unistd.h>

pid_t child = 0;

int check_pattern(const char * pattern)
{
  int count = 0;
  int brcount = 0;

  for (size_t i = 0; pattern[i] != 0; i++) {
    if (strchr("abcdefghijklmnopqrstuvwxyz0123456789.", pattern[i]) != NULL)
      continue;
    if (pattern[i] == ':') {
      brcount++;
      if (brcount > 1)
        return -1;
      continue;
    }
    if (pattern[i] == '%') {
      if (pattern[i + 1] == 'u') {
        i++;
        count++;
        continue;
      }
      if (pattern[i + 1] != '0')
        return -1;
      if (strchr("0123456789", pattern[i + 2]) == NULL)
        return -1;
      if (pattern[i + 3] != 'u')
        return -1;
      i += 3;
      count++;
    }
  }

  return count;
}

int is_dev_exists(const char * dev)
{
  struct ifreq ifr;
  int fd, err;

  if ((fd = socket( AF_INET, SOCK_DGRAM, 0)) < 0)
    return -1;

  memset(&ifr, 0, sizeof(ifr));
  if (strlen(dev) >= sizeof(ifr.ifr_name)) {
    strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
  } else {
    strcpy(ifr.ifr_name, dev);
  }

  if ((err = ioctl(fd, SIOCGIFFLAGS, (void *) &ifr)) < 0) {
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}

char * str_repall(const char * in, const char * pattern, const char * rep)
{
  size_t in_len = strlen(in);
  size_t pattern_len = strlen(pattern);
  size_t rep_len = strlen(rep);

  if (pattern_len == rep_len) {
    char * out = strdup(in);
    char * pos;
    while ((pos = strstr(out, pattern))) {
      memcpy(pos, rep, rep_len);
    }
    return out;
  }

  size_t count = 0;
  const char * pos = in;
  while ((pos = strstr(pos, pattern))) {
    count++;
    pos += pattern_len;
  }

  size_t out_len;
  if (pattern_len < rep_len) {
    out_len = in_len + (rep_len - pattern_len) * count;
  } else {
    out_len = in_len - (pattern_len - rep_len) * count;
  }

  char * out = malloc(out_len + 1);
  pos = in;
  char * out_pos = out, * pos2;
  while ((pos2 = strstr(pos, pattern))) {
    memcpy(out_pos, pos, pos2 - pos);
    out_pos += pos2 - pos;
    memcpy(out_pos, rep, rep_len);
    out_pos += rep_len;
    pos = pos2 + pattern_len;
  }
  strcpy(out_pos, pos);

  return out;
}

int in_list(char ** list, size_t idx)
{
  for (size_t i = 0; i < idx; i++)
    if (strcmp(list[i], list[idx]) == 0)
      return 1;
  return 0;
}

int own_system(char * args[], uid_t user, gid_t group)
{
  child = fork();
  if (child < 0)
    return -1;

  if (child == 0) {
    setgid(group);
    setuid(user);
    setegid(group);
    seteuid(user);
    execvp(args[0], args);
    return 0;
  }

  if (waitpid(child, NULL, 0) != child)
    return -1;

  child = 0;
  return 0;
}
