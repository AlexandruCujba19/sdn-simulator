#include "Network.h"
#include <stdexcept>
#include <iostream>
#include <limits>
#include <queue>
#include <functional>
#include <utility>
#include <algorithm>
#include <fstream>
#include <tuple>

void Network::addRouter(std::string name)
{
    Router newRouter(name);
    routers.push_back(newRouter);
    adjList.push_back(std::vector<Link>{});
}

void Network::addLink(int source, int destination, int cost)
{
    int routerCount = static_cast<int>(routers.size());
    if (source < 0 || source >= routerCount || destination < 0 || destination >= routerCount)
    {
        throw std::out_of_range("Router index is out of range.");
    }
    if (cost < 0)
    {
        
        throw std::invalid_argument("Link cost can not be negative.");
    }
    Link forwardLink = {destination, cost};
    adjList[source].push_back(forwardLink);
    Link reverseLink = {source, cost};
    adjList[destination].push_back(reverseLink);
}

void Network::printNetwork()
{
    for (std::size_t i = 0; i < routers.size(); ++i)
    {
        std::string routerStatus = "OFFLINE";

        if (routers[i].getStatus())
        {
            routerStatus = "ONLINE";
        }

        std::cout << "Router " << i
                  << " (" << routers[i].getName()
                  << ") [" << routerStatus << "]:\n";

        for (const Link &link : adjList[i])
        {
            std::string linkStatus = "OFFLINE";

            if (link.isOnline)
            {
                linkStatus = "ONLINE";
            }

            std::cout << "  -> "
                      << routers[link.destination].getName()
                      << " (cost: " << link.cost
                      << ", cable: " << linkStatus << ")\n";
        }

        std::cout << '\n';
    }
}

Route Network::shortestPath(int source, int destination) const
{
    int routerCount = static_cast<int>(routers.size());

    
    if (source < 0 || source >= routerCount ||
        destination < 0 || destination >= routerCount)
    {
        throw std::out_of_range("Router index is out of range.");
    }

    
    if (!routers[source].getStatus() ||
        !routers[destination].getStatus())
    {
        return Route{};
    }

    const long long INF = std::numeric_limits<long long>::max();

    std::vector<long long> distances(routerCount, INF);
    std::vector<int> previous(routerCount, -1);

    distances[source] = 0;

    using QueueEntry = std::pair<long long, int>;

    std::priority_queue<
        QueueEntry,
        std::vector<QueueEntry>,
        std::greater<QueueEntry>>
        pending;

    pending.push({0, source});

    while (!pending.empty())
    {
        QueueEntry current = pending.top();
        pending.pop();

        long long currentCost = current.first;
        int currentRouter = current.second;

        
        if (currentCost != distances[currentRouter])
        {
            continue;
        }

        for (const Link &link : adjList[currentRouter])
        {
            
            if (!link.isOnline)
            {
                continue;
            }

            if (!routers[link.destination].getStatus())
            {
                continue;
            }

            long long newCost = currentCost + link.cost;

            if (newCost < distances[link.destination])
            {
                distances[link.destination] = newCost;
                previous[link.destination] = currentRouter;

                pending.push({newCost, link.destination});
            }
        }
    }

    Route result{};

    if (distances[destination] == INF)
    {
        return result;
    }

    result.reachable = true;
    result.cost = distances[destination];

    int currentRouter = destination;

    while (currentRouter != -1)
    {
        result.path.push_back(currentRouter);
        currentRouter = previous[currentRouter];
    }

    std::reverse(result.path.begin(), result.path.end());

    return result;
}
void Network::setRouterStatus(int id, bool online)
{
    int routerCount = static_cast<int>(routers.size());

    if (id < 0 || id >= routerCount)
    {
        throw std::out_of_range("Router index is out of range.");
    }

    if (online)
    {
        routers[id].turnOn();
    }
    else
    {
        routers[id].turnOff();
    }
}
void Network::setLinkStatus(int source, int destination, bool online)
{
    int routerCount = static_cast<int>(routers.size());

    if (source < 0 || source >= routerCount ||
        destination < 0 || destination >= routerCount)
    {
        throw std::out_of_range("Router index is out of range.");
    }

    bool found = false;

    
    for (Link &link : adjList[source])
    {
        if (link.destination == destination)
        {
            link.isOnline = online;
            found = true;
        }
    }

    if (!found)
    {
        throw std::invalid_argument("Link does not exist.");
    }

    
    for (Link &link : adjList[destination])
    {
        if (link.destination == source)
        {
            link.isOnline = online;
        }
    }
}
void Network::loadFromFile(const std::string &filename)
{
    std::ifstream input(filename);

    if (!input.is_open())
    {
        throw std::runtime_error("Could not open network file.");
    }

    int routerCount = 0;

    if (!(input >> routerCount) || routerCount <= 0)
    {
        throw std::runtime_error("Invalid router count.");
    }

    Network loadedNetwork;

    for (int i = 0; i < routerCount; ++i)
    {
        std::string name;

        if (!(input >> name))
        {
            throw std::runtime_error("Could not read router name.");
        }

        loadedNetwork.addRouter(name);
    }

    int linkCount = 0;

    if (!(input >> linkCount) || linkCount < 0)
    {
        throw std::runtime_error("Invalid link count.");
    }

    for (int i = 0; i < linkCount; ++i)
    {
        int source = 0;
        int destination = 0;
        int cost = 0;

        if (!(input >> source >> destination >> cost))
        {
            throw std::runtime_error("Could not read link data.");
        }

        loadedNetwork.addLink(source, destination, cost);
    }

    routers.swap(loadedNetwork.routers);
    adjList.swap(loadedNetwork.adjList);
}

SpanningForest Network::minimumSpanningForest() const
{
    SpanningForest result{};

    int routerCount = static_cast<int>(routers.size());
    std::vector<bool> included(routerCount, false);

    using Candidate = std::tuple<int, int, int>;

    for (int start = 0; start < routerCount; ++start)
    {
        if (!routers[start].getStatus() || included[start])
        {
            continue;
        }

        result.components++;

        std::priority_queue<
            Candidate,
            std::vector<Candidate>,
            std::greater<Candidate>
        > pending;

        pending.push({0, -1, start});

        while (!pending.empty())
        {
            Candidate current = pending.top();
            pending.pop();

            int cost = std::get<0>(current);
            int source = std::get<1>(current);
            int destination = std::get<2>(current);

            if (included[destination])
            {
                continue;
            }

            included[destination] = true;

            if (source != -1)
            {
                result.links.push_back({source, destination, cost});
                result.cost += cost;
            }

            for (const Link &link : adjList[destination])
            {
                if (!link.isOnline ||
                    !routers[link.destination].getStatus() ||
                    included[link.destination])
                {
                    continue;
                }

                pending.push({
                    link.cost,
                    destination,
                    link.destination
                });
            }
        }
    }
      return result;
}