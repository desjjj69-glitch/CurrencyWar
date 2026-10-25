#include"EquipmentSynthesis.h"
#include"AdvancedEquipment.h"
#include"../Logger.h"
#include"../json.hpp"
#include<fstream>
#include<map>
using json = nlohmann::json;
using Logger::Log;
namespace EquipmentSynthesis {
	namespace {
		std::map<RecipeKey, std::string> Recipes;
		bool loaded = false;

		RecipeKey MakeKey(std::string a, std::string b) {
			if (b < a) std::swap(a, b);
			return { std::move(a), std::move(b) };
		}
	}
	void LoadinJson(const std::filesystem::path& gp) {
		if (loaded) {
			return;
		}
		loaded = true;

		std::ifstream in(gp / "data" / "BasicEquipmentSettings.json");
		if (!in.is_open()) {
			Log("", __func__, Logcode::OPENFAILED);
			return;
		}
		try {
			json js;
			in >> js;
			if (js.contains("SynthesisRecipes") && js["SynthesisRecipes"].is_array()) {
				for (auto& r : js["SynthesisRecipes"]) {
					if (!r.is_object()) continue;
					std::string a = r.value("A", std::string(""));
					std::string b = r.value("B", std::string(""));
					std::string result = r.value("Result", std::string(""));
					if (a.empty() || b.empty() || result.empty()) continue;
					Recipes[MakeKey(std::move(a), std::move(b))] = std::move(result);
				}
			}
			Log("Recipes: " + std::to_string(Recipes.size()), __func__, Logcode::OK);
		}
		catch (const json::parse_error& e) {
			Log(e.what(), __func__, Logcode::PARSEERROR);
		}
		catch (const std::exception& e) {
			Log(e.what(), __func__, Logcode::ERROR);
		}
	}
	std::string Query(const std::string& a, const std::string& b) {
		if (a.empty() || b.empty()) return {};
		auto it = Recipes.find(MakeKey(a, b));
		return it == Recipes.end() ? std::string() : it->second;
	}
	std::unique_ptr<BasicEquipment> Synthesize(const std::string& a, const std::string& b) {
		std::string result = Query(a, b);
		if (result.empty()) {
			return nullptr;
		}
		auto e = std::make_unique<AdvancedEquipment>();
		e->SetName(result);
		return e;
	}
}
