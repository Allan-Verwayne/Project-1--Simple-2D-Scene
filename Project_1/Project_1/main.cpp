/*
* Author: Allan Verwayne
* Assignment: Simple 2D Scene
* Date due: 2023-09-30, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
*/

#include <iostream>

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

//window setup globals
const int W_WIDTH  = 800,
          W_HEIGHT = 640;

const float BG_R = 0.0f,
            BG_B = 0.0f,
            BG_G = 0.0f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = W_WIDTH,
          VIEWPORT_HEIGHT = W_HEIGHT;

//shader globals
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

//sprite globals
const char SPRITE_PATH[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_1/Project_1/images/tinkaton1.png";
const char SPRITE_PATH_2[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_1/Project_1/images/g_corviknight.png";
const char SPRITE_PATH_3[] = "/Users/allan_home/Documents/CS-3113/Homework/Project_1/Project_1/images/star.png";

//object texture ids
GLuint sprite_texture_id;
GLuint sprite2_texture_id;
GLuint sprite3_texture_id;
ShaderProgram shader_program;
//texture globals
const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

//window SDL globals
SDL_Window* display_window;
bool game_is_running = true;

//matrices globals
glm::mat4 model_matrix;
glm::mat4 model2_matrix;
glm::mat4 model3_matrix;
glm::mat4 view_matrix;
glm::mat4 projection_matrix;

//transformation globals
//model 1
float model_x = -7.0f;
float model_y = 0.0f;

const float MAX_FACTOR = 1.01f;
const float MIN_FACTOR = 0.99f;
const int MAX_FRAMES = 50;
int frame_count = 0;
bool is_growing = true;
bool is_jumping = true;

//model 2
float model2_x = -5.5f;
float model2_y = 0.0f;

//model 3
float model3_rotate = 0.0f;
const float DEG_PER_SECOND = -90.0f;

//delta time global
const float MILLISECONDS = 1000.0;
float previous_ticks = 0.0f;

/* Functions */

//load texture function
GLuint load_texture(const char* filepath) {
    //
    int width, height, num_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &num_components,
                                     STBI_rgb_alpha);
    if (image == NULL)
        {
            std::cout << "Unable to load image. Make sure the path is correct." << std::endl;
            std::cout << filepath;
            assert(false);
        }
    //
    GLuint texture_ID;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_ID);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA,
                 width, height,
                 TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE,
                 image);
    //  biinding texture ids
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //  memory release
    stbi_image_free(image);

    return texture_ID;
}

//object creator function
void draw_object(glm::mat4 &obj_model_matrix, GLuint &obj_texture_id) {
    shader_program.set_model_matrix(obj_model_matrix);
    glBindTexture(GL_TEXTURE_2D, obj_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void initialise() {
    //Mandatory SDL Setup
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Project #1",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      W_WIDTH, W_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    //shader initialization
    
    //  load shaders for handling textures
    shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    //  matrix initialization
    model_matrix  = glm::mat4(1.0f);
    model2_matrix = glm::mat4(1.0f);
    model3_matrix = glm::mat4(1.0f);
    
    // position models to far left end
    model_matrix = glm::translate(model_matrix, glm::vec3(model_x, model_y, 0.0f));
    model2_matrix = glm::translate(model2_matrix, glm::vec3(model2_x, model2_y, 0.0f));
    
    view_matrix   = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    //  shader setup
    shader_program.set_view_matrix(view_matrix);
    shader_program.set_projection_matrix(projection_matrix);
    
    glUseProgram(shader_program.get_program_id());
    
    glClearColor(BG_R, BG_G, BG_B, BG_OPACITY);
    
    //  load player image
    sprite_texture_id  = load_texture(SPRITE_PATH);
    sprite2_texture_id = load_texture(SPRITE_PATH_2);
    sprite3_texture_id = load_texture(SPRITE_PATH_3);
    
    //  enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            game_is_running = false;
        }
    }
}

void update() {
    //  delta time setup
    float ticks = (float) SDL_GetTicks() / MILLISECONDS;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;
    
    frame_count++;
    
    if (frame_count >= MAX_FRAMES) {
        is_growing = !is_growing;
        is_jumping = !is_jumping;
        frame_count = 0;
    }
    
    //  model 1
    //  moving along x
    model_x += 1.0f * delta_time;
    //  moving on y
    model_y = is_jumping ? model_y + (0.025f) : model_y - (0.025f);
    //  reset identity
    model_matrix = glm::mat4(1.0f);
    
    //  model 2
    //  moving along x
    model2_x += 1.0f * delta_time;
    //  moving along y
    model2_y += 0.2f * delta_time;
    //  reset identity
    model2_matrix = glm::mat4(1.0f);
    
    //  model 3
    //  rotating @ model_1
    model3_rotate += DEG_PER_SECOND * delta_time;
    model3_matrix = glm::mat4(1.0f);
    
    // scale
    glm::vec3 scale_vector = glm::vec3(1.5f, 1.5f, 1.0f);
    glm::vec3 scale_vector2 = glm::vec3(2.0f, 1.5f, 1.0f);
    
    // scaling animation
    model_matrix = glm::scale(model_matrix, scale_vector);
    model2_matrix = glm::scale(model2_matrix, scale_vector2);
    
    //  model 1
    
    // moving animation
    model_matrix = glm::translate(model_matrix, glm::vec3(model_x, model_y, 0.0f));
    
    //  model 2
    
    // heartbeat
    glm::vec3 beat_vector2 = glm::vec3(is_growing ? MAX_FACTOR : MIN_FACTOR,
                                        is_growing ? MAX_FACTOR : MIN_FACTOR, 1.0f);
    // heartbeat animation
    model2_matrix = glm::scale(model2_matrix, beat_vector2);
    
    // moving animation
    model2_matrix = glm::translate(model2_matrix, glm::vec3(model2_x, model2_y, 0.0f));
    
    //  model 3
    
    // translate
    model3_matrix = glm::translate(model_matrix, glm::vec3(-0.25f, 0.0f, 0.0f));
    
    // rotate
    model3_matrix = glm::rotate(model3_matrix, glm::radians(model3_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    //vertices
    float vertices[] = {
        -0.5f, -0.5f,   0.5f, -0.5f,    0.5f, 0.5f,  // triangle 1
        
        -0.5f, -0.5f,   0.5f, 0.5f,     -0.5f, 0.5f   // triangle 2
    };
    
    //textures
    float texture_coordinates[] = {
        0.0f, 1.0f,     1.0f, 1.0f,     1.0f, 0.0f,     // triangle 1
        
        0.0f, 1.0f,     1.0f, 0.0f,     0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(shader_program.get_position_attribute());
    
    glVertexAttribPointer(shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
                          texture_coordinates);
    glEnableVertexAttribArray(shader_program.get_tex_coordinate_attribute());
    
    //bind texture
    draw_object(model_matrix, sprite_texture_id);
    draw_object(model2_matrix, sprite2_texture_id);
    draw_object(model3_matrix, sprite3_texture_id);

    glDisableVertexAttribArray(shader_program.get_position_attribute());
    glDisableVertexAttribArray(shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }

int main(int argc, char* argv[]) {
    
    initialise();
    
    while (game_is_running) {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
