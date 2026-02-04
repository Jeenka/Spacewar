#pragma once

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/CircleShape.hpp>

class Entity : public sf::Transformable, public sf::Drawable
{
public:
	Entity() {};
	Entity(const float& speed) : speed(speed) {}

	virtual void update(float deltaTime) = 0;
	virtual sf::FloatRect getBounds();
	virtual bool intersects(Entity& entity);

	void setDirection(const sf::Vector2f& newDirection) { direction = newDirection; }
	sf::Vector2f getDirection() { return direction; }
	void setSpeed(const float newSpeed) { speed = newSpeed; }

	const float getSpeed() { return speed; }

	void setCollisionRadius(float r) { shape.setRadius(r); shape.setOrigin({ r, r }); }
	void setShapePointCount(unsigned int count) { shape.setPointCount(count); }
	float getCollisionRadius() const { return shape.getRadius(); }

protected:
	sf::CircleShape shape;
	sf::Vector2f direction{};
	float speed{};
	float defaultSpeed{};

protected:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

