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

enum class AppState { MENU, SIMULATION, CHALLAN_VIEW, USER_PORTAL, PAY_CHALLAN, EXIT };

const sf::Vector2f NORTH_SPAWN_REGULAR_LANE1(522, 0);    // Starting from top-center
const sf::Vector2f SOUTH_SPAWN_REGULAR_LANE1(403, 1000); // Starting from bottom-center
const sf::Vector2f EAST_SPAWN_REGULAR_LANE1(1000, 533);  // Starting from right-center
const sf::Vector2f WEST_SPAWN_REGULAR_LANE1(0, 410);     // Starting from left-center

const sf::Vector2f NORTH_TURN_LANE1(405, 278); // Starting from top-center
const sf::Vector2f SOUTH_TURN_LANE1(523, 715); // Done
const sf::Vector2f EAST_TURN_LANE1(700, 467);  // DONE
const sf::Vector2f WEST_TURN_LANE1(290, 535);  // Starting from left-center


const sf::Vector2f NORTH_SPAWN_HEAVY_LANE2(580, 0);     // Starting from top-center
const sf::Vector2f SOUTH_SPAWN_HEAVY_LANE2(450, 1000);  // Starting from bottom-center
const sf::Vector2f EAST_SPAWN_HEAVY_LANE2(1000, 590);   // Starting from right-center
const sf::Vector2f WEST_SPAWN_HEAVY_LANE2(0, 460);      // Starting from left-center


const sf::Vector2f NORTH_TURN_LANE2(450, 278);    // Starting from top-center
const sf::Vector2f SOUTH_TURN_LANE2(575, 715);    // Done
const sf::Vector2f EAST_TURN_LANE2(700, 410);     // DONE
const sf::Vector2f WEST_TURN_LANE2(290, 590);     // Starting from left-center

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

// Function to format the time as a string
std::string formatTime(const std::time_t& time) {
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    return std::string(buffer);
}

// Function to generate the current date as a string
std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    return formatTime(now_time);
}

// Function to calculate the due date (3 days from now)
std::string getDueDate(int daysToAdd = 3) {
    auto now = std::chrono::system_clock::now();
    auto due_time = now + std::chrono::hours(daysToAdd * 24);
    std::time_t due_time_t = std::chrono::system_clock::to_time_t(due_time);
    return formatTime(due_time_t);
}

int generateMockSpeed(std::string type) {
    if (type == "E"){
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dist(1, 75);
        return dist(gen);
    }
    else if (type == "R") {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dist(1, 55);
        return dist(gen);
    }
    else {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dist(1, 35);
        return dist(gen);
    }
    
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
    std::string status; //"Active" or "Inactive"
};

struct Challan {
    std::string challanID;
    std::string vehicleID;
    std::string status;
    std::string issueDate;
    std::string dueDate;
    float payableAmount;
};



// Function to generate a random challan ID
std::string generateChallanID() {
    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> charDist(0, 25); // Distribution for letters (A-Z)
    std::uniform_int_distribution<> numDist(0, 9);   // Distribution for digits (0-9)

    std::string challanID;

    // Generate 2 random uppercase letters
    for (int i = 0; i < 2; ++i) {
        char letter = 'A' + charDist(gen); // Convert to ASCII character
        challanID += letter;
    }

    // Generate 4 random digits
    for (int i = 0; i < 4; ++i) {
        char digit = '0' + numDist(gen); // Convert to ASCII character
        challanID += digit;
    }

    return challanID;
}

// Shared resources
std::queue<SpeedViolation> violationQueue;
std::mutex queueMutex;
std::condition_variable violationNotifier;
bool stopChallanThread = false;
std::queue<Challan> challans;

