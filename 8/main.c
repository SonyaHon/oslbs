#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

char *FIFO = "/tmp/fifo";
int sub;
int fid;
char buff[11];

void sig_h(int num)
{
  if (num == SIGINT)
  {
    if (sub == 0)
    {
      unlink(FIFO);
    }
    exit(0);
  }
  if (num == SIGPIPE)
  {
    unlink(FIFO);
    exit(0);
  }
}

int main(int argc, char **argv)
{
  signal(SIGINT, sig_h);
  signal(SIGPIPE, sig_h);
  sub = 1;

  struct stat st;
  if (stat(FIFO, &st) != 0)
  {
    sub = 0;
    mkfifo(FIFO, 0666);
  }

  if (sub == 1)
  {
    fid = open(FIFO, O_RDONLY);
    for (;;)
    {
      sleep(1);
      read(fid, &buff, sizeof(char) * 11);
      printf("Time is: %s", buff);
    }
  }
  else
  {
    fid = open(FIFO, O_WRONLY);
    for (;;)
    {
      sleep(1);
      unsigned time_c = (unsigned)time(NULL);
      dprintf(fid, "%u\n", time_c);
    }
  }
}