#include "Intersection.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>

const sf::Vector2f NORTH_SPAWN_LANE1(425, 0);  // Starting from top-center
const sf::Vector2f SOUTH_SPAWN_LANE1(300, 800); // Starting from bottom-center
const sf::Vector2f EAST_SPAWN_LANE1(800, 490);  // Starting from right-center
const sf::Vector2f WEST_SPAWN_LANE1(0, 305);    // Starting from left-center

struct VehicleSprite {
    sf::Sprite sprite;
    std::string direction;
    float speed; // Pixels per second
};

int main() {
    // Initialize intersection and SFML window
    Intersection intersection;
    sf::RenderWindow window(sf::VideoMode(800, 800), "Smart Traffic Simulation");

    // Load the background texture
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("img/Intersection3.png")) {
        std::cerr << "Error: Could not load img/Intersection.png" << std::endl;
        return -1;
    }
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);

    // Load vehicle texture
    sf::Texture vehicleTexture;
    if (!vehicleTexture.loadFromFile("img/RegularCar2.png")) { // Replace with your car image
        std::cerr << "Error: Could not load img/Car.png" << std::endl;
        return -1;
    }
    

    // List to hold active vehicle sprites
    std::vector<VehicleSprite> vehicles;

    // Clock for simulation timing
    sf::Clock spawnClock;
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

        // Check for vehicle spawning every second
        if (spawnClock.getElapsedTime().asSeconds() >= 3.0f) {
            // Spawn vehicles from each direction
            for (const std::string& direction : {"NORTH", "SOUTH", "EAST", "WEST"}) {
                VehicleSprite vehicle;
                vehicle.sprite.setTexture(vehicleTexture);
                vehicle.speed = 20.0f; // Adjust speed as needed

                if (direction == "NORTH") {
                    vehicle.sprite.setPosition(NORTH_SPAWN_LANE1); // Starting position
                    vehicle.sprite.setRotation(180);
                    vehicle.direction = "NORTH";

                } else if (direction == "SOUTH") {
                    vehicle.sprite.setPosition(SOUTH_SPAWN_LANE1); // Starting position
                    vehicle.direction = "SOUTH";

                } else if (direction == "EAST") {
                    vehicle.sprite.setPosition(EAST_SPAWN_LANE1); // Starting position
                    vehicle.sprite.setRotation(-90);
                    vehicle.direction = "EAST";
                    
                } else if (direction == "WEST") {
                    vehicle.sprite.setPosition(WEST_SPAWN_LANE1); // Starting position
                    vehicle.sprite.setRotation(90);
                    vehicle.direction = "WEST";
                }

                vehicles.push_back(vehicle);
            }
            spawnClock.restart();
        }

        // Move vehicles
        float deltaTime = moveClock.restart().asSeconds();
        for (auto& vehicle : vehicles) {
            vehicle.sprite.setScale(0.7f, 0.7f); // Example scale (10% of original size)
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
        trafficLight.setFillColor((spawnClock.getElapsedTime().asSeconds() < 5.0f) ? sf::Color::Green : sf::Color::Red);
        window.draw(trafficLight);

        // Draw vehicles
        for (const auto& vehicle : vehicles) {
            window.draw(vehicle.sprite);
        }

        // Display updated window
        window.display();
    }

    return 0;
}
