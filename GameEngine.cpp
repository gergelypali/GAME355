#include "GameEngine.h"
#include "ScenePlay.h"
#include "SceneOne.h"
#include "SceneMenu.h"
#include "SceneEnd.h"
#include "VulkanScene1.h"
#include "VulkanSceneMenu.h"
#include "Action.h"
#include "AssetManager.h"
#include <iostream>

#include "Logger.h"
#include "VulkanRenderer.h"

void GameEngine::init()
{
    Logger::Instance()->setLogLevel(Logger::severity::ERROR);
    Logger::Instance()->log("GameEngine init Start");
    // TODO: load everything from a or multiple config files, everything means every config that is relevant to be in a configfile
    if (SDL_Init(SDL_INIT_VIDEO || SDL_INIT_AUDIO) < 0)
    {
        Logger::Instance()->logCritical(strcat("SDL init failed! With SDL_Error: ", SDL_GetError()));
        // TODO: figure out a way to abort the program loading if we have a critical error like this
    }

    Logger::Instance()->logVerbose("GameEngine init 1");
    if (SDLRenderer)
    {
        Logger::Instance()->logVerbose("GameEngine init SDL window create");
        m_window = SDL_CreateWindow("GameSDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_windowX, m_windowY, 0);
        if (!m_window)
        {
            Logger::Instance()->logCritical(strcat("SDL Window with SDL creation failed! With SDL_Error: ", SDL_GetError()));
        }

        m_SDLRenderer = SDL_CreateRenderer( m_window, -1, SDL_RENDERER_ACCELERATED );
        if (!m_SDLRenderer)
        {
            Logger::Instance()->logCritical(strcat("SDL Renderer creation failed! With SDL_Error: ", SDL_GetError()));
        }
    }
    else
    {
        Logger::Instance()->logVerbose("GameEngine init Vulkan window create");
        m_window = SDL_CreateWindow("GameVulkan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_windowX, m_windowY, SDL_WINDOW_VULKAN);
        if (!m_window)
        {
            Logger::Instance()->logCritical(strcat("SDL Window with Vulkan creation failed! With SDL_Error: ", SDL_GetError()));
        }
        m_vulkanRenderer = new VulkanRenderer(m_window);
    }

    Logger::Instance()->logVerbose("GameEngine init 2");
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        Logger::Instance()->logCritical(strcat("SDL Image initialization failed! With SDL_Error: ", SDL_GetError()));
    }

    Logger::Instance()->logVerbose("GameEngine init 3");
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 8, 2048) < 0)
    {
        Logger::Instance()->logCritical(strcat("MIX_openAudio failed! With SDL_Error: ", SDL_GetError()));
    }

    Logger::Instance()->logVerbose("GameEngine init 4");
    m_soundVolume = MIX_MAX_VOLUME / 20;
    Logger::Instance()->logVerbose("m_soundVolume = " + std::to_string(m_soundVolume));
    m_musicVolume = MIX_MAX_VOLUME / 20;
    Logger::Instance()->logVerbose("m_musicVolume = " + std::to_string(m_musicVolume));

    Logger::Instance()->logVerbose("GameEngine init 5");
    if (TTF_Init() < 0)
    {
        Logger::Instance()->logCritical(strcat("SDL_TTF init failed! With SDL_Error: ", SDL_GetError()));
    }

    Logger::Instance()->logVerbose("GameEngine init 6");
    m_am = std::make_shared<AssetManager>(this);
    Logger::Instance()->log("GameEngine init End");
}

void GameEngine::quit()
{
    m_am.reset();

    Logger::Instance()->log("GameEngine quit Start");
    for (auto& [key, value] : m_scenes)
    {
        value.reset();
    }
    Logger::Instance()->logVerbose("GameEngine quit 1");
    SDL_DestroyRenderer(m_SDLRenderer);
    m_SDLRenderer = nullptr;
    Logger::Instance()->logVerbose("GameEngine quit 2");
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
    Logger::Instance()->logVerbose("GameEngine quit 3");
    IMG_Quit();
    Logger::Instance()->logVerbose("GameEngine quit 4");
    Mix_Quit();
    Logger::Instance()->logVerbose("GameEngine quit 5");
    TTF_Quit();
    Logger::Instance()->logVerbose("GameEngine quit 6");
    SDL_Quit();
    Logger::Instance()->log("GameEngine quit End");

    delete(m_vulkanRenderer);
}

