#include <iostream>
#include <exception>
#include "Network.h"
#include <limits>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <string>
bool recoverInput()
{
    if (std::cin.eof() || std::cin.bad())
    {
        return false;
    }

    std::cin.clear();

    std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(),
        '\n');

    return true;
}

void printSpanningForest(const SpanningForest &forest)
{
    std::cout << "\n--- Minimum spanning forest ---\n";

    for (const TreeLink &link : forest.links)
    {
        std::cout << link.source << " - " << link.destination
                  << " (cost: " << link.cost << ")\n";
    }

    std::cout << "Total cost: " << forest.cost << '\n';
    std::cout << "Components: " << forest.components << '\n';

    if (forest.components == 0)
    {
        std::cout << "No routers are online.\n";
    }
    else if (forest.components > 1)
    {
        std::cout << "The active network is disconnected.\n";
    }
}

void printRoute(const Route &route)
{
    if (!route.reachable)
    {
        std::cout << "No route available.\n";
        return;
    }

    std::cout << "Minimum cost: " << route.cost << '\n';
    std::cout << "Path: ";

    for (std::size_t i = 0; i < route.path.size(); ++i)
    {
        if (i > 0)
        {
            std::cout << " -> ";
        }

        std::cout << route.path[i];
    }

    std::cout << '\n';
}
void logEvent(const std::string &message)
{
    std::ofstream output("events.log", std::ios::app);

    if (!output.is_open())
    {
        std::cerr << "Warning: could not open events.log.\n";
        return;
    }

    std::time_t now = std::time(nullptr);
    std::tm *localTime = std::localtime(&now);

    if (localTime == nullptr)
    {
        std::cerr << "Warning: could not obtain local time.\n";
        return;
    }

    output << '['
           << std::put_time(localTime, "%Y-%m-%d %H:%M:%S")
           << "] " << message << '\n';

    output.flush();

    if (!output)
    {
        std::cerr << "Warning: could not write to events.log.\n";
    }
}
int main()
{
    try
    {
        Network network;
        network.loadFromFile("network.txt");
        logEvent("Network loaded from network.txt.");

        while (true)
        {
            std::cout << "\n--- Network Simulator ---\n";
            std::cout << "1. Display the network\n";
            std::cout << "2. Calculate a route\n";
            std::cout << "3. Change router status\n";
            std::cout << "4. Change cable status\n";
            std::cout << "5. Calculate the minimum spanning forest\n";
            std::cout << "0. Exit\n";
            std::cout << "Choose an option: ";

            int option = 0;

            if (!(std::cin >> option))
            {
                if (!recoverInput())
                {
                    return 0;
                }

                std::cout << "The option must be an integer.\n";
                continue;
            }

            if (option == 0)
            {
                break;
            }

            try
            {
                if (option == 1)
                {
                    network.printNetwork();
                }
                else if (option == 2)
                {
                    int source = 0;
                    int destination = 0;

                    std::cout << "Enter the source and destination router indices: ";

                    if (!(std::cin >> source >> destination))
                    {
                        if (!recoverInput())
                        {
                            return 0;
                        }

                        std::cout << "Enter two integers for the router indices.\n";
                        continue;
                    }

                    Route route = network.shortestPath(source, destination);
                    printRoute(route);
                }
                else if (option == 3)
                {
                    int id = 0;
                    int status = 0;

                    std::cout << "Enter the router index and status (0/1): ";

                    if (!(std::cin >> id >> status))
                    {
                        if (!recoverInput())
                        {
                            return 0;
                        }

                        std::cout << "Enter two integers.\n";
                        continue;
                    }

                    if (status != 0 && status != 1)
                    {
                        std::cout << "The status must be 0 or 1.\n";
                    }
                    else
                    {
                        network.setRouterStatus(id, status == 1);
                        std::string statusText = "OFFLINE";

                        if (status == 1)
                        {
                            statusText = "ONLINE";
                        }

                        logEvent("Router " + std::to_string(id) + " set " + statusText);
                        std::cout << "Router status has been updated.\n";
                    }
                }
                else if (option == 4)
                {
                    int source = 0;
                    int destination = 0;
                    int status = 0;

                    std::cout << "Enter the cable endpoint indices and status (0/1): ";

                    if (!(std::cin >> source >> destination >> status))
                    {
                        if (!recoverInput())
                        {
                            return 0;
                        }

                        std::cout << "Enter three integers.\n";
                        continue;
                    }

                    if (status != 0 && status != 1)
                    {
                        std::cout << "Status must be 0 or 1.\n";
                    }
                    else
                    {
                        network.setLinkStatus(source, destination, status == 1);

                        std::string statusText = "OFFLINE";

                        if (status == 1)
                        {
                            statusText = "ONLINE";
                        }

                        logEvent(
                            "Link " + std::to_string(source) + "-" + std::to_string(destination) + " set " + statusText);

                        std::cout << "Cable status has been updated.\n";
                    }
                }
                else if (option == 5)
                {
                    SpanningForest forest = network.minimumSpanningForest();
                    printSpanningForest(forest);
                }

                else
                {
                    std::cout << "Invalid option.\n";
                }
            }
            catch (const std::exception &error)
            {
                std::cerr << "Error: " << error.what() << '\n';
            }
        }
        logEvent("Simulator closed.");
        std::cout << "Simulator closed.\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}