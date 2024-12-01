#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include <random>

struct TrafficLight {
    sf::Sprite lightSprite; // Sprite for the light
    sf::Texture redTex, yellowTex, greenTex;
    std::string state;      // Current state: "RED", "GREEN", "YELLOW"
    sf::Clock stateClock;   // Timer for state transitions
    float redDuration, yellowDuration, greenDuration; // Durations for each state

    TrafficLight(sf::Texture& redTex, sf::Texture& yellowTex, sf::Texture& greenTex, float redDur, float yellowDur, float greenDur) 
        : redDuration(redDur), yellowDuration(yellowDur), greenDuration(greenDur), state("RED") {
        this->greenTex = greenTex;
        this->redTex = redTex;
        this->yellowTex = yellowTex;
        
        lightSprite.setTexture(redTex);
    }

    void updateState() {
        float elapsed = stateClock.getElapsedTime().asSeconds();
        if (state == "RED" && elapsed >= redDuration) {
            state = "GREEN";
            lightSprite.setTexture(greenTex);
            stateClock.restart();
        } else if (state == "GREEN" && elapsed >= greenDuration) {
            state = "YELLOW";
            lightSprite.setTexture(yellowTex);
            stateClock.restart();
        } else if (state == "YELLOW" && elapsed >= yellowDuration) {
            state = "RED";
            lightSprite.setTexture(redTex);
            stateClock.restart();
        }
    }

    bool canPass() const {
        return state == "GREEN";
    }
};


const sf::Vector2f NORTH_SPAWN_REGULAR_LANE1(522, 0);  // Starting from top-center
const sf::Vector2f SOUTH_SPAWN_REGULAR_LANE1(403, 1000); // Starting from bottom-center
const sf::Vector2f EAST_SPAWN_REGULAR_LANE1(1000, 533);  // Starting from right-center
const sf::Vector2f WEST_SPAWN_REGULAR_LANE1(0, 410);    // Starting from left-center

const sf::Vector2f NORTH_TURN_LANE1(405, 278);  // Starting from top-center
const sf::Vector2f SOUTH_TURN_LANE1(523, 715); // Done
const sf::Vector2f EAST_TURN_LANE1(700, 467);  // DONE
const sf::Vector2f WEST_TURN_LANE1(290, 535);    // Starting from left-center


const sf::Vector2f NORTH_SPAWN_HEAVY_LANE2(572, 0);  // Starting from top-center
const sf::Vector2f SOUTH_SPAWN_HEAVY_LANE2(453, 1000); // Starting from bottom-center
const sf::Vector2f EAST_SPAWN_HEAVY_LANE2(1000, 580);  // Starting from right-center
const sf::Vector2f WEST_SPAWN_HEAVY_LANE2(0, 460);    // Starting from left-center

// IMPORTANT NOTES:
// I have used the scale of 1s in real life = 3s in my simulation for the spawning cars. As the sprites overlap if a wait of 1s is given


struct VehicleSprite {
    sf::Sprite sprite;
    std::string direction;
    std::string type; // "R", "E", "H"
    float speed; // Pixels per second
};


