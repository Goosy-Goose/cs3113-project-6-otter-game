#include "Scene.h"

class LevelA : public Scene {
public:
    static const int LEVEL_WIDTH  = 14;
    static const int LEVEL_HEIGHT = 8;
    

    ~LevelA();
    glm::vec3 avoid_map(float r_edge, float l_edge, float t_edge, float b_edge);
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
