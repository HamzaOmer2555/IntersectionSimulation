#include "Intersection.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include <random>

const sf::Vector2f NORTH_SPAWN_REGULAR_LANE1(522, 0);  // Starting from top-center
const sf::Vector2f SOUTH_SPAWN_REGULAR_LANE1(403, 1000); // Starting from bottom-center
const sf::Vector2f EAST_SPAWN_REGULAR_LANE1(1000, 533);  // Starting from right-center
const sf::Vector2f WEST_SPAWN_REGULAR_LANE1(0, 410);    // Starting from left-center


const sf::Vector2f NORTH_SPAWN_HEAVY_LANE1(572, 0);  // Starting from top-center
const sf::Vector2f SOUTH_SPAWN_HEAVY_LANE1(453, 1000); // Starting from bottom-center
const sf::Vector2f EAST_SPAWN_HEAVY_LANE1(1000, 580);  // Starting from right-center
const sf::Vector2f WEST_SPAWN_HEAVY_LANE1(0, 460);    // Starting from left-center

// IMPORTANT NOTES:
// I have used the scale of 1s in real life = 3s in my simulation for the spawning cars. As the sprites overlap if a wait of 1s is given


struct VehicleSprite {
    sf::Sprite sprite;
    std::string direction;
    float speed; // Pixels per second
};

