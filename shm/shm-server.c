#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>


#define BUF_SIZE 8192
#define MAX_CLIENTS 10

struct thread_args
{
	void * ptr_a;
	void * ptr_b;
};


struct shmseg {
    char* clients_bufptr[MAX_CLIENTS];
    int clients_num;
    char buf[BUF_SIZE * MAX_CLIENTS];
    int live_client;
};

void *connection_handler(void *);

int main() {

    struct thread_args thread_arg;
    
    sem_unlink("sem");
    sem_unlink("server");
    sem_t *sem = sem_open("sem" , O_CREAT | O_EXCL , S_IRUSR|S_IWUSR , 1);
    sem_t *sem_server = sem_open("server" , O_CREAT | O_EXCL , S_IRUSR|S_IWUSR , 1);

    char *bufptr;
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

    shm_unlink(a);
    shm_unlink(b);
    shmfd_a = shm_open(a, O_CREAT | O_RDWR, 0666);
    shmfd_b = shm_open(b, O_CREAT | O_RDWR, 0666);
    if (shmfd_a == -1 || shmfd_b == -1) {
      perror("create shared memory");
      return 1;
    }

    /* configure the size of the shared memory object */
    ftruncate(shmfd_a, SIZE);
    ftruncate(shmfd_b, SIZE);
    
    /* memory map the shared memory object */
    ptr_a = mmap(0, SIZE, PROT_READ, MAP_SHARED, shmfd_a, 0);
    ptr_b = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shmfd_b, 0);
    if (ptr_a == MAP_FAILED || ptr_b == MAP_FAILED){
        perror("can't map memory");
        return 1;
    }
    thread_arg.ptr_a = ptr_a;
    thread_arg.ptr_b = ptr_b;

    /* Transfer blocks of data from shared memory to stdout*/
    while (ptr_b->clients_num <= MAX_CLIENTS) {
        if(ptr_b->clients_num == ptr_a->clients_num){
            continue;
        }

        ptr_b->clients_num++;
        ptr_b->live_client++;
        pthread_t cli_thread;
        pthread_create( &cli_thread , NULL ,  connection_handler , (void *)&thread_arg);
    }

    printf("Reading Process: Complete\n");
    shm_unlink(a);
    shm_unlink(b);

    return 0;
}
void *connection_handler(void *arg)
{   
    struct shmseg * ptr_a = ((struct thread_args*)arg)->ptr_a;
    struct shmseg * ptr_b = ((struct thread_args*)arg)->ptr_b;

    char *bufptr;
    int client_number = ptr_b->clients_num - 1;
    printf("new client added\nclient_id: %d\nlive_client: %d\n", client_number + 1, ptr_b->live_client);
    printf("from client: %s\n", (ptr_a->buf + (client_number * sizeof(char [BUF_SIZE]))));
    printf("--------------------\n");

    bufptr = ptr_b->buf + (client_number * sizeof(char [BUF_SIZE]));
    sprintf(bufptr, "%s", "message received.");
    ptr_b->live_client--;
	return 0;
}