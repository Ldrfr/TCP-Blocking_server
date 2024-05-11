#ifndef __DISTRIBUTED_H__
#define __DISTRIBUTED_H__

#include <vector>
#include <cstdio>

bool readLine(std::vector<char> &line);

bool signalIgnoring();

ssize_t myRead(int fd, void *buf, size_t nbyte);
ssize_t myWrite(int fd, const void *buf, size_t nbyte);

#endif