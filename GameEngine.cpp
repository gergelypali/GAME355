#include "GameEngine.h"
#include "ScenePlay.h"
#include "SceneMenu.h"
#include "SceneEnd.h"
#include "Action.h"
#include "AssetManager.h"
#include <iostream>

void GameEngine::init()
{
    if (SDL_Init(SDL_INIT_VIDEO || SDL_INIT_AUDIO) < 0)
        printf("SDL init failed! With SDL_Error: %s\n", SDL_GetError());

    m_window = SDL_CreateWindow("GameJamGame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 768, 0);
    if (!m_window)
        printf("SDL Window creation failed! With SDL_Error: %s\n", SDL_GetError());

    m_renderer = SDL_CreateRenderer( m_window, -1, SDL_RENDERER_ACCELERATED );
    if(!m_renderer)
        printf("SDL Renderer creation failed! With SDL_Error: %s\n", SDL_GetError());

    if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
        printf("SDL Image initialization failed! With SDL_Error: %s\n", SDL_GetError());

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 8, 2048) < 0)
        printf("MIX_openAudio failed! With SDL_Error: %s\n", Mix_GetError());
    m_soundVolume = MIX_MAX_VOLUME / 2;
    m_musicVolume = MIX_MAX_VOLUME / 2;

    m_am = std::make_shared<AssetManager>(this);
}

void GameEngine::quit()
{
    for (auto& [key, value] : m_scenes)
    {
        value.reset();
    }

    SDL_DestroyRenderer(m_renderer);
    m_renderer = nullptr;
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

void GameEngine::run()
{
    changeScene("SceneMenu");

    SDL_Event event;

    while(m_running)
    {
        auto startTick = SDL_GetTicks();

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                m_running = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window))
                m_running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_Q)
                m_running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_U)
                changeScene("ScenePlay");
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_I)
                changeScene("SceneMenu");
            
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                if(event.key.repeat)
                    continue;
                if (currentScene()->getActionMap().find(event.key.keysym.scancode) == currentScene()->getActionMap().end())
                    continue;
                const std::string type = (event.type == SDL_KEYDOWN) ? "START" : "END";
                currentScene()->doAction(Action(currentScene()->getActionMap()[(event.key.keysym.scancode)], type));
            }
        }

        // update the current scene after input handling
        currentScene()->update();

        // check deltaTime, so we can force the FPS
        auto endTick = SDL_GetTicks();
        if (endTick - startTick < TICKS_PER_FRAME)
            SDL_Delay(TICKS_PER_FRAME - (endTick - startTick));
    }
}

void GameEngine::changeScene(std::string newScene)
{
    // check if we try to change to the current scene
    if (m_currentScene == newScene)
        return;

    // if we delete here the shared ptr, it will call the destructor of the scene
    m_scenes.erase(m_currentScene);
    m_currentScene = newScene;
    if (m_currentScene == "ScenePlay")
        m_scenes[m_currentScene] = std::make_shared<ScenePlay>(ScenePlay(this));
    else if (m_currentScene == "SceneMenu")
        m_scenes[m_currentScene] = std::make_shared<SceneMenu>(SceneMenu(this));
    else if (m_currentScene == "SceneEnd")
        m_scenes[m_currentScene] = std::make_shared<SceneEnd>(SceneEnd(this));
}

void GameEngine::playSound(const std::string& name)
{
    Mix_PlayChannel(-1, m_am->GetSound(name), 0);
}

void GameEngine::playMusic(const std::string& name)
{
    Mix_PlayMusic(m_am->GetMusic(name), -1);
}

void GameEngine::stopMusic()
{
    Mix_HaltMusic();
}

void GameEngine::changeSoundsVolume(int changeBy)
{
    m_soundVolume = std::max(0, m_soundVolume + MIX_MAX_VOLUME * changeBy / 100);
    Mix_Volume(-1, m_soundVolume);
    m_soundVolume = Mix_Volume(-1, -1);
}

void GameEngine::changeMusicVolume(int changeBy)
{
    m_musicVolume = std::max(0, m_musicVolume + MIX_MAX_VOLUME * changeBy / 100);
    Mix_VolumeMusic(m_musicVolume);
    m_musicVolume = Mix_VolumeMusic(-1);
}