// Function to display the user portal
AppState showUserPortal(sf::RenderWindow& window, sf::Font& font) {
    std::string enteredVehicleID; // For capturing user input
    sf::Text inputText, resultText, promptText;

    // Prompt text
    promptText.setFont(font);
    promptText.setString("Enter your Vehicle ID:");
    promptText.setCharacterSize(24);
    promptText.setFillColor(sf::Color::White);
    promptText.setPosition(50, 50);

    // Input text
    inputText.setFont(font);
    inputText.setCharacterSize(24);
    inputText.setFillColor(sf::Color::Cyan);
    inputText.setPosition(50, 100);

    // Result text
    resultText.setFont(font);
    resultText.setCharacterSize(20);
    resultText.setFillColor(sf::Color::White);
    resultText.setPosition(50, 200);

    bool showResults = false; // To indicate if the results should be displayed
    std::string resultMessage; // Store the result message

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return AppState::MENU;
            }

            // Handle keyboard input for the vehicle ID
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b') { // Handle backspace
                    if (!enteredVehicleID.empty()) {
                        enteredVehicleID.pop_back();
                    }
                } else if (event.text.unicode == '\r') { // Handle enter
                    if (!enteredVehicleID.empty()) {
                        std::queue<Challan> tempQueue = challans; // Make a copy of the queue for iteration
                        bool found = false;

                        while (!tempQueue.empty()) {
                            const Challan& challan = tempQueue.front();
                            if (challan.vehicleID == enteredVehicleID) {
                                resultMessage = 
                                    "Challan Found!\n"
                                    "Challan ID: " + challan.challanID + "\n" +
                                    "Vehicle ID: " + challan.vehicleID + "\n" +
                                    "Status: " + challan.status + "\n" +
                                    "Issue Date: " + challan.issueDate + "\n" +
                                    "Due Date: " + challan.dueDate + "\n" +
                                    "Amount: $" + std::to_string(challan.payableAmount);
                                found = true;
                                break;
                            }
                            tempQueue.pop(); // Move to the next challan
                        }

                        if (!found) {
                            resultMessage = "No challan found for Vehicle ID: " + enteredVehicleID;
                        }

                        resultText.setString(resultMessage);
                        showResults = true; // Enable results display
                    }
                } else if (event.text.unicode < 128) { // Append printable characters
                    enteredVehicleID += static_cast<char>(event.text.unicode);
                }
            }

            // Handle Escape key to go back to the menu
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                return AppState::MENU; // Exit the user portal
            }
        }

        // Update input text
        inputText.setString(enteredVehicleID);

        // Render the portal
        window.clear(sf::Color::Black);
        window.draw(promptText);
        window.draw(inputText);
        if (showResults) {
            window.draw(resultText);
        }
        window.display();
    }

}

AppState showMenu(sf::RenderWindow& window, sf::Font& font) {
    // Menu options
    std::vector<std::string> options = {
        "Start Simulation",
        "View Challan Statuses",
        "User Portal",
        "Pay Challan",
        "Exit"
    };

    // Create SFML Text objects for each menu option
    std::vector<sf::Text> menuTexts;
    for (size_t i = 0; i < options.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(options[i]);
        text.setCharacterSize(36);
        text.setFillColor(sf::Color::White);
        text.setPosition(300, 200 + i * 60); // Position each option vertically
        menuTexts.push_back(text);
    }

    // Menu selection variables
    int selectedIndex = 0;
    menuTexts[selectedIndex].setFillColor(sf::Color::Cyan); // Highlight the first option

    // Menu loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return AppState::EXIT;
            }

            // Handle keyboard input for menu navigation
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    menuTexts[selectedIndex].setFillColor(sf::Color::White); // Reset current
                    selectedIndex = (selectedIndex - 1 + menuTexts.size()) % menuTexts.size(); // Move up
                    menuTexts[selectedIndex].setFillColor(sf::Color::Cyan); // Highlight new
                } else if (event.key.code == sf::Keyboard::Down) {
                    menuTexts[selectedIndex].setFillColor(sf::Color::White); // Reset current
                    selectedIndex = (selectedIndex + 1) % menuTexts.size(); // Move down
                    menuTexts[selectedIndex].setFillColor(sf::Color::Cyan); // Highlight new
                } else if (event.key.code == sf::Keyboard::Enter) {
                    // Return based on the selected option
                    if (selectedIndex == 0) return AppState::SIMULATION;
                    if (selectedIndex == 1) return AppState::CHALLAN_VIEW;
                    if (selectedIndex == 2) return AppState::USER_PORTAL;
                    if (selectedIndex == 3) return AppState::PAY_CHALLAN;
                    if (selectedIndex == 4) return AppState::EXIT;
                }
            }
        }

        // Draw the menu
        window.clear(sf::Color::Black);
        for (const auto& text : menuTexts) {
            window.draw(text);
        }
        window.display();
    }

    return AppState::EXIT;
}


