#include"WaitingForFight_Shop.h"
#include"GetRandomNum.h"
#include<fstream>
#include<cmath>
#include"CharacterObject.h"
#include"Logger.h"
#include"json.hpp"
using json = nlohmann::json;
using Logger::Log;
namespace WaitingForFight {
	int Shop::calculateworth(int star, int cost) {
		int basic = cost;
		for (int i = 1;i < star;i++) {
			basic *= 3;
		}
		return cost == 1 ? basic : basic - 1;
	}
	int Shop::calculateworth(std::unique_ptr<BasicObject>& h) {
		auto* p = dynamic_cast<CharacterObject*>(h.get());
		return calculateworth(p->game_attribute.stars, p->game_attribute.cost);
	}
	int Shop::CountShopCharacterNum(const std::string& name, int cost) {//cost是1-based的
		return static_cast<int>(ShopPool[static_cast<size_t>(cost-1)][name].size());
	}
	int Shop::CountShopCharacterNum(const std::string& name) {
		int sum = 0;
		for (int i = 0;i < ShopPool.size();i++) {
			sum += static_cast<int>(ShopPool[i][name].size());
		}
		return sum;
	}
	bool Shop::RemoveOneFromPool(const std::string& name, int cost) {
		if (cost < 1 || static_cast<size_t>(cost) > ShopPool.size()) {
			return false;
		}
		auto& pool = ShopPool[static_cast<size_t>(cost - 1)];
		auto it = pool.find(name);
		if (it == pool.end() || it->second.empty()) {
			return false;//见底就不扣
		}
		it->second.pop_back();
		return true;
	}
	void Shop::PutBacktoPool(std::unique_ptr<BasicObject>& obj) {
		auto* c = dynamic_cast<CharacterObject*>(obj.get());
		if (!c) {
			return;
		}
		int givebacktime = static_cast<int>(std::pow(3, c->game_attribute.stars - 1));
		while (givebacktime--) {
			if (!ShopCharacterNumLimit[static_cast<size_t>(c->game_attribute.cost - 1)].count(c->name)) {
				return;//开局读取之后，不可能之后还有新的角色加入
			}
			if (!ShopPool[static_cast<size_t>(c->game_attribute.cost - 1)].count(c->name)) {
				return;
			}
			if (CountShopCharacterNum(c->name, c->game_attribute.cost) >= ShopCharacterNumLimit[static_cast<size_t>(c->game_attribute.cost - 1)][c->name]) {
				obj.reset(nullptr);
				return;
			}
			auto tmp = std::make_unique<CharacterObject>();
			tmp->NewCharacter(c->name);
			ShopPool[static_cast<size_t>(c->game_attribute.cost - 1)][c->name].push_back(std::move(tmp));
		}
		
		obj.reset(nullptr);//把当前物品直接洗掉
		//其实若采取角色含有指向装备的指针这种结构，则不必在此处洗脱装备了，但要设置装备被占用为false
	}
	int Shop::Sell(std::unique_ptr<BasicObject>& holding, int& money) {
		
		auto* h = dynamic_cast<CharacterObject*>(holding.get());
		if (!h) {
			return -1;
		}
		int worth = calculateworth(h->game_attribute.stars, h->game_attribute.cost);
		money += worth;//这里直接出售卖的结果
		PutBacktoPool(holding);
		return worth;
	}
	int Shop::Buy(int& money, int& index, std::vector<std::unique_ptr<BasicObject>>& bench,int limit) {
		if (index >= 5||index>=RealShop.size()) {
			return -1;
		}
		if (RealShop[index] == nullptr) {
			return -1;
		}
		
		auto* c = dynamic_cast<CharacterObject*>(RealShop[index].get());
		if (!c)return -1;
		if (money < c->game_attribute.cost) {
			return -1;
		}//钱不够
		int checkcnum = 0;
		for (int i = 0;i < bench.size();++i) {
			if (bench[i])++checkcnum;
		}
		if (checkcnum >= limit)return -1;//超员了
		int findfirstempty = -1;
		for (int i = 0; i < (int)bench.size(); ++i) {
			if (!bench[i]) { findfirstempty = i; break; }
		}
		if (findfirstempty == -1) return -1;//找到一个可以放置的空位
		money -= c->game_attribute.cost;
		bench[findfirstempty] = std::move(RealShop[index]);//购买一定在D牌后进行，所以RealShop大小一定为5
		return 1;
	}
	int Shop::ChooseIndex(const std::vector<double>& potential) {//保证：如果一个费用vector角色为空，则概率要重新加权
		//重新加权
		double totp = 0;
		static const long double Scale = 1000000000;
		std::vector<long long>weight(potential.size());
		auto hasAnyCard = [](const auto& m) {
			for (auto& kv : m) if (!kv.second.empty()) return true;
			return false;
			};
		for (int i = 0; i < (int)potential.size(); ++i)
			if (hasAnyCard(ShopPool[i])) totp += potential[i];
		if (totp == 0) {
			return -1;
		}
		long long tot = 0;
		for (int i = 0;i < potential.size();i++) {
			if (hasAnyCard(ShopPool[i])) {
				weight[i] = llround(Scale * (potential[i] / totp));
				tot += weight[i];
			}
			else weight[i] = 0;
		}
		if (tot == 0) {
			return -1;
		}
		long long r = GetRandomNum(0ll, tot-1);
		long long cum = 0ll;
		for (int i = 0;i < weight.size();i++) {
			cum += weight[i];
			if (r < cum) {
				return i;
			}
		}
		return -1;
	}
	int Shop::ChooseIndex(const std::vector<int>& num) {
		if (num.empty()) return -1;

		int tot = 0;
		for (int n : num) tot += n;
		if (tot <= 0) return -1;

		int r = GetRandomNum(0, tot - 1);
		int cum = 0;
		for (size_t i = 0; i < num.size(); ++i) {
			cum += num[i];
			if (r < cum) return static_cast<int>(i);
		}
		return -1;
	}
	void Shop::Dcard(int& money,int free) {
		if (!free) {
			if (money < Dcardmoney) {
				return;
			}
			money -= Dcardmoney;
		}//不接受此次D牌
		//刷新前要把原来的还回去
		//for (auto& var : RealShop) {
		//	auto* p = dynamic_cast<CharacterObject*>(var.get());
		//	if (!p) {
		//		Log("Wrong", __func__);
		//		exit(0);
		//	}
		//	Log(p->name, __func__);
		//}
		for (int i = 0;i < RealShop.size();i++) {
			PutBacktoPool(RealShop[i]);
		}//这时RealShop是全nullptr而不是空，故接下来是写入而不是push_back
		//第一次d牌可能RealShop甚至还是empty
		int choosecost = 1;
		int choosecharacter = 0;
		std::vector<int> countnum;
		
		if (RealShop.empty()) {
			RealShop = std::vector<std::unique_ptr<BasicObject>>(5);
		}
		for (int i = 0;i < 5;i++) {
			countnum.clear();
			
			choosecost = ChooseIndex(Potential[ShopLevel]) + 1;
			if (choosecost == 0) {//如果所有费用角色都没了（实际不可能）
				return;
			}//都没了直接返回，给玩家仅有的几个角色
			for (auto it = ShopPool[static_cast<size_t>(choosecost-1)].begin();it != ShopPool[static_cast<size_t>(choosecost - 1)].end();++it) {
				countnum.push_back(static_cast<int>(it->second.size()));
			}
			choosecharacter = ChooseIndex(countnum);//保证num为0的组不会被选择
			if (choosecharacter == -1) {//实际也不可能
				return;
			}
			auto it = ShopPool[static_cast<size_t>(choosecost - 1)].begin();
			auto jt = std::next(it, choosecharacter);
			auto vt = (*jt).second.end() - 1;//必须大于等于0，移除最后一个
			RealShop[i] = std::move(*vt);
			(*jt).second.erase(vt);
		}
		

	}
	void Shop::LoadinJson(const std::filesystem::path& gp) {
		static bool loadin = false;
		if (loadin) {
			return;
		}
		
		std::ifstream configin(gp / "config" / "GameResource.json");
		if (!configin.is_open()) {
			Log("", __FUNCSIG__, Logcode::OPENFAILED);
			return;
		}
		Log("", __FUNCSIG__, Logcode::OK);
		json js;
		try {
			configin >> js;
			
		}
		catch (const json::parse_error& e) {
			Log(e.what(), __FUNCSIG__, Logcode::PARSEERROR);
			return;
		}
		catch (const std::exception& e) {
			Log(e.what(), __FUNCSIG__, Logcode::ERROR);
			return;
		}
		try {
			// 只在字段存在时读取，避免异常
			if (js.contains("TotalExp"))  TotalExp = js.at("TotalExp").get<int>();
			if (js.contains("ShopLevel")) ShopLevel = js.at("ShopLevel").get<int>();
			if (js.contains("DcardMoney"))Dcardmoney = js.at("DcardMoney").get<int>();
			if (js.contains("BuyExpmoney"))BuyExpmoney = js.at("BuyExpmoney").get<int>();
			if (js.contains("ExpBuyNum"))ExpBuyNum = js.at("ExpBuyNum").get<int>();
			if (js.contains("MAX_SHOPLEVEL"))MAX_SHOPLEVEL = js.at("MAX_SHOPLEVEL").get<int>();
			if (js.contains("ShopCharacterNumLimit"))defaultlimit = js.at("ShopCharacterNumLimit").get<int>();
			// 二维数组：直接转 std::vector<std::vector<double>>
			if (js.contains("Potential"))
				Potential = js.at("Potential").get<std::vector<std::vector<double>>>();

			// 一维 int 数组例子（可选）
			if (js.contains("ExpNeedToLevelUp"))
				explevelup = js.at("ExpNeedToLevelUp").get<std::vector<int>>();
		}
		catch (const std::exception& e) {
			Log(e.what(), __FUNCSIG__, Logcode::ERROR);
		}
		
		if (ShopPool.empty()) {
			ShopPool = std::vector<std::unordered_map<std::string, std::vector<std::unique_ptr<BasicObject>>>>(5);
		}
		if (ShopCharacterNumLimit.empty()) {
			ShopCharacterNumLimit.resize(5);
		}
		//for (auto& [key, value] : Object::Character_DefaultSetting) {
		//	Log(key+value.name, __func__, Logcode::OK);
		//}
		Log("GoodParse", __FUNCSIG__, Logcode::OK);
		for (auto& [key, value] : Object::Character_DefaultSetting) {
			auto probe = std::make_unique<CharacterObject>();
			probe->NewCharacter(key);
			size_t idx = static_cast<size_t>(probe->game_attribute.cost - 1);

			auto& vec = ShopPool[idx][key];
			vec.clear();
			vec.reserve(defaultlimit);
			for (int k = 0; k < defaultlimit; ++k) {
				auto tmp = std::make_unique<CharacterObject>();
				tmp->NewCharacter(key);
				vec.push_back(std::move(tmp));
			}
			ShopCharacterNumLimit[idx][key] = defaultlimit;
		}
		loadin = true;
		Log("EndLoadin:Shop", __FUNCSIG__, Logcode::OK);
	}
	bool Shop::MaxShopLevel() {
		return ShopLevel >= MAX_SHOPLEVEL;
	}
	void Shop::BuyExp(int& money) {
		if (MaxShopLevel())return;
		if (money < BuyExpmoney)return;
		TotalExp += ExpBuyNum;
		money -= BuyExpmoney;
	}
	int Shop::LevelUp() {
		if (MaxShopLevel()) {
			return 0;
		}
		int sum = 0;
		if (ShopLevel + 1 >= (int)explevelup.size()) return 0;
		for (int i = 0;i <= ShopLevel+1;i++) {
			sum += explevelup[i];
		}
		if (sum <= TotalExp) {
			
			++ShopLevel;
			return 1;
		}
		return 0;
	}
	int Shop::CalculateExpSum() {
		if (ShopLevel + 1 >= (int)explevelup.size())return 0;
		int sum = 0;
		for (int i = 0;i <= ShopLevel;i++) {
			sum += explevelup[i];
		}
		return sum;
	}
}