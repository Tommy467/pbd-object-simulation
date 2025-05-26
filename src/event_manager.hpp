#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <functional>
#include <utility>

#include "utils.hpp"

template<typename T>
class SubEventManager {
public:
    explicit SubEventManager(std::function<T(const sf::Event&)> unpack)
        : m_unpack(std::move(unpack))
    {}

    ~SubEventManager() = default;

    void processEvent(const sf::Event &event) const {
        T sub_value = m_unpack(event);
        if (const auto it(m_call_map.find(sub_value)); it != m_call_map.end()) {
            it->second(event);
        }
    }

    void addCallback(const T &sub_value, EventCallback callback) {
        m_call_map[sub_value] = callback;
    }

private:
    EventCallbackMap<T> m_call_map;
    std::function<T(const sf::Event&)> m_unpack;
};

class EventMap {
public:
    EventMap()
        : m_key_pressed_manager([](const sf::Event &event) { return event.key.code; })
        , m_key_released_manager([](const sf::Event &event) { return event.key.code; })
        , m_mouse_pressed_manager([](const sf::Event &event) { return event.mouseButton.button; })
        , m_mouse_released_manager([](const sf::Event &event) { return event.mouseButton.button; })
    {
        this->addEventCallback(sf::Event::EventType::KeyPressed, [&](const sf::Event &event) { m_key_pressed_manager.processEvent(event); });
        this->addEventCallback(sf::Event::EventType::KeyReleased, [&](const sf::Event &event) { m_key_released_manager.processEvent(event); });
        this->addEventCallback(sf::Event::EventType::MouseButtonPressed, [&](const sf::Event &event) { m_mouse_pressed_manager.processEvent(event); });
        this->addEventCallback(sf::Event::EventType::MouseButtonReleased, [&](const sf::Event &event) { m_mouse_released_manager.processEvent(event); });
    }

    void addEventCallback(const sf::Event::EventType type, const EventCallback &callback) {
        m_events_call_map[type] = callback;
    }

    void addKeyPressedCallback(const sf::Keyboard::Key key_code, const EventCallback &callback) {
        m_key_pressed_manager.addCallback(key_code, callback);
    }

    void addKeyReleasedCallback(const sf::Keyboard::Key key_code, const EventCallback &callback) {
        m_key_released_manager.addCallback(key_code, callback);
    }

    void addMousePressedCallback(const sf::Mouse::Button button, const EventCallback &callback) {
        m_mouse_pressed_manager.addCallback(button, callback);
    }

    void addMouseReleasedCallback(const sf::Mouse::Button button, const EventCallback &callback) {
        m_mouse_released_manager.addCallback(button, callback);
    }

    void executeCallback(const sf::Event &event, const EventCallback& fallback = nullptr) const {
        if (const auto it(m_events_call_map.find(event.type)); it != m_events_call_map.end()) {
            it->second(event);
        } else if (fallback) {
            fallback(event);
        }
    }

    void removeCallback(sf::Event::EventType type) {
        if (const auto it(m_events_call_map.find(type)); it != m_events_call_map.end()) {
            m_events_call_map.erase(it);
        }
    }

private:
    SubEventManager<sf::Keyboard::Key> m_key_pressed_manager;
    SubEventManager<sf::Keyboard::Key> m_key_released_manager;
    SubEventManager<sf::Mouse::Button> m_mouse_pressed_manager;
    SubEventManager<sf::Mouse::Button> m_mouse_released_manager;
    EventCallbackMap<sf::Event::EventType> m_events_call_map;
};

class EventManager {
public:
    explicit EventManager(sf::Window &window)
        : m_window(window)
        , m_event_map()
    {}

    void processEvent(const EventCallback& fallback = nullptr) const {
        sf::Event event{};
        while (m_window.pollEvent(event)) {
            m_event_map.executeCallback(event, fallback);
        }
    }

    void addEventCallback(const sf::Event::EventType type, const EventCallback &callback) {
        m_event_map.addEventCallback(type, callback);
    }

    void addKeyPressedCallback(const sf::Keyboard::Key key, const EventCallback &callback) {
        m_event_map.addKeyPressedCallback(key, callback);
    }

    void addKeyReleasedCallback(const sf::Keyboard::Key key, const EventCallback &callback) {
        m_event_map.addKeyReleasedCallback(key, callback);
    }

    void addMousePressedCallback(const sf::Mouse::Button button, const EventCallback &callback) {
        m_event_map.addMousePressedCallback(button, callback);
    }

    void addMouseReleasedCallback(const sf::Mouse::Button button, const EventCallback &callback) {
        m_event_map.addMouseReleasedCallback(button, callback);
    }

    void removeCallback(const sf::Event::EventType type) {
        m_event_map.removeCallback(type);
    }

    sf::Window &getWindow() const {
        return m_window;
    }

    V2f getMousePosition() const {
        const V2i mouse_position = sf::Mouse::getPosition(m_window);
        return { static_cast<float>(mouse_position.x), static_cast<float>(mouse_position.y) };
    }

private:
    sf::Window &m_window;
    EventMap m_event_map;
};

#endif
