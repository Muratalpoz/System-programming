/* Melike Kurt 
   Murat Alpöz 

   chatgptden faydalandığımız kısımları yorum satırlarında belirttik.
   tüm projeyi beraber yaptık.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"

#define MAX_SHELL 10
#define DEFAULT_NSHELL 2

char *addr = NULL; /* mmap addres */
int fd = -1;       /* fd for shared file object */

/* Initialize shared memory */
int initmem() {

    /* Open shared memory file and control */
    fd = shm_open(MY_SHARED_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0) {
        perror("multishell.c:open file:");
        exit(1);
    }
    if (ftruncate(fd, 1024) == -1) {
        perror("ftruncate");
        exit(1);
    }
    
    /* Map shared memory and control */
    addr = mmap(NULL, MY_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL) {
        perror("mmap:");
        exit(1);
    }
    return 0;
}

int main(int argc, char **argv) {

    /*print number of shells when argc not equal 2*/
    if (argc != 2) {
        printf("Usage: %s <number of shells>\n", argv[0]);
        exit(1);
    }
    /*Print if number of shells is less than 1 and greater than max shell*/
    int nshells = atoi(argv[1]);
    if (nshells < 1 || nshells > MAX_SHELL) {
        printf("Number of shells must be between 1 and %d\n", MAX_SHELL);
        exit(1);
    }
    
    initmem();

    pid_t pid;
    int i;
    for (i = 0; i < nshells; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            /* child process */
            char shellnum[2];
            sprintf(shellnum, "%d", i+1);
            
            /*Executed as much as singleshell shellnum with xterm, we used chatgpt here*/
            char *shellargs[] = {"xterm", "-e", "./singleshell", shellnum, NULL};
            execvp(shellargs[0], shellargs);
            perror("execvp");
            exit(1);
        }
    }

    /* parent process */
    for (i = 0; i < nshells; i++) {
        wait(NULL);
    }

    /* create a file with timestamp we used chatgpt here */
    time_t current_time;
    char datetime[20];
    time(&current_time);
    strftime(datetime, 20, "%Y%m%d_%H%M%S", localtime(&current_time));
    char filename[50];
    sprintf(filename, "shelllog-%s.txt", datetime);

    /* write shared data to file and close file descriptors */
    int log_fd = open(filename, O_WRONLY | O_CREAT, 0666);
    write(log_fd, addr, MY_FILE_SIZE);
    close(log_fd);
    munmap(addr, 1024);
    close(fd);
    shm_unlink(MY_SHARED_FILE_NAME);

    return 0;
}
