#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>





#define BUFFER_SIZE  200 * 1024

int NUM_TIMES  =  10;
double *timer;
pthread_t * tid;

void read_file(int i) {
    
    FILE * pFile;
    long lSize;
    char * buffer;
    
    char filepath[50];
    sprintf(filepath, "file%d", i);
    pFile = fopen ( filepath , "rb" );
    if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);
//    printf("lSize = %ld\n", lSize);

    // allocate memory
    buffer = (char*) malloc (sizeof(char)*BUFFER_SIZE);
    if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}
//    printf("BUFFER_SIZE = %d\n", BUFFER_SIZE);

    while (!feof(pFile)) {
        struct timespec start;
        struct timespec end;
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        size_t result = fread(buffer, 1, BUFFER_SIZE, pFile);
        clock_gettime(CLOCK_MONOTONIC, &end);

        double start_t = ((double) start.tv_sec * 1e9 + (double) start.tv_nsec);
        double end_t =   ((double) end.tv_sec   * 1e9 + (double) end.tv_nsec);
        timer[i] += (end_t - start_t) / 1e9;
//        printf("%d Total time taken by CPU: seconds = %f\n", i, timer[i]);

        size_t position = ftell (pFile);
//        printf("%d: %.2f%%\n", i, 100.0 * (double) position / (double) lSize);
//        fflush(stdout);
    }

    // terminate
    fclose (pFile);
    free (buffer);
}

void execute(int i) {
    for (int n = 0; n < NUM_TIMES; n++) {
        read_file(i);
    }
}

void* run(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    if(pthread_equal(id,tid[0])) {
        //printf("First  thread processing: 0x%016lX\n", tid[0]);
        i = 0;
    } else if (pthread_equal(id,tid[1])) {
        //printf("Second thread processing: 0x%016lX\n", tid[1]);
        i = 1;
    } else if (pthread_equal(id,tid[2])) {
        //printf("Third  thread processing: 0x%016lX\n", tid[2]);
        i = 2;
    } else if (pthread_equal(id,tid[3])) {
        //printf("Fourth thread processing: 0x%016lX\n", tid[3]);
        i = 3;
    }

    execute(i);

    return NULL;
}

int main(int argc, char* argv[]) {
    
    unsigned int ncores=0,nthreads=0;
    asm volatile("cpuid": "=a" (ncores), "=b" (nthreads) : "a" (0xb), "c" (0x1) : );
    printf("Cores=%d, Threads=%d\n", ncores, nthreads);
    
    timer = (double *) calloc(ncores, sizeof(double));
    tid = (pthread_t *) calloc(ncores, sizeof(pthread_t));

    int cores_used = ncores;
    char * mode = "parallel";
    if (argc >= 2) {
        
        mode = argv[1];
        if (strcmp(mode, "serial") == 0) {
            cores_used = 1;
        } else if (strcmp(mode, "parallel") == 0) {
            cores_used = ncores;
        }
        
        if (argc >= 3) {
            NUM_TIMES = atoi(argv[2]);
        }
    }
    printf("%s using %d core(s) for %d time(s).\n", mode, cores_used, NUM_TIMES);
    
    if (strcmp(mode, "serial") == 0) {
        
        for (int i = 0; i < ncores; i++){
            execute(i);
        }
        
    } else if (strcmp(mode, "parallel") == 0) {
        
        for (int i = 0; i < ncores; i++){
            int err = pthread_create(&(tid[i]), NULL, &run, NULL);
            if (err != 0)
                printf("\ncan't create thread :[%s]", strerror(err));
        }
        for (int i = 0; i < ncores; i++){
            pthread_join(tid[i], NULL);
        }
    }
    
    double sum = 0;
    for (int i = 0; i < ncores; i++){
        sum += timer[i];
    }
    sum /= cores_used;

    printf("Total time taken by CPUs: seconds = %f\n", sum);

    return 0;
}
