#include"Mineral.h"
#include"NormalEquipment.h"
#include"Logger.h"
#include"json.hpp"
#include"GetRandomNum.h"
#include<algorithm>
#include<fstream>
using json = nlohmann::json;
using Logger::Log;

// 配置约定（config/Mineral.json）：
//   "Money"      : [min, max]                          随机整数
//   "Exp"        : [min, max]                          随机整数
//   "Character"  : [costMin, costMax, countMin, countMax]
//   "Equipment"  : [type, countMin, countMax]          type 为 Normal/Advanced/Privilege（Basic 视作 Normal）
// 例：
//   "Grey" : { "Money": ["1","3"], "Exp": ["2","4"], "Character": ["1","1","1","2"], "Equipment": [] }

namespace {
	int ToInt(const std::string& s, int def = 0) {
		try {
			return std::stoi(s);
		}
		catch (const std::exception&) {
			return def;
		}
	}

	int RollRange(const std::vector<std::string>& v, int def = 0) {
		if (v.size() < 2) {
			return def;
		}
		int lo = ToInt(v[0], def);
		int hi = ToInt(v[1], def);
		if (hi < lo) std::swap(lo, hi);
		return GetRandomNum(lo, hi);
	}

	std::string JsonScalarToString(const json& j) {
		if (j.is_string()) return j.get<std::string>();
		if (j.is_number_integer()) return std::to_string(j.get<long long>());
		if (j.is_number_unsigned()) return std::to_string(j.get<unsigned long long>());
		if (j.is_number_float()) return std::to_string(j.get<double>());
		if (j.is_boolean()) return j.get<bool>() ? "1" : "0";
		return {};
	}

	bool ParseStrictEquipmentType(const std::string& s, EquipmentType& out) {
		if (s == "Normal" || s == "Basic") { out = EquipmentType::Normal; return true; }
		if (s == "Advanced") { out = EquipmentType::Advanced; return true; }
		if (s == "Privilege") { out = EquipmentType::Privilege; return true; }
		return false;
	}
}

std::map<MineralType, std::map<std::string, std::vector<std::string>>> Mineral::DefaultContent;
std::map<EquipmentType, std::vector<std::string>> Mineral::EquipmentNames;
bool Mineral::loaded = false;

void Mineral::LoadinJson(const std::filesystem::path& gp) {
	if (loaded) {
		return;
	}
	loaded = true;

	// ---------- 1. 装备目录：按品阶归类，供开矿时随机 ----------
	std::ifstream equipin(gp / "data" / "BasicEquipmentSettings.json");
	if (!equipin.is_open()) {
		Log("", __func__, Logcode::OPENFAILED);
	}
	else {
		try {
			json js;
			equipin >> js;
			if (js.contains("Equipment") && js["Equipment"].is_array()) {
				for (auto& var : js["Equipment"]) {
					if (!var.is_object()) continue;
					std::string name = var.value("Name", std::string(""));
					std::string type = var.value("Type", std::string(""));
					if (name.empty()) continue;
					EquipmentType t;
					if (!ParseStrictEquipmentType(type, t)) continue;// 未分类的装备不参与开矿
					EquipmentNames[t].push_back(name);
				}
			}
		}
		catch (const json::parse_error& e) {
			Log(e.what(), __func__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __func__, Logcode::ERROR);
		}
	}

	// ---------- 2. 各品质的默认内容 ----------
	std::ifstream minerin(gp / "config" / "Mineral.json");
	if (!minerin.is_open()) {
		Log("config/Mineral.json", __func__, Logcode::OPENFAILED);
		return;
	}
	try {
		json js;
		minerin >> js;
		for (auto& [key, content] : js.items()) {
			MineralType q;
			if (!TryParseMineralType(key, q)) {
				Log("Unknown mineral type: " + key, __func__, Logcode::PARSEERROR);
				continue;
			}
			if (!content.is_object()) continue;
			auto& dst = DefaultContent[q];
			for (auto& [contentName, arr] : content.items()) {
				if (!arr.is_array()) continue;
				auto& vec = dst[contentName];
				vec.clear();
				for (auto& v : arr) {
					std::string s = JsonScalarToString(v);
					if (!s.empty()) vec.push_back(std::move(s));
				}
			}
		}
	}
	catch (const json::parse_error& e) {
		Log(e.what(), __func__, Logcode::PARSEERROR);
	}
	catch (const std::exception& e) {
		Log(e.what(), __func__, Logcode::ERROR);
	}
}

