#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <random>
#include <vector>

class Metaballs {
private:
  const int SIZE = 5;
  const float RADIUS = 1.2;
  std::random_device dev;
  std::mt19937 rng;
  struct GoalInfo {
    glm::vec3 p0;
    glm::vec3 p1;
    glm::vec3 p2;
    float starttime;
    float duration;
  };
  std::vector<glm::vec4> m_balls;
  std::vector<GoalInfo> m_ball_goals;

private:
  void RandomizeGoal(int i, float ElapsedTimeInSec) {
    std::uniform_real_distribution dist;
    GoalInfo &g = m_ball_goals[i];
    float r = (dist(rng) * 0.6 + 0.4) * RADIUS;
    float theta = dist(rng) * glm::pi<float>() * 2.0f;
    float phi = dist(rng) * glm::pi<float>() * 2.0f;
    g.p2 = glm::vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi),
                     r * cos(theta));
    // Destination point
    r = (dist(rng) * 0.6 + 0.4) * RADIUS;
    theta = dist(rng) * glm::pi<float>() * 2.0f;
    phi = dist(rng) * glm::pi<float>() * 2.0f;
    g.p1 = glm::vec3(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi),
                     r * cos(theta));
    // Central point for bézier curve

    g.p0 = glm::vec3(m_balls[i].x, m_balls[i].y, m_balls[i].z);
    g.starttime = ElapsedTimeInSec;
    g.duration =
        (dist(rng) * 10.0f + 10) * abs(glm::dot(g.p0 - g.p1, g.p2 - g.p1)) +
        3.0;
  }

public:
  Metaballs(float ElapsedTimeInSec = 0)
      : dev(), rng(dev()), m_balls({{0.0, 0.0, 0.0, 0.5},
                                    {1.0, 0.0, 0.0, 0.5},
                                    {-1.0, 0.0, 0.0, 0.5},
                                    {0.0, 1.0, 0.0, 0.5},
                                    {0.0, -1.0, 0.0, 0.5}}),
        m_ball_goals(SIZE) {
    for (int i = 1; i < SIZE; ++i) {
      RandomizeGoal(i, ElapsedTimeInSec);
    }
  }

  void UpdatePositions(float ElapsedTimeInSec) {
    // A középső labda mérete változik a többi mozog körülötte.
    m_balls[0].w = 0.5 + 0.25 * abs(sin(ElapsedTimeInSec / 2.0f));
    for (int i = 1; i < SIZE; i++) {
      GoalInfo &g = m_ball_goals[i];
      float t = (ElapsedTimeInSec - g.starttime) / g.duration;
      if (t > 1.0f) {
        RandomizeGoal(i, ElapsedTimeInSec);
        t = 0.0f;
      }
      glm::vec3 p = (1.0f - t) * (1.0f - t) * g.p0 +
                    2.0f * (1.0f - t) * t * g.p1 + t * t * g.p2;
      m_balls[i] = glm::vec4(p, m_balls[i].w);
    }
  }

  inline std::vector<glm::vec4> GetBalls() const { return m_balls; }
};
