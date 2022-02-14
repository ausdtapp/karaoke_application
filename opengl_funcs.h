#pragma once

#include <iostream>
#include <map>

//For SQL elements
#include "sql_work.h"

//OpenGL libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//GL math headers for using different matrix/vector maths
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//For importing pictures for textures (backgrounds etc)
#include "stb_image.h"

//For creating TEXT
#include <ft2build.h>
#include FT_FREETYPE_H  


//Object to handle each individual character's specifics when handling text
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    signed long Advance;    // Offset to advance to next glyph
};

extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;


//Sources for textures like the background and other visuals
extern const char* texture_vertexShaderSource;
extern const char* texture_fragmentShaderSource;


//Sources for text
extern const char* text_vertexShaderSource;
extern const char* text_fragmentShaderSource;


//Functions for creating the vertex/fragment shaders and the shader program
int vertex_shader_creator(const char* vertex_source);
int fragment_shader_creator(const char* fragment_source);
int shader_program_creator(int vertexShader, int fragmentShader);

//Generation of components for background, text, and video frame textures
void background_component_generation(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO, int &shader_program, unsigned int &background_texture);
std::map<char, Character> text_component_generation(unsigned int& text_VAO, unsigned int& text_VBO,  int& text_shaderProgram);
void video_component_generation(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO, int& shader_program, unsigned int& video_texture,/*uint8_t* video_frame_data,*/ int width, int height);

//Objects and functions for text
void render_text(std::map<char, Character> characters, std::string text, int text_shader_program, unsigned int text_VAO, unsigned int text_VBO, float x_start, float y_start, float scale, glm::vec3 color);

//Function to render background
void render_background(unsigned int VAO, unsigned int background_texture, int shaderProgram);

//Function to render video frames
void render_video_frame(unsigned int video_VAO, unsigned int video_texture, int video_shaderProgram, std::map<double, uint8_t*>::iterator frame_iterator, std::map<double, uint8_t*>::iterator frame_iterator_end, double time, int width, int height, bool stream_ended);

//Song printing function
void print_songs(std::vector<Row> songs, const unsigned int screen_width, const unsigned int screen_height, std::map<char, Character> characters, int text_shader_program, unsigned int text_VAO, unsigned int text_VBO);