std::string Mineral::OpenMineral() {
	if (specified) {
		return Summary();
	}
	specified = true;
	money = 0;
	exp = 0;
	characters.clear();
	equipments.clear();

	auto it = DefaultContent.find(quality);
	if (it == DefaultContent.end()) {
		Log(std::string("Mineral config missing: ") + QualityName(), __func__, Logcode::NOTFOUND);
		return Summary();
	}
	const auto& cfg = it->second;
	auto GetContent = [&](const char* key) -> const std::vector<std::string>* {
		auto found = cfg.find(key);
		return found == cfg.end() ? nullptr : &found->second;
		};

	// ---------- Money / Exp ----------
	if (const auto* v = GetContent("Money")) {
		money = RollRange(*v);
	}
	if (const auto* v = GetContent("Exp")) {
		exp = RollRange(*v);
	}

	// ---------- Character ----------
	if (const auto* v = GetContent("Character"); v && v->size() >= 4) {
		int costMin = ToInt((*v)[0], 1);
		int costMax = ToInt((*v)[1], costMin);
		int countMin = ToInt((*v)[2], 0);
		int countMax = ToInt((*v)[3], countMin);
		if (costMax < costMin) std::swap(costMin, costMax);
		if (countMax < countMin) std::swap(countMin, countMax);

		int count = GetRandomNum(countMin, countMax);
		if (count > 0) {
			std::vector<std::string> pool;
			for (const auto& [name, ch] : Object::Character_DefaultSetting) {
				int cost = ch.game_attribute.cost;
				if (cost >= costMin && cost <= costMax) pool.push_back(name);
			}
			if (pool.empty()) {
				Log("No character of cost " + std::to_string(costMin) + "~" + std::to_string(costMax), __func__, Logcode::NOTFOUND);
			}
			else {
				for (int i = 0; i < count; ++i) {
					int index = GetRandomNum(0, static_cast<int>(pool.size()) - 1);
					auto c = std::make_unique<CharacterObject>();
					c->NewCharacter(pool[static_cast<size_t>(index)]);
					characters.push_back(std::move(c));
				}
			}
		}
	}

	// ---------- Equipment ----------
	if (const auto* v = GetContent("Equipment"); v && v->size() >= 3) {
		EquipmentType tier;
		if (!ParseStrictEquipmentType((*v)[0], tier)) {
			Log("Unknown equipment type: " + (*v)[0], __func__, Logcode::PARSEERROR);
		}
		else {
			int countMin = ToInt((*v)[1], 0);
			int countMax = ToInt((*v)[2], countMin);
			if (countMax < countMin) std::swap(countMin, countMax);

			int count = GetRandomNum(countMin, countMax);
			const auto& pool = EquipmentNames[tier];
			if (count > 0 && pool.empty()) {
				Log(std::string("No equipment of type ") + (*v)[0], __func__, Logcode::NOTFOUND);
			}
			if (count > 0 && tier != EquipmentType::Normal) {
				// 目前只实现了 NormalEquipment，其它品阶等对应类补上后再支持
				Log(std::string("Equipment type not implemented: ") + (*v)[0], __func__, Logcode::ERROR);
			}
			else {
				for (int i = 0; i < count && !pool.empty(); ++i) {
					int index = GetRandomNum(0, static_cast<int>(pool.size()) - 1);
					const auto& name = pool[static_cast<size_t>(index)];
					auto e = std::make_unique<NormalEquipment>();
					e->SetName(name);
					equipments.push_back(std::move(e));
				}
			}
		}
	}

	return Summary();
}

std::string Mineral::Summary()const {
	std::string s;
	if (money > 0) s += "Money:" + std::to_string(money) + ";";
	if (exp > 0) s += "Exp:" + std::to_string(exp) + ";";
	if (!characters.empty()) s += "Character:" + std::to_string(characters.size()) + ";";
	if (!equipments.empty()) s += "Equipment:" + std::to_string(equipments.size()) + ";";
	if (s.empty()) return "None";
	s.pop_back();
	return s;
}
