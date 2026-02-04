#include "Game.h"
#include "Asteroid.h"
#include "Bullet.h"
#include "Button.h"
#include "EventBus.h"
#include "InputEvents.h"
#include "AsteroidComponent.h"
#include <algorithm>
#include <iostream>
#include <cmath>

Game::Game() :
    gameState(GameState::MENU),
    font("Resources/consolas.ttf"),
    bulletPool(20),
    asteroidPool(50),
    shootCooldown(0.25f),
    asteroidCooldown(1.5f),
    gameZoneMargin(100.0f),
    isPlayerInsideZone(false),
    pointsPerAsteroidLevel(5),
    timeToCompleteZone(20.0f),
    pointsPerZoneComplete(50)
{
    mouseSubId = GlobalEventBus().subscribe<MouseEvent>(
        [this](const MouseEvent& ev)
        {
            if (gameState != GameState::PLAYING) return;

            if (ev.action == MouseEvent::Action::ButtonPress &&
                ev.button == static_cast<int>(sf::Mouse::Button::Left))
            {
                tryShoot();
            }
        });
}

Game::~Game()
{
    if (mouseSubId) GlobalEventBus().unsubscribe<MouseEvent>(mouseSubId);
}

void Game::run()
{
    sf::Clock deltaTimeClock;

    window.create(sf::VideoMode::getDesktopMode(), "Spacewar Test");

    initializeUI();
    initializeTexts();

    while (window.isOpen())
    {
        sf::Time deltaTime = deltaTimeClock.restart();

        handleInput();
        update(deltaTime.asSeconds());
        render();
    }
}

void Game::handleInput()
{
    while (const std::optional event = window.pollEvent())
    {
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            KeyEvent ke{ static_cast<int>(keyPressed->scancode), KeyEvent::Action::Press };
            GlobalEventBus().publish<KeyEvent>(ke);
        }
        if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
        {
            KeyEvent ke{ static_cast<int>(keyReleased->scancode), KeyEvent::Action::Release };
            GlobalEventBus().publish<KeyEvent>(ke);
        }
        if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
        {
            MouseEvent me{ static_cast<float>(mouseMoved->position.x), static_cast<float>(mouseMoved->position.y), -1, MouseEvent::Action::Move };
            GlobalEventBus().publish<MouseEvent>(me);
        }
        if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            MouseEvent me{ static_cast<float>(mousePressed->position.x), static_cast<float>(mousePressed->position.y), static_cast<int>(mousePressed->button), MouseEvent::Action::ButtonPress };
            GlobalEventBus().publish<MouseEvent>(me);
        }
        if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
        {
            MouseEvent me{ static_cast<float>(mouseReleased->position.x), static_cast<float>(mouseReleased->position.y), static_cast<int>(mouseReleased->button), MouseEvent::Action::ButtonRelease };
            GlobalEventBus().publish<MouseEvent>(me);
        }

        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }

        switch (gameState)
        {
        case GameState::MENU:
            handleMenuInput(*event);
            break;

        case GameState::GAME_OVER:
        case GameState::WIN:
            backToMenuButton->update(sf::Vector2f(sf::Mouse::getPosition(window)), event);
            break;

        case GameState::PLAYING:
            handleGameInput(*event);
            break;

        case GameState::PAUSED:
            handlePausedInput(*event);
            break;

        default:
            break;
        }
    }
}

void Game::handleMenuInput(const sf::Event& event)
{
    startButton->update(sf::Vector2f(sf::Mouse::getPosition(window)), event);
    exitButton->update(sf::Vector2f(sf::Mouse::getPosition(window)), event);
}

void Game::handleGameInput(const sf::Event& event)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
        {
            pause();
        }
    }
}

void Game::handlePausedInput(const sf::Event& event)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        resume();
    }
}

void Game::update(float deltaTime)
{
    if (gameState == GameState::PLAYING)
    {
        for (Entity* entity : entities)
        {
            if (!entity) continue;

            entity->update(deltaTime);
        }

        isPlayerInsideZone = player.intersects(zone);
        if (isPlayerInsideZone)
        {
            if (!zoneTimer.isRunning())
            {
                zoneTimer.restart();
            }

            if (zoneTimer.getElapsedTime().asSeconds() >= timeToCompleteZone)
            {
                ++zonesCompleted;
                score += pointsPerZoneComplete;
                spawnZone();
            }
        }
        else
        {
            if (zoneTimer.isRunning())
            {
                zoneTimer.stop();
            }
        }

        constrainPlayerMovement();

        checkCollisions();
        trySpawnAsteroid();
    }

    const std::string playtimeSecondsString = "Time: " + std::to_string(static_cast<int>(playtimeTimer.getElapsedTime().asSeconds()));
    playtimeText->setString(playtimeSecondsString);

    const std::string scoreString = "Score: " + std::to_string(score);
    scoreText->setString(scoreString);
    scoreText->setOrigin(sf::Vector2f(scoreText->getLocalBounds().size.x + 10.0f, 0.0f));

    const std::string remainintTimeInZoneText = std::to_string(static_cast<int>(1 + timeToCompleteZone - zoneTimer.getElapsedTime().asSeconds()));
    timeInZoneText->setString(remainintTimeInZoneText);
}

