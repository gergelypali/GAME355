#include "AssetManager.h"
#include "GameEngine.h"
#include "Animation.h"

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

AssetManager::AssetManager(GameEngine* ge)
{
    m_ge = ge;

    // hardcode the assets we are going to load and stuff
    AddTexture("spriteStackPurpleCar", "textures/PurpleCar.png");
    AddTexture("spriteStackRedMotorcycle", "textures/RedMotorcycle.png");
    AddTexture("spriteStackGreenBigCar", "textures/GreenBigCar.png");
    AddTexture("spriteStackWhiteMotorcycle", "textures/WhiteMotorcycle.png");
    AddTexture("spriteStackBrownMotorcycle", "textures/BrownMotorcycle.png");
    AddTexture("spriteStackGreenCar", "textures/GreenCar.png");
    AddTexture("spriteStackBlueCar", "textures/BlueCar.png");
    AddTexture("spriteStackYellowCar", "textures/YellowCar.png");
    AddTexture("spriteStackRedCar", "textures/RedCar.png");
    AddTexture("characters", "textures/characters.png");
    AddTexture("roadParts", "textures/roads.png");
    AddTexture("menuBG", "textures/menuBG.jpg");
    AddTexture("startGameButton", "textures/newGameButtonText.png");
    AddTexture("startGameButtonAnim", "textures/newGameButtonAnim.png");
    AddTexture("exitGameButton", "textures/exitGameButtonText.png");
    AddTexture("exitGameButtonAnim", "textures/exitGameButtonAnim.png");
    AddTexture("retryGameButtonAnim", "textures/retryGameButtonAnim.png");

    // and animations
    AddAnimation("walkDown", 20, std::vector<std::pair<int,int>>{std::pair{0, 0}, std::pair{0, 1}, std::pair{0, 2}, std::pair{0, 3}});
    AddAnimation("walkLeft", 20, std::vector<std::pair<int,int>>{std::pair{2, 0}, std::pair{2, 1}, std::pair{2, 2}, std::pair{2, 3}});
    AddAnimation("walkUp", 20, std::vector<std::pair<int,int>>{std::pair{4, 0}, std::pair{4, 1}, std::pair{4, 2}, std::pair{4, 3}});
    AddAnimation("walkRight", 20, std::vector<std::pair<int,int>>{std::pair{6, 0}, std::pair{6, 1}, std::pair{6, 2}, std::pair{6, 3}});

    // menu animations
    AddAnimation("menuButtonAnim", 10, std::vector<std::pair<int,int>>{std::pair{0, 0}, std::pair{0, 1}, std::pair{0, 2}, std::pair{1, 0}, std::pair{1, 1}, std::pair{1, 2}, std::pair{2, 0}, std::pair{2, 1}, std::pair{2, 2}});
}

AssetManager::~AssetManager()
{
    for (auto& [key, val]: m_textures)
    {
        SDL_DestroyTexture(val);
        val = nullptr;
    }
    for (auto& [key, val]: m_animations)
    {
        val.reset();
    }
    for (auto& [key, val]: m_sounds)
    {
        Mix_FreeChunk(val);
        val = nullptr;
    }
    for (auto& [key, val]: m_musics)
    {
        Mix_FreeMusic(val);
        val = nullptr;
    }
}

void AssetManager::AddTexture(const std::string &name, const std::string &pathToFile)
{
    SDL_Texture* textureToAdd{nullptr};
    textureToAdd = IMG_LoadTexture(m_ge->renderer(), pathToFile.c_str());
    if (!textureToAdd)
    {
        printf("IMG_LoadTexture failed! Cannot load image from: %s\nWith SDL_Error: %s\n", pathToFile.c_str(), SDL_GetError());
        return;
    }
    m_textures.insert({name, textureToAdd});
}

void AssetManager::AddAnimation(const std::string &name, int animSpeed, const std::vector<std::pair<int, int>> &sequence)
{
    auto newAnim = std::make_shared<Animation>(animSpeed, sequence);
    m_animations[name] = newAnim;
}

void AssetManager::AddSound(const std::string &name, const std::string &pathToFile)
{
    Mix_Chunk* sound = Mix_LoadWAV(pathToFile.c_str());
    if (sound != nullptr)
    {
        m_sounds[name] = sound;
        return;
    }

    printf("Cannot load sound file %s! With SDL_Error: %s\n", pathToFile, SDL_GetError());
}

void AssetManager::AddMusic(const std::string &name, const std::string &pathToFile)
{
    Mix_Music* music = Mix_LoadMUS(pathToFile.c_str());
    if (music != nullptr)
    {
        m_musics[name] = music;
        return;
    }

    printf("Cannot load music file %s! With SDL_Error: %s\n", pathToFile, SDL_GetError());
}

SDL_Texture* AssetManager::GetTexture(const std::string &name)
{
    return m_textures[name];
}

std::shared_ptr<Animation> AssetManager::GetAnimation(const std::string &name)
{
    return m_animations[name];
}

Mix_Chunk *AssetManager::GetSound(const std::string &name)
{
    return m_sounds[name];
}

Mix_Music *AssetManager::GetMusic(const std::string &name)
{
    return m_musics[name];
}
