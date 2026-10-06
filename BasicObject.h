#pragma once
#include"WaitingForFight_ResourceType.h"
class BasicObject {
public:
	virtual ~BasicObject() = default;
	virtual WaitingForFight::ResourceType Type()const = 0;
};