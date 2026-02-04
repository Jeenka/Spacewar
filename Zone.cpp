#include "Zone.h"
#include <SFML/Graphics/RenderWindow.hpp>

Zone::Zone()
{
	shape.setFillColor(sf::Color::Transparent);
	shape.setRadius(300.0f);
	shape.setPointCount(25);
	shape.setOutlineColor(sf::Color::White);
	shape.setOutlineThickness(2.0f);
	shape.setOrigin({ shape.getRadius(), shape.getRadius() });
}

void Zone::update(float deltaTime)
{
	rotate(sf::degrees(25.0f * deltaTime));
}
