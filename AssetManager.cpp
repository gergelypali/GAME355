#include "AssetManager.h"
#include "GameEngine.h"
#include "Animation.h"
#include "VulkanRenderer.h"
#include "Logger.h"

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

AssetManager::AssetManager(GameEngine* ge)
{
    m_ge = ge;

    // hardcode the assets we are going to load and stuff
    AddTexture("steam", "textures/steam_logo.png");
    AddTexture("plane", "textures/red_plane.png");
    AddTexture("brick", "textures/brick_bg.png");
    AddTexture("startButton", "textures/start_button.png");
    AddTexture("exitButton", "textures/exit_button.png");

    // and animations
    AddAnimation("walkDown", 150, std::vector<std::pair<int,int>>{std::pair{0, 0}, std::pair{0, 1}, std::pair{0, 2}, std::pair{0, 3}});
    AddAnimation("walkLeft", 150, std::vector<std::pair<int,int>>{std::pair{2, 0}, std::pair{2, 1}, std::pair{2, 2}, std::pair{2, 3}});
    AddAnimation("walkUp", 150, std::vector<std::pair<int,int>>{std::pair{4, 0}, std::pair{4, 1}, std::pair{4, 2}, std::pair{4, 3}});
    AddAnimation("walkRight", 150, std::vector<std::pair<int,int>>{std::pair{6, 0}, std::pair{6, 1}, std::pair{6, 2}, std::pair{6, 3}});

    // menu animations
    AddAnimation("menuButtonAnim", 168, std::vector<std::pair<int,int>>{std::pair{0, 0}, std::pair{0, 1}, std::pair{0, 2}, std::pair{1, 0}, std::pair{1, 1}, std::pair{1, 2}, std::pair{2, 0}, std::pair{2, 1}, std::pair{2, 2}});

    // fonts
    AddFont("Nasa21", "fonts/Nasa21.ttf");

    // vertices and indices
    AddVertexBuffer("rectangleVertex", "./2dRectangleVertex.txt");
    AddVertexBuffer("wallsVertex", "./2dWallsVertex.txt");
    AddVertexBuffer("xarrowVertex", "./arrowVertex.txt");
    AddIndexBuffer("rectangleIndex", "./2dRectangleIndex.txt");
    AddIndexBuffer("triangleIndex", "./2dTriangleIndex.txt");
    AddIndexBuffer("newformIndex", "./2dNewFormIndex.txt");
    AddIndexBuffer("wallsNoneIndex", "./2dWallsNoneIndex.txt");
    AddIndexBuffer("wallsWestIndex", "./2dWallsWestIndex.txt");
    AddIndexBuffer("wallsNorthIndex", "./2dWallsNorthIndex.txt");
    AddIndexBuffer("wallsNorthWestIndex", "./2dWallsNorthWestIndex.txt");
    AddIndexBuffer("rightArrow", "./2drightArrowIndex.txt");
    AddIndexBuffer("leftArrow", "./2dleftArrowIndex.txt");
    AddIndexBuffer("downArrow", "./2ddownArrowIndex.txt");
    AddIndexBuffer("upArrow", "./2dupArrowIndex.txt");
}

AssetManager::~AssetManager()
{
    for (auto& [key, val]: m_textures)
    {
        SDL_DestroyTexture(val);
        val = nullptr;
    }
    m_textures.clear();
    for (auto& [key, val]: m_animations)
    {
        val.reset();
    }
    m_animations.clear();
    for (auto& [key, val]: m_sounds)
    {
        Mix_FreeChunk(val);
        val = nullptr;
    }
    m_sounds.clear();
    for (auto& [key, val]: m_musics)
    {
        Mix_FreeMusic(val);
        val = nullptr;
    }
    m_musics.clear();
    for (auto& [key, val]: m_fonts)
    {
        TTF_CloseFont(val);
        val = nullptr;
    }
    m_fonts.clear();
    for (auto& [key, val]: m_vertexBuffers)
    {
        m_ge->vulkanRenderer()->freeBuffer(val.buffer, val.bufferMemory);
    }
    m_vertexBuffers.clear();
    for (auto& [key, val]: m_indexBuffers)
    {
        m_ge->vulkanRenderer()->freeBuffer(val.buffer, val.bufferMemory);
    }
    m_indexBuffers.clear();
    for (auto& [key, val]: m_vulkanTextures)
    {
        m_ge->vulkanRenderer()->destroyImage(val.image, val.imageMemory, val.imageView);
    }
    m_vulkanTextures.clear();
}

