#include "Start.h"
#include "Utility.h"
#include <vector>

unsigned int LEVELSTART_DATA[] =
{
   11
};

GLuint g_font_texture_id;

Start::~Start()
{
    delete m_game_state.player;
    delete m_game_state.map;
    delete m_game_state.enemies;
    Mix_FreeChunk(m_game_state.chomp_sfx);
}

void Start::initialise()
{
    m_game_state.curr_scene_id = 0;
    m_game_state.next_scene_id = -1;

    g_font_texture_id = Utility::load_texture("assets/font1.png");
    m_game_state.player = new Entity();
    GLuint map_texture_id = Utility::load_texture("assets/Sand.png");
    m_game_state.map = new Map(9, 6, LEVELSTART_DATA, map_texture_id,
        1.0f, 22, 11);
}

void Start::update(float delta_time)
{
}

void Start::render(ShaderProgram* program)
{

    Utility::draw_text(program, g_font_texture_id, "Press [t] to start", 0.23f, 0.0f,
        glm::vec3(-2.5f, 0.0f, 0.0f));
   
}
