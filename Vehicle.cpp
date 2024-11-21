#include "Vehicle.h"
#include <random>

Vehicle::Vehicle(const std::string& plate, VehicleType vehicleType)
    : numberPlate(plate), type(vehicleType), challanStatus(ChallanStatus::INACTIVE) {
    setRandomSpeed();
}

void Vehicle::setRandomSpeed() {
    std::random_device rd;
    std::mt19937 gen(rd());
    int maxSpeed = (type == VehicleType::REGULAR) ? 60 : (type == VehicleType::HEAVY) ? 40 : 80;
    std::uniform_int_distribution<> dis(1, maxSpeed);
    speed = dis(gen);
}

void Vehicle::incrementSpeed() {
    speed += 5;
}

int Vehicle::getSpeed() const {
    return speed;
}

std::string Vehicle::getNumberPlate() const {
    return numberPlate;
}

VehicleType Vehicle::getType() const {
    return type;
}

ChallanStatus Vehicle::getChallanStatus() const {
    return challanStatus;
}

void Vehicle::activateChallan() {
    challanStatus = ChallanStatus::ACTIVE;
}