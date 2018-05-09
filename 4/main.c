#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

int sub;
void *mem;
int shm_id;
int THREADS_NUM;

pthread_mutex_t mutex;
pthread_t *thread_id;

void abrt(int num)
{
  if (num == SIGINT)
  {
    shmdt(mem);
    if (sub == 0)
    {
      shmctl(shm_id, IPC_RMID, NULL);
    }
    exit(0);
  }
}

void *wrt(void *args)
{
  int id = *((int *)args);
  for (;;)
  {
    sleep(rand() % THREADS_NUM + 1);
    pthread_mutex_lock(&mutex);
    unsigned time_c = (unsigned)time(NULL);
    sprintf((char *)(mem), "thread: %d: %u\n", id, time_c);
    sleep(rand() % 3);
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void *rd(void *args)
{
  int id = *((int *)args);
  for (;;)
  {
    sleep(rand() % THREADS_NUM + 1);
    pthread_mutex_lock(&mutex);
    printf("Thread: %d: %s", id, (char *)mem);
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int main(int argc, char **argv)
{

  if (argc < 2)
  {
    printf("Pls enter number of threads to start\n");
    exit(0);
  }
  THREADS_NUM = atoi(argv[1]);
  thread_id = (pthread_t *)malloc(THREADS_NUM * sizeof(pthread_t));

  pthread_mutex_init(&mutex, NULL);

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

  if (sub == 0)
  { //master
    int i = 0;
    for (; i < THREADS_NUM; i++)
    {
      pthread_create(&thread_id[i], NULL, &wrt, &i);
    }
  }
  else
  { //slave
    int i = 0;
    for (; i < THREADS_NUM; i++)
    {
      pthread_create(&thread_id[i], NULL, &rd, &i);
    }
  }

  int i = 0;
  for (; i < THREADS_NUM; i++)
  {
    pthread_join(thread_id[i], NULL);
  }

  return 0;
}