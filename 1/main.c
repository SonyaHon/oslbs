#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
  pid_t pid;
  switch(pid = fork()) {
    case -1: {
      perror("Fork error. Exiting...");
      exit(1);
    }
    case 0: {
      char* file_name = "res_file";
      if(argc > 1) {
        file_name = argv[1];
      }
      FILE* fd = fopen(file_name, "w");
      if(fd == NULL) {
        perror(strcat("Can`t open file ", file_name));
        exit(2);
      }
      fprintf(fd, "pid: %d, ppid: %d\n\n", getpid(), getppid());

      srand(time(NULL));
       int i = 0;
       for(;i < 1024 * 1024 * 10; i++) {
         char r_el = rand();
         fwrite(&r_el, sizeof(char), 1, fd);
       }

      fclose(fd);
      return 0;
    }
    default: {
      int e_code;
      waitpid(pid, &e_code, 0);
      
      struct tms buffer;
      times(&buffer);
      printf("Proccess %d ended in %fms width %i exit code.\n", pid, (double)(buffer.tms_cutime), e_code);
      return 0;
    }
  }

  return 0;
}