#pragma once

#include "TrafficLight.h"
#include "Vehicle.h"
#include <unordered_map>
#include <queue>
#include <memory>
#include <mutex>

class Intersection {
private:
    std::unordered_map<std::string, std::unique_ptr<TrafficLight>> trafficLights;
    std::unordered_map<std::string, std::queue<Vehicle>> lanes;
    std::mutex mtx;

public:
    Intersection();
    void addVehicle(const std::string& direction, Vehicle vehicle);
    void manageTraffic();
};


