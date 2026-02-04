#pragma once

#include "Entity.h"
#include <algorithm>

class Asteroid
    : public Entity
{
public:
    explicit Asteroid(int initialLevel = 3) noexcept;
    ~Asteroid() override;

    void update(float deltaTime) override;

    int getLevel() const noexcept;
    float getDefaultSpeed() const noexcept;

    void setLevel(int newLevel) noexcept;
    void decreaseLevel() noexcept;

    // Forward component setters/getters so external code (Game, pools, split logic)
    // updates the component store instead of only the wrapper instance.
    void setDirection(const sf::Vector2f& dir) noexcept;
    sf::Vector2f getDirection() const noexcept;

    void setSpeed(float s) noexcept;
    float getSpeed() const noexcept;

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    size_t componentId{ 0 }; // id in component manager
};
