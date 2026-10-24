#pragma once
#include<string>
// 晶矿品质，对应 assets/Mineral 下的四张图
enum class MineralType {
	Grey = 0,
	Blue,
	Gold,
	Prismatic
};
inline bool TryParseMineralType(const std::string& s, MineralType& out) {
	if (s == "Grey") { out = MineralType::Grey; return true; }
	if (s == "Blue") { out = MineralType::Blue; return true; }
	if (s == "Gold") { out = MineralType::Gold; return true; }
	if (s == "Prismatic") { out = MineralType::Prismatic; return true; }
	return false;
}
inline MineralType ParseMineralType(const std::string& s) {
	MineralType t = MineralType::Grey;
	TryParseMineralType(s, t);
	return t;
}
inline const char* MineralTypeName(MineralType t) {
	switch (t) {
	case MineralType::Grey:      return "Grey";
	case MineralType::Blue:      return "Blue";
	case MineralType::Gold:      return "Gold";
	case MineralType::Prismatic: return "Prismatic";
	default:                     return "Unknown";
	}
}
// 绘制用：assets/Mineral 下的文件名
inline std::string MineralTextureFile(MineralType t) {
	return std::string(MineralTypeName(t)) + ".png";
}