AppState payChallan(sf::RenderWindow& window, sf::Font& font) {
    std::string enteredVehicleID, enteredChallanID, enteredAmount;
    sf::Text inputText, resultText, promptText, instructionsText;

    // Instructions text
    instructionsText.setFont(font);
    instructionsText.setString("Enter Vehicle ID, Challan ID, and Amount to Pay:");
    instructionsText.setCharacterSize(24);
    instructionsText.setFillColor(sf::Color::White);
    instructionsText.setPosition(50, 50);

    // Prompt text
    promptText.setFont(font);
    promptText.setCharacterSize(24);
    promptText.setFillColor(sf::Color::White);
    promptText.setPosition(50, 120);

    // Input text
    inputText.setFont(font);
    inputText.setCharacterSize(24);
    inputText.setFillColor(sf::Color::Cyan);
    inputText.setPosition(50, 160);

    // Result text
    resultText.setFont(font);
    resultText.setCharacterSize(20);
    resultText.setFillColor(sf::Color::White);
    resultText.setPosition(50, 240);

    std::string currentField = "VehicleID"; // Track which input field is being edited
    bool showResults = false;
    std::string resultMessage;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return AppState::MENU;
            }

            // Handle keyboard input for the current field
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b') { // Handle backspace
                    if (currentField == "VehicleID" && !enteredVehicleID.empty()) {
                        enteredVehicleID.pop_back();
                    } else if (currentField == "ChallanID" && !enteredChallanID.empty()) {
                        enteredChallanID.pop_back();
                    } else if (currentField == "Amount" && !enteredAmount.empty()) {
                        enteredAmount.pop_back();
                    }
                } else if (event.text.unicode < 128 && event.text.unicode != '\r') { // Append printable characters
                    if (currentField == "VehicleID") {
                        enteredVehicleID += static_cast<char>(event.text.unicode);
                    } else if (currentField == "ChallanID") {
                        enteredChallanID += static_cast<char>(event.text.unicode);
                    } else if (currentField == "Amount") {
                        enteredAmount += static_cast<char>(event.text.unicode);
                    }
                }
            }

            // Handle Enter key to move to the next field or submit
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                if (currentField == "VehicleID" && !enteredVehicleID.empty()) {
                    currentField = "ChallanID";
                } else if (currentField == "ChallanID" && !enteredChallanID.empty()) {
                    currentField = "Amount";
                } else if (currentField == "Amount" && !enteredAmount.empty()) {
                    // Perform the update operation
                    std::queue<Challan> tempQueue;
                    bool found = false;

                    while (!challans.empty()) {
                        Challan challan = challans.front();
                        challans.pop();

                        if (challan.vehicleID == enteredVehicleID && challan.challanID == enteredChallanID) {
                            if (std::stod(enteredAmount) == challan.payableAmount) {
                                challan.status = "Paid";
                                resultMessage = "Challan status updated to 'Paid' successfully!";
                                found = true;
                            } else {
                                resultMessage = "Invalid amount. Please enter the correct payable amount.";
                                found = true;
                            }
                        }
                        tempQueue.push(challan);
                    }

                    challans = tempQueue; // Restore the queue after processing

                    if (!found) {
                        resultMessage = "No matching challan found for the entered details.";
                    }

                    resultText.setString(resultMessage);
                    showResults = true;
                }
            }

            // Handle Escape key to go back to the menu
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                return AppState::MENU; // Exit the update screen
            }
        }

        // Update prompt text based on the current field
        if (currentField == "VehicleID") {
            promptText.setString("Vehicle ID: " + enteredVehicleID);
        } else if (currentField == "ChallanID") {
            promptText.setString("Challan ID: " + enteredChallanID);
        } else if (currentField == "Amount") {
            promptText.setString("Amount: " + enteredAmount);
        }

        // Render the screen
        window.clear(sf::Color::Black);
        window.draw(instructionsText);
        window.draw(promptText);
        if (showResults) {
            window.draw(resultText);
        }
        window.display();
    }
    return AppState::MENU;
}



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
            float amount = 0;
            if (violation.type == "R")
            {
                amount = 5000 + 0.17*5000;
            }
            else if (violation.type == "H")
            {
                amount = 7000 + 0.17*7000;
            }

            Challan challan = {
                generateChallanID(),
                violation.vehicleID,
                violation.status,
                getCurrentDate(),
                getDueDate(),
                amount
            };
            challans.push(challan);

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

