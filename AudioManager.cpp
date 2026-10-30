#include "AudioManager.h"
#include "Logger.h"
#include "json.hpp"
#include <fstream>
#include <optional>

using json = nlohmann::json;
using Logger::Log;

namespace {
	const char* AudioTypeName(AudioType type) {
		switch (type) {
		case AudioType::Click: return "Click";
		}
		return "Unknown";
	}

	std::optional<AudioType> ParseAudioType(const std::string& name) {
		if (name == "Click") return AudioType::Click;
		return std::nullopt;
	}
}

AudioManager::~AudioManager() {
	Shutdown();
}

bool AudioManager::Init(const std::filesystem::path& gamePath, MIX_Mixer* mixer) {
	Shutdown();

	gamePath_ = gamePath;
	mixer_ = mixer;
	if (!mixer_) {
		Log("AudioManager: mixer is null; audio playback disabled", __func__, Logcode::ERROR);
	}

	std::ifstream in(gamePath_ / "data" / "AudioSettings.json");
	if (!in.is_open()) {
		Log("data/AudioSettings.json", __func__, Logcode::OPENFAILED);
		return false;
	}

	json js;
	try {
		in >> js;
	}
	catch (const json::parse_error& e) {
		Log(e.what(), __func__, Logcode::PARSEERROR);
		return false;
	}
	catch (const std::exception& e) {
		Log(e.what(), __func__, Logcode::ERROR);
		return false;
	}

	// One-shot event mapping: JSON key must match an AudioType name.
	if (js.contains("AudioEvents") && js["AudioEvents"].is_object()) {
		for (const auto& [name, value] : js["AudioEvents"].items()) {
			auto type = ParseAudioType(name);
			if (!type) {
				Log("AudioManager: unknown AudioType: " + name, __func__, Logcode::NOTFOUND);
				continue;
			}
			if (!value.is_string()) {
				Log("AudioManager: path is not a string: " + name, __func__, Logcode::ERROR);
				continue;
			}
			const std::string file = value.get<std::string>();
			if (file.empty()) continue;
			eventPaths_[*type] = AudioRoot() / file;
		}
	}

	// Persistent stream mapping: any string key -> file under assets/Audio.
	if (js.contains("AudioStreams") && js["AudioStreams"].is_object()) {
		for (const auto& [name, value] : js["AudioStreams"].items()) {
			if (!value.is_string()) {
				Log("AudioManager: stream path is not a string: " + name, __func__, Logcode::ERROR);
				continue;
			}
			const std::string file = value.get<std::string>();
			if (file.empty()) continue;
			streamPaths_[name] = AudioRoot() / file;
		}
	}

	Log("AudioManager: " + std::to_string(eventPaths_.size()) + " events, "
		+ std::to_string(streamPaths_.size()) + " streams", __func__, Logcode::OK);
	return true;
}

void AudioManager::Shutdown() {
	for (auto& [name, track] : streamTracks_) {
		if (track) MIX_DestroyTrack(track);
	}
	streamTracks_.clear();

	for (auto& [name, audio] : streamAudio_) {
		if (audio) MIX_DestroyAudio(audio);
	}
	streamAudio_.clear();

	for (auto& [type, audio] : eventAudio_) {
		if (audio) MIX_DestroyAudio(audio);
	}
	eventAudio_.clear();

	eventPaths_.clear();
	streamPaths_.clear();
	mixer_ = nullptr;
	gamePath_.clear();
}

std::filesystem::path AudioManager::AudioRoot() const {
	return gamePath_ / "assets" / "Audio";
}