void GameEngine::updateFPS(const double frameLength)
{
    FPS = std::min(MAXFPS, 1000.0 / frameLength);
    TICKS_PER_FRAME = 1000.0 / FPS;
}

void GameEngine::run()
{
    Logger::Instance()->log("GameEngine run Start");
    changeScene("VulkanSceneMenu");

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
            
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                Logger::Instance()->log("SDL Key event Start");
                Logger::Instance()->logVerbose("SDL_EventType = " + std::to_string(event.type));
                Logger::Instance()->logVerbose("SDL_Scancode = " + std::to_string(event.key.keysym.scancode));
                if(event.key.repeat)
                {
                    Logger::Instance()->log("SDL Key event Repeat");
                    continue;
                }
                if (currentScene()->getActionMap().find(event.key.keysym.scancode) == currentScene()->getActionMap().end())
                {
                    Logger::Instance()->log("SDL Key event NotFound");
                    continue;
                }
                const std::string type = (event.type == SDL_KEYDOWN) ? "START" : "END";
                currentScene()->doAction(Action(currentScene()->getActionMap()[(event.key.keysym.scancode)], type, event));
                Logger::Instance()->log("SDL Key event End");
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
            {
                Logger::Instance()->log("SDL Mouse Button event Start");
                Logger::Instance()->logVerbose("SDL_EventType = " + std::to_string(event.type));
                Logger::Instance()->logVerbose("SDL_MouseButtonEvent = " + std::to_string(event.button.button));
                if (currentScene()->getActionMap().find(event.button.button) == currentScene()->getActionMap().end())
                {
                    Logger::Instance()->log("SDL Mouse Button event NotFound");
                    continue;
                }
                const std::string type = (event.type == SDL_MOUSEBUTTONDOWN) ? "START" : "END";
                currentScene()->doAction(Action(currentScene()->getActionMap()[(event.button.button)], type, event));
                Logger::Instance()->log("SDL Mouse Button event End");
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                if (currentScene()->getActionMap().find(event.motion.type) == currentScene()->getActionMap().end())
                {
                    continue;
                }
                const std::string type = "START";
                currentScene()->doAction(Action(currentScene()->getActionMap()[(event.motion.type)], type, event));
            }
        }

        // update the current scene after input handling
        currentScene()->update();

        // check deltaTime, so we can force the FPS
        auto endTick = SDL_GetTicks();
        auto frameLength{endTick - startTick};
        updateFPS(frameLength);
        if (frameLength < TICKS_PER_FRAME)
            SDL_Delay(TICKS_PER_FRAME - (frameLength));
    }
    Logger::Instance()->log("GameEngine run End");
}

