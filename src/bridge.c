/* Copyright (c) 2024 Krypto-IT Jakub Juszczakiewicz
 * All rights reserved.
 */

#include <linux/if.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <linux/if_bridge.h>
#include <unistd.h>

extern unsigned int if_nametoindex(const char *ifname);

int bridge_add_interface(const char * bridge, const char * dev)
{
  struct ifreq ifr;
  int err;
  int br_sock;

  if ((br_sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    return errno;

  int ifindex = if_nametoindex(dev);
  if (ifindex == 0) {
    close(br_sock);
    return -1;
  }

  strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
  ifr.ifr_ifindex = ifindex;
  err = ioctl(br_sock, SIOCBRADDIF, &ifr);
  if (err < 0) {
    unsigned long args[4] = { BRCTL_ADD_IF, ifindex, 0, 0 };

    ifr.ifr_data = (char *)args;
    err = ioctl(br_sock, SIOCDEVPRIVATE, &ifr);
  }

  close(br_sock);

  if (errno < 0)
    return err;
  return 0;
}

int bridge_del_interface(const char * bridge, const char * dev)
{
  struct ifreq ifr;
  int err;
  int br_sock;

  if ((br_sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    return errno;

  int ifindex = if_nametoindex(dev);

  if (ifindex == 0) {
    close(br_sock);
    return ENODEV;
  }

  strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
  ifr.ifr_ifindex = ifindex;
  err = ioctl(br_sock, SIOCBRDELIF, &ifr);
  if (err < 0)
  {
    unsigned long args[4] = { BRCTL_DEL_IF, ifindex, 0, 0 };

    ifr.ifr_data = (char *)args;
    err = ioctl(br_sock, SIOCDEVPRIVATE, &ifr);
  }

  close(br_sock);

  if (errno < 0)
    return err;
  return 0;
}