void showChallanStatuses(sf::RenderWindow& window, sf::Font& font) {
    sf::Text title;
    title.setFont(font);
    title.setString("Challan Statuses:");
    title.setCharacterSize(36);
    title.setFillColor(sf::Color::White);
    title.setPosition(50, 50);

    // Generate challan list
    std::vector<sf::Text> challanTexts;
    std::queue<Challan> tempQueue = challans; // Copy the original queue

    int yOffset = 120;
    while (!tempQueue.empty()) {
        const auto& challan = tempQueue.front();

        // Prepare text for the challan
        sf::Text text;
        text.setFont(font);
        text.setString("ID: " + challan.challanID + " | Vehicle: " + challan.vehicleID + 
                       " | Status: " + challan.status + " | Due: " + challan.dueDate +
                       " | Amount: $" + std::to_string(challan.payableAmount));
        text.setCharacterSize(22);
        text.setFillColor(sf::Color::White);
        text.setPosition(20, yOffset);

        challanTexts.push_back(text);

        yOffset += 40;
        tempQueue.pop(); // Remove the processed challan from the temporary queue
    }

    float scrollOffset = 0.0f; // Initial scroll position

    // Display challan statuses
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                return;
            }

            // Handle scrolling with the mouse wheel
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) { // Scroll up
                    scrollOffset = std::min(scrollOffset + 20.0f, 0.0f);
                } else if (event.mouseWheelScroll.delta < 0) { // Scroll down
                    scrollOffset = std::max(scrollOffset - 20.0f, -static_cast<float>(yOffset - window.getSize().y + 50));
                }
            }
        }

        // Adjust positions based on the scroll offset
        for (size_t i = 0; i < challanTexts.size(); ++i) {
            challanTexts[i].setPosition(20, 120 + i * 40 + scrollOffset);
        }

        window.clear(sf::Color::Black);
        window.draw(title);
        for (const auto& text : challanTexts) {
            if (text.getPosition().y > 50 && text.getPosition().y < window.getSize().y) { // Only draw visible items
                window.draw(text);
            }
        }
        window.display();
    }
}


// IMPORTANT NOTES:
// I have used the scale of 1s in real life = 3s in my simulation for the spawning cars. As the sprites overlap if a wait of 1s is given

