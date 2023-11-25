#include <iostream>
#include <vector>
#include <SDL.h>
#include <glm.hpp>
#include <gtx/norm.hpp>

// Define window dimensions and other constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float GLOBAL_ANIMATION_SPEED = 0.1f;

// Rectangle structure definition
struct Rectangle {
    glm::vec2 center;
    int width;
    int height;
    glm::vec2 direction;
    float speed;
    glm::vec3 color;
    float timeSinceLastShot; // Add a timeSinceLastShot attribute

    SDL_Rect getSDLRect() {
        SDL_Rect rect;
        rect.x = static_cast<int>(center.x - width / 2.0f);
        rect.y = static_cast<int>(center.y - height / 2.0f);
        rect.w = width;
        rect.h = height;
        return rect;
    }
};

// Player rectangle
Rectangle player;

// Bullet structure definition
struct Bullet {
    bool operator==(const Bullet& other) const {
        return position == other.position && width == other.width && height == other.height &&
            speed == other.speed && color == other.color;
    }

    glm::vec2 position;
    int width;
    int height;
    float speed;
    glm::vec3 color;

    SDL_Rect getSDLRect() {
        SDL_Rect rect;
        rect.x = static_cast<int>(position.x - width / 2.0f);
        rect.y = static_cast<int>(position.y - height / 2.0f);
        rect.w = width;
        rect.h = height;
        return rect;
    }
};

// Vector to store bullets
std::vector<Bullet> bullets;

// Enemy structure definition
struct Enemy {
    bool operator==(const Enemy& other) const {
        return position == other.position && width == other.width && height == other.height &&
            direction == other.direction && speed == other.speed && color == other.color;
    }

    glm::vec2 position;
    int width;
    int height;
    glm::vec2 direction;
    float speed;
    float originalSpeed;
    glm::vec3 color;
    float timeSinceLastMove;
    int live;

    SDL_Rect getSDLRect() {
        SDL_Rect rect;
        rect.x = static_cast<int>(position.x - width / 2.0f);
        rect.y = static_cast<int>(position.y - height / 2.0f);
        rect.w = width;
        rect.h = height;
        return rect;

    }
};

// Vector to store enemies
std::vector<Enemy> enemies;

// SDL window and renderer
SDL_Window* window = nullptr;
SDL_Renderer* windowRenderer = nullptr;
SDL_Event currentEvent;
bool quit = false;

// Function to initialize SDL and the game window
bool initWindow() {
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cout << "SDL initialization failed" << std::endl;
        success = false;
    }
    else {
        window = SDL_CreateWindow(
            "Space Invaders",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN);

        if (window == nullptr) {
            std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
            success = false;
        }
        else {
            windowRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

            if (windowRenderer == nullptr) {
                std::cout << "Failed to create the renderer: " << SDL_GetError() << std::endl;
                success = false;
            }
            else {
                SDL_SetRenderDrawColor(windowRenderer, 0, 0, 0, 255);
                SDL_RenderClear(windowRenderer);
            }
        }
    }

    return success;
}

// Function to process events (keyboard input)
void processEvents() {
    while (SDL_PollEvent(&currentEvent) != 0) {
        if (currentEvent.type == SDL_QUIT) {
            quit = true;
        }

        if (currentEvent.type == SDL_KEYDOWN) {
            switch (currentEvent.key.keysym.sym) {
            case SDLK_LEFT:
                player.direction.x = -0.1f;
                break;
            case SDLK_RIGHT:
                player.direction.x = 0.1f;
                break;
            case SDLK_SPACE:
                if (player.timeSinceLastShot >= 100.0f) { // 1000 milliseconds = 1 second
                    // Fire a bullet
                    Bullet bullet;
                    bullet.position = player.center;
                    bullet.width = 5;
                    bullet.height = 5;
                    bullet.speed = 1;
                    bullet.color = glm::vec3(255, 165, 0);
                    bullets.push_back(bullet);

                    // Reset the timer for shooting
                    player.timeSinceLastShot = 0.0f;
                }
                break;
            }
        }

        if (currentEvent.type == SDL_KEYUP) {
            switch (currentEvent.key.keysym.sym) {
            case SDLK_LEFT:
            case SDLK_RIGHT:
                player.direction.x = 0.0f;
                break;
            }
        }
    }
}

