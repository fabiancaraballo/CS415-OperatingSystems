#ifndef __HELPERS_H__
#define __HELPERS_H_

#include <sys/time.h>
#include <pthread.h>

#define QUACKSIZE       255
#define CAPTIONSIZE     120
#define MAXENTRIES      100
#define MAXTOPICS       4
#define MAXTRIES        3
#define NUMPROXIES      8

typedef struct
{
    int entrynum;
    struct timeval timestamp;
    int pubID;
    char photoUrl[QUACKSIZE];        /* URL to photo  */
    char photoCaption[CAPTIONSIZE];  /* photo caption */
} topicentry_t, *ptopicentry_t;

typedef struct
{
    int head;
    int tail;
    int size;
    int topic_counter;
    pthread_mutex_t lck;
    pthread_cond_t added, removed;
    topicentry_t data[MAXENTRIES];
} topicqueue_t, *ptopicqueue_t;

typedef struct
{
    int id;
    int status;
    char *filename;
    pthread_t thread;
} worker_t, *pworker_t;

int tq_init(ptopicqueue_t tq, int sz);
int tq_enqueue(ptopicqueue_t tq, ptopicentry_t ent);
int tq_getentry(ptopicqueue_t tq, ptopicentry_t ent, int last_entry);
int tq_dequeue(ptopicqueue_t tq, int delta);

ptopicqueue_t get_queue(int topic);

#endif
