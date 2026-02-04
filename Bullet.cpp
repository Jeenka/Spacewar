#include "Bullet.h"

Bullet::Bullet() : Entity(800.0f)
{
	shape.setFillColor(sf::Color::Red);
	shape.setRadius(5.0f);
	shape.setPointCount(5);
	shape.setOrigin({ shape.getRadius(), shape.getRadius() });
}

void Bullet::update(float deltaTime)
{
	move(direction * speed * deltaTime);
}