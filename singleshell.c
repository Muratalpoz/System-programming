/* Melike Kurt 
   Murat Alpöz 

   chatgptden faydalandığımız kısımları yorum satırlarında belirttik.
   tüm projeyi beraber yaptık.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#define INBUF_SIZE 256
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"
#define LOG_FILE_NAME "myshell.log"

char *addr = NULL;
int fd = -1;
char log_time[25] = {'\0'};

/* Initialize shared memory */
int initmem()
{
   /* Open shared memory file and control */
   fd = shm_open(MY_SHARED_FILE_NAME, O_CREAT | O_RDWR, 0666);
   if (fd < 0){
        perror("singleshell.c:fd:line31");
        exit(1);
        
    }

    /* Map shared memory and control */
    addr = mmap(NULL, MY_FILE_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL){
        perror("singleshell.c:mmap:");
        close(fd);
        exit(1);
    }
    return 0;
}


/* log time'ı konsola yaz burada chatgptden faydalandık */
void print_log_time(){
    time_t now;
    struct tm *local_time;

    now = time(NULL);
    local_time = localtime(&now);
    strftime(log_time, 25, "%Y-%m-%d %H:%M:%S", local_time);

    printf("Shell terminated on %s\n", log_time);
}

int main(int argc, char *argv[]) {
    char inbuf[INBUF_SIZE] = {'\0'};
    int nbyte; /* input byte count */
    time_t now;
    struct tm *local_time;
    FILE *fp_log;

    pid_t shell_pid = getpid();
    pid_t parent_pid = getppid();
    now = time(NULL);
    local_time = localtime(&now);
    strftime(log_time, 25, "%Y-%m-%d %H:%M:%S", local_time);

    /* log file'ı aç */
    fp_log = fopen(LOG_FILE_NAME, "a");
    if(fp_log == NULL){
        perror("myshell.log:");
        exit(1);
    }

     /* log file'a mesajı yaz */
    fprintf(fp_log, "Shell started with pid %d (parent pid %d) on %s\n", shell_pid, parent_pid, log_time);
    fclose(fp_log);

    while (1) {
        /*shell prompt yazdır */
        write(1, "$", 2);

        /* kullanıcıdan input'u oku */
        if ((nbyte = read(0, inbuf, 255)) <= 0) {
            perror("input <=0 byte");
        } else {
            inbuf[nbyte - 1] = '\0';
        }

        printf("inbuf:%.255s\n", inbuf);

        /* kullanıcı 'exit' girdiyse diye kontrol et ve öyleyse çık */
        if (strncmp(inbuf, "exit", 4) == 0) {
          // log file'ı aç
          fp_log = fopen(LOG_FILE_NAME, "a");
            if(fp_log == NULL){
                perror("myshell.log:");
                exit(1);
            }

            /*mesajı log file'a yazdır ve kapat */
            fprintf(fp_log, "Shell terminated on %s\n", log_time);
            fclose(fp_log);

            /* konsola log time'ı yazdır ve çık */
            print_log_time();
            exit(0);
        }
        
/* fork() fonksiyonu çağrılır, yeni bir çocuk süreci oluşturulur ve çocuk sürecin id'si child_pid değişkenine atanır */
        pid_t child_pid = fork();

/* Şimdiki zaman alınır ve yerel zamana dönüştürülür, log_time değişkenine formatlanarak atanır burada chatgptden faydalandık */
        now = time(NULL);
        local_time = localtime(&now);
        strftime(log_time, 25, "%Y-%m-%d %H:%M:%S", local_time);

/* Eğer child_pid 0 ise, o zaman bu blok çocuk süreç tarafından işletilir */
        if (child_pid == 0) {
            printf("Child process!!!\n");
          /* inbuf adlı programı execl kullanarak çağırır */
            int r = execl(inbuf, inbuf, NULL); 

 /* execl hata verirse, path'i /bin/ ile birleştirip tekrar deniyor */
            if (r == -1) {
                char command[255] = {'/', 'b', 'i', 'n', '/', '\0'};
                strncat(command, inbuf, 250);
                r = execl(command, inbuf, NULL);
 /* hata verirse, execl fonksiyonu çağrılınca perror mesajı yazdırılır */
                if (r == -1)
                    perror("execl");
            }
/* execl hatası olursa, log dosyasını açarak hatayı log dosyasına yazdırır ve çıkış yapar */
            if (r == -1){
                fp_log = fopen(LOG_FILE_NAME, "a"); 
                if(fp_log == NULL){
                    perror("myshell.log:");
                    exit(1);
                }
                fclose(fp_log);
                print_log_time();
                exit(0);
            }
        }    
        initmem();
        /* Unmap the shared memory */
        munmap(addr, 1024);

        /* Close the shared memory file */
        close(fd); 
    }
    return 0;
}
