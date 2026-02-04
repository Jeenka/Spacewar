#include "Player.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include "EventBus.h"
#include "InputEvents.h"

Player::Player() : Entity(0.0f),
	drag(100.0f),
	accelerationSpeed(400.0f),
	maxSpeed(400.0f),
	turnRate(200.0f)
{
	shape.setFillColor(sf::Color::White);
	shape.setRadius(30.0f);
	shape.setPointCount(3);
	shape.setOrigin({ shape.getRadius(), shape.getRadius() });

	headingShape.setFillColor(sf::Color::Black);
	headingShape.setRadius(10.0f);
	headingShape.setPointCount(3);
	headingShape.setOrigin({ headingShape.getRadius(), headingShape.getRadius() + 7.0f });
	headingShape.setPosition(shape.getPosition());

	// Subscribe to key events
	keySubId = GlobalEventBus().subscribe<KeyEvent>(
		[this](const KeyEvent& ev)
		{
			using KE = KeyEvent;
			// Map arrow keys / WASD to thrust and turning for demonstration.
			const int keyUp = static_cast<int>(sf::Keyboard::Key::Up);
			const int keyDown = static_cast<int>(sf::Keyboard::Key::Down);
			const int keyLeft = static_cast<int>(sf::Keyboard::Key::Left);
			const int keyRight = static_cast<int>(sf::Keyboard::Key::Right);
			const int keyW = static_cast<int>(sf::Keyboard::Key::W);
			const int keyS = static_cast<int>(sf::Keyboard::Key::S);
			const int keyA = static_cast<int>(sf::Keyboard::Key::A);
			const int keyD = static_cast<int>(sf::Keyboard::Key::D);

			if (ev.action == KE::Action::Press)
			{
				if (ev.key == keyUp || ev.key == keyW) { thrust = 1.0f; }
				if (ev.key == keyDown || ev.key == keyS) { thrust = -1.0f; }
				if (ev.key == keyLeft || ev.key == keyA) { turnDirection = -1.0f; }
				if (ev.key == keyRight || ev.key == keyD) { turnDirection = 1.0f; }
			}
			else // Release
			{
				if (ev.key == keyUp || ev.key == keyW) { if (thrust > 0.0f) thrust = 0.0f; }
				if (ev.key == keyDown || ev.key == keyS) { if (thrust < 0.0f) thrust = 0.0f; }
				if (ev.key == keyLeft || ev.key == keyA) { if (turnDirection < 0.0f) turnDirection = 0.0f; }
				if (ev.key == keyRight || ev.key == keyD) { if (turnDirection > 0.0f) turnDirection = 0.0f; }
			}
		});
}

Player::~Player()
{
	// Unsubscribe to avoid dangling handlers (safe even if id==0)
	if (keySubId) GlobalEventBus().unsubscribe<KeyEvent>(keySubId);
	if (mouseSubId) GlobalEventBus().unsubscribe<MouseEvent>(mouseSubId);
}

void Player::update(float deltaTime)
{
	rotate(sf::degrees(turnDirection * turnRate * deltaTime));

	float rotation = getRotation().asRadians() + sf::degrees(-90.0f).asRadians();
	direction = sf::Vector2f(cos(rotation), sin(rotation));
	speed = std::clamp(speed + thrust * accelerationSpeed * deltaTime - drag * deltaTime, 0.0f, maxSpeed);
	move(direction * speed * deltaTime);
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	Entity::draw(target, states);

	states.transform = getTransform();
	target.draw(headingShape, states);
}