int main() {
    // Initialize intersection and SFML window
    Intersection intersection;
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Smart Traffic Simulation");

    // Load the background texture
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("img/Intersection5.png")) {
        std::cerr << "Error: Could not load img/Intersection.png" << std::endl;
        return -1;
    }
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);
    //backgroundSprite.setScale(1.25f, 1.25f);

    // Load vehicle texture
    // Load textures for regular, emergency, and heavy vehicles
    sf::Texture regularCarTexture, emergencyCarTexture, heavyCarTexture;
    if (!regularCarTexture.loadFromFile("img/RegularCar2.png") ||
        !emergencyCarTexture.loadFromFile("img/EmergencyCar.png") ||
        !heavyCarTexture.loadFromFile("img/HeavyCar.png")) {
        std::cerr << "Error: Could not load vehicle textures" << std::endl;
        return -1;
    }
    

    // List to hold active vehicle sprites
    std::vector<VehicleSprite> vehicles;

    // Sprites for the traffic lights
    sf::Texture redLightTex, greenLightTex, YellowLightTex;
    if (!redLightTex.loadFromFile("img/RedLight.png") ||
        !YellowLightTex.loadFromFile("img/YellowLight.png") ||
        !greenLightTex.loadFromFile("img/GreenLight.png")) {
        std::cerr << "Error: Could not load traffic textures" << std::endl;
        return -1;
    }
    sf::Sprite north_light, south_light, east_light, west_light;
    north_light.setTexture(redLightTex);
    north_light.setPosition(470, 420);
    
    south_light.setTexture(redLightTex);
    south_light.setPosition(470, 530);
    
    east_light.setTexture(redLightTex);
    east_light.setPosition(530, 480);

    west_light.setTexture(redLightTex);
    west_light.setPosition(420, 480);





    // Clock for simulation timing for each direction
    sf::Clock northClock, southClock, eastClock, westClock, heavyCarClock;

    // Separate timers for emergency cars
    sf::Clock northEmergencyClock, southEmergencyClock, eastEmergencyClock, westEmergencyClock;

    // Random number generator for probabilities
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    sf::Clock moveClock;

    // Traffic light representation (example only)
    sf::CircleShape trafficLight(15);
    trafficLight.setFillColor(sf::Color::Red);
    trafficLight.setPosition(380, 300);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Spawn vehicles from each direction at their respective intervals
        if (northClock.getElapsedTime().asSeconds() >= 1.5f) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(NORTH_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(180);
            vehicle.direction = "NORTH";
            vehicle.speed = 20.0f;
            vehicles.push_back(vehicle);
            northClock.restart();
        }

        if (southClock.getElapsedTime().asSeconds() >= 3.0f) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(SOUTH_SPAWN_REGULAR_LANE1);
            vehicle.direction = "SOUTH";
            vehicle.speed = 20.0f;
            vehicles.push_back(vehicle);
            southClock.restart();
        }

        if (eastClock.getElapsedTime().asSeconds() >= 2.25f) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(EAST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(-90);
            vehicle.direction = "EAST";
            vehicle.speed = 20.0f;
            vehicles.push_back(vehicle);
            eastClock.restart();
        }

        if (westClock.getElapsedTime().asSeconds() >= 3.0f) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(WEST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(90);
            vehicle.direction = "WEST";
            vehicle.speed = 20.0f;
            vehicles.push_back(vehicle);
            westClock.restart();
        }

        // Spawn emergency vehicles
        if (northEmergencyClock.getElapsedTime().asSeconds() >= 15.0f && dis(gen) < 0.2) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(emergencyCarTexture);
            vehicle.sprite.setPosition(NORTH_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(180);
            vehicle.direction = "NORTH";
            vehicle.speed = 30.0f;
            vehicles.push_back(vehicle);
            northEmergencyClock.restart();
        }

        if (southEmergencyClock.getElapsedTime().asSeconds() >= 6.0f && dis(gen) < 0.05) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(emergencyCarTexture);
            vehicle.sprite.setPosition(SOUTH_SPAWN_REGULAR_LANE1);
            vehicle.direction = "SOUTH";
            vehicle.speed = 30.0f;
            vehicles.push_back(vehicle);
            southEmergencyClock.restart();
        }

        if (eastEmergencyClock.getElapsedTime().asSeconds() >= 20.0f && dis(gen) < 0.1) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(emergencyCarTexture);
            vehicle.sprite.setPosition(EAST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(-90);
            vehicle.direction = "EAST";
            vehicle.speed = 30.0f;
            vehicles.push_back(vehicle);
            eastEmergencyClock.restart();
        }

        if (westEmergencyClock.getElapsedTime().asSeconds() >= 6.0f && dis(gen) < 0.3) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(emergencyCarTexture);
            vehicle.sprite.setPosition(WEST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(90);
            vehicle.direction = "WEST";
            vehicle.speed = 30.0f;
            vehicles.push_back(vehicle);
            westEmergencyClock.restart();
        }

        // Spawn heavy cars
        if (heavyCarClock.getElapsedTime().asSeconds() >= 15.0f) {
            for (const std::string& direction : {"NORTH", "SOUTH", "EAST", "WEST"}) {
                VehicleSprite vehicle;
                vehicle.sprite.setTexture(heavyCarTexture);
                vehicle.speed = 10.0f;

                if (direction == "NORTH") {
                    vehicle.sprite.setPosition(NORTH_SPAWN_HEAVY_LANE1);
                    vehicle.sprite.setRotation(180);
                } else if (direction == "SOUTH") {
                    vehicle.sprite.setPosition(SOUTH_SPAWN_HEAVY_LANE1);
                } else if (direction == "EAST") {
                    vehicle.sprite.setPosition(EAST_SPAWN_HEAVY_LANE1);
                    vehicle.sprite.setRotation(-90);
                } else if (direction == "WEST") {
                    vehicle.sprite.setPosition(WEST_SPAWN_HEAVY_LANE1);
                    vehicle.sprite.setRotation(90);
                }

                vehicle.direction = direction;
                vehicles.push_back(vehicle);
            }
            heavyCarClock.restart();
        }

        // Move vehicles
        float deltaTime = moveClock.restart().asSeconds();
        for (auto& vehicle : vehicles) {
            vehicle.sprite.setScale(0.55f, 0.55f); // Scale the vehicle sprite
            if (vehicle.direction == "NORTH") {
                vehicle.sprite.move(0, vehicle.speed * deltaTime); // Move down
            } else if (vehicle.direction == "SOUTH") {
                vehicle.sprite.move(0, -vehicle.speed * deltaTime); // Move up
            } else if (vehicle.direction == "EAST") {
                vehicle.sprite.move(-vehicle.speed * deltaTime, 0); // Move left
            } else if (vehicle.direction == "WEST") {
                vehicle.sprite.move(vehicle.speed * deltaTime, 0); // Move right
            }
        }

        // Clear window and draw
        window.clear();
        window.draw(backgroundSprite);

        // Draw traffic light
        // trafficLight.setFillColor((spawnClock.getElapsedTime().asSeconds() < 5.0f) ? sf::Color::Green : sf::Color::Red);
        window.draw(north_light);
        window.draw(east_light);
        window.draw(west_light);
        window.draw(south_light);

        // Draw vehicles
        for (const auto& vehicle : vehicles) {
            window.draw(vehicle.sprite);
        }

        // Display updated window
        window.display();
    }

    return 0;
}
