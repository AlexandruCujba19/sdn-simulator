    #ifndef ROUTER_H
#define ROUTER_H

#include <string>

class Router
{
private:
    std::string name;
    bool isOnline; // Router power/status; cable failures are tracked separately.

public:
    // Constructor
    Router(std::string routerName);

    // Getters
    const std::string &getName() const;
    bool getStatus() const;

    // Setters (Actions)
    void turnOff();
    void turnOn();
};

#endif

