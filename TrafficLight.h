#pragma once


#include <mutex>
#include <condition_variable>

enum class LightState { GREEN, YELLOW, RED };

class TrafficLight {
private:
    LightState state;
    int greenDuration;
    int yellowDuration;
    int redDuration;
    std::mutex mtx;
    std::condition_variable cv;

public:
    TrafficLight(int green, int yellow, int red);
    void start();
    void setState(LightState newState, int duration);
    LightState getState() const;
};


