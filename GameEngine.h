#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <map>
#include <memory>
#include <string>
#include <chrono>
#include <random>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include "Vector.h"

class Scene;
class AssetManager;
class VulkanRenderer;

class GameEngine
{
private:
    double MAXFPS = 60.0;
    double FPS = MAXFPS;
    double TICKS_PER_FRAME = 1000.0 / FPS;

    // main game variables
    bool m_running{true};
    std::map<std::string, std::shared_ptr<Scene>> m_scenes;
    std::shared_ptr<AssetManager> m_am{nullptr};
    std::string m_currentScene = "NONE";

    // SDL variables
    SDL_Window* m_window{nullptr};
    SDL_Renderer* m_SDLRenderer{nullptr};

    // vulkan variables
    VulkanRenderer* m_vulkanRenderer{nullptr};

    bool SDLRenderer{false};

    // audio part
    int m_soundVolume{0};
    int m_musicVolume{0};

    // random number generators;
	std::mt19937 m_generator{std::chrono::system_clock::now().time_since_epoch().count()};
	std::uniform_real_distribution<float> m_uniformDistribution{0.0f, 1.0f};
	std::normal_distribution<float> m_normalDistribution{0.0f, 1.0f};
	std::uniform_real_distribution<float> m_uniformDistributionNegative{-1.0f, 1.0f};

    // main methods
    void init();
    void update();
    std::shared_ptr<Scene> currentScene() { return m_scenes[m_currentScene]; };
    void quit();
    void updateFPS(const double frameLength);

    // systems
    void sUserInput();

public:
    GameEngine(){ init(); };
    ~GameEngine(){ quit(); };

    bool isSDL() { return SDLRenderer; };

    // main public methods
    /// @brief start and run the main loop continuously
    void run();
    /// @brief exit the current scene and load a new one to use
    /// @param newScene name of the new scene to use
    void changeScene(std::string newScene);
    /// @brief get the assetmanager to use
    /// @return the assetmanager object
    std::shared_ptr<AssetManager> assetManager() { return m_am; };
    /// @brief get the active window
    /// @return the sdl window object
    SDL_Window* window() { return m_window; };
    /// @brief get the renderer
    /// @return the sdl renderer object
    SDL_Renderer* renderer() { return m_SDLRenderer; };

    VulkanRenderer* vulkanRenderer() { return m_vulkanRenderer; };

    /// @brief get that the game is running or not
    /// @return bool shows the game is running
    bool isRunning() { return m_running; };
    /// @brief call this to stop the game and exit
    void stopGameloop() { m_running = false; };

    // audio handling
    /// @brief play one time the given sound already loaded to the assetmanager
    /// @param name name of the stored sound to play
    void playSound(const std::string& name);
    /// @brief play music continuously until stopmusic is called
    /// @param name name of the stored music to play
    void playMusic(const std::string& name);
    /// @brief stop the actively playing music
    void stopMusic();

    /// @brief change the volume for sounds
    /// @param changeBy increase or decrease the volume by this percent
    void changeSoundsVolume(int changeBy);
    /// @brief change the volume for music
    /// @param changeBy increase or decrease the volume by this percent
    void changeMusicVolume(int changeBy);

    // random Generator methods
    /// @brief call the random number generator to give a uniform distributed number between 0 and 1
    /// @return float value of a random number
    float getUniform() { return m_uniformDistribution(m_generator); };
    /// @brief call the random number generator to give a normal distributed number around 0 with 1 range
    /// @return float value of a random number
    float getNormal() { return m_normalDistribution(m_generator); };
    /// @brief call the random number generator to give a uniform distributed number between -1 and 1
    /// @return float value of a random number
    float getUniformNegative() { return m_uniformDistributionNegative(m_generator); };

    // deltaTime getter
    /// @brief get the current length of a frame in ms
    /// @return ms value of the length
    const double getFrameTime() { return TICKS_PER_FRAME; };
    /// @brief get the actual fps of the game
    /// @return fps value
    const double getFPS() { return FPS; };

    /// @brief Render a given text to the screen
    void renderText(const std::string& textToRender, TTF_Font* font, const SDL_Color& color, int fontSize, const MATH::Vec2& pos);

};

#endif