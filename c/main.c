#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef struct Ship {
    int id;
    int weight;
    int wait;
    int sleep;
} Ship;

pthread_mutex_t dock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t dock_cond = PTHREAD_COND_INITIALIZER;

int ships;
int tugboat_semaphore, dock_semaphore;

int random_int(int low, int high);

void *ship_action(void *ptr);

int main(void) {
    srandom(time(NULL));

    int max_sleep, max_wait;

    printf("How many docks does the port have? > ");
    scanf("%d", &dock_semaphore);

    printf("How many tugboats are available? > ");
    scanf("%d", &tugboat_semaphore);

    printf("How many ships want to enter a port? > ");
    scanf("%d", &ships);

    printf("How long a ship waits to enter the port? > ");
    scanf("%d", &max_wait);

    printf("How long is the maximum time in port? > ");
    scanf("%d", &max_sleep);

    pthread_t *ship_threads = malloc(ships * sizeof(pthread_t));

    for (int i = 0; i < ships; i++) {
        Ship *ship = malloc(sizeof(Ship));
        ship->id = i + 1;
        ship->weight = random_int(1, tugboat_semaphore / 2);
        ship->wait = random_int(0, max_wait);
        ship->sleep = random_int(0, max_sleep);
        printf("Ship %d, weight %d, wait %d, sleep %d\n",
               ship->id, ship->weight, ship->wait, ship->sleep);

        pthread_create(&ship_threads[i], NULL, ship_action, ship);
    }

    for (int i = 0; i < ships; i++) {
        pthread_join(ship_threads[i], NULL);
    }

    free(ship_threads);
    pthread_mutex_destroy(&dock_mutex);
    pthread_cond_destroy(&dock_cond);

    return 0;
}

int random_int(const int low, const int high) {
    return (int) random() % (high - low + 1) - low;
}

void *ship_action(void *ptr) {
    Ship *ship = ptr;

    sleep(ship->wait);

    pthread_mutex_lock(&dock_mutex);

    while (dock_semaphore <= 0 || tugboat_semaphore < ship->weight) {
        printf("Ship %d waiting\n", ship->id);
        pthread_cond_wait(&dock_cond, &dock_mutex);
    }

    dock_semaphore--;
    tugboat_semaphore -= ship->weight;
    printf("Ship %d enters port, %d docks and %d tugboats left, staying for %d seconds\n",
           ship->id, dock_semaphore, tugboat_semaphore, ship->sleep);
    pthread_mutex_unlock(&dock_mutex);
    sleep(ship->sleep);

    pthread_mutex_lock(&dock_mutex);
    dock_semaphore++;
    tugboat_semaphore += ship->weight;
    printf("Ship %d leaves port, %d docks and %d tugboats left\n",
           ship->id, dock_semaphore, tugboat_semaphore);
    pthread_cond_broadcast(&dock_cond);
    pthread_mutex_unlock(&dock_mutex);

    free(ship);
    return NULL;
}
