#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include <cmath>

void Entity::ai_activate(Entity* player, Map* map)
{
    switch (m_ai_type)
    {
    case HUNTER:
        ai_swim(map);
        break;

    /*case CLOUD:
        ai_pulse();
        break;
    case TARGET:
        ai_bounce();
        break;*/
    default:
        break;
    }
}

void Entity::ai_swim(Map* map)
{
    switch (m_ai_state) {
    case AI_IDLING:
        break;
    case AI_RIGHT:
        //m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
        move_right();
        break;
    case AI_LEFT:
        //m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
        move_left();
    default:
        break;
    }
}

// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f), m_current_animation(RIGHT),
    m_texture_id(0), m_velocity(0.0f), m_acceleration(0.0f), m_width(0.0f), m_height(0.0f)
{
}

// Parameterized constructor (player constructor)
Entity::Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, std::vector<std::vector<int>> animations, PlayerAnimation animation, float animation_time,
    int animation_frames, int animation_index, int animation_cols, int animation_rows, float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_acceleration(acceleration), m_jumping_power(jump_power),
    m_animation_cols(animation_cols), m_animation_frames(animation_frames), m_animation_index(animation_index),
    m_animation_rows(animation_rows), m_animation_indices(nullptr), m_current_animation(animation), m_animations(animations),
    m_animation_time(animation_time), m_texture_id(texture_id), m_velocity(0.0f),
    m_width(width), m_height(height), m_entity_type(EntityType)
{
    set_animation_state(m_current_animation);
    //set_walking(animations);
}

// Simpler constructor for partial initialization
Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f), m_current_animation(RIGHT),
    m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height), m_entity_type(EntityType)
{
    set_animation_state(m_current_animation);
}

// ai constructor
    // ai needs jump power, animation time, frame amount,animation col amt, animation row amt
Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AIState AIState, PlayerAnimation PlayerAnim, std::vector<std::vector<int>> animations,
    float jump_power, int animation_frames, int animation_cols, int animation_rows, glm::vec3 position)
    : m_position(position), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f), m_jumping_power(jump_power), m_is_jumping(false),
    m_speed(speed), m_animation_cols(animation_cols), m_animation_frames(animation_frames), m_animation_index(0),
    m_animation_rows(animation_rows), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width),
    m_height(height), m_entity_type(EntityType), m_ai_type(AIType), m_ai_state(AIState), m_current_animation(PlayerAnim), m_animations(animations)
{

    if (EntityType == ENEMY && AIType == HUNTER) {
        //set_animation_state(m_current_animation);
    }
    else if (EntityType == ENEMY && AIType == TARGET) {
        // TODO: when making targets, set m_current animation to a random 0-3 (4 different images)
        /*int rand_skin = rand() % 4;
        m_animation_indices = m_animations[rand_skin].data();
        m_animation_frames = (int)m_animations[rand_skin].size();*/
        /* set init pos for pulse */
        m_target_init_pos = m_position;
        m_target_y_theta = ((float)rand() / (float)RAND_MAX) * 5;
    }
    else if (EntityType == ENEMY && AIType == CLOUD) {
        BASE_SCALE = ((float)rand() / (float)RAND_MAX) * 0.3f + 0.35f;
        MAX_AMPLITUDE = BASE_SCALE;
        PULSE_SPEED = ((float)rand() / (float)RAND_MAX) * 0.2f + 0.5f;
    }
}
    

Entity::~Entity() { }


void Entity::set_animation_state(PlayerAnimation new_animation)
{
    m_current_animation = new_animation;
    // Update the texture and animation indices based on the current animation
    m_animation_indices = m_animations[m_current_animation].data();
    // Update the number of rows to match the new texture spritesheet
    m_animation_frames = (int)m_animations[m_current_animation].size();
}



void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool const Entity::check_collision(Entity* other) const
{
    if (!other->m_is_active) return false;
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

// probably need to hardcode collidable entity count in main
void Entity::player_eat(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];
        if (collidable_entity->get_entity_type() == ENEMY && collidable_entity->get_ai_type() == TARGET) {
            float x_dist = pow(m_position.x - collidable_entity->get_position().x, 2);
            float y_dist = pow(m_position.y - collidable_entity->get_position().y, 2);
            float dist = sqrt(x_dist + y_dist);
            if (dist < 1.2 && collidable_entity->m_is_active) {
                collidable_entity->deactivate();
                m_num_targets_eaten += 1;
            }
        }
    }
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];
        if (collidable_entity->m_is_active && !(collidable_entity->m_ai_type == TARGET) )
        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0)
            {
                m_position.y -= y_overlap;
                m_velocity.y = 0;

                // Collision!
                m_collided_top = true;
            }
            else if (m_velocity.y < 0)
            {
                m_position.y += y_overlap;
                float old_yvel = m_velocity.y;
                m_velocity.y = 0;

                // Collision!
                m_collided_bottom = true;
                
            }
            if (m_collided_top || m_collided_bottom) {
                if (m_entity_type == PLAYER && (collidable_entity->m_ai_type == HUNTER || collidable_entity->m_ai_type == CLOUD)) {
                    collidable_entity->deactivate();
                }
            }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];
        if (collidable_entity->m_is_active && !(collidable_entity->m_ai_type == TARGET) )
        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
            if (m_velocity.x >= 0)
            {
                m_position.x -= x_overlap;
                m_velocity.x = 0;

                // Collision!
                m_collided_right = true;

            }
            else if (m_velocity.x < 0)
            {
                m_position.x += x_overlap;
                m_velocity.x = 0;

                // Collision!
                m_collided_left = true;
            }
            if (m_collided_left || m_collided_right) {
                if (m_entity_type == PLAYER && (collidable_entity->m_ai_type == HUNTER || collidable_entity->m_ai_type == CLOUD)) {
                    collidable_entity->deactivate();
                }
            }
        }
    }
}


