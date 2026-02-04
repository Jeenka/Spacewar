#include "Entity.h"
#include <SFML/Graphics/RenderWindow.hpp>

sf::FloatRect Entity::getBounds()
{
	return shape.getGlobalBounds();
}

bool Entity::intersects(Entity& entity)
{
    float distance = sf::Vector2f(getPosition() - entity.getPosition()).length();
    return distance < (shape.getRadius() + entity.shape.getRadius());
}

void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform = getTransform();
	target.draw(shape, states);
}
