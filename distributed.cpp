#include "distributed.h"

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void handler(int signo) { (void)signo; }

bool signalIgnoring() {
  struct sigaction act;
  act.sa_handler = handler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);

  if (sigaction(SIGINT, &act, 0) == -1) {
    fprintf(stderr, "Can't setup SIGINT ignoring\n");
    return false;
  }

  if (sigaction(SIGPIPE, &act, 0) == -1) {
    fprintf(stderr, "Can't setup SIGPIPE ignoring\n");
    return false;
  }

  return true;
}


