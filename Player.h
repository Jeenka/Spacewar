#pragma once

#include "Entity.h"
#include "EventBus.h"
#include "InputEvents.h"

class Player : public Entity
{
public:
    Player();
    ~Player();

    virtual void update(float deltaTime) override;

    void setTurnDirection(float newTurnDirection) { turnDirection = newTurnDirection; }
    void setThrust(float newThrust) { thrust = newThrust; }

    float getRadius() { return shape.getRadius(); }

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    sf::CircleShape headingShape;
    float drag;
    float thrust{};
    float accelerationSpeed;
    float maxSpeed;
    float turnDirection{};
    float turnRate;

    // Event subscription ids (0 means not subscribed)
    EventBus::HandlerId keySubId{0};
    EventBus::HandlerId mouseSubId{0};
};

