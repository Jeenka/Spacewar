#pragma once

#include "Entity.h"

class Player;

class Zone : public Entity
{
public:
	Zone();

	virtual void update(float deltaTime) override;

protected:
	sf::CircleShape zoneCircle;

};
