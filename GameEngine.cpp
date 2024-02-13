#include "GameEngine.h"
#include "ScenePlay.h"
#include "SceneOne.h"
#include "SceneMenu.h"
#include "SceneEnd.h"
#include "Action.h"
#include "AssetManager.h"
#include <iostream>

void GameEngine::init()
{
    // TODO: load everything from a or multiple config files, everything means every config that is relevant to be in a configfile
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
    m_soundVolume = MIX_MAX_VOLUME / 20;
    m_musicVolume = MIX_MAX_VOLUME / 20;

    if (TTF_Init() < 0)
        printf("SDL_TTF init failed! With SDL_Error: %s\n", Mix_GetError());

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
    TTF_Quit();
    SDL_Quit();
}

void GameEngine::updateFPS(const double frameLength)
{
    FPS = std::min(MAXFPS, 1000.0 / frameLength);
    TICKS_PER_FRAME = 1000.0 / FPS;
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
            /* just for debugging and testing
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_U)
                changeScene("SceneOne");
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_I)
                changeScene("SceneMenu");
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_N)
                MAXFPS += 10;
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_M)
                MAXFPS -= 10;
            */
            
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                if(event.key.repeat)
                    continue;
                if (currentScene()->getActionMap().find(event.key.keysym.scancode) == currentScene()->getActionMap().end())
                    continue;
                const std::string type = (event.type == SDL_KEYDOWN) ? "START" : "END";
                currentScene()->doAction(Action(currentScene()->getActionMap()[(event.key.keysym.scancode)], type, event));
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
            {
                if (currentScene()->getActionMap().find(event.button.button) == currentScene()->getActionMap().end())
                    continue;
                const std::string type = (event.type == SDL_MOUSEBUTTONDOWN) ? "START" : "END";
                currentScene()->doAction(Action(currentScene()->getActionMap()[(event.button.button)], type, event));
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                if (currentScene()->getActionMap().find(event.motion.type) == currentScene()->getActionMap().end())
                    continue;
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
    else if (m_currentScene == "SceneOne")
        m_scenes[m_currentScene] = std::make_shared<SceneOne>(SceneOne(this));
}

void GameEngine::playSound(const std::string& name)
{
    Mix_Volume(-1, m_soundVolume);
    Mix_PlayChannel(-1, m_am->GetSound(name), 0);
}

void GameEngine::playMusic(const std::string& name)
{
    Mix_VolumeMusic(m_musicVolume);
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

void GameEngine::renderText(const std::string& textToRender, TTF_Font* font, const SDL_Color& color, int fontSize, const MATH::Vec2& pos)
{
    // TODO optimalization: static and dynamic text in different storage; so for static we are not destroying the texture every time but reuse it in every render cycle
    // in that way we can do the rendering faster; dynamic text is the usual, every time create a new texture and render it, or check if the text has changed, aaaaand
    // create a new texture in that case only

    TTF_SetFontSize(font, fontSize);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, textToRender.c_str(), color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer(), textSurface);
    SDL_SetTextureBlendMode(textTexture, SDL_BlendMode::SDL_BLENDMODE_BLEND);
    // can add alpha to text also
    //SDL_SetTextureAlphaMod(textTexture, textBufferMap[index].alpha);
    SDL_Rect rect;
    rect.x = pos.x - (textSurface->w / 2.0f);
    rect.y = pos.y - (textSurface->h / 2.0f);
    rect.w = textSurface->w;
    rect.h = textSurface->h;

    SDL_RenderCopy(renderer(), textTexture, NULL, &rect);

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}