void Game::render()
{
    window.clear();

    switch (gameState)
    {
    case GameState::MENU:
        window.draw(*startButton.get());
        window.draw(*exitButton.get());
        break;
    case GameState::PAUSED:
        window.draw(*pauseText.get());
    case GameState::PLAYING:
        for (Entity* entity : entities)
        {
            window.draw(*entity);
        }
        if (isPlayerInsideZone)
        {
            window.draw(*timeInZoneText.get());
        }
        window.draw(*playtimeText.get());
        window.draw(*scoreText.get());
        break;

    case GameState::GAME_OVER:
    case GameState::WIN:
        window.draw(*backToMenuButton.get());
        window.draw(*resultsText.get());
        break;

    default:
        break;
    }

    window.display();
}

void Game::checkCollisions()
{
    for (Bullet* bullet : bulletPool.getActiveObjects())
    {
        if (!bullet) continue;

        if (isEntityOutOfBounds(*bullet))
        {
            bulletPool.release(bullet);
            entities.remove(bullet);
            continue;
        }

        const sf::Vector2f bulletPos = bullet->getPosition();
        const float bulletRadius = bullet->getCollisionRadius();
        bool bulletConsumed = false;

        for (Asteroid* asteroid : asteroidPool.getActiveObjects())
        {
            if (!asteroid) continue;

            float asteroidRadius = AsteroidComponentManager::instance().getRadiusByOwner(asteroid);
            if (asteroidRadius <= 0.0f) continue;

            const sf::Vector2f asteroidPos = asteroid->getPosition();
            const float dx = bulletPos.x - asteroidPos.x;
            const float dy = bulletPos.y - asteroidPos.y;
            const float distSq = dx * dx + dy * dy;
            const float minDist = bulletRadius + asteroidRadius;

            if (distSq <= (minDist * minDist))
            {
                score += pointsPerAsteroidLevel * asteroid->getLevel();

                bulletPool.release(bullet);
                entities.remove(bullet);

                splitAsteroid(asteroid);
                bulletConsumed = true;
                break;
            }
        }

        if (bulletConsumed) continue;
    }

    const sf::Vector2f playerPos = player.getPosition();
    const float playerRadius = player.getCollisionRadius();

    for (Asteroid* asteroid : asteroidPool.getActiveObjects())
    {
        if (!asteroid) continue;

        if (isEntityOutOfBounds(*asteroid))
        {
            asteroidPool.release(asteroid);
            entities.remove(asteroid);
            continue;
        }

        float asteroidRadius = AsteroidComponentManager::instance().getRadiusByOwner(asteroid);
        if (asteroidRadius <= 0.0f) continue;

        const sf::Vector2f asteroidPos = asteroid->getPosition();
        const float dx = playerPos.x - asteroidPos.x;
        const float dy = playerPos.y - asteroidPos.y;
        const float distSq = dx * dx + dy * dy;
        const float minDist = playerRadius + asteroidRadius;

        if (distSq <= (minDist * minDist))
        {
            gameState = GameState::GAME_OVER;
            finishGame();
            break;
        }
    }
}

void Game::restart()
{
    gameState = GameState::PLAYING;
    zonesCompleted = 0;
    score = 0;
    player.setSpeed(0.0f);
    player.setRotation(sf::Angle());
    player.setPosition(sf::Vector2f(window.getSize()) / 2.0f);

    entities.clear();
    entities.push_back(&player);
    entities.push_back(&zone);

    for (Bullet* bullet : bulletPool.getActiveObjects())
    {
        entities.remove(bullet);
    }

    for (Asteroid* asteroid : asteroidPool.getActiveObjects())
    {
        asteroidPool.release(asteroid);
    }

    shootTimer.restart();
    asteroidTimer.restart();
    playtimeTimer.restart();

    spawnZone();
}

void Game::pause()
{
    gameState = GameState::PAUSED;
    playtimeTimer.stop();

    if (zoneTimer.isRunning())
    {
        zoneTimer.stop();
    }
}

