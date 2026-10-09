#include"Fight_Logic.h"
#include"Logger.h"
#include"json.hpp"
#include"GetRandomNum.h"
#include<cmath>
using Logger::Log;
using json = nlohmann::json;
namespace Fight {
	template <class K, class V>
	inline V MapGet(const std::unordered_map<K, V>& m, const K& key, V def = V{}) {
		auto it = m.find(key);
		return it == m.end() ? def : it->second;
	}
	Attribute::AttackValueType Fight_Logic::CalculateDamage(BasicActor* caster, BasicActor* target,
		const Attribute::AttackForm& form,double multiply) {
		
		auto CalculateCharacterDamage = [&](const Character_Actor* ch, const Enemy_Actor* enemy) ->Attribute::AttackValueType {
			using AttackValueType = Attribute::AttackValueType;
			double base = static_cast<double>(ch->config.basic_attribute.BasicAttack);//基础攻击力
			double collect_attackboost_percent = 0, collect_attackboost_add = 0;
			double collect_damageboost_percent = 0, collect_damageboost_add = 0;
			for (auto& var : ch->RealTimeBuffs) {
				collect_attackboost_percent += var.config->percent.BasicAttackBoost * var.stacks;
				collect_attackboost_add += var.config->add.BasicAttackBoost * var.stacks;
				for (auto& t : form.types) {
					collect_damageboost_add += MapGet(var.config->add.Boost_AttackTypeDamageBoost, t) * var.stacks;
					collect_damageboost_percent += MapGet(var.config->percent.Boost_AttackTypeDamageBoost, t) * var.stacks;
				}//攻击类型增伤
				collect_damageboost_add += MapGet(var.config->add.Boost_ElementDamageBoost, form.element) * var.stacks;
				collect_damageboost_percent += MapGet(var.config->percent.Boost_ElementDamageBoost, form.element) * var.stacks;//攻击属性增伤
				collect_damageboost_add += MapGet(var.config->add.Boost_ElementDamageBoost, Attribute::ElementType::AllElement) * var.stacks;
				collect_damageboost_percent += MapGet(var.config->percent.Boost_ElementDamageBoost, Attribute::ElementType::AllElement) * var.stacks;//全属性增伤

			}
			auto attack = (base * (1+collect_attackboost_percent)) + collect_attackboost_add;//基础攻击力乘区
			auto crit_damage = attack * (1 + GetRandomNum(ch->config.basic_attribute.CritRate) * ch->config.basic_attribute.CritDamage);//暴击伤害乘区
			auto multiply_damage = crit_damage * multiply;//倍率乘区
			//增伤乘区
			auto boost_damage = multiply_damage * (1 + collect_damageboost_percent) + collect_damageboost_add;
			
			
			//抗性区
			auto resist_damage =
				boost_damage * 
				(1 - 
					std::max(
						MapGet(enemy->config.attribute.Resistance_ElementDamageResistance, form.element),
						MapGet(enemy->config.attribute.Resistance_ElementDamageResistance, Attribute::ElementType::AllElement)
				));
				
			
			//防御区
			auto GetLevelCoeff = [](int level)->double {
				return 200.f + 10.f * static_cast<double>(level);
				};
			auto chcoeff = GetLevelCoeff(ch->config.basic_attribute.Level);
			auto defense_damage =
				chcoeff / (chcoeff +
					std::max(
						static_cast<double>(enemy->config.attribute.defense),
						GetLevelCoeff(enemy->config.attribute.Level)
					)
					) * resist_damage;
			//减伤区
			
			//易伤区
			// 暂时没写
			//玩法强度增幅
			
			//玩法伤害增幅

			//玩法幸运一击
			return static_cast<Attribute::AttackValueType>(std::llround(defense_damage));
			};
		auto CalculateEnemyDamage = [&](Enemy_Actor* enemy, Character_Actor* ch,
			const Attribute::AttackForm& form, double multiply) ->Attribute::AttackValueType {
				double base = static_cast<double>(enemy->config.attribute.attack);
				return static_cast<Attribute::AttackValueType>(std::llround(base*multiply));
			};
		switch (caster->Type()) {
		case ActorType::Character:
		{
			auto* ch = static_cast<Character_Actor*>(caster);
			auto* enemy = static_cast<Enemy_Actor*>(target);
			return CalculateCharacterDamage(ch, enemy);
		}
		break;
		case ActorType::Enemy:
		{
			auto* ch = static_cast<Character_Actor*>(target);
			auto* enemy = static_cast<Enemy_Actor*>(caster);
			return CalculateEnemyDamage(enemy,ch,form,multiply);
		}
		break;
		}
		return 0;
	}
}