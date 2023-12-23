#include <pthread.h>
#include <stdlib.h>

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <vector>

struct Ship {
    int id;
    int weight;
    int sleep;
};

std::vector<Ship> ships_vec;
std::vector<pthread_t> ship_threads;
pthread_mutex_t dock_mutex;
pthread_cond_t dock_cond;
pthread_mutex_t tugboat_mutex;
pthread_cond_t tugboat_cond;

int docks, tugboats, ships;
int tugboat_semaphore;
int ship_weight, ship_sleep;

void *ship_action(void *i);

int main() {
    std::cout << "How many docks does the port have? >";
    std::cin >> docks;

    std::cout << "How many tugboats are available? >";
    std::cin >> tugboats;

    std::cout << "How many ships want to enter a port? >";
    std::cin >> ships;
    ship_threads.reserve(ships);

    for (int i = 0; i < ships; i++) {
        std::cout << "\nShip " << i << "\n";
        std::cout << "Enter the ship's weight >";
        std::cin >> ship_weight;
        std::cout << "Enter how long the ship stays in the dock >";
        std::cin >> ship_sleep;

        // Ship *ship = (Ship *)malloc(sizeof(Ship));
        Ship ship;
        ship.id = i;
        ship.weight = ship_weight;
        ship.sleep = ship_sleep;
        ships_vec.push_back(ship);
    }

    for (auto ship = ships_vec.begin(); ship != ships_vec.end(); ship++) {
        pthread_t new_thread;
        Ship *ship2 = (Ship *)(&(*ship));
        pthread_create(&new_thread, nullptr, ship_action, (void *)ship2);
        ship_threads.push_back(new_thread);
    }

    // for (int i = 0; i < ships; i++) {
    //     pthread_t new_thread;
    //     pthread_create(&new_thread, nullptr, ship_action, (void *) *ships_vec[i]);
    //     ship_threads.push_back(new_thread);
    // }

    for (auto ship = ship_threads.begin(); ship != ship_threads.end(); ship++) {
        pthread_join(*ship, nullptr);
    }

    // for (int i = 0; i < ships; i++) {
    //     pthread_join(ship_threads.at(i), nullptr);
    // }
}

void *ship_action(void *ptr) {
    Ship *ship = (Ship *)ptr;
    std::cout << "Id " << ship->id << ", weight: " << ship->weight << "\n";
    pthread_exit(nullptr);
}