#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

struct Ship {
    int id;
    int weight;
    int wait;
    int sleep;
};

std::vector<std::unique_ptr<Ship> > ships_vec;
std::vector<std::thread> ship_threads;
std::mutex dock_mutex;
std::condition_variable dock_cond;

int ships;
int tugboat_semaphore, dock_semaphore;
int ship_weight, ship_sleep;

std::random_device rd;
std::mt19937 gen(rd());

void ship_action(const Ship *ship);

int random(const int low, const int high) {
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}

int main() {
    int max_sleep, max_wait;

    std::cout << "How many docks does the port have? > ";
    std::cin >> dock_semaphore;

    std::cout << "How many tugboats are available? > ";
    std::cin >> tugboat_semaphore;

    std::cout << "How many ships want to enter a port? > ";
    std::cin >> ships;
    ship_threads.reserve(ships);

    std::cout << "How long a ship waits to enter the port? > ";
    std::cin >> max_wait;

    std::cout << "How long is the maximum time in port? > ";
    std::cin >> max_sleep;

    for (int i = 1; i <= ships; i++) {
        auto ship = std::make_unique<Ship>(i, random(1, tugboat_semaphore / 2), random(0, max_wait),
                                           random(0, max_sleep));
        std::printf("Ship %d, weight %d, wait %d, sleep %d\n", ship->id, ship->weight, ship->wait, ship->sleep);
        ships_vec.push_back(std::move(ship));
    }

    std::cout << '\n';

    for (auto &ship: ships_vec) {
        // pthread_t new_thread;
        // pthread_create(&new_thread, nullptr, ship_action, (void *) ship);
        ship_threads.emplace_back(ship_action, ship.get());
    }

    for (auto &ship: ship_threads) {
        ship.join();
    }
}

void ship_action(const Ship *ship) {
    sleep(ship->wait); {
        std::unique_lock lock(dock_mutex);
        dock_cond.wait(lock, [ship] {
            std::printf("Ship %d waiting\n", ship->id);
            return dock_semaphore > 0 && tugboat_semaphore >= ship->weight;
        });
        dock_semaphore--;
        tugboat_semaphore -= ship->weight;
        std::printf("Ship %d enters port, %d docks and %d tugboats left, staying for %d seconds\n", ship->id,
                    dock_semaphore, tugboat_semaphore, ship->sleep);
    }

    sleep(ship->sleep); {
        std::unique_lock lock(dock_mutex);
        dock_semaphore++;
        tugboat_semaphore += ship->weight;
        std::printf("Ship %d leaves port, %d docks and %d tugboats left\n", ship->id, dock_semaphore,
                    tugboat_semaphore);
    }

    dock_cond.notify_all();
}
