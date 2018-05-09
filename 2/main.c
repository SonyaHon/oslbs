#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// Terminal colors
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int main(int argc, char **argv) {
    DIR *dp;
    struct dirent *ep;

    char* dir_name = "./"; 
    if(argc > 1) {
        dir_name = argv[1];
    }

    if((dp = opendir(dir_name)) == NULL) {
        perror("Error");
        (void)exit(1);
    }

    while( (ep = readdir(dp)))  {

        if(strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0) {
            printf("%s%s%s\n", ANSI_COLOR_CYAN, ep->d_name, ANSI_COLOR_RESET);
            continue;
        }
        else {
            struct stat fileStat;
            lstat(ep->d_name, &fileStat);

            char* color = S_ISDIR(fileStat.st_mode) ? ANSI_COLOR_CYAN : ANSI_COLOR_RESET;
            struct tm* timeinfo = localtime (&fileStat.st_mtime);
            char date[100];
            strftime(date, 100, "%b %d %H:%S", timeinfo);
            char filename[60];
            strcpy(filename, "%s%s%s%s%s%s%s%s%s%s %d %s %s %lld %s ");
            strcat(filename, color);
            strcat(filename, "%s\x1b[0m\n");

            printf(filename, 
            (S_ISDIR(fileStat.st_mode)) ? "d" : "-",
            (fileStat.st_mode & S_IRUSR) ? "r" : "-",
            (fileStat.st_mode & S_IWUSR) ? "w" : "-",
            (fileStat.st_mode & S_IXUSR) ? "x" : "-",
            (fileStat.st_mode & S_IRGRP) ? "r" : "-",
            (fileStat.st_mode & S_IWGRP) ? "w" : "-",
            (fileStat.st_mode & S_IXGRP) ? "x" : "-",
            (fileStat.st_mode & S_IROTH) ? "r" : "-",
            (fileStat.st_mode & S_IWOTH) ? "w" : "-",
            (fileStat.st_mode & S_IXOTH) ? "x" : "-",
            fileStat.st_nlink,
            getpwuid(fileStat.st_uid)->pw_name,
            getgrgid(fileStat.st_gid)->gr_name,
            fileStat.st_size,
            date,
            ep->d_name
            );
        }
    }
    (void)closedir(dp);

    return 0;
}