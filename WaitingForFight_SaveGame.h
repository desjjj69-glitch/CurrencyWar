#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace WaitingForFight {
	class Logic;
}

namespace WaitingForFight::SaveGame {

	std::filesystem::path SavePath(const std::filesystem::path& gamePath);
	bool HasSave(const std::filesystem::path& gamePath);
	bool Delete(const std::filesystem::path& gamePath);

	// Read only the fields needed before WaitingForFight::LoadinJson.
	bool PeekMeta(const std::filesystem::path& gamePath,
		std::vector<std::string>& bossNames,
		int& difficulty);

	bool Save(const Logic& logic, const std::filesystem::path& gamePath);
	bool Load(Logic& logic, const std::filesystem::path& gamePath);

}