void GameEngine::changeScene(std::string newScene)
{
    Logger::Instance()->log("GameEngine changeScene Start");
    Logger::Instance()->logVerbose("newScene = " + newScene);
    // check if we try to change to the current scene
    if (m_currentScene == newScene)
        return;

    // if we delete here the shared ptr, it will call the destructor of the scene
    Logger::Instance()->logVerbose("GameEngine changeScene 1");
    m_scenes.erase(m_currentScene);
    Logger::Instance()->logVerbose("GameEngine changeScene 2");
    m_currentScene = newScene;
    if (m_currentScene == "ScenePlay")
    {
        m_scenes[m_currentScene] = std::make_shared<ScenePlay>(ScenePlay(this));
        Logger::Instance()->logVerbose("GameEngine changeScene ScenePlay branch");
    }
    else if (m_currentScene == "SceneMenu")
    {
        m_scenes[m_currentScene] = std::make_shared<SceneMenu>(SceneMenu(this));
        Logger::Instance()->logVerbose("GameEngine changeScene SceneMenu branch");
    }
    else if (m_currentScene == "SceneEnd")
    {
        m_scenes[m_currentScene] = std::make_shared<SceneEnd>(SceneEnd(this));
        Logger::Instance()->logVerbose("GameEngine changeScene SceneEnd branch");
    }
    else if (m_currentScene == "SceneOne")
    {
        m_scenes[m_currentScene] = std::make_shared<SceneOne>(SceneOne(this));
        Logger::Instance()->logVerbose("GameEngine changeScene SceneOne branch");
    }
    else if (m_currentScene == "VulkanScene1")
    {
        m_scenes[m_currentScene] = std::make_shared<VulkanScene1>(VulkanScene1(this));
        Logger::Instance()->logVerbose("GameEngine changeScene VulkanScene1 branch");
    }
    else if (m_currentScene == "VulkanSceneMenu")
    {
        m_scenes[m_currentScene] = std::make_shared<VulkanSceneMenu>(VulkanSceneMenu(this));
        Logger::Instance()->logVerbose("GameEngine changeScene VulkanSceneMenu branch");
    }
    Logger::Instance()->log("GameEngine changeScene End");
}

void GameEngine::playSound(const std::string& name)
{
    Logger::Instance()->log("GameEngine playSound Start");
    Logger::Instance()->logVerbose("sound's name = " + name);
    Mix_Volume(-1, m_soundVolume);
    Mix_PlayChannel(-1, m_am->GetSound(name), 0);
    Logger::Instance()->log("GameEngine playSound End");
}

void GameEngine::playMusic(const std::string& name)
{
    Logger::Instance()->log("GameEngine playMusic Start");
    Logger::Instance()->logVerbose("music's name = " + name);
    Mix_VolumeMusic(m_musicVolume);
    Mix_PlayMusic(m_am->GetMusic(name), -1);
    Logger::Instance()->log("GameEngine playMusic End");
}

void GameEngine::stopMusic()
{
    Logger::Instance()->log("GameEngine stopMusic Start");
    Mix_HaltMusic();
    Logger::Instance()->log("GameEngine stopMusic End");
}

void GameEngine::changeSoundsVolume(int changeBy)
{
    Logger::Instance()->log("GameEngine changeSoundsVolume Start");
    m_soundVolume = std::max(0, m_soundVolume + MIX_MAX_VOLUME * changeBy / 100);
    Mix_Volume(-1, m_soundVolume);
    m_soundVolume = Mix_Volume(-1, -1);
    Logger::Instance()->log("GameEngine changeSoundsVolume End");
}

void GameEngine::changeMusicVolume(int changeBy)
{
    Logger::Instance()->log("GameEngine changeMusicVolume Start");
    m_musicVolume = std::max(0, m_musicVolume + MIX_MAX_VOLUME * changeBy / 100);
    Mix_VolumeMusic(m_musicVolume);
    m_musicVolume = Mix_VolumeMusic(-1);
    Logger::Instance()->log("GameEngine changeMusicVolume End");
}

void GameEngine::renderText(const std::string& textToRender, TTF_Font* font, const SDL_Color& color, int fontSize, const MATH::Vec2& pos)
{
    // TODO optimalization: static and dynamic text in different storage; so for static we are not destroying the texture every time but reuse it in every render cycle
    // in that way we can do the rendering faster; dynamic text is the usual, every time create a new texture and render it, or check if the text has changed, aaaaand
    // create a new texture in that case only

    TTF_SetFontSize(font, fontSize);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, textToRender.c_str(), color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer(), textSurface);
    SDL_SetTextureBlendMode(textTexture, SDL_BlendMode::SDL_BLENDMODE_BLEND);
    SDL_Rect rect;
    rect.x = pos.x - (textSurface->w / 2.0f);
    rect.y = pos.y - (textSurface->h / 2.0f);
    rect.w = textSurface->w;
    rect.h = textSurface->h;

    SDL_RenderCopy(renderer(), textTexture, NULL, &rect);

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}
