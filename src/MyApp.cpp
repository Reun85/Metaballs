#include "MyApp.h"
#include "SDL_GLDebugMessageCallback.h"

#include <SDL2/SDL_keycode.h>
#include <imgui.h>

CMyApp::CMyApp() {}

CMyApp::~CMyApp() {}

void CMyApp::SetupDebugCallback() {
  // engedélyezzük és állítsuk be a debug callback függvényt ha debug
  // context-ben vagyunk
  GLint context_flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
  if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                          GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
  }
}

void CMyApp::InitShaders() {
  m_programID = glCreateProgram();
  AssembleProgram(m_programID, "Vert.glsl", "Frag.glsl");
}

void CMyApp::CleanShaders() { glDeleteProgram(m_programID); }

void CMyApp::InitTextures() {
  const std::vector<std::filesystem::path> faces{
      "Assets/skybox/xpos.png", "Assets/skybox/xneg.png",
      "Assets/skybox/ypos.png", "Assets/skybox/yneg.png",
      "Assets/skybox/zpos.png", "Assets/skybox/zneg.png"};
  const std::vector<std::filesystem::path> faces2{
      "Assets/skybox2/right.jpg", "Assets/skybox2/left.jpg",
      "Assets/skybox2/top.jpg",   "Assets/skybox2/bottom.jpg",
      "Assets/skybox2/front.jpg", "Assets/skybox2/back.jpg"};
  m_backgroundtexture = LoadCubeMap(faces2);
}

void CMyApp::CleanTextures() { glDeleteTextures(1, &m_backgroundtexture); }

bool CMyApp::Init() {
  SetupDebugCallback();

  // törlési szín legyen kékes
  glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

  InitShaders();
  InitTextures();

  //
  // egyéb inicializálás
  //

  glEnable(GL_CULL_FACE); // kapcsoljuk be a hátrafelé néző lapok eldobását
  glCullFace(GL_BACK); // GL_BACK: a kamerától "elfelé" néző lapok, GL_FRONT: a
                       // kamera felé néző lapok

  glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)

  m_camera.SetView(
      glm::vec3(0, 0.0, 2.0),    // honnan nézzük a színteret	   - eye
      glm::vec3(0.0, 0.0, 0.0),  // a színtér melyik pontját nézzük - at
      glm::vec3(0.0, 1.0, 0.0)); // felfelé mutató irány a világban - up

  return true;
}

void CMyApp::Clean() {
  CleanShaders();
  CleanTextures();
}

#include <iostream>
void CMyApp::Update(const SUpdateInfo &updateInfo) {
  if (m_timeMoves) {
    m_ElapsedTimeInSec += updateInfo.DeltaTimeInSec;
    m_balls.UpdatePositions(m_ElapsedTimeInSec);
  }
  m_camera.Update(updateInfo.DeltaTimeInSec);

  m_frames++;
  m_frametimes += updateInfo.DeltaTimeInSec;
  // HACK: VERY DIRTY
  if (m_frametimes > 3.0f) {
    std::cout << "FPS: "
              << static_cast<float>(m_frames) / static_cast<float>(m_frametimes)
              << std::endl;
    m_frames = 0;
    m_frametimes = 0;
  }
}

