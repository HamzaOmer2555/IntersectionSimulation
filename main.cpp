#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include <random>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

const sf::Vector2f NORTH_SPAWN_REGULAR_LANE1(522, 0);  // Starting from top-center
const sf::Vector2f SOUTH_SPAWN_REGULAR_LANE1(403, 1000); // Starting from bottom-center
const sf::Vector2f EAST_SPAWN_REGULAR_LANE1(1000, 533);  // Starting from right-center
const sf::Vector2f WEST_SPAWN_REGULAR_LANE1(0, 410);    // Starting from left-center

const sf::Vector2f NORTH_TURN_LANE1(405, 278);  // Starting from top-center
const sf::Vector2f SOUTH_TURN_LANE1(523, 715); // Done
const sf::Vector2f EAST_TURN_LANE1(700, 467);  // DONE
const sf::Vector2f WEST_TURN_LANE1(290, 535);    // Starting from left-center


const sf::Vector2f NORTH_SPAWN_HEAVY_LANE2(580, 0);  // Starting from top-center
const sf::Vector2f SOUTH_SPAWN_HEAVY_LANE2(450, 1000); // Starting from bottom-center
const sf::Vector2f EAST_SPAWN_HEAVY_LANE2(1000, 590);  // Starting from right-center
const sf::Vector2f WEST_SPAWN_HEAVY_LANE2(0, 460);    // Starting from left-center


const sf::Vector2f NORTH_TURN_LANE2(400, 278);  // Starting from top-center
const sf::Vector2f SOUTH_TURN_LANE2(575, 715); // Done
const sf::Vector2f EAST_TURN_LANE2(700, 410);  // DONE
const sf::Vector2f WEST_TURN_LANE2(290, 590);    // Starting from left-center

const int REGULAR_VEHICLE_SPEED_LIMIT = 60;
const int HEAVY_VEHICLE_SPEED_LIMIT = 40;
const int EMERGENCY_VEHICLE_SPEED_LIMIT = 80;

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

    


    bool canPass() const {
        return state == "GREEN";
    }
};

struct VehicleSprite {
    std::string plateNumber;
    sf::Sprite sprite;
    std::string direction;
    std::string type; // "R", "E", "H"
    float speed; // actual sprite speed
    int mockSpeed; // for challan status
    bool hasTurned;
};