void const Entity::check_collision_y(Map* map)
{
    if (!m_is_active) return;
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);

    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2) - 0.1, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2) + 0.1, m_position.y - (m_height / 2), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }

    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        if (!map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_entity_type == ENEMY) {
            m_enemy_left_pit_detected = true;
        }
        if (!map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_entity_type == ENEMY) {
            m_enemy_right_pit_detected = true;
        }
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;

    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        if (m_entity_type == ENEMY) {
            if (!map->is_solid(bottom_right, &penetration_x, &penetration_y))
                m_enemy_right_pit_detected = true;
        }

    }

    //if (m_collided_bottom){ m_is_jumping = false; }
}

void const Entity::check_collision_x(Map* map)
{
    if (!m_is_active) return;
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}



void Entity::update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map)
{
    if (!m_is_active) return;

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    m_enemy_left_pit_detected = false;
    m_enemy_right_pit_detected = false;

    // --- ANIMATIONS --- //
    if (m_animation_indices != NULL)
    {
        m_animation_time += delta_time;
        float frames_per_second = (float)1 / SECONDS_PER_FRAME;

        if (m_animation_time >= frames_per_second)
        {
            m_animation_time = 0.0f;
            m_animation_index++;

            if (m_animation_index >= m_animation_frames)
            {
                m_animation_index = 0;
            }
        }
    }

    // -- AI Movement -- //
    if (m_entity_type == ENEMY && m_ai_type == HUNTER) {
        if (m_position.x > player->get_position().x) {
            m_velocity = glm::vec3(-m_speed, m_velocity.y, 0.0f);
            m_ai_state = AI_LEFT;
            if (m_scale.x > 0) { m_scale.x = m_scale.x *(- 1.0f); }
        }
        else {
            m_velocity = glm::vec3(m_speed, m_velocity.y, 0.0f);
            m_ai_state = AI_RIGHT;
            if (m_scale.x < 0) { m_scale.x = m_scale.x * (-1.0f); }

        }

        if (m_position.y > player->get_position().y) {
            m_velocity = glm::vec3(m_velocity.x, -m_speed, 0.0f);
        }
        else {
            m_velocity = glm::vec3(m_velocity.x, m_speed, 0.0f);
        }
    }


    // --- MOVEMENT --- //
    if (m_entity_type == PLAYER) {
        m_velocity += m_acceleration * delta_time;
    }
        /* x movement */
    m_position.x += m_velocity.x * delta_time;
    check_collision_x(collidable_entities, collidable_entity_count);
        /* y movement */
    m_position.y += m_velocity.y * delta_time;
    check_collision_y(collidable_entities, collidable_entity_count);
    
    /* if player hits enemy, die and lose life */
    if (m_entity_type == PLAYER && (m_collided_left || m_collided_right || m_collided_bottom || m_collided_top)) {
        m_player_died = true;
        // TODO: link num_lives
        m_num_lives -= 1;
    }

    check_collision_x(map);
    check_collision_y(map);
        /* momentum */
    float accel = 3.55f;
    if (m_velocity.x > 0.0f) { m_acceleration.x = -accel; }
    if (m_velocity.x < 0.0f) { m_acceleration.x = accel; }
    if (m_velocity.y > 0.0f) { m_acceleration.y = -accel; }
    if (m_velocity.y < 0.0f) { m_acceleration.y = accel; }
    

    // TRANSFORMATION FOR TARGETS
    if (m_entity_type == ENEMY && m_ai_type == TARGET) 
    {
        m_target_y_theta += m_speed * delta_time;
        m_position.y = m_target_init_pos.y + m_target_dist * glm::sin(m_target_y_theta);
    }
    // TRANSFORMATION FOR CLOUD
    if (m_entity_type == ENEMY && m_ai_type == CLOUD) {
        m_target_pulse_time += PULSE_SPEED * delta_time;
        m_width = BASE_SCALE + MAX_AMPLITUDE * glm::sin(m_target_pulse_time);
        m_height = BASE_SCALE + MAX_AMPLITUDE * glm::sin(m_target_pulse_time);
        m_scale = glm::vec3(m_width, m_height, 0.0f);
    }
    

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
}


void Entity::render(ShaderProgram* program)
{
    if (!m_is_active) 
        return;
    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}