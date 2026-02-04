#include "Button.h"

Button::Button(sf::Vector2f position, sf::Vector2f size, const sf::Font& font, const std::string& label, std::function<void()> callback) :
    text(font),
    color(sf::Color(100, 200, 100)),
    hoverColor(sf::Color(75, 150, 75)),
    pressedColor(sf::Color(50, 125, 50)),
    isPressed(false)
{
    shape.setSize(size);
    shape.setOrigin(shape.getLocalBounds().getCenter());
    shape.setPosition(position);
    shape.setFillColor(color);
    shape.setOutlineThickness(4.0f);
    shape.setOutlineColor(hoverColor);

    text.setString(label);
    text.setCharacterSize(32);
    text.setFillColor(sf::Color::White);
    text.setOrigin(text.getLocalBounds().getCenter());
    text.setPosition(shape.getPosition());

    onClick = callback;
}

void Button::update(const sf::Vector2f& mousePos, const std::optional<sf::Event>& event)
{
    bool isHovered = shape.getGlobalBounds().contains(mousePos);
    if (isHovered)
    {
        if (!isPressed)
        {
            shape.setFillColor(hoverColor);

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                shape.setFillColor(pressedColor);
                isPressed = true;
                return;
            }
        }

        if (event->is<sf::Event::MouseButtonReleased>())
        {
            isPressed = false;
            shape.setFillColor(color);
            onClick();
        }
    }
    else
    {
        isPressed = false;
        shape.setFillColor(color);
    }
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform = getTransform();
    target.draw(shape, states);
    target.draw(text, states);
}
