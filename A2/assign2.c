/*
 * assign2.c
 *
 * Name: Cobey Hollier
 * Student Number: V00893715
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include "train.h"


/*
 * If you uncomment the following line, some debugging
 * output will be produced.
 *
 * Be sure to comment this line out again before you submit 
 */

#define DEBUG	1

void ArriveBridge (TrainInfo *train);
void CrossBridge (TrainInfo *train);
void LeaveBridge (TrainInfo *train);

sem_t bridge;
sem_t mainLock;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int numEast = 0;
int numWest = 0;

pthread_cond_t nextEast = PTHREAD_COND_INITIALIZER;
pthread_cond_t nextWest = PTHREAD_COND_INITIALIZER;
/*
 * This function is started for each thread created by the
 * main thread.  Each thread is given a TrainInfo structure
 * that specifies information about the train the individual 
 * thread is supposed to simulate.
 */
void * Train ( void *arguments )
{
    TrainInfo	*train = (TrainInfo *)arguments;

    /* Sleep to simulate different arrival times */
    usleep (train->length*SLEEP_MULTIPLE);

    ArriveBridge (train);
    CrossBridge  (train);
    LeaveBridge  (train); 

    /* I decided that the paramter structure would be malloc'd 
     * in the main thread, but the individual threads are responsible
     * for freeing the memory.
     *
     * This way I didn't have to keep an array of parameter pointers
     * in the main thread.
     */
    free (train);
    return NULL;
}

/*
 * You will need to add code to this function to ensure that
 * the trains cross the bridge in the correct order.
 */
void ArriveBridge ( TrainInfo *train ) {
    printf ("Train %2d arrives going %s\n", train->trainId, 
            (train->direction == DIRECTION_WEST ? "West" : "East"));

    int priority;
    pthread_mutex_lock(&mutex);
    if(train->direction == DIRECTION_EAST) {
        numEast++;
        priority = numEast;
    } else {
        numWest++;
        priority = numWest;
    }
    while(priority != 0) {
        priority--;
        if(train-> direction == DIRECTION_EAST) {
            pthread_cond_wait(&nextEast, &mutex);
        } else {
            pthread_cond_wait(&nextWest, &mutex);
        }
    }
    if(train->direction == DIRECTION_EAST) {
        numEast--;
    } else {
        numWest--;
    }
    pthread_mutex_unlock(&mutex);
    sem_wait(&bridge);
}

/*
 * Simulate crossing the bridge.  You shouldn't have to change this
 * function.
 */
void CrossBridge ( TrainInfo *train )
{
    printf ("Train %2d is ON the bridge (%s)\n", train->trainId,
            (train->direction == DIRECTION_WEST ? "West" : "East"));
    fflush(stdout);

    /* 
     * This sleep statement simulates the time it takes to 
     * cross the bridge.  Longer trains take more time.
     */
    usleep (train->length*SLEEP_MULTIPLE);

    printf ("Train %2d is OFF the bridge(%s)\n", train->trainId, 
            (train->direction == DIRECTION_WEST ? "West" : "East"));
    fflush(stdout);
}

/*
 * Add code here to make the bridge available to waiting
 * trains...
 */
void LeaveBridge ( TrainInfo *train )
{
    sem_post(&bridge);
    sem_post(&mainLock);
}

int main ( int argc, char *argv[]) {
    char 		*filename = NULL;
    pthread_t	*tids;
    int		i;
    sem_init(&bridge, 0, 1);
    sem_init(&mainLock, 0, 0);
    int trainCount = 0;


    /* Parse the arguments */
    if ( argc < 2 ) {
        printf ("Usage: part1 n {filename}\n\t\tn is number of trains\n");
        printf ("\t\tfilename is input file to use (optional)\n");
        exit(0);
    }

    if ( argc >= 2 ) {
        trainCount = atoi(argv[1]);
    }
    if ( argc == 3 ) {
        filename = argv[2];
    }	

    initTrain(filename);

    /*
     * Since the number of trains to simulate is specified on the command
     * line, we need to malloc space to store the thread ids of each train
     * thread.
     */
    tids = (pthread_t *) malloc(sizeof(pthread_t)*trainCount);

    /*
     * Create all the train threads pass them the information about
     * length and direction as a TrainInfo structure
     */
    for (i=0;i<trainCount;i++) {
        TrainInfo *info = createTrain();

        printf ("Train %2d headed %s length is %d\n", info->trainId,
                (info->direction == DIRECTION_WEST ? "West" : "East"),
                info->length );

        if ( pthread_create (&tids[i],0, Train, (void *)info) != 0 )
        {
            printf ("Failed creation of Train.\n");
            exit(0);
        }
    }

    int trainsGone = 0;
    while(trainsGone < trainCount) {
        int lastIteration = 0;
        while(1) {
            if(numEast > 0) {
                pthread_mutex_lock(&mutex);
                pthread_cond_broadcast(&nextEast);
                pthread_mutex_unlock(&mutex);
                sem_wait(&mainLock);
                trainsGone++;
            } else {
                lastIteration = 0;
                break;
            }
            if(numWest > 0) {
                if(lastIteration) {
                    break;
                } else {
                    lastIteration = 1;
                }
            }
        }
        if(numWest > 0) {
            pthread_mutex_lock(&mutex);
            pthread_cond_broadcast(&nextWest);
            pthread_mutex_unlock(&mutex);
            sem_wait(&mainLock);
            trainsGone++;
        }
    }

    // This code waits for all train threads to terminate
    for (i=0;i<trainCount;i++) {
        pthread_join (tids[i], NULL);
    }

    free(tids);
    // TODO: Need to close input file
    return 0;
}

