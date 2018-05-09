#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>

int sub;
void *mem;
int shm_id;

int semid;
struct sembuf sem;

void abrt(int num)
{
  if (num == SIGINT)
  {
    shmdt(mem);
    if (sub == 0)
    {
      shmctl(shm_id, IPC_RMID, NULL);
      semctl(semid, 0, IPC_RMID);
    }
    exit(0);
  }
}

int main()
{
  signal(SIGINT, abrt);

  key_t shm_key = ftok("/dev/random", 0xDEFEC7ED);
  shm_id = shmget(shm_key, 256, 0);
  sub = 1;

  if (shm_id == -1)
  {
    shm_id = shmget(shm_key, 256, IPC_CREAT | 0666);
    if (shm_id == -1)
    {
      perror("ShmGet error");
    }
    sub = 0;
  }

  mem = shmat(shm_id, NULL, 0);
  if (mem == (void *)-1)
  {
    perror("ShmAt error");
    exit(-1);
  }

  sem.sem_num = 0;
  sem.sem_flg = 0;

  if (sub == 0)
  {
    semid = semget(shm_key, 1, IPC_CREAT | IPC_EXCL | 0666);
    semctl(semid, 0, SETVAL, 1);
    for (;;)
    {
      sleep(1);
      unsigned time_c = (unsigned)time(NULL);
      sprintf((char *)(mem), "%u\n", time_c);
      sem.sem_op = 1;
      semop(semid, &sem, 1);
    }
  }
  else
  {
    semid = semget(shm_key, 1, 0660);
    for (;;)
    {
      sem.sem_op = -1;
      if (semop(semid, &sem, 1) == -1)
      {
        perror("SemOp error");
        abrt(SIGINT);
      }
      printf("Time is: %s", (char *)mem);
    }
  }

  return 0;
}