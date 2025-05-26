#ifndef WINDOW_HANDLER_HPP
#define WINDOW_HANDLER_HPP

#include <string>
#include <SFML/Graphics.hpp>

#include "event_manager.hpp"
#include "viewport_handler.hpp"
#include "utils.hpp"

class WindowHandler {
public:
    WindowHandler(const std::string &window_name, sf::Vector2u window_size)
        : m_window(sf::VideoMode(window_size.x, window_size.y), window_name, sf::Style::Default)
        , m_viewport_handler({static_cast<float>(window_size.x), static_cast<float>(window_size.y)})
        , m_event_manager(m_window)
    {
        m_window.setFramerateLimit(60);
        registerCallbacks(m_event_manager);
    }

    sf::Vector2u getWindowSize() const {
        return m_window.getSize();
    }

    V2f getMouseWorldPosition() const {
        return m_viewport_handler.getMouseWorldPosition();
    }

    EventManager &getEventManager() {
        return m_event_manager;
    }

    void setFramerateLimit(const uint32_t limit) {
        m_window.setFramerateLimit(limit);
    }

    void setFocus(const V2f focus) {
        m_viewport_handler.setFocus(focus);
    }

    void setZoom(const float zoom) {
        m_viewport_handler.setZoom(zoom);
    }

    void registerCallbacks(EventManager &event_manager) {
        event_manager.addEventCallback(sf::Event::Closed, [&](const sf::Event&) { m_window.close(); });
        event_manager.addMousePressedCallback(sf::Mouse::Left, [&](const sf::Event&) { m_viewport_handler.click(event_manager.getMousePosition()); });
        event_manager.addMouseReleasedCallback(sf::Mouse::Left, [&](const sf::Event&) { m_viewport_handler.unClick(); });
        event_manager.addEventCallback(sf::Event::MouseMoved, [&](const sf::Event&) { m_viewport_handler.setMousePosition(event_manager.getMousePosition()); });
        event_manager.addEventCallback(sf::Event::MouseWheelScrolled, [&](const sf::Event &e) { m_viewport_handler.wheelZoom(e.mouseWheelScroll.delta); });
    }

    void draw(const sf::Drawable &drawable, sf::RenderStates render_states = {}) {
        render_states.transform = m_viewport_handler.getTransform();
        m_window.draw(drawable, render_states);
    }

    bool run() const {
        m_event_manager.processEvent();
        return m_window.isOpen();
    }

    void clear(sf::Color color = sf::Color::Black) {
        m_window.clear(color);
    }

    void display() {
        m_window.display();
    }

    void displayText(const sf::Font &font, const std::string &text, const V2f &position, const sf::Color color = sf::Color::White, const uint32_t size = 25) {
        sf::Text text_obj(text, font, size);
        text_obj.setFillColor(color);
        text_obj.setPosition(position);
        m_window.draw(text_obj);
    }

private:
    sf::RenderWindow m_window;
    ViewportHandler m_viewport_handler;
    EventManager m_event_manager;
};

#endif
