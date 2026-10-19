#include "Network.h"
#include <climits>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
int checks = 0;

void check(bool condition, const std::string &message)
{
    ++checks;
    if (!condition)
        throw std::runtime_error(message);
}

template <typename Exception, typename Action>
void rejects(Action action, const std::string &message)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    check(caught, message);
}

void checkRoute(const Network &network, int from, int to, long long cost,
                const std::vector<int> &path)
{
    const Route result = network.shortestPath(from, to);
    check(result.reachable && result.cost == cost && result.path == path,
          "Unexpected route " + std::to_string(from) + " -> " + std::to_string(to));
}
}

int main(int argc, char *argv[])
{
    try
    {
        const std::string root = argc > 1 ? argv[1] : ".";
        Network network;
        network.loadFromFile(root + "/data/topology.txt");
        checkRoute(network, 0, 2, 15, {0, 1, 2});
        checkRoute(network, 2, 0, 15, {2, 1, 0});
        checkRoute(network, 0, 0, 0, {0});
        auto forest = network.minimumSpanningForest();
        check(forest.cost == 15 && forest.links.size() == 2 && forest.components == 1,
              "Initial minimum spanning tree");

        network.setRouterStatus(1, false);
        checkRoute(network, 0, 2, 50, {0, 2});
        checkRoute(network, 2, 0, 50, {2, 0});
        check(!network.shortestPath(1, 2).reachable, "Offline source");
        check(!network.shortestPath(0, 1).reachable, "Offline destination");
        check(!network.shortestPath(1, 1).reachable, "Offline self route");
        forest = network.minimumSpanningForest();
        check(forest.cost == 50 && forest.links.size() == 1 && forest.components == 1,
              "Prim ignores offline routers");
        network.setRouterStatus(1, true);
        checkRoute(network, 0, 2, 15, {0, 1, 2});

        network.setLinkStatus(2, 1, false);
        checkRoute(network, 0, 2, 50, {0, 2});
        checkRoute(network, 2, 0, 50, {2, 0});
        network.setRouterStatus(1, false);
        network.setRouterStatus(1, true);
        checkRoute(network, 0, 2, 50, {0, 2}); // Power cycling cannot repair a cable.
        network.setLinkStatus(0, 2, false);
        check(!network.shortestPath(0, 2).reachable, "Isolated destination");
        check(!network.shortestPath(2, 0).reachable, "Isolation is symmetric");
        forest = network.minimumSpanningForest();
        check(forest.cost == 10 && forest.components == 2 && forest.links.size() == 1,
              "Disconnected network gives a forest");
        network.setLinkStatus(1, 2, true);
        checkRoute(network, 0, 2, 15, {0, 1, 2});

        rejects<std::out_of_range>([&] { network.shortestPath(-1, 0); }, "Negative index");
        rejects<std::out_of_range>([&] { network.setRouterStatus(3, false); }, "Large index");
        rejects<std::out_of_range>([&] { network.addLink(0, 3, 1); }, "Invalid link index");
        rejects<std::invalid_argument>([&] { network.addLink(0, 2, -1); }, "Negative cost");
        network.addRouter("Isolated");
        rejects<std::invalid_argument>([&] { network.setLinkStatus(0, 3, false); }, "Missing cable");
        check(!network.shortestPath(0, 3).reachable, "Isolated router");

        rejects<std::out_of_range>([&] {
            network.loadFromFile(root + "/tests/fixtures/invalid.txt");
        }, "Invalid endpoint in topology file");
        checkRoute(network, 0, 2, 15, {0, 1, 2});
        checkRoute(network, 3, 3, 0, {3});
        rejects<std::runtime_error>([&] {
            network.loadFromFile(root + "/tests/fixtures/empty.txt");
        }, "Empty topology");
        rejects<std::runtime_error>([&] {
            network.loadFromFile(root + "/tests/fixtures/nonexistent.txt");
        }, "Missing topology");

        Network zero;
        for (int i = 0; i < 4; ++i)
            zero.addRouter(std::to_string(i));
        zero.addLink(0, 1, 0);
        zero.addLink(1, 2, 0);
        zero.addLink(0, 2, 0);
        zero.addLink(2, 3, 2);
        zero.addLink(0, 3, 9);
        checkRoute(zero, 0, 3, 2, {0, 2, 3});
        forest = zero.minimumSpanningForest();
        check(forest.cost == 2 && forest.links.size() == 3, "Zero-cost cycle in Prim");

        Network large;
        for (int i = 0; i < 3; ++i)
            large.addRouter(std::to_string(i));
        large.addLink(0, 1, INT_MAX);
        large.addLink(1, 2, INT_MAX);
        checkRoute(large, 0, 2, 2LL * INT_MAX, {0, 1, 2});
        check(large.minimumSpanningForest().cost == 2LL * INT_MAX, "64-bit MST cost");
        for (int i = 0; i < 3; ++i)
            large.setRouterStatus(i, false);
        forest = large.minimumSpanningForest();
        check(forest.components == 0 && forest.links.empty(), "All routers offline");
        check(Network{}.minimumSpanningForest().components == 0, "Empty graph");
        std::cout << "Passed " << checks << " checks.\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAILED: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
