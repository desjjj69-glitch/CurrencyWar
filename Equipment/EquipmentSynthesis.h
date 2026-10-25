#pragma once
#include"BasicEquipment.h"
#include<filesystem>
#include<memory>
#include<string>
#include<utility>
namespace EquipmentSynthesis {
	// A+B 与 B+A 等价，key 内部会按字典序规范化
	using RecipeKey = std::pair<std::string, std::string>;

	// 读入 data/BasicEquipmentSettings.json 的 SynthesisRecipes（只读一次）
	void LoadinJson(const std::filesystem::path&);
	// 查询配方，返回结果装备名；没有配方返回空串
	std::string Query(const std::string& a, const std::string& b);
	// 查询并生成合成结果（默认 Advanced）；没有配方返回 nullptr
	std::unique_ptr<BasicEquipment> Synthesize(const std::string& a, const std::string& b);
}
