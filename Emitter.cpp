//
// Created by yahor on 3.4.18.
//

#include "Emitter.h"

void Emitter::Initialize() { }

void Emitter::Load() { }

void Emitter::Update() { }

void Emitter::Dispose() { }

float Emitter::IntensityAt(const Vector2 &other) const {
    Vector2 mp(m_position.x, m_position.y);
    float distance = mp.distance(other);
    float nd = std::min(distance / m_radius_px, 1.0f);

    return 1.0f / (m_falloff_constant + (nd * m_falloff_linear) + (nd * nd * m_falloff_quadratic));
}