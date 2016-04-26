#include "multiProc.h"

int forkPointer = 0;

void SigInt(int signo)
{
  pid_t pid;
  int stat;

  if (signo == SIGCHLD)
  {
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
      forkPointer--; // child terminated
  }
}