void updateTrafficLights(TrafficLight& northLight, TrafficLight& southLight, TrafficLight& eastLight, TrafficLight& westLight, sf::Clock& trafficCycleClock, float cycleDuration, float yellowDuration) {
    float elapsed = trafficCycleClock.getElapsedTime().asSeconds();

    if (elapsed >= cycleDuration) {
        trafficCycleClock.restart(); // Restart the cycle timer
    }

    // Define the time points for the light changes
    float greenPhaseDuration = (cycleDuration / 2) - yellowDuration;

    if (elapsed < greenPhaseDuration) {
        // North-South GREEN, East-West RED
        if (northLight.state != "GREEN") {
            northLight.state = "GREEN";
            northLight.lightSprite.setTexture(northLight.greenTex);
        }
        if (southLight.state != "GREEN") {
            southLight.state = "GREEN";
            southLight.lightSprite.setTexture(southLight.greenTex);
        }
        if (eastLight.state != "RED") {
            eastLight.state = "RED";
            eastLight.lightSprite.setTexture(eastLight.redTex);
        }
        if (westLight.state != "RED") {
            westLight.state = "RED";
            westLight.lightSprite.setTexture(westLight.redTex);
        }
    } else if (elapsed < greenPhaseDuration + yellowDuration) {
        // North-South YELLOW, East-West remains RED
        if (northLight.state != "YELLOW") {
            northLight.state = "YELLOW";
            northLight.lightSprite.setTexture(northLight.yellowTex);
        }
        if (southLight.state != "YELLOW") {
            southLight.state = "YELLOW";
            southLight.lightSprite.setTexture(southLight.yellowTex);
        }
    } else if (elapsed < cycleDuration / 2) {
        // North-South RED, East-West remains RED
        if (northLight.state != "RED") {
            northLight.state = "RED";
            northLight.lightSprite.setTexture(northLight.redTex);
        }
        if (southLight.state != "RED") {
            southLight.state = "RED";
            southLight.lightSprite.setTexture(southLight.redTex);
        }
    } else if (elapsed < (cycleDuration / 2) + greenPhaseDuration) {
        // East-West GREEN, North-South RED
        if (eastLight.state != "GREEN") {
            eastLight.state = "GREEN";
            eastLight.lightSprite.setTexture(eastLight.greenTex);
        }
        if (westLight.state != "GREEN") {
            westLight.state = "GREEN";
            westLight.lightSprite.setTexture(westLight.greenTex);
        }
        if (northLight.state != "RED") {
            northLight.state = "RED";
            northLight.lightSprite.setTexture(northLight.redTex);
        }
        if (southLight.state != "RED") {
            southLight.state = "RED";
            southLight.lightSprite.setTexture(southLight.redTex);
        }
    } else if (elapsed < (cycleDuration / 2) + greenPhaseDuration + yellowDuration) {
        // East-West YELLOW, North-South remains RED
        if (eastLight.state != "YELLOW") {
            eastLight.state = "YELLOW";
            eastLight.lightSprite.setTexture(eastLight.yellowTex);
        }
        if (westLight.state != "YELLOW") {
            westLight.state = "YELLOW";
            westLight.lightSprite.setTexture(westLight.yellowTex);
        }
    } else {
        // East-West RED, North-South remains RED
        if (eastLight.state != "RED") {
            eastLight.state = "RED";
            eastLight.lightSprite.setTexture(eastLight.redTex);
        }
        if (westLight.state != "RED") {
            westLight.state = "RED";
            westLight.lightSprite.setTexture(westLight.redTex);
        }
    }
}

int generateMockEmergencySpeed() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(1, 75);
    return dist(gen);
}

int generateMockRegularSpeed() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(1, 55);
    return dist(gen);
}

int generateMockHeavySpeed() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(1, 35);
    return dist(gen);
}

// Function to generate a random plate number
std::string generateRandomPlate() {
    std::random_device rd;                  // Seed for random number generation
    std::mt19937 gen(rd());                 // Mersenne Twister RNG
    std::uniform_int_distribution<> charDist(0, 25); // Distribution for letters (A-Z)
    std::uniform_int_distribution<> numDist(0, 9);   // Distribution for digits (0-9)

    std::string plate;

    // Generate 3 random uppercase letters
    for (int i = 0; i < 3; ++i) {
        char letter = 'A' + charDist(gen); // Convert to ASCII character
        plate += letter;
    }

    // Generate 3 random digits
    for (int i = 0; i < 3; ++i) {
        char digit = '0' + numDist(gen); // Convert to ASCII character
        plate += digit;
    }

    return plate;
}

// Struct to represent a speed violation
struct SpeedViolation {
    std::string vehicleID;
    std::string type;
    float speed;
    std::string direction;
};

// Shared resources
std::queue<SpeedViolation> violationQueue;
std::mutex queueMutex;
std::condition_variable violationNotifier;
bool stopChallanThread = false;

// Challan processing function
void challanProcessor() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);

        // Wait for a notification or stop signal
        violationNotifier.wait(lock, [] { return !violationQueue.empty() || stopChallanThread; });

        if (stopChallanThread && violationQueue.empty()) {
            break; // Exit the thread if stop signal is received
        }

        // Process all violations in the queue
        while (!violationQueue.empty()) {
            SpeedViolation violation = violationQueue.front();
            violationQueue.pop();
            lock.unlock(); // Unlock while processing to allow main thread to add more

            // Simulate challan processing
            std::cout << "Processing challan for Vehicle: " << violation.vehicleID
                    << " | Speed: " << violation.speed
                    << " | Direction: " << violation.direction << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate processing delay
            lock.lock(); // Re-lock to access the queue
        }
    }
}

