/* Copyright (c) 2024 Krypto-IT Jakub Juszczakiewicz
 * All rights reserved.
 */

#include "tap.h"

#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <linux/if.h>

int tap_create(const char * dev, uid_t user, gid_t group)
{
  struct ifreq ifr;
  int fd, err;
  const char * tundev = "/dev/net/tun";

  if ((fd = open(tundev, O_RDWR)) < 0) {
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));
  if (strlen(dev) >= sizeof(ifr.ifr_name)) {
    strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
  } else {
    strcpy(ifr.ifr_name, dev);
  }

  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
    close(fd);
    return err;
  }

  if (ioctl(fd, TUNSETOWNER, user) < 0) {
    ioctl(fd, TUNSETPERSIST, 0);
    close(fd);
    return -1;
  }

  if (ioctl(fd, TUNSETGROUP, group) < 0) {
    ioctl(fd, TUNSETPERSIST, 0);
    close(fd);
    return -1;
  }

  if (ioctl(fd, TUNSETPERSIST, 1) < 0) {
    ioctl(fd, TUNSETPERSIST, 0);
    close(fd);
    return -1;
  }
  close(fd);

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    return 0;

  memset(&ifr, 0, sizeof(ifr));
  if (strlen(dev) >= sizeof(ifr.ifr_name)) {
    strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
  } else {
    strcpy(ifr.ifr_name, dev);
  }

  ifr.ifr_flags |= IFF_UP;
  ioctl(fd, SIOCSIFFLAGS, &ifr);

  return 0;
}

int tap_destroy(const char * dev)
{
  struct ifreq ifr;
  int fd, err;
  const char * tundev = "/dev/net/tun";

  if ((fd = open(tundev, O_RDWR)) < 0) {
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));
  if (strlen(dev) >= sizeof(ifr.ifr_name)) {
    strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
  } else {
    strcpy(ifr.ifr_name, dev);
  }

  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
    close(fd);
    return err;
  }

  ioctl(fd, TUNSETPERSIST, 0);
  close(fd);
}
