import random
import threading
import time


class Ship:
    def __init__(self, id, weight, wait, sleep):
        self.id = id
        self.weight = weight
        self.wait = wait
        self.sleep = sleep


ship_list = []
ship_threads = []
dock_lock = threading.Lock()
dock_cond = threading.Condition(dock_lock)

dock_semaphore = 0
tugboat_semaphore = 0


def ship_action(ship):
    time.sleep(ship.wait)

    with dock_cond:
        print(f"Ship {ship.id} waiting")
        dock_cond.wait_for(lambda: dock_semaphore > 0 and tugboat_semaphore >= ship.weight)

        global dock_semaphore, tugboat_semaphore
        dock_semaphore -= 1
        tugboat_semaphore -= ship.weight
        print(
            f"Ship {ship.id} enters port, {dock_semaphore} docks and {tugboat_semaphore} tugboats left, staying for {ship.sleep} seconds")

    time.sleep(ship.sleep)

    with dock_cond:
        dock_semaphore += 1
        tugboat_semaphore += ship.weight
        print(f"Ship {ship.id} leaves port, {dock_semaphore} docks and {tugboat_semaphore} tugboats left")

        dock_cond.notify_all()


def main():
    global dock_semaphore, tugboat_semaphore

    dock_semaphore = int(input("How many docks does the port have? > "))
    tugboat_semaphore = int(input("How many tugboats are available? > "))
    ships = int(input("How many ships want to enter a port? > "))
    max_wait = int(input("How long a ship waits to enter the port? > "))
    max_sleep = int(input("How long is the maximum time in port? > "))

    for i in range(1, ships + 1):
        ship = Ship(id=i, weight=random.randint(1, tugboat_semaphore // 2), wait=random.randint(0, max_wait),
                    sleep=random.randint(0, max_sleep))
        print(f"Ship {ship.id}, weight {ship.weight}, wait {ship.wait}, sleep {ship.sleep}")
        ship_list.append(ship)
    print()

    for ship in ship_list:
        thread = threading.Thread(target=ship_action, args=(ship,))
        ship_threads.append(thread)
        thread.start()

    for thread in ship_threads:
        thread.join()


if __name__ == "__main__":
    main()
