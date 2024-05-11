#ifndef __SERVER_H__
#define __SERVER_H__

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cinttypes>
#include <csignal>
#include <cstdio>
#include <cerrno>

#include <limits.h>
#include <stdlib.h>

//static void Loop(int ld, int database_size);    // contains infinite loop // databaes isn't defined in this header
static int Connect();
static int Disconnect(int fd);

#endif