void AssetManager::AddTexture(const std::string &name, const std::string &pathToFile)
{
    if (m_ge->isSDL())
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
    else
    {
        textureData textureToAdd{};

        void* pixels{nullptr};
        SDL_Surface* image = IMG_Load(pathToFile.c_str());
        pixels = image->pixels;
        int size = image->h * image->pitch;// in bytes
        textureToAdd.height = image->h;
        textureToAdd.width = image->w;

        if(!pixels)
        {
            Logger::Instance()->logError("AssetManager: cannot load texture to vulkanTexture: " + pathToFile);
            return;
        }

        m_ge->vulkanRenderer()->loadTexture(textureToAdd, pixels, size, image->w, image->h);

        m_vulkanTextures.insert({name, textureToAdd});
        SDL_FreeSurface(image);
    }
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

VkDescriptorSet &AssetManager::GetVulkanTexture(const std::string &name)
{
    return m_vulkanTextures[name].set;
}

MATH::Vec2 AssetManager::GetVulkanTextureSize(const std::string &name)
{
    return MATH::Vec2{m_vulkanTextures[name].width, m_vulkanTextures[name].height};
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

void AssetManager::AddFont(const std::string &name, const std::string &pathToFile)
{
    auto newFont = TTF_OpenFont(pathToFile.c_str(), m_initFontSize);
    if (newFont == nullptr)
    {
        printf("Cannot load font file %s! With SDL_Error: %s\n", pathToFile, SDL_GetError());
        return;
    }

    m_fonts[name] = newFont;
}

TTF_Font *AssetManager::GetFont(const std::string &name)
{
    return m_fonts[name];
}

void AssetManager::AddVertexBuffer(const std::string& name, const std::string& pathToFile)
{
    if (m_vertexBuffers.find(name)!=m_vertexBuffers.end())
        return;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    if (m_ge->vulkanRenderer()->load2dVertexBuffer(pathToFile, buffer, bufferMemory))
        m_vertexBuffers.insert({name, vulkanBufferData{buffer, bufferMemory, 0}});
}

VkBuffer &AssetManager::GetVertexBuffer(const std::string& name)
{
    if (m_vertexBuffers.find(name) == m_vertexBuffers.end())
        throw std::out_of_range("AssetManager: cannot get vertex asset named: " + name);
    return m_vertexBuffers[name].buffer;
}

int AssetManager::GetIndexSize(const std::string& name)
{
    if (m_indexBuffers.find(name) == m_indexBuffers.end())
        throw std::out_of_range("AssetManager: cannot get index asset named: " + name);
    return m_indexBuffers[name].size;
}

void AssetManager::AddIndexBuffer(const std::string& name, const std::string& pathToFile)
{
    if (m_indexBuffers.find(name)!=m_indexBuffers.end())
        return;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    int size{0};
    if (m_ge->vulkanRenderer()->loadIndexBuffer(pathToFile, buffer, bufferMemory, size))
        m_indexBuffers.insert({name, vulkanBufferData{buffer, bufferMemory, size}});
}

VkBuffer &AssetManager::GetIndexBuffer(const std::string& name)
{
    if (m_indexBuffers.find(name) == m_indexBuffers.end())
        throw std::out_of_range("AssetManager: cannot get index asset named: " + name);
    return m_indexBuffers[name].buffer;
}