int main() {
    // Initialize intersection and SFML window
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Smart Traffic Simulation");

    AppState state = AppState::MENU;
    // Vehicle count by type for each direction
    int northRegularCount = 0, northEmergencyCount = 0, northHeavyCount = 0;
    int southRegularCount = 0, southEmergencyCount = 0, southHeavyCount = 0;
    int eastRegularCount = 0, eastEmergencyCount = 0, eastHeavyCount = 0;
    int westRegularCount = 0, westEmergencyCount = 0, westHeavyCount = 0;

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
    
    // Queues for each direction
    std::queue<VehicleSprite> northQueue;
    std::queue<VehicleSprite> southQueue;
    std::queue<VehicleSprite> eastQueue;
    std::queue<VehicleSprite> westQueue;
    
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
    float cycleDuration = 25.0f; // Total duration for one complete cycle (e.g., 10 seconds)

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
    timerText.setPosition(5, 700); // Top-left corner

    sf::Clock simulationClock;
    const float simulationDuration = 300.0f; // 5 minutes in seconds
    float elapsedTime;

    sf::Clock speedTimer;
    
    

    bool isSimulation = false;

    // Initiate the challan thread
    std::thread challanThread(challanProcessor);

    while (state != AppState::EXIT)
    {
        if (state == AppState::MENU)
        {
            state = showMenu(window, font);
        }
        else if (state == AppState::SIMULATION)
        {
            isSimulation = true;
            
            while (window.isOpen() && isSimulation) {
            sf::Event event;

            while (window.pollEvent(event)) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                    state = AppState::MENU; 
                    isSimulation = false;   
                    break;                 
                }

                if (event.type == sf::Event::Closed){
                    window.close();
                }
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
                vehicle.plateNumber = generateRandomPlate();
                vehicle.sprite.setPosition(NORTH_SPAWN_REGULAR_LANE1);
                vehicle.sprite.setRotation(180);
                vehicle.direction = "NORTH";
                vehicle.speed = 30.0f;
                vehicle.type = "E";
                vehicle.hasTurned = false;
                vehicle.mockSpeed = generateMockSpeed(vehicle.type);
                northQueue.push(vehicle);
                northEmergency = true;
                northEmergencyClock.restart();
            }

            if (southEmergencyClock.getElapsedTime().asSeconds() >= 6.0f && dis(gen) < 0.05) {
                VehicleSprite vehicle;
                vehicle.sprite.setTexture(emergencyCarTexture);
                vehicle.plateNumber = generateRandomPlate();
                vehicle.sprite.setPosition(SOUTH_SPAWN_REGULAR_LANE1);
                vehicle.direction = "SOUTH";
                vehicle.speed = 30.0f;
                vehicle.hasTurned = false;
                vehicle.type = "E";
                vehicle.mockSpeed = generateMockSpeed(vehicle.type);
                southQueue.push(vehicle);
                southEmergency = true;
                southEmergencyClock.restart();
            }

            if (eastEmergencyClock.getElapsedTime().asSeconds() >= 20.0f && dis(gen) < 0.1) {
                VehicleSprite vehicle;
                vehicle.sprite.setTexture(emergencyCarTexture);
                vehicle.plateNumber = generateRandomPlate();
                vehicle.sprite.setPosition(EAST_SPAWN_REGULAR_LANE1);
                vehicle.sprite.setRotation(-90);
                vehicle.direction = "EAST";
                vehicle.speed = 30.0f;
                vehicle.hasTurned = false;
                vehicle.type = "E";
                vehicle.mockSpeed = generateMockSpeed(vehicle.type);
                eastQueue.push(vehicle);
                eastEmergency = true;
                eastEmergencyClock.restart();
            }

            if (westEmergencyClock.getElapsedTime().asSeconds() >= 6.0f && dis(gen) < 0.3) {
                VehicleSprite vehicle;
                vehicle.sprite.setTexture(emergencyCarTexture);
                vehicle.plateNumber = generateRandomPlate();
                vehicle.sprite.setPosition(WEST_SPAWN_REGULAR_LANE1);
                vehicle.sprite.setRotation(90);
                vehicle.direction = "WEST";
                vehicle.speed = 30.0f;
                vehicle.type = "E";
                vehicle.hasTurned = false;
                vehicle.mockSpeed = generateMockSpeed(vehicle.type);
                westQueue.push(vehicle);
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
                vehicle.mockSpeed = generateMockSpeed(vehicle.type);
                northQueue.push(vehicle);
                northClock.restart();
            }

            if (southClock.getElapsedTime().asSeconds() >= 4.0f && !southEmergency) {
                VehicleSprite vehicle;
                vehicle.sprite.setTexture(regularCarTexture);
                vehicle.plateNumber = generateRandomPlate();
                vehicle.sprite.setPosition(SOUTH_SPAWN_REGULAR_LANE1);
                vehicle.direction = "SOUTH";
                vehicle.speed = 30.0f;
                vehicle.hasTurned = false;
                vehicle.type = "R";
                vehicle.mockSpeed = generateMockSpeed(vehicle.type);
                southQueue.push(vehicle);
                southClock.restart();
            }

            if (eastClock.getElapsedTime().asSeconds() >= 3.5f && !eastEmergency) {
                VehicleSprite vehicle;
                vehicle.sprite.setTexture(regularCarTexture);
                vehicle.plateNumber = generateRandomPlate();
                vehicle.sprite.setPosition(EAST_SPAWN_REGULAR_LANE1);
                vehicle.sprite.setRotation(-90);
                vehicle.direction = "EAST";
                vehicle.speed = 30.0f;
                vehicle.type = "R";
                vehicle.hasTurned = false;
                vehicle.mockSpeed = generateMockSpeed(vehicle.type);
                eastQueue.push(vehicle);
                eastClock.restart();
            }

            if (westClock.getElapsedTime().asSeconds() >= 4.0f && !westEmergency) {
                VehicleSprite vehicle;
                vehicle.sprite.setTexture(regularCarTexture);
                vehicle.sprite.setPosition(WEST_SPAWN_REGULAR_LANE1);
                vehicle.plateNumber = generateRandomPlate();
                vehicle.sprite.setRotation(90);
                vehicle.direction = "WEST";
                vehicle.speed = 30.0f;
                vehicle.hasTurned = false;
                vehicle.type = "R";
                vehicle.mockSpeed = generateMockSpeed(vehicle.type);
                westQueue.push(vehicle);
                westClock.restart();
            }
            int northCount, southCount, eastCount, westCount;
            northCount = southCount = eastCount = westCount = 0;

            for (const auto& vehicle : vehicles)
            {
                if (vehicle.direction == "NORTH" && !vehicle.hasTurned)
                {
                    northCount++;
                }
                else if (vehicle.direction == "SOUTH" && !vehicle.hasTurned)
                {
                    southCount++;
                }
                else if (vehicle.direction == "EAST" && !vehicle.hasTurned)
                {
                    eastCount++;
                }
                else if (vehicle.direction == "WEST" && !vehicle.hasTurned)
                {
                    westCount++;
                }
            }

            if (northCount <= 6)
            {
                if (!northQueue.empty()){
                    vehicles.push_back(northQueue.front());
                    northQueue.pop();
                }
            }

            if (southCount <= 6)
            {
                if (!southQueue.empty()){
                    vehicles.push_back(southQueue.front());
                    southQueue.pop();
                }
            }

            if (eastCount <= 5)
            {
                if (!eastQueue.empty()){
                    vehicles.push_back(eastQueue.front());
                    eastQueue.pop();
                }
            }

            if (westCount <= 5)
            {
                if (!westQueue.empty()){
                    vehicles.push_back(westQueue.front());
                    westQueue.pop();
                }
            }

            northEmergency = southEmergency = eastEmergency = westEmergency = false;
            
            
            // spawn heavy cars only during minute 2-3. For one minute
            if (elapsedTime >= 120 && elapsedTime <= 180)
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
                        vehicle.plateNumber = generateRandomPlate();
                        vehicle.mockSpeed = generateMockSpeed(vehicle.type);
                        vehicles.push_back(vehicle);
                    }
                    heavyCarClock.restart();
                }
            }
            else
            {
                heavyCarClock.restart();
            }

            updateTrafficLights(northLight, southLight, eastLight, westLight, trafficCycleClock, cycleDuration, 4);

            

            // Inside the update loop
            // I have updated the mock speed when the vehicle hasnt crossed the traffic lights
            if (speedTimer.getElapsedTime().asSeconds() >= 5.0f) {
                // Increase the mock speed of all vehicles
                for (auto& vehicle : vehicles) {
                    if (!vehicle.hasTurned)
                        vehicle.mockSpeed += 5; 
                }
                speedTimer.restart(); // Reset the timer
            }

            // implement the catching of vehicles here if they exceed the speed limit
            // Check for speed violations
            for (auto& vehicle : vehicles) {
                if (!vehicle.hasTurned){
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
                            vehicle.direction,    // Direction of travel
                            "Active"
                        };

                        // Add the violation to the queue
                        {
                            std::lock_guard<std::mutex> lock(queueMutex);
                            violationQueue.push(violation);
                        }
                        vehicle.mockSpeed = 0;
                        // Notify the challan thread
                        violationNotifier.notify_one();
                    }
                }
                
            }

            // Reset counts before recalculating
            northRegularCount = northEmergencyCount = northHeavyCount = 0;
            southRegularCount = southEmergencyCount = southHeavyCount = 0;
            eastRegularCount = eastEmergencyCount = eastHeavyCount = 0;
            westRegularCount = westEmergencyCount = westHeavyCount = 0;

            for (const auto& vehicle : vehicles) {
                if (vehicle.direction == "NORTH") {
                    if (vehicle.type == "R") ++northRegularCount;
                    else if (vehicle.type == "E") ++northEmergencyCount;
                    else if (vehicle.type == "H") ++northHeavyCount;
                } else if (vehicle.direction == "SOUTH") {
                    if (vehicle.type == "R") ++southRegularCount;
                    else if (vehicle.type == "E") ++southEmergencyCount;
                    else if (vehicle.type == "H") ++southHeavyCount;
                } else if (vehicle.direction == "EAST") {
                    if (vehicle.type == "R") ++eastRegularCount;
                    else if (vehicle.type == "E") ++eastEmergencyCount;
                    else if (vehicle.type == "H") ++eastHeavyCount;
                } else if (vehicle.direction == "WEST") {
                    if (vehicle.type == "R") ++westRegularCount;
                    else if (vehicle.type == "E") ++westEmergencyCount;
                    else if (vehicle.type == "H") ++westHeavyCount;
                }
            }

            sf::Text northText, southText, eastText, westText;

            // Set font and properties
            northText.setFont(font);
            northText.setCharacterSize(18);
            northText.setFillColor(sf::Color::White);
            northText.setPosition(750, 10); // Top-right corner

            southText.setFont(font);
            southText.setCharacterSize(18);
            southText.setFillColor(sf::Color::White);
            southText.setPosition(10, 900); // Bottom-left corner

            eastText.setFont(font);
            eastText.setCharacterSize(18);
            eastText.setFillColor(sf::Color::White);
            eastText.setPosition(750, 900); // Bottom-right corner

            westText.setFont(font);
            westText.setCharacterSize(18);
            westText.setFillColor(sf::Color::White);
            westText.setPosition(10, 10); // Top-left corner

            // Update west text
            westText.setString(
                "West:\n" +
                std::to_string(westRegularCount) + " Regular\n" +
                std::to_string(westEmergencyCount) + " Emergency\n" +
                std::to_string(westHeavyCount) + " Heavy\n"
            );

            // Update north text
            northText.setString(
                "North:\n" +
                std::to_string(northRegularCount) + " Regular\n" +
                std::to_string(northEmergencyCount) + " Emergency\n" +
                std::to_string(northHeavyCount) + " Heavy\n"
            );

            // Update south text
            southText.setString(
                "South:\n" +
                std::to_string(southRegularCount) + " Regular\n" +
                std::to_string(southEmergencyCount) + " Emergency\n" +
                std::to_string(southHeavyCount) + " Heavy\n"
            );

            // Update east text
            eastText.setString(
                "East:\n" +
                std::to_string(eastRegularCount) + " Regular\n" +
                std::to_string(eastEmergencyCount) + " Emergency\n" +
                std::to_string(eastHeavyCount) + " Heavy\n"
            );


            // Move vehicles
            float deltaTime = moveClock.restart().asSeconds();
            for (size_t i = 0; i < vehicles.size(); ++i) {
                bool canMove = true;
                bool canEmergencyPass = true;
                srand(time(0));
                
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
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(WEST_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                            }
                            
                        }

                        vehicles[i].sprite.setRotation(-90); // Update rotation for westward movement
                    } else if (turn == 1) { // Go STRAIGHT
                        vehicles[i].direction = "TURN_NORTH";
                        if (vehicles[i].type == "H") {
                            vehicles[i].sprite.setPosition(SOUTH_TURN_LANE2); // Reposition to west lane
                        }
                        else {
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(SOUTH_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1); // Reposition to west lane
                            }
                        }
                        
                    } else if (turn == 2) { // Turn RIGHT
                        vehicles[i].direction = "TURN_WEST";
                        if (vehicles[i].type == "H") {
                            vehicles[i].sprite.setPosition(EAST_TURN_LANE2); // Reposition to west lane
                        }
                        else {
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(EAST_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to west lane
                            }
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
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(EAST_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to west lane
                            }
                        }
                        vehicles[i].sprite.setRotation(90);
                    } else if (turn == 1) {
                        vehicles[i].direction = "TURN_SOUTH";if (vehicles[i].type == "H") {
                            vehicles[i].sprite.setPosition(NORTH_TURN_LANE2); // Reposition to west lane
                        }
                        else {
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(NORTH_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                            }
                        }
                    } else if (turn == 2) {
                        vehicles[i].direction = "TURN_EAST";
                        if (vehicles[i].type == "H") {
                            vehicles[i].sprite.setPosition(WEST_TURN_LANE2); // Reposition to west lane
                        }
                        else {
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(WEST_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                            }
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
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(NORTH_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                            }
                        }
                        vehicles[i].sprite.setRotation(0);
                    } else if (turn == 1) {
                        vehicles[i].direction = "TURN_EAST";
                        if (vehicles[i].type == "H") {
                            vehicles[i].sprite.setPosition(WEST_TURN_LANE2); // Reposition to west lane
                        }
                        else {
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(WEST_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(WEST_TURN_LANE1); // Reposition to west lane
                            }
                        }
                        //vehicles[i].sprite.setRotation(90);
                    } else if (turn == 2) {
                        vehicles[i].direction = "TURN_NORTH";
                        if (vehicles[i].type == "H") {
                            vehicles[i].sprite.setPosition(SOUTH_TURN_LANE2); // Reposition to west lane
                        }
                        else {
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(SOUTH_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1); // Reposition to west lane
                            }
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
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(SOUTH_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(SOUTH_TURN_LANE1); // Reposition to west lane
                            }
                        }
                        vehicles[i].sprite.setRotation(180);
                    } else if (turn == 1) {
                        vehicles[i].direction = "TURN_WEST";
                        if (vehicles[i].type == "H") {
                            vehicles[i].sprite.setPosition(EAST_TURN_LANE2); // Reposition to west lane
                        }
                        else {
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(EAST_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(EAST_TURN_LANE1); // Reposition to west lane
                            }
                        }
                    } else if (turn == 2) {
                        vehicles[i].direction = "TURN_SOUTH";
                        if (vehicles[i].type == "H") {
                            vehicles[i].sprite.setPosition(NORTH_TURN_LANE2); // Reposition to west lane
                        }
                        else {
                            if (elapsedTime < 120 || elapsedTime > 180)
                            {
                                int decide = std::rand() % 2;
                                if (decide == 0)
                                {
                                    vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                                }
                                else
                                {
                                    vehicles[i].sprite.setPosition(NORTH_TURN_LANE2); // Reposition to west lane
                                }
                            }
                            else
                            {
                                vehicles[i].sprite.setPosition(NORTH_TURN_LANE1); // Reposition to west lane
                            }
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


            window.draw(westText);
            window.draw(northText);
            window.draw(southText);
            window.draw(eastText);


            // Draw vehicles
            for (const auto& vehicle : vehicles) {
                window.draw(vehicle.sprite);
            }

            // Display updated window
            window.display();
            }
        }
        else if (state == AppState::CHALLAN_VIEW)
        {
            showChallanStatuses(window, font);
            state = AppState::MENU;
        }
        else if (state == AppState::USER_PORTAL)
        {
            state = showUserPortal(window, font);
        }
        else if (state == AppState::PAY_CHALLAN)
        {
            state = payChallan(window, font);
        }
    }


    challanThread.join();
    window.close();
    return 0;
}