void CMyApp::Render() {
  // töröljük a frampuffert (GL_COLOR_BUFFER_BIT)...
  // ... és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // shader bekapcsolasa

  glUseProgram(m_programID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_backgroundtexture);

  glUniform1i(glGetUniformLocation(m_programID, "texImage"), 0);

  // Balls
  // Updated in Update()
  glUniform1i(glGetUniformLocation(m_programID, "ball_count"),
              m_balls.GetBalls().size());

  glUniform4fv(glGetUniformLocation(m_programID, "balls"),
               m_balls.GetBalls().size(),
               reinterpret_cast<GLfloat *>(m_balls.GetBalls().data()));

  // Camera settings
  glUniform3fv(glGetUniformLocation(m_programID, "eye"), 1,
               (glm::value_ptr(m_camera.GetEye())));
  glUniform3fv(glGetUniformLocation(m_programID, "at"), 1,
               (glm::value_ptr(m_camera.GetAt())));
  glUniform3fv(glGetUniformLocation(m_programID, "up"), 1,
               (glm::value_ptr(m_camera.GetWorldUp())));
  glUniform1f(glGetUniformLocation(m_programID, "aspect"),
              (m_camera.GetAspect()));
  glUniform1f(glGetUniformLocation(m_programID, "fov"), (m_camera.GetAngle()));

  // Lights
  // The program doesn't change them, this is just a demonstration that its
  // possible. The fourth coordinate is the type of the light source. 1 = point
  // light, 0 = directional light

  const static glm::vec3 light_ambient = glm::vec3(0.05);
  const static float light_constant_attenuation = 1.0;
  const static float light_linear_attenuation = 0.5;
  const static float light_quadratic_attenuation = 0.5;

  const static std::vector<glm::vec4> light_positions = {
      glm::vec4(0.0, 1.0, 0.0, 0.0), glm::vec4(10, 0.0, 0.0, 1.0),
      glm::vec4(-10, -10.0, 0.0, 1.0)};

  const static std::vector<glm::vec3> light_diffuse = {
      glm::vec3(1.0, 1.0, 1.0), glm::vec3(1.0, 1.0, 1.0),
      glm::vec3(1.0, 1.0, 1.0)};
  const static std::vector<glm::vec3> light_specular = {
      glm::vec3(1.0, 1.0, 1.0), glm::vec3(1.0, 1.0, 1.0),
      glm::vec3(1.0, 1.0, 1.0)};

  glUniform3fv(glGetUniformLocation(m_programID, "lightsource_ambient"), 1,
               glm::value_ptr(light_ambient));
  glUniform1f(glGetUniformLocation(m_programID, "lightConstantAttenuation"),
              light_constant_attenuation);
  glUniform1f(glGetUniformLocation(m_programID, "lightLinearAttenuation"),
              light_linear_attenuation);
  glUniform1f(glGetUniformLocation(m_programID, "lightQuadraticAttenuation"),
              light_quadratic_attenuation);

  glUniform1i(glGetUniformLocation(m_programID, "lightsources_count"),
              light_positions.size());
  glUniform4fv(glGetUniformLocation(m_programID, "lightsources_pos"),
               light_positions.size(),
               reinterpret_cast<const GLfloat *>(light_positions.data()));
  glUniform3fv(glGetUniformLocation(m_programID, "lightsources_diffuse"),
               light_diffuse.size(),
               reinterpret_cast<const GLfloat *>(light_diffuse.data()));
  glUniform3fv(glGetUniformLocation(m_programID, "lightsources_specular"),
               light_specular.size(),
               reinterpret_cast<const GLfloat *>(light_specular.data()));

  glUniform4fv(glGetUniformLocation(m_programID, "balls"),
               m_balls.GetBalls().size(),
               reinterpret_cast<GLfloat *>(m_balls.GetBalls().data()));

  // kirajzolás:
  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDrawArrays.xhtml
  glDrawArrays(
      GL_TRIANGLES, // primitív típusa; amikkel mi foglalkozunk: GL_POINTS,
                    // GL_LINE_STRIP, GL_LINES, GL_TRIANGLE_STRIP,
                    // GL_TRIANGLE_FAN, GL_TRIANGLES
      0, // ha van tároló amiben a kirajzolandó geometriák csúcspontjait
         // tároljuk, akkor annak hányadik csúcspontjától rajzoljunk - most
         // nincs ilyen, csak arra használjuk, hogy a gl_VertexID számláló a
         // shader-ben melyik számról induljon, azaz most nulláról
      6); // hány csúcspontot használjunk a primitívek kirajzolására - most:
          // gl_VertexID számláló 0-tól indul és 5-ig megy, azaz összesen 6x fut
          // le a vertex shader

  // shader kikapcsolasa
  glUseProgram(0);
}

void CMyApp::RenderGUI() {}

// https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
// https://wiki.libsdl.org/SDL2/SDL_Keysym
// https://wiki.libsdl.org/SDL2/SDL_Keycode
// https://wiki.libsdl.org/SDL2/SDL_Keymod

void CMyApp::KeyboardDown(const SDL_KeyboardEvent &key) {
  if (key.repeat == 0) // Először lett megnyomva
  {
    if (key.keysym.sym == SDLK_F5 && key.keysym.mod & KMOD_CTRL) {
      CleanShaders();
      InitShaders();
    }
    if (key.keysym.sym == SDLK_F1) {
      GLint polygonModeFrontAndBack[2] = {};
      // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGet.xhtml
      glGetIntegerv(
          GL_POLYGON_MODE,
          polygonModeFrontAndBack); // Kérdezzük le a jelenlegi polygon módot!
                                    // Külön adja a front és back módokat.
      GLenum polygonMode = (polygonModeFrontAndBack[0] != GL_FILL
                                ? GL_FILL
                                : GL_LINE); // Váltogassuk FILL és LINE között!
      // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPolygonMode.xhtml
      glPolygonMode(GL_FRONT_AND_BACK, polygonMode); // Állítsuk be az újat!
    }
    if (key.keysym.sym == SDLK_F2) {
      if (glIsEnabled(GL_CULL_FACE) == GL_TRUE) {
        glDisable(GL_CULL_FACE);
      } else {
        glEnable(GL_CULL_FACE);
      }
    }
    if (key.keysym.sym == SDLK_SPACE) {
      m_timeMoves = !m_timeMoves;
    }
  }
  m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(const SDL_KeyboardEvent &key) {
  m_camera.KeyboardUp(key);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseMotionEvent

void CMyApp::MouseMove(const SDL_MouseMotionEvent &mouse) {
  m_camera.MouseMove(mouse);
}

// https://wiki.libsdl.org/SDL2/SDL_MouseButtonEvent

void CMyApp::MouseDown(const SDL_MouseButtonEvent &mouse) {}

void CMyApp::MouseUp(const SDL_MouseButtonEvent &mouse) {}

// https://wiki.libsdl.org/SDL2/SDL_MouseWheelEvent

void CMyApp::MouseWheel(const SDL_MouseWheelEvent &wheel) {}

// a két paraméterben az új ablakméret szélessége (_w) és magassága (_h)
// található
void CMyApp::Resize(int _w, int _h) {
  glViewport(0, 0, _w, _h);
  m_camera.Resize(_w, _h);
}
