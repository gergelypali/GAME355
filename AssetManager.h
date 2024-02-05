#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <SDL_image.h>
#include <SDL_mixer.h>

class GameEngine;
class Animation;

class AssetManager
{
private:
    GameEngine* m_ge{nullptr};

    // maps to store the assets
    std::map<std::string, SDL_Texture*> m_textures;
    std::map<std::string, std::shared_ptr<Animation>> m_animations;
    std::map<std::string, Mix_Chunk*> m_sounds;
    std::map<std::string, Mix_Music*> m_musics;

public:
    AssetManager() = delete;
    AssetManager(GameEngine* ge);
    ~AssetManager();

    void AddTexture(const std::string& name, const std::string& pathToFile);
    void AddAnimation(
        const std::string& name,
        int animSpeed,
        const std::vector<std::pair<int,int>>& sequence
        );

    /// @brief Load audio files to buffer, only supports FLAC, MP3, Ogg, VOC, and WAV type. Use Sound for files that needs to be encoded in memory all the time.
    /// @param name Unique name of the sound file
    /// @param pathToFile Full path to the sound file
    void AddSound(const std::string& name, const std::string& pathToFile);

    /// @brief Load audio files to buffer, only supports FLAC, MP3, Ogg, VOC, and WAV type. Use Music for files that needs to be played for longer type, so encoding happends real time.
    /// @param name Unique name of the music file
    /// @param pathToFile Full path to the music file
    void AddMusic(const std::string& name, const std::string& pathToFile);

    SDL_Texture* GetTexture(const std::string& name);
    std::shared_ptr<Animation> GetAnimation(const std::string& name);
    Mix_Chunk* GetSound(const std::string& name);
    Mix_Music* GetMusic(const std::string& name);

};

#endif