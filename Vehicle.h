#pragma once

#include <string>

enum class VehicleType { REGULAR, HEAVY, EMERGENCY };
enum class ChallanStatus { INACTIVE, ACTIVE };

class Vehicle {
private:
    std::string numberPlate;
    VehicleType type;
    int speed;
    ChallanStatus challanStatus;

public:
    Vehicle(const std::string& plate, VehicleType vehicleType);

    void setRandomSpeed();
    void incrementSpeed();
    int getSpeed() const;
    std::string getNumberPlate() const;
    VehicleType getType() const;
    ChallanStatus getChallanStatus() const;
    void activateChallan();
};
