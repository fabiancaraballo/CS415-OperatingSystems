#include "helpers.c"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

topicqueue_t queues[MAXTOPICS];
worker_t publishers[NUMPROXIES];
worker_t subscribers[NUMPROXIES];


int main(int argc, char *argv[])
{
    char line[255];
    char  tmp[255];
    int delta;
    int pubc = 0;
    int subc = 0;

    init_all();

    while(scanf(" %[^\n]", line))
    {
        if (strncmp(line, "create topic ", 7) == 0)
        {
            int topic, size;
            sscanf(line, "%*s %*s %d %*s %d", &topic, &size);
            if (topic < 0 || topic >= MAXTOPICS)
            {
                printf("Invalid topic number %d\n", topic);
                continue;
            }
            tq_init(get_queue(topic), size);
        }
        else if (strncmp(line, "query topics", 12) == 0)
        {
 	    int i;
            for (i = 0; i < MAXTOPICS; i++)
            {
                if (get_queue(i)->size)
                    printf("topic %d %d\n", i, get_queue(i)->size);
            }
        }
        else if (strncmp(line, "add publisher", 13) == 0)
        {
            if (pubc >= NUMPROXIES)
            {
                printf("maximum number of publishers reached\n");
                continue;
            }
            publishers[pubc].status = 0;
            sscanf(line, "%*s %*s %s", tmp);
            publishers[pubc].filename = strdup(tmp);
            pubc++;
        }
        else if (strncmp(line, "add subscriber", 14) == 0)
        {
            if (subc >= NUMPROXIES)
            {
                printf("maximum number of subscribers reached\n");
                continue;
            }
            subscribers[subc].status = 0;
            sscanf(line, "%*s %*s %s", tmp);
            subscribers[subc].filename = strdup(tmp);
            subc++;
        }
        else if (strncmp(line, "query publishers", 16) == 0)
        {
	    int i;
            for (i = 0; i < NUMPROXIES; i++)
            {
                if (publishers[i].status == 0)
                    printf("publisher thread %d %s\n", i + 1, publishers[i].filename);
            }
        }
        else if (strncmp(line, "query subscriber", 16) == 0)
        {
	    int i;
            for (i = 0; i < NUMPROXIES; i++)
            {
                if (subscribers[i].status == 0)
                    printf("subscriber thread %d %s\n", i + 1, subscribers[i].filename);
            }
        }
        else if (strncmp(line, "delta ", 6) == 0)
        {
            sscanf(line, "%*s %d\n", &delta);
        }
        else if (strncmp(line, "start", 5) == 0)
        {
            run_proxy(delta);
            break;
        }
        else
        {
            printf("Invalid command `%s`\n", line);
        }
    }

    return 0;
}