void Game::resume()
{
    gameState = GameState::PLAYING;
    playtimeTimer.start();

    if (isPlayerInsideZone)
    {
        zoneTimer.start();
    }
}

void Game::finishGame()
{
    std::string endGameText;
    if (gameState == GameState::WIN)
    {
        endGameText = "You have completed all objectives!";
    }
    else
    {
        endGameText = "You died!";
    }

    entities.clear();

    for (Bullet* bullet : bulletPool.getActiveObjects())
    {
        entities.remove(bullet);
    }

    for (Asteroid* asteroid : asteroidPool.getActiveObjects())
    {
        asteroidPool.release(asteroid);
    }

    endGameText += "\nTotal time played: " + std::to_string(static_cast<int>(playtimeTimer.getElapsedTime().asSeconds())) + " seconds";
    endGameText += "\nTotal Score: " + std::to_string(score) + " points";

    resultsText->setString(endGameText);
    resultsText->setOrigin(resultsText->getLocalBounds().getCenter());
    resultsText->setPosition(sf::Vector2f(window.getSize().x / 2, window.getSize().y / 3));
}

void Game::tryShoot()
{
    if (shootTimer.getElapsedTime().asSeconds() <= shootCooldown) return;

    Bullet* bullet = bulletPool.acquire();
    if (bullet)
    {
        sf::Vector2f direction = getDirectionToMouse(player.getPosition());
        bullet->setPosition(player.getPosition());
        bullet->setDirection(direction);
        entities.push_back(bullet);
    }

    shootTimer.restart();
}

void Game::trySpawnAsteroid()
{
    if (asteroidTimer.getElapsedTime().asSeconds() <= asteroidCooldown) return;

    Asteroid* asteroid = asteroidPool.acquire();
    if (!asteroid) return;

    sf::Vector2u windowSize = window.getSize();

    float x, y;
    float spawnMargin = 50.0f;
    int side = rand() % 4;

    switch (side) {
    case 0: // Up
        x = rand() % windowSize.x;
        y = -spawnMargin;
        break;
    case 1: // Down
        x = rand() % windowSize.x;
        y = windowSize.y + spawnMargin;
        break;
    case 2: // Left
        x = -spawnMargin;
        y = rand() % windowSize.y;
        break;
    case 3: // Right
        x = windowSize.x + spawnMargin;
        y = rand() % windowSize.y;
        break;
    default:
        x = 0;
        y = 0;
        break;
    }

    sf::Vector2f center;
    if (isPlayerInsideZone)
    {
        center = player.getPosition();
    }
    else
    {
        center = sf::Vector2f(windowSize.x / 2.0f + rand() % 500 - 250, windowSize.y / 2.0f + rand() % 500 - 250);
    }

    sf::Vector2f direction = center - sf::Vector2f(x, y);
    normalizeVector(direction);

    asteroid->setPosition({ x, y });

    AsteroidComponentManager::instance().setDirectionByOwner(asteroid, direction);
    AsteroidComponentManager::instance().setLevelByOwner(asteroid, rand() % 3 + 1);
    float speed = AsteroidComponentManager::instance().getDefaultSpeedByOwner(asteroid) + (rand() % 200 - 100);
    AsteroidComponentManager::instance().setSpeedByOwner(asteroid, speed);

    entities.push_back(asteroid);

    asteroidTimer.restart();
}

void Game::splitAsteroid(Asteroid* asteroid)
{
    if (!asteroid) return;

    int currentLevel = AsteroidComponentManager::instance().getLevelByOwner(asteroid);
    if (currentLevel <= 1)
    {
        asteroidPool.release(asteroid);
        entities.remove(asteroid);
        return;
    }

    AsteroidComponentManager::instance().decreaseLevel(AsteroidComponentManager::instance().getIdForOwner(asteroid));

    Asteroid* newAsteroid = asteroidPool.acquire();
    if (!newAsteroid) return;

    int newLevel = AsteroidComponentManager::instance().getLevelByOwner(asteroid);
    AsteroidComponentManager::instance().setLevelByOwner(newAsteroid, newLevel);

    newAsteroid->setPosition(asteroid->getPosition());

    auto dirId = AsteroidComponentManager::instance().getIdForOwner(asteroid);
    sf::Vector2f origDir = AsteroidComponentManager::instance().getDirection(dirId);
    sf::Vector2f dirA = origDir;
    sf::Vector2f dirB = origDir;

    float angleA = static_cast<float>(rand() % 50 - 25);
    float angleB = static_cast<float>(rand() % 50 - 25);
    dirA = dirA;
    dirB = dirB;

    auto rotateVec = [](sf::Vector2f v, float degrees)->sf::Vector2f {
        const float rad = degrees * 3.14159265358979323846f / 180.0f;
        float c = std::cos(rad), s = std::sin(rad);
        return sf::Vector2f(v.x * c - v.y * s, v.x * s + v.y * c);
    };

    sf::Vector2f newDirA = rotateVec(origDir, angleA);
    sf::Vector2f newDirB = rotateVec(origDir, angleB);

    AsteroidComponentManager::instance().setDirectionByOwner(newAsteroid, newDirA);
    AsteroidComponentManager::instance().setDirectionByOwner(asteroid, newDirB);

    float speedA = AsteroidComponentManager::instance().getDefaultSpeedByOwner(newAsteroid) + (rand() % 200 - 100);
    float speedB = AsteroidComponentManager::instance().getDefaultSpeedByOwner(asteroid) + (rand() % 200 - 100);

    AsteroidComponentManager::instance().setSpeedByOwner(newAsteroid, speedA);
    AsteroidComponentManager::instance().setSpeedByOwner(asteroid, speedB);

    entities.push_back(newAsteroid);
}