// IMPORTANT NOTES:
// I have used the scale of 1s in real life = 3s in my simulation for the spawning cars. As the sprites overlap if a wait of 1s is given

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
        !emergencyCarTexture.loadFromFile("img/EmergencyCar.png") ||
        !heavyCarTexture.loadFromFile("img/Truck.png")) {
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

    TrafficLight northLight(redLightTex, YellowLightTex, greenLightTex, 4.0f, 2.0f, 7.0f);
    TrafficLight southLight(redLightTex, YellowLightTex, greenLightTex, 4.0f, 2.0f, 7.0f);
    TrafficLight eastLight(redLightTex, YellowLightTex, greenLightTex, 4.0f, 2.0f, 7.0f);
    TrafficLight westLight(redLightTex, YellowLightTex, greenLightTex, 4.0f, 2.0f, 7.0f);

    northLight.lightSprite.setPosition(505, 348);
    northLight.lightSprite.setScale(0.1f, 0.1f);
    northLight.lightSprite.rotate(180);


    southLight.lightSprite.setPosition(467, 648);
    southLight.lightSprite.setScale(0.1f, 0.1f);

    eastLight.lightSprite.setPosition(645, 520);
    eastLight.lightSprite.setScale(0.1f, 0.1f);
    eastLight.lightSprite.rotate(-90);
    
    westLight.lightSprite.setPosition(345, 482);
    westLight.lightSprite.setScale(0.1f, 0.1f);
    westLight.lightSprite.rotate(+90);


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

    sf::Clock trafficCycleClock;
    float cycleDuration = 15.0f; // Total duration for one complete cycle (e.g., 10 seconds)

    // Load font for timer
    sf::Font font;
    if (!font.loadFromFile("fonts/fonty_font.ttf")) { // Replace with the path to your font file
        std::cerr << "Error: Could not load font!" << std::endl;
        return -1;
    }

    // Timer text
    sf::Text timerText;
    timerText.setFont(font);
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(5, 950); // Top-left corner

    sf::Clock simulationClock;
    const float simulationDuration = 300.0f; // 5 minutes in seconds
    float elapsedTime;

    sf::Clock speedTimer;

    // Initiate the challan thread
    std::thread challanThread(challanProcessor);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Calculate remaining time
        elapsedTime = simulationClock.getElapsedTime().asSeconds();

        if (elapsedTime >= 500.0f) {
            std::cout << "Simulation complete!" << std::endl;
            window.close(); // Exit the simulation after 5 minutes
            break;
        }

        // Spawn emergency vehicles
        // Max speed = 80km/hr
        if (northEmergencyClock.getElapsedTime().asSeconds() >= 15.0f && dis(gen) < 0.2) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(emergencyCarTexture);
            vehicle.sprite.setPosition(NORTH_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(180);
            vehicle.direction = "NORTH";
            vehicle.speed = 30.0f;
            vehicle.type = "E";
            vehicle.hasTurned = false;
            vehicle.mockSpeed = generateMockEmergencySpeed();
            vehicles.push_back(vehicle);
            northEmergency = true;
            northEmergencyClock.restart();
        }

        if (southEmergencyClock.getElapsedTime().asSeconds() >= 6.0f && dis(gen) < 0.05) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(emergencyCarTexture);
            vehicle.sprite.setPosition(SOUTH_SPAWN_REGULAR_LANE1);
            vehicle.direction = "SOUTH";
            vehicle.speed = 30.0f;
            vehicle.hasTurned = false;
            vehicle.mockSpeed = generateMockEmergencySpeed();
            vehicle.type = "E";
            vehicles.push_back(vehicle);
            southEmergency = true;
            southEmergencyClock.restart();
        }

        if (eastEmergencyClock.getElapsedTime().asSeconds() >= 20.0f && dis(gen) < 0.1) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(emergencyCarTexture);
            vehicle.sprite.setPosition(EAST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(-90);
            vehicle.direction = "EAST";
            vehicle.speed = 30.0f;
            vehicle.hasTurned = false;
            vehicle.type = "E";
            vehicle.mockSpeed = generateMockEmergencySpeed();
            vehicles.push_back(vehicle);
            eastEmergency = true;
            eastEmergencyClock.restart();
        }

        if (westEmergencyClock.getElapsedTime().asSeconds() >= 6.0f && dis(gen) < 0.3) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(emergencyCarTexture);
            vehicle.sprite.setPosition(WEST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(90);
            vehicle.direction = "WEST";
            vehicle.speed = 30.0f;
            vehicle.type = "E";
            vehicle.hasTurned = false;
            vehicle.mockSpeed = generateMockEmergencySpeed();
            vehicles.push_back(vehicle);
            westEmergency = true;
            westEmergencyClock.restart();
        }

        // Spawn regular vehicles from each direction at their respective intervals
        if (northClock.getElapsedTime().asSeconds() >= 3.0f && !northEmergency) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(NORTH_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(180);
            vehicle.plateNumber = generateRandomPlate();
            vehicle.direction = "NORTH";
            vehicle.speed = 30.0f;
            vehicle.type = "R";
            vehicle.hasTurned = false;
            vehicle.mockSpeed = generateMockRegularSpeed();
            std::cout<<"Vehicle Information: "<<vehicle.plateNumber<<", "<<vehicle.type<<", "<<vehicle.direction<<", "<<vehicle.mockSpeed<<". "<<std::endl;
            vehicles.push_back(vehicle);
            northClock.restart();
        }

        if (southClock.getElapsedTime().asSeconds() >= 4.0f && !southEmergency) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(SOUTH_SPAWN_REGULAR_LANE1);
            vehicle.direction = "SOUTH";
            vehicle.speed = 30.0f;
            vehicle.hasTurned = false;
            vehicle.type = "R";
            vehicle.mockSpeed = generateMockRegularSpeed();
            vehicles.push_back(vehicle);
            southClock.restart();
        }

        if (eastClock.getElapsedTime().asSeconds() >= 3.5f && !eastEmergency) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(EAST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(-90);
            vehicle.direction = "EAST";
            vehicle.speed = 30.0f;
            vehicle.type = "R";
            vehicle.hasTurned = false;
            vehicle.mockSpeed = generateMockRegularSpeed();
            vehicles.push_back(vehicle);
            eastClock.restart();
        }

        if (westClock.getElapsedTime().asSeconds() >= 4.0f && !westEmergency) {
            VehicleSprite vehicle;
            vehicle.sprite.setTexture(regularCarTexture);
            vehicle.sprite.setPosition(WEST_SPAWN_REGULAR_LANE1);
            vehicle.sprite.setRotation(90);
            vehicle.direction = "WEST";
            vehicle.speed = 30.0f;
            vehicle.hasTurned = false;
            vehicle.type = "R";
            vehicle.mockSpeed = generateMockRegularSpeed();
            vehicles.push_back(vehicle);
            westClock.restart();
        }

        northEmergency = southEmergency = eastEmergency = westEmergency = false;
        
        
        // spawn heavy cars only during minute 2-3. For one minute
        if (elapsedTime >= 0 && elapsedTime <= 60)
        {// Spawn heavy cars
            if (heavyCarClock.getElapsedTime().asSeconds() >= 15.0f) {
                for (const std::string& direction : {"NORTH", "SOUTH", "EAST", "WEST"}) {
                    VehicleSprite vehicle;
                    vehicle.sprite.setTexture(heavyCarTexture);
                    vehicle.speed = 45.0f;

                    if (direction == "NORTH") {
                        vehicle.sprite.setPosition(NORTH_SPAWN_HEAVY_LANE2);
                        vehicle.sprite.setRotation(180);
                    } else if (direction == "SOUTH") {
                        vehicle.sprite.setPosition(SOUTH_SPAWN_HEAVY_LANE2);
                    } else if (direction == "EAST") {
                        vehicle.sprite.setPosition(EAST_SPAWN_HEAVY_LANE2);
                        vehicle.sprite.setRotation(-90);
                    } else if (direction == "WEST") {
                        vehicle.sprite.setPosition(WEST_SPAWN_HEAVY_LANE2);
                        vehicle.sprite.setRotation(90);
                    }

                    vehicle.direction = direction;
                    vehicle.type = "H";
                    vehicle.hasTurned = false;
                    vehicle.mockSpeed = generateMockHeavySpeed();
                    vehicles.push_back(vehicle);
                }
                heavyCarClock.restart();
            }
        }
        else
        {
            heavyCarClock.restart();
        }

        updateTrafficLights(northLight, southLight, eastLight, westLight, trafficCycleClock, cycleDuration, 3);

        

        // Inside the update loop
        if (speedTimer.getElapsedTime().asSeconds() >= 5.0f) {
            // Increase the mock speed of all vehicles
            for (auto& vehicle : vehicles) {    
                vehicle.mockSpeed += 5; 
            }
            speedTimer.restart(); // Reset the timer
        }

        // implement the catching of vehicles here if they exceed the speed limit
        // Check for speed violations
        for (const auto& vehicle : vehicles) {
            int speedLimit = 0;

            // Determine the speed limit based on vehicle type
            if (vehicle.type == "R") {
                speedLimit = REGULAR_VEHICLE_SPEED_LIMIT;
            } else if (vehicle.type == "H") {
                speedLimit = HEAVY_VEHICLE_SPEED_LIMIT;
            } else if (vehicle.type == "E") {
                speedLimit = EMERGENCY_VEHICLE_SPEED_LIMIT;
            }

            // Check if the vehicle exceeds the speed limit
            if (vehicle.mockSpeed > speedLimit) {
                // Create a SpeedViolation object
                SpeedViolation violation = {
                    vehicle.plateNumber, // Vehicle ID
                    vehicle.type,        // vehicle type
                    vehicle.mockSpeed,   // Current speed
                    vehicle.direction    // Direction of travel
                };

                // Add the violation to the queue
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    violationQueue.push(violation);
                }

                // Notify the challan thread
                violationNotifier.notify_one();
            }
        }

        // Move vehicles
        float deltaTime = moveClock.restart().asSeconds();
        for (size_t i = 0; i < vehicles.size(); ++i) {
            bool canMove = true;
            bool canEmergencyPass = true;
            
            // Implement turning logic after crossnig signal
            if (vehicles[i].direction == "NORTH" && vehicles[i].sprite.getPosition().y > 350) {
                // Randomly decide the next direction
                int turn = std::rand() % 3; // 0 = LEFT, 1 = STRAIGHT, 2 = RIGHT
                if (turn == 0) { // Turn LEFT
                    vehicles[i].direction = "TURN_EAST";
                
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(WEST_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                    }

                    vehicles[i].sprite.setRotation(-90); // Update rotation for westward movement
                } else if (turn == 1) { // Go STRAIGHT
                    vehicles[i].direction = "TURN_NORTH";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(SOUTH_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1); // Reposition to west lane
                    }
                    
                } else if (turn == 2) { // Turn RIGHT
                    vehicles[i].direction = "TURN_WEST";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(EAST_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to west lane
                    }
                    vehicles[i].sprite.setRotation(90); // Update rotation for eastward movement
                }
                vehicles[i].hasTurned = true;                    
                continue; // Skip further checks for this car in the current frame
            }

            if (vehicles[i].direction == "SOUTH" && vehicles[i].sprite.getPosition().y < 650) {
                int turn = std::rand() % 3;
                if (turn == 0) {
                    vehicles[i].direction = "TURN_WEST";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(EAST_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to west lane
                    }
                    vehicles[i].sprite.setRotation(90);
                } else if (turn == 1) {
                    vehicles[i].direction = "TURN_SOUTH";if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(NORTH_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                    }
                } else if (turn == 2) {
                    vehicles[i].direction = "TURN_EAST";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(WEST_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                    }
                    vehicles[i].sprite.setRotation(-90);
                }
                vehicles[i].hasTurned = true;                    
                continue;
            }

            if (vehicles[i].direction == "EAST" && vehicles[i].sprite.getPosition().x < 650) {
                int turn = std::rand() % 3;
                if (turn == 0) {
                    vehicles[i].direction = "TURN_SOUTH";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(NORTH_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                    }
                    vehicles[i].sprite.setRotation(0);
                } else if (turn == 1) {
                    vehicles[i].direction = "TURN_EAST";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(WEST_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                    }
                    //vehicles[i].sprite.setRotation(90);
                } else if (turn == 2) {
                    vehicles[i].direction = "TURN_NORTH";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(SOUTH_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1); // Reposition to west lane
                    }
                    vehicles[i].sprite.setRotation(180);
                }
                vehicles[i].hasTurned = true;                    
                continue;
            }

            if (vehicles[i].direction == "WEST" && vehicles[i].sprite.getPosition().x > 350) {
                int turn = std::rand() % 3;
                if (turn == 0) {
                    vehicles[i].direction = "TURN_NORTH";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(SOUTH_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                    }
                    vehicles[i].sprite.setRotation(180);
                } else if (turn == 1) {
                    vehicles[i].direction = "TURN_WEST";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(EAST_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to west lane
                    }
                } else if (turn == 2) {
                    vehicles[i].direction = "TURN_SOUTH";
                    if (vehicles[i].type == "H") {
                        vehicles[i].sprite.setPosition(NORTH_TURN_LANE2); // Reposition to west lane
                    }
                    else {
                        vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                    }
                    vehicles[i].sprite.setRotation(0);
                }
                vehicles[i].hasTurned = true;                    
                continue;
            }

            // Scale vehicles based on type
            if (vehicles[i].type == "R") {
                vehicles[i].sprite.setScale(0.55f, 0.55f);
            } else if (vehicles[i].type == "E") {
                vehicles[i].sprite.setScale(0.55f, 0.55f);
            } else if (vehicles[i].type == "H") {
                vehicles[i].sprite.setScale(0.70f, 0.70f); 
            }

            

            // Check traffic light states and proximity to other vehicles
            if (vehicles[i].direction == "NORTH") {
                if (!northLight.canPass() && vehicles[i].sprite.getPosition().y >= 300) {
                    canMove = false; // Stop if at the traffic light
                }
                // Check whether emergency vehicle is top of the lane, if so let it move
                if (vehicles[i].type == "E") {
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "NORTH" && !vehicles[j].hasTurned && 
                            vehicles[j].sprite.getPosition().y > vehicles[i].sprite.getPosition().y) {
                            canEmergencyPass = false; // Another vehicle is ahead
                            break;
                        }
                    }
                }
                if (vehicles[i].type == "H") {
                    // Check for vehicle ahead
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "NORTH" && vehicles[j].type == "H" &&
                            vehicles[j].sprite.getPosition().y > vehicles[i].sprite.getPosition().y &&
                            vehicles[j].sprite.getPosition().y - vehicles[i].sprite.getPosition().y < 50.0f) { // Minimum gap
                            canMove = false;
                            break;
                        }
                    }
                }
                else {
                    // Check for vehicle ahead
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "NORTH" &&
                            vehicles[j].sprite.getPosition().y > vehicles[i].sprite.getPosition().y &&
                            vehicles[j].sprite.getPosition().y - vehicles[i].sprite.getPosition().y < 50.0f) { // Minimum gap
                            canMove = false;
                            break;
                        }
                    }
                }
                
            } else if (vehicles[i].direction == "SOUTH") {
                if (!southLight.canPass() && vehicles[i].sprite.getPosition().y <= 715) {
                    canMove = false;
                }
                // Check whether emergency vehicle is top of the lane, if so let it move
                if (vehicles[i].type == "E") {
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "SOUTH" && !vehicles[j].hasTurned && 
                            vehicles[j].sprite.getPosition().y < vehicles[i].sprite.getPosition().y) {
                            canEmergencyPass = false; // Another vehicle is ahead
                            break;
                        }
                    }
                }
                if (vehicles[i].type == "H") {
                    // Check for vehicle ahead
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "SOUTH" && vehicles[j].type == "H" &&
                            vehicles[j].sprite.getPosition().y < vehicles[i].sprite.getPosition().y &&
                            vehicles[i].sprite.getPosition().y - vehicles[j].sprite.getPosition().y < 50.0f) { // Minimum gap
                            canMove = false;
                            break;
                        }
                    }
                }
                else
                {    // Check for vehicle ahead
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "SOUTH" &&
                            vehicles[j].sprite.getPosition().y < vehicles[i].sprite.getPosition().y &&
                            vehicles[i].sprite.getPosition().y - vehicles[j].sprite.getPosition().y < 50.0f) { // Minimum gap
                            canMove = false;
                            break;
                        }
                    }
                }
            } else if (vehicles[i].direction == "EAST") {
                if (!eastLight.canPass() && vehicles[i].sprite.getPosition().x <= 700) {
                    canMove = false;
                }

                // Check whether emergency vehicle is top of the lane, if so let it move
                if (vehicles[i].type == "E") {
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "EAST" && !vehicles[j].hasTurned && 
                            vehicles[j].sprite.getPosition().x < vehicles[i].sprite.getPosition().x) {
                            canEmergencyPass = false; // Another vehicle is ahead
                            break;
                        }
                    }
                }
                if (vehicles[i].type == "H") {
                    // Check for vehicle ahead
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "EAST" && vehicles[j].type == "H" &&
                            vehicles[j].sprite.getPosition().x < vehicles[i].sprite.getPosition().x &&
                            vehicles[i].sprite.getPosition().x - vehicles[j].sprite.getPosition().x < 50.0f) { // Minimum gap
                            canMove = false;
                            break;
                        }
                    }
                }
                else
                {    // Check for vehicle ahead
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "EAST" &&
                            vehicles[j].sprite.getPosition().x < vehicles[i].sprite.getPosition().x &&
                            vehicles[i].sprite.getPosition().x - vehicles[j].sprite.getPosition().x < 50.0f) { // Minimum gap
                            canMove = false;
                            break;
                        }
                    }
                }
            } else if (vehicles[i].direction == "WEST") {
                if (!westLight.canPass() && vehicles[i].sprite.getPosition().x >= 290) {
                    canMove = false;
                }

                // Check whether emergency vehicle is top of the lane, if so let it move
                if (vehicles[i].type == "E") {
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "WEST" && !vehicles[j].hasTurned && 
                            vehicles[j].sprite.getPosition().x > vehicles[i].sprite.getPosition().x) {
                            canEmergencyPass = false; // Another vehicle is ahead
                            break;
                        }
                    }
                }
                if (vehicles[i].type == "H") {
                    // Check for vehicle ahead
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "WEST" && vehicles[j].type == "H" &&
                            vehicles[j].sprite.getPosition().x > vehicles[i].sprite.getPosition().x &&
                            vehicles[j].sprite.getPosition().x - vehicles[i].sprite.getPosition().x < 50.0f) { // Minimum gap
                            canMove = false;
                            break;
                        }
                    }
                }
                else
                {    // Check for vehicle ahead
                    for (size_t j = 0; j < vehicles.size(); ++j) {
                        if (i != j && vehicles[j].direction == "WEST" &&
                            vehicles[j].sprite.getPosition().x > vehicles[i].sprite.getPosition().x &&
                            vehicles[j].sprite.getPosition().x - vehicles[i].sprite.getPosition().x < 50.0f) { // Minimum gap
                            canMove = false;
                            break;
                        }
                    }
                }
            }



            // Move the vehicle if allowed
            if (canMove || (canEmergencyPass && vehicles[i].type == "E")) {
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


        // Update timer text
        int minutes = static_cast<int>(elapsedTime) / 60;
        int seconds = static_cast<int>(elapsedTime) % 60;

        timerText.setString("Time Left: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s");

        // Clear window and draw
        window.clear();
        window.draw(backgroundSprite);
        window.draw(timerText); // Draw the timer

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

    challanThread.join();
    return 0;
}
