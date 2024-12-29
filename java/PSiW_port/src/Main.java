import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.Scanner;

class Ship {
    int id;
    int weight;
    int wait;
    int sleep;

    public Ship(int id, int weight, int wait, int sleep) {
        this.id = id;
        this.weight = weight;
        this.wait = wait;
        this.sleep = sleep;
    }
}

public class Main {

    private static final List<Ship> shipList = new ArrayList<>();
    private static final List<Thread> shipThreads = new ArrayList<>();
    private static int dockSemaphore;
    private static int tugboatSemaphore;

    private static final Object dockLock = new Object();

    public static void main(String[] args) {
        var scanner = new Scanner(System.in);
        var random = new Random();

        System.out.print("How many docks does the port have? > ");
        dockSemaphore = scanner.nextInt();
        scanner.nextLine();

        System.out.print("How many tugboats are available? > ");
        tugboatSemaphore = scanner.nextInt();
        scanner.nextLine();

        System.out.print("How many ships want to enter a port? > ");
        int ships = scanner.nextInt();
        scanner.nextLine();

        System.out.print("How long a ship waits to enter the port? > ");
        int maxWait = scanner.nextInt();
        scanner.nextLine();

        System.out.print("How long is the maximum time in port? > ");
        int maxSleep = scanner.nextInt();
        scanner.nextLine();

        for (int i = 1; i <= ships; i++) {
            int weight = random.nextInt(tugboatSemaphore / 2) + 1;
            int wait = random.nextInt(maxWait + 1);
            int sleep = random.nextInt(maxSleep + 1);

            var ship = new Ship(i, weight, wait, sleep);
            System.out.printf("Ship %d, weight %d, wait %d, sleep %d%n", ship.id, ship.weight, ship.wait, ship.sleep);
            shipList.add(ship);
        }
        System.out.println();

        for (var ship : shipList) {
            var thread = new Thread(() -> shipAction(ship));
            shipThreads.add(thread);
            thread.start();
        }

        for (var thread : shipThreads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }
    }

    private static void shipAction(Ship ship) {
        try {
            Thread.sleep(Duration.ofSeconds(ship.wait));
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }

        synchronized (dockLock) {
            while (dockSemaphore <= 0 || tugboatSemaphore < ship.weight) {
                try {
                    System.out.printf("Ship %d waiting%n", ship.id);
                    dockLock.wait();
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }
            dockSemaphore--;
            tugboatSemaphore -= ship.weight;
            System.out.printf("Ship %d enters port, %d docks and %d tugboats left, staying for %d seconds%n",
                    ship.id, dockSemaphore, tugboatSemaphore, ship.sleep);
        }

        try {
            Thread.sleep(Duration.ofSeconds(ship.sleep));
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }

        synchronized (dockLock) {
            dockSemaphore++;
            tugboatSemaphore += ship.weight;
            System.out.printf("Ship %d leaves port, %d docks and %d tugboats left%n", ship.id, dockSemaphore, tugboatSemaphore);
            dockLock.notifyAll();
        }
    }

}