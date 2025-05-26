#ifndef VIEWPORT_HANDLER_HPP
#define VIEWPORT_HANDLER_HPP

#include <SFML/Graphics.hpp>

#include "utils.hpp"

struct State {
    explicit State(const V2f render_size, const float base_zoom = 1.0f)
        : center(render_size.x * 0.5f, render_size.y * 0.5f)
        , offset(center / base_zoom)
        , zoom(base_zoom)
        , isClicking(false)
    {}

    void updateState()
    {
        const float z = zoom;
        transform = sf::Transform::Identity;
        transform.translate(center);
        transform.scale(z, z);
        transform.translate(-offset);
    }

    void updateMousePosition(V2f new_mouse_position) {
        mouse_position = new_mouse_position;
        mouse_world_position = offset + (mouse_position - center) / zoom;
    }

    V2f center;
    V2f offset;
    float zoom;
    bool isClicking;
    V2f mouse_position;
    V2f mouse_world_position;
    sf::Transform transform;
};


class ViewportHandler {
public:
    explicit ViewportHandler(V2f size)
        : state(size)
    {
        state.updateState();
    }

    [[nodiscard]]
    float getZoom() const {
        return state.zoom;
    }

    void setZoom(const float _zoom) {
        state.zoom = _zoom;
        state.updateState();
    }

    void zoom(const float f) {
        state.zoom *= f;
        state.updateState();
    }

    void wheelZoom(const float w) {
        if (w != 0) {
            const float delta = w > 0 ? 1.2f : 1.0f / 1.2f;
            zoom(delta);
        }
    }

    void reset() {
        state.zoom = 1.0f;
        setFocus(state.center);
    }

    [[nodiscard]]
    V2f getMouseWorldPosition() const {
        return state.mouse_world_position;
    }

    [[nodiscard]]
    V2f getScreenCoords(const V2f world_pos) const {
        return state.transform.transformPoint(world_pos);
    }

    [[nodiscard]]
    sf::Transform getTransform() const {
        return state.transform;
    }

    void setFocus(const V2f focus_position) {
        state.offset = focus_position;
        state.updateState();
    }

    void setMousePosition(const V2f new_mouse_position) {
        if (state.isClicking) {
            state.offset += (state.mouse_position - new_mouse_position) / state.zoom;
            state.updateState();
        }
        state.updateMousePosition(new_mouse_position);
    }

    void click(V2f click_position)
    {
        state.mouse_position = click_position;
        state.isClicking = true;
    }

    void unClick()
    {
        state.isClicking = false;
    }

private:
    State state;
};


#endif