int main() {
    // Initialize intersection and SFML window
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Smart Traffic Simulation");

    // Load the background texture
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("img/Intersection6.png")) {
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
        !emergencyCarTexture.loadFromFile("img/police.png") ||
        !heavyCarTexture.loadFromFile("img/truck.png")) {
        std::cerr << "Error: Could not load vehicle textures" << std::endl;
        return -1;
    }
    

    // List to hold active vehicle sprites
    std::vector<VehicleSprite> vehicles;

    // Sprites for the traffic lights
    sf::Texture redLightTex, greenLightTex, YellowLightTex;
    if (!redLightTex.loadFromFile("img/redlight.png") ||
        !YellowLightTex.loadFromFile("img/yellowLight.png") ||
        !greenLightTex.loadFromFile("img/greenLight.png")) {
        std::cerr << "Error: Could not load traffic textures" << std::endl;
        return -1;
    }

    TrafficLight northLight(redLightTex, YellowLightTex, greenLightTex, 5.0f, 2.0f, 8.0f);
    TrafficLight southLight(redLightTex, YellowLightTex, greenLightTex, 5.0f, 2.0f, 8.0f);
    TrafficLight eastLight(redLightTex, YellowLightTex, greenLightTex, 5.0f, 2.0f, 8.0f);
    TrafficLight westLight(redLightTex, YellowLightTex, greenLightTex, 5.0f, 2.0f, 8.0f);

    northLight.lightSprite.setPosition(480, 400);
    northLight.lightSprite.setScale(0.1f, 0.1f);

    southLight.lightSprite.setPosition(480, 515);
    southLight.lightSprite.setScale(0.1f, 0.1f);

    eastLight.lightSprite.setPosition(530, 460);
    eastLight.lightSprite.setScale(0.1f, 0.1f);
    
    westLight.lightSprite.setPosition(420, 460);
    westLight.lightSprite.setScale(0.1f, 0.1f);





    // Clock for simulation timing for each direction
    sf::Clock northClock, southClock, eastClock, westClock, heavyCarClock;

    // Separate timers for emergency cars
    sf::Clock northEmergencyClock, southEmergencyClock, eastEmergencyClock, westEmergencyClock;
    bool northEmergency, southEmergency, eastEmergency, westEmergency;
    northEmergency = southEmergency = eastEmergency = westEmergency = false;

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

        // // Spawn emergency vehicles
        // if (northEmergencyClock.getElapsedTime().asSeconds() >= 15.0f && dis(gen) < 0.2) {
        //     VehicleSprite vehicle;
        //     vehicle.sprite.setTexture(emergencyCarTexture);
        //     vehicle.sprite.setPosition(NORTH_SPAWN_HEAVY_LANE2);
        //     vehicle.sprite.setRotation(180);
        //     vehicle.sprite.setScale(0.5f, 0.5f);
        //     vehicle.direction = "NORTH";
        //     vehicle.speed = 20.0f;
        //     vehicle.type = "E";
        //     vehicles.push_back(vehicle);
        //     northEmergency = true;
        //     northEmergencyClock.restart();
        // }

        // if (southEmergencyClock.getElapsedTime().asSeconds() >= 6.0f && dis(gen) < 0.05) {
        //     VehicleSprite vehicle;
        //     vehicle.sprite.setTexture(emergencyCarTexture);
        //     vehicle.sprite.setPosition(SOUTH_SPAWN_HEAVY_LANE2);
        //     vehicle.direction = "SOUTH";
        //     vehicle.speed = 20.0f;
        //     vehicle.type = "E";
        //     vehicles.push_back(vehicle);
        //     southEmergency = true;
        //     southEmergencyClock.restart();
        // }

        // if (eastEmergencyClock.getElapsedTime().asSeconds() >= 20.0f && dis(gen) < 0.1) {
        //     VehicleSprite vehicle;
        //     vehicle.sprite.setTexture(emergencyCarTexture);
        //     vehicle.sprite.setPosition(EAST_SPAWN_HEAVY_LANE2);
        //     vehicle.sprite.setRotation(-90);
        //     vehicle.direction = "EAST";
        //     vehicle.speed = 20.0f;
        //     vehicle.type = "E";
        //     vehicles.push_back(vehicle);
        //     eastEmergency = true;
        //     eastEmergencyClock.restart();
        // }

        // if (westEmergencyClock.getElapsedTime().asSeconds() >= 6.0f && dis(gen) < 0.3) {
        //     VehicleSprite vehicle;
        //     vehicle.sprite.setTexture(emergencyCarTexture);
        //     vehicle.sprite.setPosition(WEST_SPAWN_HEAVY_LANE2);
        //     vehicle.sprite.setRotation(90);
        //     vehicle.direction = "WEST";
        //     vehicle.speed = 20.0f;
        //     vehicle.type = "E";
        //     vehicles.push_back(vehicle);
        //     westEmergency = true;
        //     westEmergencyClock.restart();
        // }

        // Spawn vehicles from each direction at their respective intervals
        if (northClock.getElapsedTime().asSeconds() >= 3.0f && !northEmergency) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(NORTH_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(180);
            vehicle.direction = "NORTH";
            vehicle.speed = 30.0f;
            vehicle.type = "R";
            vehicles.push_back(vehicle);
            northClock.restart();
        }

        if (southClock.getElapsedTime().asSeconds() >= 4.0f && !southEmergency) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(SOUTH_SPAWN_REGULAR_LANE1);
            vehicle.direction = "SOUTH";
            vehicle.speed = 30.0f;
            vehicle.type = "R";
            vehicles.push_back(vehicle);
            southClock.restart();
        }

        if (eastClock.getElapsedTime().asSeconds() >= 3.5f && !eastEmergency) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(EAST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(-90);
            vehicle.direction = "EAST";
            vehicle.speed = 35.0f;
            vehicle.type = "R";
            vehicles.push_back(vehicle);
            eastClock.restart();
        }

        if (westClock.getElapsedTime().asSeconds() >= 4.0f && !westEmergency) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(WEST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(90);
            vehicle.direction = "WEST";
            vehicle.speed = 35.0f;
            vehicle.type = "R";
            vehicles.push_back(vehicle);
            westClock.restart();
        }

        northEmergency = southEmergency = eastEmergency = westEmergency = false;
        


        // // Spawn heavy cars
        // if (heavyCarClock.getElapsedTime().asSeconds() >= 15.0f) {
        //     for (const std::string& direction : {"NORTH", "SOUTH", "EAST", "WEST"}) {
        //         VehicleSprite vehicle;
        //         vehicle.sprite.setTexture(heavyCarTexture);
        //         vehicle.speed = 10.0f;

        //         if (direction == "NORTH") {
        //             vehicle.sprite.setPosition(NORTH_SPAWN_HEAVY_LANE2);
        //             vehicle.sprite.setRotation(180);
        //         } else if (direction == "SOUTH") {
        //             vehicle.sprite.setPosition(SOUTH_SPAWN_HEAVY_LANE2);
        //         } else if (direction == "EAST") {
        //             vehicle.sprite.setPosition(EAST_SPAWN_HEAVY_LANE2);
        //             vehicle.sprite.setRotation(-90);
        //         } else if (direction == "WEST") {
        //             vehicle.sprite.setPosition(WEST_SPAWN_HEAVY_LANE2);
        //             vehicle.sprite.setRotation(90);
        //         }

        //         vehicle.direction = direction;
        //         vehicle.type = "H";
        //         vehicles.push_back(vehicle);
        //     }
        //     heavyCarClock.restart();
        // }

        northLight.updateState();
        southLight.updateState();
        eastLight.updateState();
        westLight.updateState();


       // Move vehicles
        float deltaTime = moveClock.restart().asSeconds();
        for (size_t i = 0; i < vehicles.size(); ++i) {
            bool canMove = true;
            
            // Check if the car has crossed the traffic light
            if (vehicles[i].direction == "NORTH" && vehicles[i].sprite.getPosition().y > 350) {
                // Randomly decide the next direction
                int turn = std::rand() % 3; // 0 = LEFT, 1 = STRAIGHT, 2 = RIGHT
                if (turn == 0) { // Turn LEFT
                    vehicles[i].direction = "TURN_EAST";
                    vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                    vehicles[i].sprite.setRotation(-90); // Update rotation for westward movement
                } else if (turn == 1) { // Go STRAIGHT
                    vehicles[i].direction = "TURN_NORTH";
                    vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1); // Reposition to south lane
                    
                } else if (turn == 2) { // Turn RIGHT
                    vehicles[i].direction = "TURN_WEST";
                    vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to east lane
                    vehicles[i].sprite.setRotation(90); // Update rotation for eastward movement
                }
                continue; // Skip further checks for this car in the current frame
            }

            // Similar logic for SOUTH, EAST, and WEST directions
            if (vehicles[i].direction == "SOUTH" && vehicles[i].sprite.getPosition().y < 650) {
                int turn = std::rand() % 3;
                if (turn == 0) {
                    vehicles[i].direction = "TURN_WEST";
                    vehicles[i].sprite.setPosition(EAST_TURN_LANE1);
                    vehicles[i].sprite.setRotation(90);
                } else if (turn == 1) {
                    vehicles[i].direction = "TURN_SOUTH";
                    vehicles[i].sprite.setPosition(NORTH_TURN_LANE1);
                } else if (turn == 2) {
                    vehicles[i].direction = "TURN_EAST";
                    vehicles[i].sprite.setPosition(WEST_TURN_LANE1);
                    vehicles[i].sprite.setRotation(-90);
                }
                continue;
            }

            if (vehicles[i].direction == "EAST" && vehicles[i].sprite.getPosition().x < 650) {
                int turn = std::rand() % 3;
                if (turn == 0) {
                    vehicles[i].direction = "TURN_SOUTH";
                    vehicles[i].sprite.setPosition(NORTH_TURN_LANE1);
                    vehicles[i].sprite.setRotation(0);
                } else if (turn == 1) {
                    vehicles[i].direction = "TURN_EAST";
                    vehicles[i].sprite.setPosition(WEST_TURN_LANE1);
                    //vehicles[i].sprite.setRotation(90);
                } else if (turn == 2) {
                    vehicles[i].direction = "TURN_NORTH";
                    vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1);
                    vehicles[i].sprite.setRotation(180);
                }
                continue;
            }

            if (vehicles[i].direction == "WEST" && vehicles[i].sprite.getPosition().x > 350) {
                int turn = std::rand() % 3;
                if (turn == 0) {
                    vehicles[i].direction = "TURN_NORTH";
                    vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1);
                    vehicles[i].sprite.setRotation(180);
                } else if (turn == 1) {
                    vehicles[i].direction = "TURN_WEST";
                    vehicles[i].sprite.setPosition(EAST_TURN_LANE1);
                } else if (turn == 2) {
                    vehicles[i].direction = "TURN_SOUTH";
                    vehicles[i].sprite.setPosition(NORTH_TURN_LANE1);
                    vehicles[i].sprite.setRotation(0);
                }
                continue;
            }

            // Scale vehicles based on type
            if (vehicles[i].type == "R") {
                vehicles[i].sprite.setScale(0.55f, 0.55f);
            } else if (vehicles[i].type == "E") {
                vehicles[i].sprite.setScale(0.45f, 0.45f);
            } else if (vehicles[i].type == "H") {
                vehicles[i].sprite.setScale(0.45f, 0.45f); 
            }

            

            // Check traffic light states and proximity to other vehicles
            if (vehicles[i].direction == "NORTH") {
                if (!northLight.canPass() && vehicles[i].sprite.getPosition().y >= 300) {
                    canMove = false; // Stop if at the traffic light
                }
                // Check for vehicle ahead
                for (size_t j = 0; j < vehicles.size(); ++j) {
                    if (i != j && vehicles[j].direction == "NORTH" &&
                        vehicles[j].sprite.getPosition().y > vehicles[i].sprite.getPosition().y &&
                        vehicles[j].sprite.getPosition().y - vehicles[i].sprite.getPosition().y < 50.0f) { // Minimum gap
                        canMove = false;
                        break;
                    }
                }
            } else if (vehicles[i].direction == "SOUTH") {
                if (!southLight.canPass() && vehicles[i].sprite.getPosition().y <= 715) {
                    canMove = false;
                }
                // Check for vehicle ahead
                for (size_t j = 0; j < vehicles.size(); ++j) {
                    if (i != j && vehicles[j].direction == "SOUTH" &&
                        vehicles[j].sprite.getPosition().y < vehicles[i].sprite.getPosition().y &&
                        vehicles[i].sprite.getPosition().y - vehicles[j].sprite.getPosition().y < 50.0f) { // Minimum gap
                        canMove = false;
                        break;
                    }
                }
            } else if (vehicles[i].direction == "EAST") {
                if (!eastLight.canPass() && vehicles[i].sprite.getPosition().x <= 700) {
                    canMove = false;
                }
                // Check for vehicle ahead
                for (size_t j = 0; j < vehicles.size(); ++j) {
                    if (i != j && vehicles[j].direction == "EAST" &&
                        vehicles[j].sprite.getPosition().x < vehicles[i].sprite.getPosition().x &&
                        vehicles[i].sprite.getPosition().x - vehicles[j].sprite.getPosition().x < 50.0f) { // Minimum gap
                        canMove = false;
                        break;
                    }
                }
            } else if (vehicles[i].direction == "WEST") {
                if (!westLight.canPass() && vehicles[i].sprite.getPosition().x >= 290) {
                    canMove = false;
                }
                // Check for vehicle ahead
                for (size_t j = 0; j < vehicles.size(); ++j) {
                    if (i != j && vehicles[j].direction == "WEST" &&
                        vehicles[j].sprite.getPosition().x > vehicles[i].sprite.getPosition().x &&
                        vehicles[j].sprite.getPosition().x - vehicles[i].sprite.getPosition().x < 50.0f) { // Minimum gap
                        canMove = false;
                        break;
                    }
                }
            }

            // Move the vehicle if allowed
            if (canMove) {
                if (vehicles[i].direction == "NORTH" || vehicles[i].direction == "TURN_NORTH") {
                    vehicles[i].sprite.move(0, vehicles[i].speed * deltaTime);
                } else if (vehicles[i].direction == "SOUTH" || vehicles[i].direction == "TURN_SOUTH") {
                    vehicles[i].sprite.move(0, -vehicles[i].speed * deltaTime);
                } else if (vehicles[i].direction == "EAST" || vehicles[i].direction == "TURN_EAST") {
                    vehicles[i].sprite.move(-vehicles[i].speed * deltaTime, 0);
                } else if (vehicles[i].direction == "WEST" || vehicles[i].direction == "TURN_WEST") {
                    vehicles[i].sprite.move(vehicles[i].speed * deltaTime, 0);
                }
            }
        }

        // Get the mouse position relative to the window
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

        // Print mouse coordinates to the terminal
        std::cout << "Mouse Coordinates: (" << mousePosition.x << ", " << mousePosition.y << ")\r";
        std::flush(std::cout); // Ensure it continuously updates in the terminal



        // Clear window and draw
        window.clear();
        window.draw(backgroundSprite);

        // Draw traffic light
        // trafficLight.setFillColor((spawnClock.getElapsedTime().asSeconds() < 5.0f) ? sf::Color::Green : sf::Color::Red);
        window.draw(northLight.lightSprite);
        window.draw(eastLight.lightSprite);
        window.draw(westLight.lightSprite);
        window.draw(southLight.lightSprite);

        // Draw vehicles
        for (const auto& vehicle : vehicles) {
            window.draw(vehicle.sprite);
        }

        // Display updated window
        window.display();
    }

    return 0;
}
