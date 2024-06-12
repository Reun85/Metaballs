#pragma once

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Utils
#include "FFCamera.h"
#include "GLUtils.hpp"
#include "Metaballs.h"

struct SUpdateInfo {
  float ElapsedTimeInSec = 0.0f; // Program indulása óta eltelt idő
  float DeltaTimeInSec = 0.0f;   // Előző Update óta eltelt idő
};

class CMyApp {
public:
  CMyApp();
  ~CMyApp();

  bool Init();
  void Clean();

  void Update(const SUpdateInfo &);
  void Render();
  void RenderGUI();

  void KeyboardDown(const SDL_KeyboardEvent &);
  void KeyboardUp(const SDL_KeyboardEvent &);
  void MouseMove(const SDL_MouseMotionEvent &);
  void MouseDown(const SDL_MouseButtonEvent &);
  void MouseUp(const SDL_MouseButtonEvent &);
  void MouseWheel(const SDL_MouseWheelEvent &);
  void Resize(int, int);

protected:
  void SetupDebugCallback();

  //
  // Adat változók
  //
  FFCamera m_camera;

  float m_ElapsedTimeInSec = 0.0f;
  Metaballs m_balls;
  bool m_timeMoves = true;
  glm::vec2 m_screen_size;
  int m_frames = 0;
  float m_frametimes = 0;

  //
  // OpenGL-es dolgok
  //

  // shaderekhez szükséges változók
  GLuint m_programID = 0; // shaderek programja
  GLuint m_backgroundtexture;

  // Shaderek inicializálása, és törtlése
  void InitShaders();
  void CleanShaders();
  void InitTextures();
  void CleanTextures();
};
