#include "LevelC.h"
#include "Utility.h"
#include <vector>

#define LEVEL_HEIGHT 12
#define LEVEL_WIDTH 14
// CHANGE ^v^
unsigned int LEVELC_DATA[] =
{
   12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
   12, 16, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 17, 12,
   12, 13, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 12,
   12, 13, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 12,
   12, 13, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 12,
   12, 13, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 12,
   12, 13, 10, 10, 10, 33, 34, 35, 10, 10, 10, 10, 11, 12,
   12, 13, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 12,
   12, 13, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 12,
   12, 13, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 12,
   12, 68,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 71, 12,
   12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
};

LevelC::~LevelC()
{
    delete m_game_state.player;
    delete m_game_state.map;
    delete[] m_game_state.enemies;
    Mix_FreeChunk(m_game_state.chomp_sfx);
}


void LevelC::initialise()
{
    // ----- ATTRIBUTES ----- //
    // CHANGE: curr scene id
    m_game_state.curr_scene_id = 3;
    m_game_state.next_scene_id = -1;

    // CHANGE: numbers of stuff
    m_num_hunters = 2;
    m_num_targets = 7;
    m_num_clouds = 4;

    // CHANGE: edges
    m_left_edge = 2.25f;
    m_right_edge = 11.5f;
    m_bottom_edge = -9.75;
    m_top_edge = -2.25;

    // ------ MAP ----- //
    // CHANGE: texture used (and map data) (remember level width and height)
    GLuint map_texture_id = Utility::load_texture("assets/Abyss.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id,
        1.0f, 11, 13);

    // ----- PLAYER ----- //
    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

    GLuint player_texture_id = Utility::load_texture("assets/player-swiming.png");

    std::vector<std::vector<int>> player_animations =
    {
        {0, 1, 2, 3, 4, 5, 6},              // RIGHT animation frames
        {7, 8, 9, 10, 11, 12, 13}           // LEFT animation frames
    };

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        3.25f,                      // speed
        acceleration,              // acceleration
        4.0f,                      // jumping power
        player_animations,         // animation index sets
        RIGHT,                      // animation type
        0.0f,                      // animation time
        7,                         // animation frame amount
        0,                         // current animation index
        7,                         // animation column amount
        2,                         // animation row amount
        1.0f,                   // width (15/18)
        0.385f,                      // height (18/18)
        PLAYER                     // entity type
    );
    // CHANGE: player init pos
    m_player_init_pos = glm::vec3(3.0f, -2.4f, 0.0f);
    m_game_state.player->set_position(m_player_init_pos);
    m_game_state.player->set_scale(glm::vec3(1.0f, 0.385f, 0.0f));
    m_game_state.player->m_num_lives = 3;

    // ----- ENEMIES ----- // 
    int num_enemies = m_num_hunters + m_num_targets + m_num_clouds;
    m_game_state.enemies = new Entity[num_enemies];
    float horizontal_diff = m_right_edge - m_left_edge;
    float vertical_diff = m_top_edge - m_bottom_edge;

    // hunters //
    GLuint hunter_texture_id = Utility::load_texture("assets/Shark1.png");
    std::vector<std::vector<int>> hunter_animations =
    {
        {0, 1, 2, 3, 4, 5, 6, 7},              // RIGHT animation frames
        {8, 9, 10, 11, 12, 13, 14, 15}         // LEFT animation frames
    };
    /* make the hunters */
    for (int i = 0; i < m_num_hunters; i++)
    {   // CHANGE enemy start pos
        glm::vec3 enemy_pos = glm::vec3(10.0f - 3*i, -8.5f, 0.0f);

        m_game_state.enemies[i] = Entity(
            hunter_texture_id,
            0.3f,                   // speed
            0.88f,                  // width (32/32) (made width a bit smaller than actual)
            0.5f,                   // height (19/32) (made smaller than actual height)
            ENEMY,                  // entity type
            HUNTER,                 // AI type
            AI_IDLING,              // AI State 
            RIGHT,                  // animation
            hunter_animations,      // vector of animation indices
            0.0f,                   // jump power
            8,                      // animation frames
            8,                      // animation cols
            2,                      // animation rows
            enemy_pos
        );

    }

    // targets //
    GLuint target_texture_id = Utility::load_texture("assets/Coral1.png");
    std::vector<std::vector<int>> target_animations = { {0}, {1}, {2}, {3} };
    for (int i = m_num_hunters; i < m_num_hunters + m_num_targets; i++)
    {
        glm::vec3 enemy_pos = avoid_map(m_right_edge, m_left_edge, m_top_edge, m_bottom_edge);
        //test line: enemy_pos = glm::vec3(i + 3.0, -3.0, 0.0);
        m_game_state.enemies[i] = Entity(
            target_texture_id,
            1.0f,                   // speed
            1.0f,                // width (30/32)
            1.0f,               // height (19/32)
            ENEMY,                  // entity type
            TARGET,                 // AI type
            AI_IDLING,              // AI State 
            RIGHT,                  // animation value
            target_animations,      // vec of animation indices
            0.0f,                   // jump power
            1,                      // animation frames
            4,                      // animation cols
            1,                      // animation rows
            enemy_pos
        );

        m_game_state.enemies[i].set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    }

    // clouds //
    GLuint cloud_texture_id = Utility::load_texture("assets/Cloud.png");
    std::vector<std::vector<int>> cloud_animations = { {0} };
    for (int i = m_num_hunters + m_num_targets; i < num_enemies; i++)
    {
        glm::vec3 enemy_pos = avoid_map(m_right_edge, m_left_edge, m_top_edge, m_bottom_edge);

        m_game_state.enemies[i] = Entity(
            cloud_texture_id,
            0.5f,                   // speed
            1.0f,                // width (30/32)
            1.0f,               // height (19/32)
            ENEMY,                  // entity type
            CLOUD,                  // AI type
            AI_IDLING,              // AI State 
            RIGHT,
            cloud_animations,       // vector of animation indices
            0.0f,                   // jump power
            1,                      // animation frames
            1,                      // animation cols
            1,                      // animation rows
            enemy_pos
        );
    }


    // ----- AUDIO ----- //
    int aud = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_game_state.hurt_sfx = Mix_LoadWAV("assets/Squeak.wav");
    m_game_state.chomp_sfx = Mix_LoadWAV("assets/Chomp.wav");
    m_game_state.swim_sfx = Mix_LoadWAV("assets/Swim_Whoosh.wav");

}

