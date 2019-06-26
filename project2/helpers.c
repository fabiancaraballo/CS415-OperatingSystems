#include "helpers.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

topicqueue_t queues[MAXTOPICS];
worker_t publishers[NUMPROXIES];
worker_t subscribers[NUMPROXIES];

int tq_init(ptopicqueue_t tq, int sz)
{
    if (tq == NULL)
        return -1;

    tq->head = 0;
    tq->tail = 0;
    tq->size = sz;
    tq->topic_counter = 0;
    if (pthread_mutex_init(&tq->lck, NULL))
        return -1;

    if (pthread_cond_init(&tq->added, NULL))
        return -1;

    if (pthread_cond_init(&tq->removed, NULL))
        return -1;

    memset(tq->data, 0, sizeof(tq->data));
    int i;
    for (i = 0; i < tq->size; i++)
        tq->data[i].pubID = -1;

    return 0;
}

int tq_enqueue(ptopicqueue_t tq, ptopicentry_t ent)
{
    if (tq == NULL || ent == NULL)
        return -1;

    pthread_mutex_lock(&tq->lck);

    if ((tq->head + 1) % tq->size == tq->tail)
    {
        pthread_mutex_unlock(&tq->lck);
        return -1;
    }

    int idx = tq->head;
    tq->data[idx].pubID = ent->pubID;
    tq->data[idx].entrynum = ++tq->topic_counter;

    gettimeofday(&tq->data[idx].timestamp, NULL);
    strcpy(tq->data[idx].photoUrl, ent->photoUrl);
    strcpy(tq->data[idx].photoCaption, ent->photoCaption);

    tq->head = (tq->head + 1) % tq->size;
    pthread_mutex_unlock(&tq->lck);
    return 0;
}

int tq_getentry(ptopicqueue_t tq, ptopicentry_t ent, int last_entry)
{
    if (tq == NULL || ent == NULL)
        return -1;

    pthread_mutex_lock(&tq->lck);

    if (tq->head == tq->tail)
    {
        pthread_mutex_unlock(&tq->lck);
        return 0;
    }

    int new_entry = 0, idx = -1;
    int i;
    for (i = 0; i < tq->size; i++)
    {
        idx = (tq->tail + i) % tq->size;
        if (tq->data[idx].pubID == -1)
            break;

        if (tq->data[idx].entrynum > last_entry)
        {
            new_entry = tq->data[idx].entrynum;
            break;
        }
    }

    if (new_entry != 0)
        memcpy(ent, &tq->data[idx], sizeof(topicentry_t));

    pthread_mutex_unlock(&tq->lck);
    return new_entry;
}

int tq_dequeue(ptopicqueue_t tq, int delta)
{
    if (tq == NULL)
        return -1;

    pthread_mutex_lock(&tq->lck);

    if (tq->head == tq->tail)
    {
        pthread_mutex_unlock(&tq->lck);
        return 0;
    }

    int idx = tq->tail;
    struct timeval now;

    gettimeofday(&now, NULL);
    int time_diff = difftime(now.tv_sec, tq->data[idx].timestamp.tv_sec);

    if (time_diff >= delta)
    {
        memset(&tq->data[idx], 0, sizeof(topicentry_t));
        tq->data[idx].pubID = -1;
        tq->tail = (tq->tail + 1) % tq->size;
    }

    pthread_mutex_unlock(&tq->lck);
    return 0;
}

ptopicqueue_t get_queue(int topic)
{
    if (topic < 0 || topic >= MAXTOPICS)
        return NULL;

    return &queues[topic];
}

void *cleanup(void *args)
{
    int delta, all_stopped, i, j;
    printf("dequeue thread created\n");

    delta = *((int*)args);

    do
    {
        all_stopped = 1;
        for (i = 0; i < NUMPROXIES && all_stopped; i++)
            if (subscribers[i].status == 0 || publishers[i].status == 0)
                all_stopped = 0;

        for (j = 0; j < MAXTOPICS; j++)
            tq_dequeue(get_queue(j), delta);

    } while(!all_stopped);

    *((int*)args) = 0;
    printf("dequeue thread exited\n");
    return NULL;
}

