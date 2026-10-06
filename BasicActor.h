#pragma once
#include"Fight_ActorType.h"
#include<cmath>
#include<string>
namespace Fight {
	static constexpr double ActionValueInaRound = 10000;
	class BasicActor {
	public:
		double ActionValue = -1;
		double Speed=0;
		virtual ActorType Type()const = 0;
		virtual std::string Name()const = 0;
		BasicActor() = default;
		int id()const { return id_; }
		void SetId(int id) { id_ = id; }
	private:
		
		int id_=-1;
	};
	struct ActorList {
		int id = -1;
		double ActionValue = -1;
	};
	inline bool operator<(const BasicActor& a, const BasicActor& b) {
		if (a.ActionValue != b.ActionValue) {
			return a.ActionValue < b.ActionValue;
		}
		return a.id() < b.id();
	}
	inline bool operator<(const ActorList& a, const ActorList& b) {
		if (a.ActionValue != b.ActionValue) {
			return a.ActionValue < b.ActionValue;
		}
		return a.id < b.id;
	}
}