void LevelC::update(float delta_time)
{
    int num_enemies = m_num_hunters + m_num_targets + m_num_clouds;

    /* update the player */
    m_game_state.player->update(delta_time, NULL,
        m_game_state.enemies, num_enemies, m_game_state.map);


    /* death from hitting ai */
    if (m_game_state.player->get_player_died()) {
        m_player_dead = true;
    }

    /* flip the next scene flag from -1 to scene # */
    if (m_game_state.player->get_targets_eaten() >= m_num_targets) {
        m_game_state.next_scene_id = m_game_state.curr_scene_id + 1;
    }

    /* update the enemy */
    for (int i = 0; i < num_enemies; i++) {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL,
            0, m_game_state.map);
    }

    /* update the effects */
    //m_game_state.effect->update(delta_time);

}

void LevelC::render(ShaderProgram* program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    int num_enemies = m_num_hunters + m_num_targets + m_num_clouds;
    for (int i = 0; i < num_enemies; i++) {
        m_game_state.enemies[i].render(program);
    }
    //m_game_state.effect->render();
}


glm::vec3 LevelC::avoid_map(float r_edge, float l_edge, float t_edge, float b_edge) {
    float horizontal_diff = r_edge - l_edge;
    float vertical_diff = t_edge - b_edge;
    // CHANGE: these values
    // the randomly_generated values cannot be within: 
    // x < 6.25 && y > -5.0
    // 8.75 < x < 11.25 && y > -6.25
    // 5.5 < x < 8.5 && y < -7
    /* the rand vals are auto generated within the borders */
    float randx = ((float)rand() / (float)RAND_MAX) * horizontal_diff + l_edge;
    float randy = ((float)rand() / (float)RAND_MAX) * vertical_diff - t_edge;

    bool in_section_1 = randx < 6.25f && randy > -5.0f;
    bool in_section_2 = 8.75f < randx && randx < 11.25f && randy > -6.25f;
    bool in_section_3 = 5.5f < randx && randx < 8.5f && randy < -7.0f;

    while (in_section_1 || in_section_2 || in_section_3) {
        randx = ((float)rand() / (float)RAND_MAX) * horizontal_diff + l_edge;
        randy = ((float)rand() / (float)RAND_MAX) * vertical_diff - t_edge;
        in_section_1 = randx < 6.25f && randy > -5.0f;
        in_section_2 = 8.75f < randx && randx < 11.25f && randy > -6.25f;
        in_section_3 = 5.5f < randx && randx < 8.5f && randy < -7.0f;
    }
    return glm::vec3(randx, -randy, 0.0f);
}