void *subscriber(void *args)
{
    FILE *fp;
    char tmp[255];
    pworker_t me;
    topicentry_t ent;
    FILE *output[MAXTOPICS];
    int topic;
    int entry = 0;
    int tries;
    int  seconds, i;

    for (i = 0; i < MAXTOPICS; i++)
        output[i] = NULL;

    me = (pworker_t)args;
    printf("subscriber thread %d %s\n", me->id, me->filename);

    fp = fopen(me->filename, "r");
    if (fp == NULL)
    {
        me->status = -1;
        printf("failed to open file %s, subscriber thread %d exiting.\n", me->filename, me->id);
        return NULL;
    }

    while(fscanf(fp, "%d %d", &topic, &seconds) > 0)
    {
        if (output[topic] == NULL)
        {
            sprintf(tmp, "subscriber_%d_topic_%d.html", me->id, topic);
            output[topic] = fopen(tmp, "w+");
            if (output[topic] == NULL)
            {
                printf("failed to open file `%s`\n", tmp);
                continue;
            }

            fprintf(output[topic], "<!DOCTYPE html><head></head><body>Topic %d<table><tr><td align=\"left\"><img SRC=\"puddles.gif\" WIDTH=\"140\" HEIGHT=\"140\">&nbsp&nbsp&nbsp&nbsp&nbsp</a></td><td align=\"center\"><h1>InstaQuack</h1><h1>Subscriber %d : Topic %d</h1></td><td align=\"right\">&nbsp&nbsp&nbsp&nbsp&nbsp<img SRC=\"puddles.gif\" WIDTH=\"140\" HEIGHT=\"140\"></a></td></tr></table>",
                    topic, me->id, topic);
        }

        ptopicqueue_t tq = get_queue(topic);
        if (tq == NULL)
        {
            printf("subscriber thread %d: no such topic %d exist.\n", me->id, topic);
            continue;
        }

        tries = 0;
        do
        {
            int ret = tq_getentry(tq, &ent, entry);
            if (ret != 0)
            {
                entry = ret;
                break;
            }
            pthread_yield();
        } while(++tries < MAXTRIES);

        if (tries != MAXTRIES)
        {
            printf("subscriber thread %d: %s (topic: %d)\n", me->id, ent.photoUrl, topic);
            fprintf(output[topic], "<hr><img SRC=\"%s\"></a><br>%s<hr>", ent.photoUrl, ent.photoCaption);
        }

        sleep(seconds);
    }

    fclose(fp);

     int j;
    for (j = 0; j < MAXTOPICS; j++)
    {
        if (output[j])
        {
            fprintf(output[j], "</body></html>");
            fclose(output[j]);
        }
    }

    printf("subscriber thread %d Exited\n", me->id);
    me->status = 1;
    return NULL;
}

void *publisher(void *args)
{
    FILE *fp;
    pworker_t me;
    topicentry_t ent;
    int topic, tries, seconds;

    me = (pworker_t)args;

    printf("publisher thread %d %s\n", me->id, me->filename);

    fp = fopen(me->filename, "r");
    if (fp == NULL)
    {
        me->status = -1;
        printf("failed to open file %s, publisher thread %d exiting.\n", me->filename, me->id);
        return NULL;
    }

    while(fscanf(fp, "%d\n%[^\n]\n%[^\n]\n%d\n", &topic, ent.photoUrl, ent.photoCaption, &seconds) > 0)
    {
        ent.pubID = me->id;

        ptopicqueue_t tq = get_queue(topic);
        if (tq == NULL)
        {
            printf("publisher thread %d: no such topic %d exist.\n", me->id, topic);
            continue;
        }

        do
        {
            tries = 0;
            if (!tq_enqueue(tq, &ent))
                break;

            pthread_yield();
        } while(++tries < MAXTRIES);

        sleep(seconds);
    }

    fclose(fp);
    printf("publisher thread %d Exited\n", me->id);
    me->status = 1;
    return NULL;
}

void init_all()
{
    int i,j; 
    for (j = 0; j < MAXTOPICS; j++)
        get_queue(j)->size = 0;

    for (i = 0; i < NUMPROXIES; i++)
    {
        publishers[i].id = i;
        subscribers[i].id = i;
        publishers[i].status = -1;
        subscribers[i].status = -1;
    }
}

void run_proxy(int delta)
{
    int i, j;
    pthread_t cleanupth;

    for (i = 0; i < NUMPROXIES; i++)
    {
        if (!publishers[i].status)
            pthread_create(&publishers[i].thread, NULL, &publisher, &publishers[i]);
    }

    for (j = 0; j < NUMPROXIES; j++)
    {
        if (!subscribers[j].status)
            pthread_create(&subscribers[j].thread, NULL, &subscriber, &subscribers[j]);
    }

    pthread_create(&cleanupth, NULL, &cleanup, &delta);
    while(delta != 0)
    {
        sleep(1);
        pthread_yield();
    }
}