void Game::spawnZone()
{
    if (zonesCompleted > 2)
    {
        gameState = GameState::WIN;
        finishGame();
        return;
    }

    sf::Vector2u windowSize = window.getSize();
    sf::Vector2u zoneLocation;
    zoneLocation.x = windowSize.x / 4 * (zonesCompleted + 1);
    zoneLocation.y = windowSize.y / 2;

    zone.setPosition(sf::Vector2f(zoneLocation));
}

float Game::isEntityOutOfBounds(const Entity& entity)
{
    sf::Vector2f entityPosition = entity.getPosition();

    float left = 0 - gameZoneMargin;
    float right = window.getSize().x + gameZoneMargin;
    float top = 0 - gameZoneMargin;
    float bottom = window.getSize().y + gameZoneMargin;

    bool outOfBounds = (entityPosition.x <= left || entityPosition.x >= right || entityPosition.y <= top || entityPosition.y >= bottom);
    return outOfBounds;
}

void Game::initializeUI()
{
    const sf::Vector2f screenCenter = sf::Vector2f(window.getSize()) / 2.0f;
    startButton = std::make_unique<Button>(screenCenter + sf::Vector2f(0.0f, -35.0f), sf::Vector2f(150.0f, 50.0f), font, "START", [this]() { restart(); });
    exitButton = std::make_unique<Button>(screenCenter + sf::Vector2f(0.0f, 35.0f), sf::Vector2f(150.0f, 50.0f), font, "EXIT", [this]() { window.close(); });
    backToMenuButton = std::make_unique<Button>(screenCenter, sf::Vector2f(250.0f, 50.0f), font, "BACK TO MENU", [this]() { gameState = GameState::MENU; });
}

void Game::initializeTexts()
{
    const sf::Vector2f screenCenter = sf::Vector2f(window.getSize()) / 2.0f;
    pauseText = std::make_unique<sf::Text>(font, "PAUSE");
    pauseText->setOrigin(pauseText->getLocalBounds().getCenter());
    pauseText->setPosition(screenCenter);

    timeInZoneText = std::make_unique<sf::Text>(font, "");
    timeInZoneText->setPosition({ screenCenter.x, 0.0f });

    scoreText = std::make_unique<sf::Text>(font, "");
    scoreText->setPosition(sf::Vector2f(window.getSize().x, 0.0f));

    playtimeText = std::make_unique<sf::Text>(font, "");
    playtimeText->setPosition(sf::Vector2f(10.0f, 0.0f));

    resultsText = std::make_unique<sf::Text>(font, "");
}

void Game::constrainPlayerMovement()
{
    sf::Vector2f playerCorrectedPosition;
    playerCorrectedPosition.x = std::clamp(player.getPosition().x, player.getRadius(), static_cast<float>(window.getSize().x) - player.getRadius());
    playerCorrectedPosition.y = std::clamp(player.getPosition().y, player.getRadius(), static_cast<float>(window.getSize().y) - player.getRadius());
    player.setPosition(playerCorrectedPosition);
}

void Game::normalizeVector(sf::Vector2f& vector)
{
    float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (length > 0) {
        vector.x /= length;
        vector.y /= length;
    }
}

sf::Vector2f Game::getDirectionToMouse(const sf::Vector2f& playerPosition)
{
    sf::Vector2i mouseScreenPosition = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorldPosition = window.mapPixelToCoords(mouseScreenPosition);

    sf::Vector2f direction = mouseWorldPosition - playerPosition;

    normalizeVector(direction);

    return direction;
}
