#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <list>
#include "ObjectPool.h"
#include "Player.h"
#include "Zone.h"
#include "Button.h"
#include "EventBus.h"

class Entity;
class Bullet;
class Asteroid;

class Game
{
public:
	Game();
	~Game();
	void run();

private:
	enum class GameState { MENU, PLAYING, PAUSED, GAME_OVER, WIN };
	GameState gameState;
	sf::RenderWindow window;
	sf::Font font;

	Player player;
	ObjectPool<Bullet> bulletPool;
	ObjectPool<Asteroid> asteroidPool;
	std::list<Entity*> entities;
	
	Zone zone;
	sf::Clock zoneTimer;
	sf::Clock playtimeTimer;
	float timeToCompleteZone;
	int zonesCompleted{};
	bool isPlayerInsideZone;

	float shootCooldown;
	sf::Clock shootTimer;
	float asteroidCooldown;
	sf::Clock asteroidTimer;

	float gameZoneMargin;
	int pointsPerAsteroidLevel;
	int pointsPerZoneComplete;
	int score{};

	std::unique_ptr<Button> startButton;
	std::unique_ptr<Button> exitButton;
	std::unique_ptr<Button> backToMenuButton;

	std::unique_ptr<sf::Text> pauseText;
	std::unique_ptr<sf::Text> timeInZoneText;
	std::unique_ptr<sf::Text> scoreText;
	std::unique_ptr<sf::Text> playtimeText;
	std::unique_ptr<sf::Text> resultsText;

	void handleInput();
	void handleMenuInput(const sf::Event& event);
	void handleGameInput(const sf::Event& event);
	void handlePausedInput(const sf::Event& event);
	void update(float deltaTime);
	void render();
	void checkCollisions();
	void restart();
	void pause();
	void resume();
	void finishGame();

	void tryShoot();
	void trySpawnAsteroid();
	void splitAsteroid(Asteroid* asteroid);
	void spawnZone();
	float isEntityOutOfBounds(const Entity& entity);

	void initializeUI();
	void initializeTexts();

	void constrainPlayerMovement();

	void normalizeVector(sf::Vector2f& vector);
	sf::Vector2f getDirectionToMouse(const sf::Vector2f& playerPosition);

	EventBus::HandlerId mouseSubId{0};
};
