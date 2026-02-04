#include "Asteroid.h"
#include "AsteroidComponent.h"
#include <SFML/Graphics/RenderTarget.hpp>

Asteroid::Asteroid(int initialLevel) noexcept
    : Entity(static_cast<const float&>(0.0f))
{
    componentId = AsteroidComponentManager::instance().create(this, initialLevel);
    speed = AsteroidComponentManager::instance().getDefaultSpeed(componentId);
}

Asteroid::~Asteroid()
{
    if (componentId) AsteroidComponentManager::instance().destroy(componentId);
}

void Asteroid::update(float deltaTime)
{
    if (componentId)
    {
        AsteroidComponentManager::instance().update(componentId, deltaTime);
    }
}

int Asteroid::getLevel() const noexcept
{
    return componentId ? AsteroidComponentManager::instance().getLevel(componentId) : 0;
}

float Asteroid::getDefaultSpeed() const noexcept
{
    return componentId ? AsteroidComponentManager::instance().getDefaultSpeed(componentId) : 0.0f;
}

void Asteroid::setLevel(int newLevel) noexcept
{
    if (componentId) AsteroidComponentManager::instance().setLevel(componentId, newLevel);
}

void Asteroid::decreaseLevel() noexcept
{
    if (componentId) AsteroidComponentManager::instance().decreaseLevel(componentId);
}

void Asteroid::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform = getTransform();
    const sf::CircleShape* s = componentId ? AsteroidComponentManager::instance().getShape(componentId) : nullptr;
    if (s)
    {
        target.draw(*s, states);
    }
    else
    {
        Entity::draw(target, states);
    }
}
