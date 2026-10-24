#pragma once
#include"BasicObject.h"
#include"MineralType.h"
#include"CharacterObject.h"
#include"BasicEquipment.h"
#include<filesystem>
#include<map>
#include<memory>
#include<string>
#include<vector>
class Mineral :public BasicObject {
public:
	Mineral() = default;
	explicit Mineral(MineralType q) :quality(q) {}

	WaitingForFight::ResourceType Type()const override {
		return WaitingForFight::ResourceType::Mineral;
	}

	// ---------- 品质（绘制等使用） ----------
	MineralType Quality()const { return quality; }
	void SetQuality(MineralType q) { quality = q; }
	const char* QualityName()const { return MineralTypeName(quality); }
	std::string TextureFile()const { return MineralTextureFile(quality); }

	// ---------- 内容是否已经被指定 ----------
	// 首次 OpenMineral 时按品质抽取并置为 true；也允许外部先填好内容再置 true
	bool IsSpecified()const { return specified; }
	void SetSpecified(bool v) { specified = v; }
	// 允许外部（如开局写入的固定晶矿）直接指定内容
	void SetMoney(int v) { money = v; }
	void SetExp(int v) { exp = v; }

	// 打开晶矿：首次调用按品质随机生成内容，返回内容摘要（如 "Money:2;Exp:4;Character:1"），
	// 重复调用不会重新抽取。具体内容通过下面四个 Get 取。
	std::string OpenMineral();

	std::vector<std::unique_ptr<CharacterObject>>& GetCharacter() { return characters; }
	const std::vector<std::unique_ptr<CharacterObject>>& GetCharacter()const { return characters; }
	std::vector<std::unique_ptr<BasicEquipment>>& GetEquipment() { return equipments; }
	const std::vector<std::unique_ptr<BasicEquipment>>& GetEquipment()const { return equipments; }
	int GetMoney()const { return money; }
	int GetExp()const { return exp; }

	// 从 config/Mineral.json 与 data/BasicEquipmentSettings.json 读入默认内容表（只读一次）
	static void LoadinJson(const std::filesystem::path&);

private:
	MineralType quality = MineralType::Grey;
	bool specified = false;
	int money = 0;
	int exp = 0;
	std::vector<std::unique_ptr<CharacterObject>> characters;
	std::vector<std::unique_ptr<BasicEquipment>> equipments;

	// 品质 -> 内容名 -> 具体参数（参数含义见 Mineral.cpp）
	static std::map<MineralType, std::map<std::string, std::vector<std::string>>> DefaultContent;
	// 装备品阶 -> 该品阶所有装备名
	static std::map<EquipmentType, std::vector<std::string>> EquipmentNames;
	static bool loaded;

	std::string Summary()const;
};
