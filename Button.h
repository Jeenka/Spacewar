#pragma once

#include <SFML/Graphics.hpp>
#include <functional>

class Button : public sf::Transformable, public sf::Drawable
{

public:
    Button(sf::Vector2f position, sf::Vector2f size, const sf::Font& font, const std::string& label, std::function<void()> callback);

    void update(const sf::Vector2f& mousePos, const std::optional<sf::Event>& event);

private:
    sf::RectangleShape shape;
    sf::Text text;
    sf::Color color;
    sf::Color hoverColor;
    sf::Color pressedColor;
    bool isPressed;
    std::function<void()> onClick;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

