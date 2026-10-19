#include "Router.h"

// Constructor
Router::Router(std::string routerName)
{
    this->name = routerName;
    this->isOnline = true;
}

const std::string &Router::getName() const
{
    return this->name;
}

bool Router::getStatus() const
{
    return this->isOnline;
}

// Simulates a router crash
void Router::turnOff()
{
    this->isOnline = false;
}

// Brings the router back online
void Router::turnOn()
{
    this->isOnline = true;
}
