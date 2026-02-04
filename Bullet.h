#pragma once

#include "Entity.h"

class Bullet : public Entity
{
public:
	Bullet();

	virtual void update(float deltaTime) override;

};

