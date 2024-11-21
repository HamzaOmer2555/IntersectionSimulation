#include <iostream>
#include <thread>
#include <chrono>
#include "Intersection.h"


Intersection::Intersection() {
    trafficLights["NORTH"] = std::make_unique<TrafficLight>(10, 2, 10);
    trafficLights["SOUTH"] = std::make_unique<TrafficLight>(10, 2, 10);
    trafficLights["EAST"] = std::make_unique<TrafficLight>(10, 2, 10);
    trafficLights["WEST"] = std::make_unique<TrafficLight>(10, 2, 10);
}

void Intersection::addVehicle(const std::string& direction, Vehicle vehicle) {
    std::lock_guard<std::mutex> lock(mtx);
    if (lanes[direction].size() < 10) {
        lanes[direction].push(vehicle);
    }
}

void Intersection::manageTraffic() {
    for (auto& [direction, light] : trafficLights) {
        std::thread(&TrafficLight::start, light.get()).detach();
    }

    while (true) {
        for (auto& [direction, lane] : lanes) {
            std::lock_guard<std::mutex> lock(mtx);
            if (!lane.empty()) {
                Vehicle& frontVehicle = lane.front();
                if (trafficLights[direction]->getState() == LightState::GREEN) {
                    if (frontVehicle.getSpeed() > ((frontVehicle.getType() == VehicleType::REGULAR) ? 60 :
                                                   (frontVehicle.getType() == VehicleType::HEAVY) ? 40 : 80)) {
                        frontVehicle.activateChallan();
                        std::cout << "Challan issued to vehicle: " << frontVehicle.getNumberPlate() << "\n";
                    }
                    lane.pop();
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