MIX_Audio* AudioManager::LoadEventAudio(AudioType type) {
	auto cached = eventAudio_.find(type);
	if (cached != eventAudio_.end()) return cached->second;

	if (!mixer_) {
		eventAudio_[type] = nullptr;
		return nullptr;
	}

	auto pathIt = eventPaths_.find(type);
	if (pathIt == eventPaths_.end()) {
		Log(std::string("AudioManager: no event mapping for ") + AudioTypeName(type),
			__func__, Logcode::NOTFOUND);
		eventAudio_[type] = nullptr;
		return nullptr;
	}

	MIX_Audio* audio = MIX_LoadAudio(mixer_, pathIt->second.string().c_str(), true);
	eventAudio_[type] = audio;
	if (!audio) {
		Log("AudioManager: failed to load " + pathIt->second.string()
			+ ": " + SDL_GetError(), __func__, Logcode::OPENFAILED);
	}
	return audio;
}

MIX_Audio* AudioManager::LoadStreamAudio(const std::string& name) {
	auto cached = streamAudio_.find(name);
	if (cached != streamAudio_.end()) return cached->second;

	if (!mixer_) {
		streamAudio_[name] = nullptr;
		return nullptr;
	}

	auto pathIt = streamPaths_.find(name);
	if (pathIt == streamPaths_.end()) {
		Log("AudioManager: no stream mapping for " + name, __func__, Logcode::NOTFOUND);
		streamAudio_[name] = nullptr;
		return nullptr;
	}

	// Stream persistent audio instead of decoding the whole file at once.
	MIX_Audio* audio = MIX_LoadAudio(mixer_, pathIt->second.string().c_str(), false);
	streamAudio_[name] = audio;
	if (!audio) {
		Log("AudioManager: failed to load " + pathIt->second.string()
			+ ": " + SDL_GetError(), __func__, Logcode::OPENFAILED);
	}
	return audio;
}

MIX_Track* AudioManager::GetOrCreateTrack(const std::string& name) {
	auto it = streamTracks_.find(name);
	if (it != streamTracks_.end()) return it->second;

	if (!mixer_) return nullptr;

	MIX_Track* track = MIX_CreateTrack(mixer_);
	streamTracks_[name] = track;
	if (!track) {
		Log("AudioManager: failed to create track for " + name
			+ ": " + SDL_GetError(), __func__, Logcode::CREATEFAILED);
	}
	return track;
}

bool AudioManager::PlayOneShot(AudioType type) {
	if (!mixer_) return false;

	MIX_Audio* audio = LoadEventAudio(type);
	if (!audio) return false;
	if (!MIX_PlayAudio(mixer_, audio)) {
		Log(std::string("AudioManager: failed to play ") + AudioTypeName(type)
			+ ": " + SDL_GetError(), __func__, Logcode::ERROR);
		return false;
	}
	return true;
}

bool AudioManager::PlayPersistent(const std::string& name) {
	if (!mixer_) return false;

	MIX_Audio* audio = LoadStreamAudio(name);
	if (!audio) return false;
	MIX_Track* track = GetOrCreateTrack(name);
	if (!track) return false;

	// Already playing: keep it running instead of restarting on every call.
	if (MIX_TrackPlaying(track)) return true;

	if (!MIX_SetTrackAudio(track, audio)) {
		Log("AudioManager: failed to assign audio " + name
			+ ": " + SDL_GetError(), __func__, Logcode::ERROR);
		return false;
	}

	SDL_PropertiesID props = SDL_CreateProperties();
	if (!props) {
		Log("AudioManager: failed to create playback properties for " + name
			+ ": " + SDL_GetError(), __func__, Logcode::ERROR);
		return false;
	}
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
	const bool ok = MIX_PlayTrack(track, props);
	SDL_DestroyProperties(props);
	if (!ok) {
		Log("AudioManager: failed to play " + name
			+ ": " + SDL_GetError(), __func__, Logcode::ERROR);
	}
	return ok;
}

void AudioManager::StopPersistent(const std::string& name) {
	auto it = streamTracks_.find(name);
	if (it == streamTracks_.end() || !it->second) return;
	if (!MIX_TrackPlaying(it->second)) return;

	if (!MIX_StopTrack(it->second, 0)) {
		Log("AudioManager: failed to stop " + name
			+ ": " + SDL_GetError(), __func__, Logcode::ERROR);
	}
}
