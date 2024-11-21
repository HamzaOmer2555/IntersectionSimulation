#include "TrafficLight.h"
#include <thread>
#include <chrono>

TrafficLight::TrafficLight(int green, int yellow, int red)
    : state(LightState::RED), greenDuration(green), yellowDuration(yellow), redDuration(red) {}

void TrafficLight::start() {
    while (true) {
        switch (state) {
            case LightState::RED:
                setState(LightState::GREEN, greenDuration);
                break;
            case LightState::GREEN:
                setState(LightState::YELLOW, yellowDuration);
                break;
            case LightState::YELLOW:
                setState(LightState::RED, redDuration);
                break;
        }
    }
}

void TrafficLight::setState(LightState newState, int duration) {
    std::unique_lock<std::mutex> lock(mtx);
    state = newState;
    cv.notify_all();
    std::this_thread::sleep_for(std::chrono::seconds(duration));
}

LightState TrafficLight::getState() const {
    return state;
}


