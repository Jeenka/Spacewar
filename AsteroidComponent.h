#pragma once

#include <atomic>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>

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

    static AsteroidComponentManager& instance()
    {
        static AsteroidComponentManager mgr;
        return mgr;
    }

    Id create(Asteroid* owner, int initialLevel)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        Id id = nextId_++;
        AsteroidComponent comp;
        comp.id = id;
        comp.owner = owner;
        comp.level = std::max(0, initialLevel);
        comp.rotationSpeed = 25.0f;
        comp.defaultSpeed = 400.0f;
        comp.speed = comp.defaultSpeed;
        comp.direction = { 0.f, 0.f };
        const float radius = BaseRadius + comp.level * RadiusStep;
        comp.shape.setRadius(radius);
        comp.shape.setPointCount(8);
        comp.shape.setFillColor(sf::Color(50, 50, 50));
        comp.shape.setOutlineColor(sf::Color(100, 100, 100));
        comp.shape.setOutlineThickness(4.0f);
        comp.shape.setOrigin({ radius, radius });

        components_.emplace(id, std::move(comp));
        ownerMap_.emplace(owner, id);

        if (owner) {
            owner->setCollisionRadius(radius);
            owner->setShapePointCount(8);
        }

        return id;
    }

    void destroy(Id id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = components_.find(id);
        if (it != components_.end()) {
            // remove owner mapping
            if (it->second.owner) {
                ownerMap_.erase(it->second.owner);
            }
            components_.erase(it);
        }
    }

    void update(Id id, float deltaTime)
    {
        AsteroidComponent* c = getUnlocked(id);
        if (!c) return;

        c->shape.rotate(sf::degrees(c->rotationSpeed * deltaTime));

        if (c->owner)
        {
            c->owner->move(c->direction * c->speed * deltaTime);
        }
    }

    void setLevel(Id id, int newLevel)
    {
        AsteroidComponent* c = getUnlocked(id);
        if (!c) return;
        c->level = std::max(0, newLevel);
        const float r = BaseRadius + c->level * RadiusStep;
        c->shape.setRadius(r);
        c->shape.setOrigin({ r, r });
        // keep wrapper collision in sync
        if (c->owner) c->owner->setCollisionRadius(r);
    }

    void decreaseLevel(Id id)
    {
        AsteroidComponent* c = getUnlocked(id);
        if (!c) return;
        if (c->level > 0) --c->level;
        const float r = BaseRadius + c->level * RadiusStep;
        c->shape.setRadius(r);
        c->shape.setOrigin({ r, r });
        if (c->owner) c->owner->setCollisionRadius(r);
    }

    // accessors used by external systems (owner-based APIs)
    Id getIdForOwner(Asteroid* owner)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = ownerMap_.find(owner);
        return it != ownerMap_.end() ? it->second : 0;
    }

    void setDirectionByOwner(Asteroid* owner, const sf::Vector2f& dir)
    {
        Id id = getIdForOwner(owner);
        if (id) setDirection(id, dir);
    }

    void setSpeedByOwner(Asteroid* owner, float s)
    {
        Id id = getIdForOwner(owner);
        if (id) setSpeed(id, s);
    }

    void setLevelByOwner(Asteroid* owner, int level)
    {
        Id id = getIdForOwner(owner);
        if (id) setLevel(id, level);
    }

    int getLevelByOwner(Asteroid* owner)
    {
        Id id = getIdForOwner(owner);
        return id ? getLevel(id) : 0;
    }

    float getDefaultSpeedByOwner(Asteroid* owner)
    {
        Id id = getIdForOwner(owner);
        return id ? getDefaultSpeed(id) : 0.0f;
    }

    // original id-based accessors
    int getLevel(Id id)
    {
        AsteroidComponent* c = getUnlocked(id);
        return c ? c->level : 0;
    }

    float getDefaultSpeed(Id id)
    {
        AsteroidComponent* c = getUnlocked(id);
        return c ? c->defaultSpeed : 0.0f;
    }

    void setDirection(Id id, const sf::Vector2f& dir)
    {
        AsteroidComponent* c = getUnlocked(id);
        if (!c) return;
        c->direction = dir;
    }

    sf::Vector2f getDirection(Id id)
    {
        AsteroidComponent* c = getUnlocked(id);
        return c ? c->direction : sf::Vector2f{};
    }

    void setSpeed(Id id, float s)
    {
        AsteroidComponent* c = getUnlocked(id);
        if (!c) return;
        c->speed = s;
    }

    float getSpeed(Id id)
    {
        AsteroidComponent* c = getUnlocked(id);
        return c ? c->speed : 0.0f;
    }

    const sf::CircleShape* getShape(Id id)
    {
        AsteroidComponent* c = getUnlocked(id);
        return c ? &c->shape : nullptr;
    }

private:
    AsteroidComponentManager() = default;
    ~AsteroidComponentManager() = default;

    AsteroidComponent* getUnlocked(Id id)
    {
        // _caller_ should lock for write; here we lock per-access for safety.
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = components_.find(id);
        if (it == components_.end()) return nullptr;
        return &it->second;
    }

    std::mutex mutex_;
    std::atomic<Id> nextId_{ 1 };
    std::unordered_map<Id, AsteroidComponent> components_;
    std::unordered_map<Asteroid*, Id> ownerMap_;

    static constexpr float BaseRadius{ 10.0f };
    static constexpr float RadiusStep{ 10.0f };
};