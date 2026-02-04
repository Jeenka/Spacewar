#include "AsteroidComponent.h"
#include "Asteroid.h"
#include <algorithm>
#include <vector>

AsteroidComponentManager& AsteroidComponentManager::instance()
{
    static AsteroidComponentManager mgr;
    return mgr;
}

AsteroidComponentManager::Id AsteroidComponentManager::create(Asteroid* owner, int initialLevel)
{
    std::unique_lock lock(mutex_);

    Id id = nextId_++;
    AsteroidComponent comp;
    comp.id = id;
    comp.owner = owner;
    comp.level = std::max(0, initialLevel);
    comp.rotationSpeed = 25.0f;
    comp.defaultSpeed = 400.0f;
    comp.speed = comp.defaultSpeed;
    comp.direction = {0.f, 0.f};

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
        owner->setShapePointCount(8u);
    }

    return id;
}

void AsteroidComponentManager::destroy(Id id)
{
    std::unique_lock lock(mutex_);
    auto it = components_.find(id);
    if (it != components_.end()) {
        if (it->second.owner) ownerMap_.erase(it->second.owner);
        components_.erase(it);
    }
}

void AsteroidComponentManager::update(Id id, float deltaTime)
{
    std::unique_lock lock(mutex_);
    AsteroidComponent* c = findComponentLocked(id);
    if (!c) return;

    c->shape.rotate(sf::degrees(c->rotationSpeed * deltaTime));

    if (c->owner) c->owner->move(c->direction * c->speed * deltaTime);
}

void AsteroidComponentManager::setLevel(Id id, int newLevel)
{
    std::unique_lock lock(mutex_);
    AsteroidComponent* c = findComponentLocked(id);
    if (!c) return;
    c->level = std::max(0, newLevel);
    const float r = BaseRadius + c->level * RadiusStep;
    c->shape.setRadius(r);
    c->shape.setOrigin({ r, r });
    if (c->owner) c->owner->setCollisionRadius(r);
}

void AsteroidComponentManager::decreaseLevel(Id id)
{
    std::unique_lock lock(mutex_);
    AsteroidComponent* c = findComponentLocked(id);
    if (!c) return;
    if (c->level > 0) --c->level;
    const float r = BaseRadius + c->level * RadiusStep;
    c->shape.setRadius(r);
    c->shape.setOrigin({ r, r });
    if (c->owner) c->owner->setCollisionRadius(r);
}

int AsteroidComponentManager::getLevel(Id id)
{
    std::shared_lock lock(mutex_);
    AsteroidComponent* c = findComponentShared(id);
    return c ? c->level : 0;
}

float AsteroidComponentManager::getDefaultSpeed(Id id)
{
    std::shared_lock lock(mutex_);
    AsteroidComponent* c = findComponentShared(id);
    return c ? c->defaultSpeed : 0.0f;
}

void AsteroidComponentManager::setDirection(Id id, const sf::Vector2f& dir)
{
    std::unique_lock lock(mutex_);
    AsteroidComponent* c = findComponentLocked(id);
    if (!c) return;
    c->direction = dir;
}

sf::Vector2f AsteroidComponentManager::getDirection(Id id)
{
    std::shared_lock lock(mutex_);
    AsteroidComponent* c = findComponentShared(id);
    return c ? c->direction : sf::Vector2f{};
}

void AsteroidComponentManager::setSpeed(Id id, float s)
{
    std::unique_lock lock(mutex_);
    AsteroidComponent* c = findComponentLocked(id);
    if (!c) return;
    c->speed = s;
}

float AsteroidComponentManager::getSpeed(Id id)
{
    std::shared_lock lock(mutex_);
    AsteroidComponent* c = findComponentShared(id);
    return c ? c->speed : 0.0f;
}

float AsteroidComponentManager::getRadius(Id id)
{
    std::shared_lock lock(mutex_);
    AsteroidComponent* c = findComponentShared(id);
    return c ? c->shape.getRadius() : 0.0f;
}

sf::CircleShape AsteroidComponentManager::getShapeCopy(Id id)
{
    std::shared_lock lock(mutex_);
    AsteroidComponent* c = findComponentShared(id);
    return c ? c->shape : sf::CircleShape{};
}

AsteroidComponentManager::Id AsteroidComponentManager::getIdForOwner(Asteroid* owner)
{
    std::shared_lock lock(mutex_);
    auto it = ownerMap_.find(owner);
    return (it != ownerMap_.end()) ? it->second : 0;
}

void AsteroidComponentManager::setDirectionByOwner(Asteroid* owner, const sf::Vector2f& dir)
{
    Id id = getIdForOwner(owner);
    if (id) setDirection(id, dir);
}

void AsteroidComponentManager::setSpeedByOwner(Asteroid* owner, float s)
{
    Id id = getIdForOwner(owner);
    if (id) setSpeed(id, s);
}

void AsteroidComponentManager::setLevelByOwner(Asteroid* owner, int level)
{
    Id id = getIdForOwner(owner);
    if (id) setLevel(id, level);
}

int AsteroidComponentManager::getLevelByOwner(Asteroid* owner)
{
    Id id = getIdForOwner(owner);
    return id ? getLevel(id) : 0;
}

float AsteroidComponentManager::getDefaultSpeedByOwner(Asteroid* owner)
{
    Id id = getIdForOwner(owner);
    return id ? getDefaultSpeed(id) : 0.0f;
}

float AsteroidComponentManager::getRadiusByOwner(Asteroid* owner)
{
    Id id = getIdForOwner(owner);
    return id ? getRadius(id) : 0.0f;
}

std::vector<AsteroidComponentManager::Id> AsteroidComponentManager::snapshotIds()
{
    std::shared_lock lock(mutex_);
    std::vector<Id> out;
    out.reserve(components_.size());
    for (auto const& kv : components_) out.push_back(kv.first);
    return out;
}

AsteroidComponent* AsteroidComponentManager::findComponentLocked(Id id)
{
    auto it = components_.find(id);
    return it != components_.end() ? &it->second : nullptr;
}

AsteroidComponent* AsteroidComponentManager::findComponentShared(Id id)
{
    auto it = components_.find(id);
    return it != components_.end() ? &it->second : nullptr;
}