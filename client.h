#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cinttypes>
#include <csignal>
#include <cstdio>
#include <cerrno>

#include <cstring>
#include <cstdio>
#include <vector>
//#include <string>
#include <iosfwd>


static int Connect();
static int Cycle(int fd);
//static bool FillVector(std::vector<char>& v);
static int Disconnect(int fd);
//static bool WriteRead(int fd, const std::vector<char>& txt);

#endif