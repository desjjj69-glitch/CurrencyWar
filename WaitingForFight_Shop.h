#pragma once
#include"CharacterObject.h"
#include<memory>
#include<vector>
#include<filesystem>
#include<unordered_map>
#include<string>
namespace WaitingForFight {
	class Shop {
	public:
		int TotalExp=0;
		int ShopLevel=3;
		int Dcardmoney = 2;
		int FreeDcardTimes = 0;
		int BuyExpmoney = 4;
		int ExpBuyNum = 4;
		std::vector<std::vector<double>>Potential;
		std::vector<int>explevelup;
		std::vector<std::unordered_map<std::string,std::vector<std::unique_ptr<BasicObject>>>> ShopPool;
		std::vector<std::unique_ptr<BasicObject>> RealShop;
		int defaultlimit = 9;
		std::vector<std::unordered_map<std::string, int>> ShopCharacterNumLimit;
		int calculateworth(int, int);
		int calculateworth(std::unique_ptr<BasicObject>&);
		int Sell(std::unique_ptr<BasicObject>&, int&);
		void PutBacktoPool(std::unique_ptr<BasicObject>&);
		bool RemoveOneFromPool(const std::string&, int);//按名字和费用从池中扣一份；见底返回 false
		int Buy(int&,int&,std::vector<std::unique_ptr<BasicObject>>&,int);//money,index,备战席直接传进来,人数限制
		bool Dcard(int&,int free=0);//money,控制D牌是不是免费；返回是否真的刷新
		void LoadinJson(const std::filesystem::path&);
		void BuyExp(int&);
		int LevelUp();
		bool MaxShopLevel();
		int CalculateExpSum();
	private:
		int CountShopCharacterNum(const std::string&,int);//根据名字和费用查找
		int CountShopCharacterNum(const std::string&);//查找所有的
		int ChooseIndex(const std::vector<double>&);
		int ChooseIndex(const std::vector<int>&);
		int MAX_SHOPLEVEL = 10;
	};
}