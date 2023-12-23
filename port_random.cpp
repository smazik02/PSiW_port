#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <vector>

struct Ship {
    unsigned int id;
    unsigned int weight;
    unsigned int sleep;
};

std::vector<Ship *> ships_vec;
std::vector<pthread_t> ship_threads;
pthread_mutex_t dock_mutex;
pthread_cond_t dock_cond;
// pthread_mutex_t tugboat_mutex;
// pthread_cond_t tugboat_cond;

int docks, tugboats, ships;
int tugboat_semaphore, dock_semaphore;
int ship_weight, ship_sleep;

void *ship_action(void *i);

int main() {
    srand((unsigned)time(NULL));
    dock_mutex = PTHREAD_MUTEX_INITIALIZER;
    dock_cond = PTHREAD_COND_INITIALIZER;

    std::cout << "How many docks does the port have? >";
    std::cin >> docks;
    dock_semaphore = docks;

    std::cout << "How many tugboats are available? >";
    std::cin >> tugboats;
    tugboat_semaphore = tugboats;

    std::cout << "How many ships want to enter a port? >";
    std::cin >> ships;
    ship_threads.reserve(ships);

    for (int i = 0; i < ships; i++) {
        Ship *ship = (Ship *)malloc(sizeof(Ship));
        ship->id = i;
        ship->weight = 1 + rand() % (tugboats / 2);
        ship->sleep = 1 + rand() % 10;
        printf("Ship %d, weight %d, sleep %d\n", ship->id, ship->weight, ship->weight);
        ships_vec.push_back(ship);
    }

    printf("\n");

    for (auto ship = ships_vec.begin(); ship != ships_vec.end(); ship++) {
        pthread_t new_thread;
        pthread_create(&new_thread, nullptr, ship_action, (void *)*ship);
        ship_threads.push_back(new_thread);
    }

    for (auto ship = ship_threads.begin(); ship != ship_threads.end(); ship++) {
        pthread_join(*ship, nullptr);
    }
}

void *ship_action(void *ptr) {
    Ship *ship = (Ship *)ptr;
    printf("Ship %d\n", ship->id);

    pthread_mutex_lock(&dock_mutex);
    while (dock_semaphore <= 0 || tugboat_semaphore < ship->weight) {
        printf("Ship %d waiting\n", ship->id);
        pthread_cond_wait(&dock_cond, &dock_mutex);
    }
    --dock_semaphore;
    tugboat_semaphore -= ship->weight;
    printf("Ship %d enters port, %d docks and %d tugboats left, staying for %d seconds\n", ship->id, dock_semaphore, tugboat_semaphore, ship->sleep);
    pthread_mutex_unlock(&dock_mutex);
    sleep(ship->sleep);

    pthread_mutex_lock(&dock_mutex);
    ++dock_semaphore;
    tugboat_semaphore += ship->weight;
    printf("Ship %d leaves port, %d docks and %d tugboats left\n", ship->id, dock_semaphore, tugboat_semaphore);
    pthread_mutex_unlock(&dock_mutex);
    pthread_cond_broadcast(&dock_cond);
    pthread_exit(nullptr);
}