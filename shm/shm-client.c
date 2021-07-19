#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define BILLION  1000000000.0
#define BUF_SIZE 8192
#define MAX_CLIENTS 10

struct shmseg {
    char* clients_bufptr[MAX_CLIENTS];
    int clients_num;
    char buf[BUF_SIZE * MAX_CLIENTS];
    int live_client;
};

int main(int argc, char *argv[]) {

    if (argc != 2){
		printf("run program: \n./shm-client text\n");
		return 1;
	}

    int numtimes;

    // struct shmseg *shmp;
    char *bufptr;
    int spaceavailable;

    /* the size (in bytes) of shared memory object */
    const int SIZE = sizeof(struct shmseg);

    /* name of the shared memory object */
    const char *a = "a";
    const char *b = "b";

    /* shared memory file descriptor */
    int shmfd_a;
    int shmfd_b;

    /* pointer to shared memory obect */
    struct shmseg *ptr_a;
    struct shmseg *ptr_b;

    /* create the shared memory object */
    shmfd_a = shm_open(a, O_RDWR, 0666);
    shmfd_b = shm_open(b, O_RDWR, 0666);

    if (shmfd_a == -1 || shmfd_b == -1) {
      perror("create shared memory");
      printf("please run server first.\n");
      return 1;
    }

    /* configure the size of the shared memory object */
    ftruncate(shmfd_a, SIZE);
    ftruncate(shmfd_b, SIZE);

    /* memory map the shared memory object */
    ptr_a = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shmfd_a, 0);
    ptr_b = mmap(0, SIZE, PROT_READ, MAP_SHARED, shmfd_b, 0);
    if (ptr_a == MAP_FAILED || ptr_b == MAP_FAILED){
        perror("can't map memory");
        return 1;
    }

    struct timespec start, end;

    sem_t *sem = sem_open("sem" , O_EXCL);

    sem_wait(sem);
    int client_nums = ptr_a->clients_num;
    ptr_a->clients_num++;
    ptr_a->live_client++;
    sem_post(sem);

    char* ch = argv[1];
    bufptr = ptr_a->buf + (client_nums * sizeof(char [BUF_SIZE]));

    // write data to shm
    clock_gettime(CLOCK_REALTIME, &start);
    sprintf(bufptr, "%s", ch);
    ptr_a->clients_bufptr[client_nums + 1] = bufptr + sizeof(char [BUF_SIZE]);

    // read data from
    while (strcmp(ptr_b->buf + (client_nums * sizeof(char [BUF_SIZE])), "message received.") != 0);
    clock_gettime(CLOCK_REALTIME, &end);
    sem_wait(sem);
    ptr_a->live_client--;
    sem_post(sem);
    printf("from server: %s\n", (ptr_b->buf + (client_nums * sizeof(char [BUF_SIZE]))));

    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / BILLION;
    printf("spent time: %f sec.\n", time_spent);

    return 0;
}