void updateGame() {
    // Update player position
    player.center += player.speed * GLOBAL_ANIMATION_SPEED * player.direction;

    // Update bullets
    for (auto& bullet : bullets) {
        bullet.position.y -= bullet.speed * GLOBAL_ANIMATION_SPEED;

        // Check for collisions with enemies
        for (auto& enemy : enemies) {
            if (glm::distance(bullet.position, enemy.position) < (bullet.width + enemy.width) / 2.0f) {
                // Remove the enemy
                enemy.live--;
                if (enemy.live == 3) {
                    enemy.color = glm::vec3(0, 255, 0); // Red color
                }
                else if (enemy.live == 2) {
                    enemy.color = glm::vec3(255, 255, 0); // Red color
                }
                else if (enemy.live == 1) {
                    enemy.color = glm::vec3(255, 0, 0); // Red color
                }
                else if (enemy.live == 0) {
                    enemies.erase(std::remove(enemies.begin(), enemies.end(), enemy), enemies.end());

                }
                // Remove the bullet
                bullets.erase(std::remove(bullets.begin(), bullets.end(), bullet), bullets.end());
                break;
            }
        }

        // Check if the bullet is out of the window
        if (bullet.position.y < 0) {
            bullets.erase(std::remove(bullets.begin(), bullets.end(), bullet), bullets.end());
        }
    }

    // Update enemies
    for (auto& enemy : enemies) {
        // Move enemies horizontally
        enemy.position.x += enemy.speed * GLOBAL_ANIMATION_SPEED * enemy.direction.x;

        // Oscillate enemies vertically every 5 seconds
        enemy.timeSinceLastMove += GLOBAL_ANIMATION_SPEED;
        if (enemy.timeSinceLastMove > 800) {
            enemy.position.y += 4.0f;  // Adjust this value to change the vertical oscillation distance
            enemy.direction.x = -enemy.direction.x;  // Reverse horizontal direction
            enemy.timeSinceLastMove = 0.0f;  // Reset the timer
        }

        // Check for collisions with the player
        if (player.center.y - player.height / 2 < enemy.position.y + enemy.height / 2) {
            std::cout << "Game Over!" << std::endl;
            SDL_Delay(3000);
            quit = true;
        }
    }

    // Adjust enemy speed to get a bit closer over time
    for (auto& enemy : enemies) {
        enemy.speed = enemy.originalSpeed + 0.0001f;  // Adjust this value to change the speed increase
    }
    player.timeSinceLastShot += GLOBAL_ANIMATION_SPEED;
}


// Function to render the game
void renderGame() {
    // Clear the screen
    SDL_SetRenderDrawColor(windowRenderer, 255, 255, 255, 255);
    SDL_RenderClear(windowRenderer);

    // Render player
    SDL_SetRenderDrawColor(windowRenderer, 0, 0, 0, 255);
    SDL_Rect playerRect = player.getSDLRect();
    SDL_RenderFillRect(windowRenderer, &playerRect);

    // Render bullets
    for (auto& bullet : bullets) {
        SDL_SetRenderDrawColor(windowRenderer, static_cast<int>(bullet.color.r), static_cast<int>(bullet.color.g),
            static_cast<int>(bullet.color.b), 255);
        SDL_Rect bulletRect = bullet.getSDLRect();
        SDL_RenderFillRect(windowRenderer, &bulletRect);
    }

    // Render enemies
    for (auto& enemy : enemies) {
        SDL_SetRenderDrawColor(windowRenderer, static_cast<int>(enemy.color.r), static_cast<int>(enemy.color.g),
            static_cast<int>(enemy.color.b), 255);
        SDL_Rect enemyRect = enemy.getSDLRect();
        SDL_RenderFillRect(windowRenderer, &enemyRect);
    }

    // Update the window
    SDL_RenderPresent(windowRenderer);
}

// Function to clean up resources
void cleanup() {
    SDL_DestroyRenderer(windowRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!initWindow()) {
        std::cout << "Failed to initialize" << std::endl;
        return -1;
    }

    // Initialize player
    player.center.x = WINDOW_WIDTH / 2.0f;
    player.center.y = WINDOW_HEIGHT - 30;
    player.width = 60;
    player.height = 30;
    player.direction = glm::vec2(0.0f, 0.0f);
    player.speed = 3.0f;
    player.color = glm::vec3(255, 255, 255);

    // Initialize enemies
    for (int j = 0; j < 3; ++j) { // Add two more rows
        for (int i = 0; i < 6; ++i) {
            Enemy enemy;
            enemy.live = 3;
            enemy.position = glm::vec2(i * 100 + 50, j * 80 + 50); // Adjust the spacing between rows
            enemy.width = player.width/2;
            enemy.height = player.height;
            enemy.direction = glm::vec2(1.0f, 1.0f); // Move horizontally
            enemy.speed = 0.2f;  // Adjust this value to change the speed
            enemy.originalSpeed = enemy.speed;  // Store the original speed
            enemy.color = glm::vec3(0, 255, 0); // Red color
            enemy.timeSinceLastMove = 0.0f;  // Initialize time since the last move
            enemies.push_back(enemy);
        }
    }

    // Game loop
    while (!quit) {
        processEvents();
        updateGame();
        renderGame();
    }

    cleanup();
    return 0;
}