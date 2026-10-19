#ifndef NETWORK_H
#define NETWORK_H

#include "Router.h"
#include <string>
#include <vector>

struct Link
{
    int destination;
    int cost;
    bool isOnline=true;
};

struct Route
{
    bool reachable = false;
    long long cost = 0;
    std::vector<int> path;
};

struct TreeLink
{
    int source;
    int destination;
    int cost;
};

struct SpanningForest
{
    std::vector<TreeLink> links;
    long long cost = 0;
    int components = 0;
};

class Network
{
private:
    std::vector<Router> routers;
    std::vector<std::vector<Link>> adjList;

public:
    void addRouter(std::string name);
    void addLink(int source, int destination, int cost);
    void printNetwork();
    Route shortestPath(int source, int destination) const;
    void setRouterStatus(int id, bool online);
    void setLinkStatus(int source, int destination, bool online);
    void loadFromFile(const std::string &filename);
    SpanningForest minimumSpanningForest() const;
};

#endif