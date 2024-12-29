namespace PSiW_port;

internal class Ship
{
    public int Id { get; init; }
    public int Weight { get; init; }
    public int Wait { get; init; }
    public int Sleep { get; init; }
}

class Program
{
    private static readonly List<Ship> Ships = [];
    private static readonly List<Task> ShipTasks = [];
    private static int _dockSemaphore;
    private static int _tugboatSemaphore;

    private static readonly object DockLock = new();

    private static readonly Random Random = new();

    private static void Main()
    {
        Console.Write("How many docks does the port have? > ");
        _dockSemaphore = int.Parse(Console.ReadLine());

        Console.Write("How many tugboats are available? > ");
        _tugboatSemaphore = int.Parse(Console.ReadLine());

        Console.Write("How many ships want to enter a port? > ");
        int shipCount = int.Parse(Console.ReadLine());

        Console.Write("How long a ship waits to enter the port? > ");
        int maxWait = int.Parse(Console.ReadLine());

        Console.Write("How long is the maximum time in port? > ");
        int maxSleep = int.Parse(Console.ReadLine());

        for (var i = 1; i <= shipCount; i++)
        {
            var ship = new Ship
            {
                Id = i,
                Weight = Random.Next(1, _tugboatSemaphore / 2 + 1),
                Wait = Random.Next(0, maxWait + 1),
                Sleep = Random.Next(0, maxSleep + 1)
            };
            Console.WriteLine($"Ship {ship.Id}, weight {ship.Weight}, wait {ship.Wait}, sleep {ship.Sleep}");
            Ships.Add(ship);
        }

        Console.WriteLine();

        foreach (var ship in Ships)
        {
            ShipTasks.Add(Task.Run(() => ShipAction(ship)));
        }

        Task.WaitAll(ShipTasks.ToArray());
    }

    private static void ShipAction(Ship ship)
    {
        Thread.Sleep(TimeSpan.FromSeconds(ship.Wait));

        lock (DockLock)
        {
            while (_dockSemaphore <= 0 || _tugboatSemaphore < ship.Weight)
            {
                Console.WriteLine($"Ship {ship.Id} waiting");
                Monitor.Wait(DockLock);
            }

            _dockSemaphore--;
            _tugboatSemaphore -= ship.Weight;
            Console.WriteLine(
                $"Ship {ship.Id} enters port, {_dockSemaphore} docks and {_tugboatSemaphore} tugboats left, staying for {ship.Sleep} seconds");
        }

        Thread.Sleep(TimeSpan.FromSeconds(ship.Sleep));

        lock (DockLock)
        {
            _dockSemaphore++;
            _tugboatSemaphore += ship.Weight;
            Console.WriteLine(
                $"Ship {ship.Id} leaves port, {_dockSemaphore} docks and {_tugboatSemaphore} tugboats left");
            Monitor.PulseAll(DockLock);
        }
    }
}