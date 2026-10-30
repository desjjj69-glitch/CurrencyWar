#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <SDL3_mixer/SDL_mixer.h>

enum class AudioType {
	Click,
};

class AudioManager {
public:
	AudioManager() = default;
	~AudioManager();

	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

	// Loads config/Audio.json and remembers the mappings. No audio is decoded here.
	bool Init(const std::filesystem::path& gamePath, MIX_Mixer* mixer);
	void Shutdown();

	// Fire-and-forget one-shot sound (e.g. AudioType::Click).
	bool PlayOneShot(AudioType type);

	// Looping background/ambient audio identified by a JSON key, e.g. "Fight_Bgm1".
	bool PlayPersistent(const std::string& name);

	// Stops the named persistent audio if it is currently playing; otherwise no-op.
	void StopPersistent(const std::string& name);

private:
	std::filesystem::path AudioRoot() const;
	MIX_Audio* LoadEventAudio(AudioType type);
	MIX_Audio* LoadStreamAudio(const std::string& name);
	MIX_Track* GetOrCreateTrack(const std::string& name);

	std::filesystem::path gamePath_;
	MIX_Mixer* mixer_ = nullptr;

	std::map<AudioType, std::filesystem::path> eventPaths_;
	std::unordered_map<std::string, std::filesystem::path> streamPaths_;

	// Lazily populated caches. A null value means loading was attempted and failed.
	std::map<AudioType, MIX_Audio*> eventAudio_;
	std::unordered_map<std::string, MIX_Audio*> streamAudio_;
	std::unordered_map<std::string, MIX_Track*> streamTracks_;
};
