#include "FFCamera.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>

/// Free fly camera
FFCamera::FFCamera() {
  SetView(glm::vec3(0.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3(0.0f, 1.0f, 0.0f));
}

FFCamera::FFCamera(glm::vec3 _eye, glm::vec3 _at, glm::vec3 _worldUp) {
  SetView(_eye, _at, _worldUp);
}

FFCamera::~FFCamera() {}

void FFCamera::SetView(glm::vec3 _eye, glm::vec3 _at, glm::vec3 _worldUp) {
  m_eye = _eye;
  m_at = _at;
  m_worldUp = _worldUp;

  glm::vec3 ToAim = m_at - m_eye;

  m_distance = glm::length(ToAim);

  m_u = atan2f(ToAim.z, ToAim.x);
  m_v = acosf(ToAim.y);

  UpdateParams();

  m_viewDirty = true;
}

void FFCamera::SetProj(float _angle, float _aspect, float _zn, float _zf) {
  m_angle = _angle;
  m_aspect = _aspect;
  m_zNear = _zn;
  m_zFar = _zf;

  m_projectionDirty = true;
}

void FFCamera::LookAt(glm::vec3 _at) { SetView(m_eye, _at, m_up); }

void FFCamera::SetAngle(const float _angle) noexcept {
  m_angle = _angle;
  m_projectionDirty = true;
}

void FFCamera::SetAspect(const float _aspect) noexcept {
  m_aspect = _aspect;
  m_projectionDirty = true;
}

void FFCamera::SetZNear(const float _zn) noexcept {
  m_zNear = _zn;
  m_projectionDirty = true;
}

void FFCamera::SetZFar(const float _zf) noexcept {
  m_zFar = _zf;
  m_projectionDirty = true;
}

void FFCamera::Update(float _deltaTime) {
  if (m_goForward != 0.0f || m_goRight != 0.0f || m_goUp != 0.0f) {
    glm::vec3 deltaPosition =
        (m_goForward * m_forward + m_goRight * m_right + m_goUp * m_up) *
        m_speed * _deltaTime;
    m_eye += deltaPosition;
    m_at += deltaPosition;
    m_viewDirty = true;
  }

  if (m_viewDirty) {
    m_viewMatrix = glm::lookAt(m_eye, m_at, m_worldUp);
  }

  if (m_projectionDirty) {
    m_matProj = glm::perspective(m_angle, m_aspect, m_zNear, m_zFar);
  }

  if (m_viewDirty || m_projectionDirty) {
    m_matViewProj = m_matProj * m_viewMatrix;
    m_viewDirty = false;
    m_projectionDirty = false;
  }
}

void FFCamera::UpdateUV(float du, float dv) {
  m_u += du;
  m_v = glm::clamp<float>(m_v + dv, 0.1f, 3.1f);

  UpdateParams();
}

void FFCamera::UpdateParams() {
  glm::vec3 lookDirection(cosf(m_u) * sinf(m_v), cosf(m_v),
                          sinf(m_u) * sinf(m_v));

  m_forward = glm::normalize(lookDirection);
  m_at = m_forward * m_distance + m_eye;
  m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
  m_up = glm::normalize(glm::cross(m_right, m_forward));

  m_viewDirty = true;
}

void FFCamera::SetSpeed(float _val) { m_speed = _val; }

void FFCamera::Resize(int _w, int _h) { SetAspect(_w / (float)_h); }

void FFCamera::KeyboardDown(const SDL_KeyboardEvent &key) {
  switch (key.keysym.sym) {
  case SDLK_LSHIFT:
  case SDLK_RSHIFT:
    if (!m_slow) {
      m_slow = true;
      m_speed /= 4.0f;
    }
    break;
  case SDLK_w:
    m_goForward = 1;
    break;
  case SDLK_s:
    m_goForward = -1;
    break;
  case SDLK_a:
    m_goRight = -1;
    break;
  case SDLK_d:
    m_goRight = 1;
    break;
  case SDLK_e:
    m_goUp = 1;
    break;
  case SDLK_q:
    m_goUp = -1;
    break;
  }
}

void FFCamera::KeyboardUp(const SDL_KeyboardEvent &key) {
  float current_speed = m_speed;
  switch (key.keysym.sym) {
  case SDLK_LSHIFT:
  case SDLK_RSHIFT:
    if (m_slow) {
      m_slow = false;
      m_speed *= 4.0f;
    }
    break;
  case SDLK_w:
  case SDLK_s:
    m_goForward = 0;
    break;
  case SDLK_a:
  case SDLK_d:
    m_goRight = 0;
    break;
  case SDLK_q:
  case SDLK_e:
    m_goUp = 0;
    break;
  }
}

void FFCamera::MouseMove(const SDL_MouseMotionEvent &mouse) {
  if (mouse.state & SDL_BUTTON_LMASK) {
    UpdateUV(mouse.xrel / 300.0f, mouse.yrel / 200.0f);
  }
}
