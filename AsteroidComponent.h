#pragma once

#include <atomic>
#include <unordered_map>
#include <shared_mutex>
#include <memory>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>

class Asteroid;

struct AsteroidComponent
{
    size_t id{};
    Asteroid* owner{ nullptr };
    int level{ 3 };
    float rotationSpeed{ 25.0f };
    float defaultSpeed{ 400.0f };
    float speed{};
    sf::Vector2f direction{};
    sf::CircleShape shape;

    AsteroidComponent() = default;
};

class AsteroidComponentManager
{
public:
    using Id = size_t;

    static AsteroidComponentManager& instance();

    Id create(Asteroid* owner, int initialLevel);
    void destroy(Id id);

    void update(Id id, float deltaTime);

    void setLevel(Id id, int newLevel);
    void decreaseLevel(Id id);

    int getLevel(Id id);
    float getDefaultSpeed(Id id);

    void setDirection(Id id, const sf::Vector2f& dir);
    sf::Vector2f getDirection(Id id);

    void setSpeed(Id id, float s);
    float getSpeed(Id id);

    float getRadius(Id id);
    sf::CircleShape getShapeCopy(Id id);

    Id getIdForOwner(Asteroid* owner);
    void setDirectionByOwner(Asteroid* owner, const sf::Vector2f& dir);
    void setSpeedByOwner(Asteroid* owner, float s);
    void setLevelByOwner(Asteroid* owner, int level);
    int getLevelByOwner(Asteroid* owner);
    float getDefaultSpeedByOwner(Asteroid* owner);
    float getRadiusByOwner(Asteroid* owner);

    std::vector<Id> snapshotIds();

private:
    AsteroidComponentManager() = default;
    ~AsteroidComponentManager() = default;

    // internal helpers
    AsteroidComponent* findComponentLocked(Id id);
    AsteroidComponent* findComponentShared(Id id);

    mutable std::shared_mutex mutex_;
    std::atomic<Id> nextId_{1};
    std::unordered_map<Id, AsteroidComponent> components_;
    std::unordered_map<Asteroid*, Id> ownerMap_;

    static constexpr float BaseRadius{ 10.0f };
    static constexpr float RadiusStep{ 10.0f };
};