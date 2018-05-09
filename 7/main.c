#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

int THREADS_NUM;
int shm_id;
void *mem;
int sub;

pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_t *r_threads_id;
pthread_t *w_thread_id;

int last_b;

void abrt(int num)
{
  if (num == SIGINT)
  {
    shmdt(mem);
    shmctl(shm_id, IPC_RMID, NULL);
    exit(0);
  }
}

void *wrt(void *args)
{
  unsigned block_n = 0;
  for (;;)
  {
    pthread_mutex_lock(&mutex);
    int i = 0;
    for (; i < 1024 * 16; i++)
    {
      ((char *)mem)[(block_n * 1024 * 16) + i] = '0' + (char)(rand() % 9);
    }

    last_b = block_n;
    if (++block_n >= 16)
    {
      block_n = 0;
    }

    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    sleep(rand() % 3);
  }
  return NULL;
}

void *rd(void *args)
{
  int id = *((int *)args);
  for (;;)
  {
    pthread_mutex_lock(&mutex);
    while (last_b == -1)
    {
      pthread_cond_wait(&cond, &mutex);
    }

    printf("Thread: %d.\nBlock#: %d.\nData: ", id, last_b);
    int i = 0;
    for (; i < 16; i++)
    {
      printf("%c", *((char *)mem) + last_b * 1024 * 16 + i);
    }
    printf("\n");
    sleep(rand() % 3);
    last_b = -1;
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int main(int argc, char **argv)
{
  last_b = -1;
  if (argc < 2)
  {
    printf("Pls pass amount of threads\n");
    exit(0);
  }
  THREADS_NUM = atoi(argv[1]);
  r_threads_id = (pthread_t *)malloc(sizeof(pthread_t) * THREADS_NUM);
  w_thread_id = (pthread_t *)malloc(sizeof(pthread_t));
  signal(SIGINT, abrt);
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);

  key_t shm_key = ftok("/dev/random", 0xDEFEC7ED);
  shm_id = shmget(shm_key, 16 * 16 * 1024, IPC_CREAT | 0666);
  if (shm_id == -1)
  {
    perror("ShmGet error");
    exit(0);
  }

  mem = shmat(shm_id, NULL, 0);
  if (mem == (void *)-1)
  {
    perror("ShmAt error");
    abrt(SIGINT);
  }

  int i = 0;
  for (; i < THREADS_NUM; i++)
  {
    pthread_create(&(r_threads_id[i]), NULL, &rd, &i);
  }
  pthread_create(&(w_thread_id[0]), NULL, &wrt, NULL);
  pthread_join(w_thread_id[0], NULL);
  return 0;
}