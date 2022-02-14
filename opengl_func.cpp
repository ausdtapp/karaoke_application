
#include "opengl_funcs.h"

//Window information
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;



//****************************************************************************************************************
//****************************************************************************************************************
// 
//Standard texture shaders

//The source for our standard texture's vertex shader, likely will be used for the selection screen
const char* texture_vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"out vec3 ourColor;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"   ourColor = aColor;\n"
"   gl_Position = vec4(aPos, 1.0);\n"
"   TexCoord = aTexCoord;\n"
"}\0";


//The source for basic textures (will have to make a uniform sampler2d for EACH texture I want to include), will probably make a separate source for TEXT based textures
const char* texture_fragmentShaderSource = "#version 330 core\n"
"in vec3 ourColor;\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D ourTexture;\n"
"void main()\n"
"{\n"
"   FragColor = texture(ourTexture, TexCoord);\n"
"}\n\0";    

//****************************************************************************************************************
//****************************************************************************************************************
// 
//Standard text shaders   

//The source for our standard texture's vertex shader
const char* text_vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec4 vertex;\n"
"out vec2 TexCoords;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
"   TexCoords = vertex.zw;\n"
"}\0";


//The source for basic textures (will have to make a uniform sampler2d for EACH texture I want to include), will probably make a separate source for TEXT based textures
const char* text_fragmentShaderSource = "#version 330 core\n"
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"uniform sampler2D text;\n"
"uniform vec3 textColor;\n"
"void main()\n"
"{\n"
"   vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
"   color = vec4(textColor, 1.0) * sampled;\n"
"}\n\0";


//Shader functions used for everything rendered
//These use the above shader sources for text and textures respectively 

int vertex_shader_creator(const char* vertex_source) {
    int vertex_Shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_Shader, 1, &vertex_source, NULL);
    glCompileShader(vertex_Shader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertex_Shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_Shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return vertex_Shader;
}

int fragment_shader_creator(const char* fragment_source) {
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    int success;
    char infoLog[512];
    glShaderSource(fragmentShader, 1, &fragment_source, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return fragmentShader;
}


int shader_program_creator(int vertexShader, int fragmentShader) {
    int shaderProgram = glCreateProgram();
    int success;
    char infoLog[512];
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}


//Function to generate/bind VAO/VBO/EBO and generating/binding texture information of the background texture

void background_component_generation(unsigned int &VAO, unsigned int &VBO, unsigned int &EBO, int &shader_program, unsigned int &background_texture) {
    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "background_texture"), 0);
    float vertices[] = {
        // positions          // colors           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };

    unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // ---------

    glGenTextures(1, &background_texture);
    glBindTexture(GL_TEXTURE_2D, background_texture);

    // set the texture wrapping parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

    //Takes path to local image for the background of the window

    unsigned char* data = stbi_load("test_image.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    glBindVertexArray(0);
}

//Generates components for video frames

void video_component_generation(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO, int& shader_program, unsigned int& video_texture,/* uint8_t* video_frame_data,*/ int width, int height) {
    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "background_texture"), 0);



    //Coordinates are changed from previous coordinates to change the size of where video is rendered
    //Also different because the way video data is decoded it causes the OpenGL rendering to be flipped over y-axis, so this adjusts for that
    float vertices[] = {
        // positions          // colors           // texture coords
         0.75f,  0.75f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // top right
         0.75f, -0.75f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // bottom right
        -0.75f, -0.75f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // bottom left
        -0.75f,  0.75f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // top left 
    };

    unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    glGenTextures(1, &video_texture);
    glBindTexture(GL_TEXTURE_2D, video_texture);

    // set the texture wrapping parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindVertexArray(0);
}



//Generates textures for all characters

std::map<char, Character> text_component_generation(unsigned int& text_VAO, unsigned int& text_VBO, int &text_shaderProgram) {

    //Projects the text to be based on screen's dimensions rather than the -1.0 to 1.0 measurement
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    glUseProgram(text_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(text_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


    //Use freetype's freetype object to laod arial characters into face object 
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        //return -1;
    }
    FT_Face face;
    if (FT_New_Face(ft, "C:\\Windows\\fonts\\arial.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        //return -1;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        //return -1;
    }

    //Object that contains all the textures and pixel information for text characters
    std::map<char, Character> characters;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character glyph = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        characters.insert(std::pair<char, Character>(c, glyph));
    }

    //clear freetype resources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);



    //Assign values to VAO/VBO objects
    glGenVertexArrays(1, &text_VAO);
    glGenBuffers(1, &text_VBO);
    glBindVertexArray(text_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, text_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return characters;
}


//****************    Text creation

//Funtion to bind and draw text textures using the map of characters created in main
void render_text(std::map<char, Character> characters, std::string text, int text_shader_program,unsigned int text_VAO, unsigned int text_VBO, float x_start, float y_start, float scale, glm::vec3 color) {
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glUseProgram(text_shader_program);
    glUniform3f(glGetUniformLocation(text_shader_program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(text_VAO);
    std::string::const_iterator c;

    for (c = text.begin(); c != text.end(); c++) {
        Character ch = characters[*c];
        float xpos = x_start + ch.Bearing.x * scale;
        float ypos = y_start - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        //Two triangles
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
             { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        //render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, text_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //advance cursor for next glyph
        x_start += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

}

//Function uses render text for printing text of songs searched for after taking a vector of sql "rows"

void print_songs(std::vector<Row> songs, const unsigned int screen_width, const unsigned int screen_height, std::map<char, Character> characters, int text_shader_program, unsigned int text_VAO, unsigned int text_VBO) {
    float y_modifier = 0.7f;
    for (int i = 0; i < songs.size(); i++) {
        int result_num = songs[i][0];
        string song = songs[i][1];
        string artist = songs[i][2];
        std::string main_string = std::to_string(result_num);
        std::string song_string = song;
        std::string artist_string = artist;
        main_string = main_string + ". " + song_string + " by " + artist_string;
        render_text(characters, main_string, text_shader_program, text_VAO, text_VBO, 0.075f * screen_width, y_modifier * screen_height, 0.25f, glm::vec3(0.0f, 0.0f, 0.0f));
        y_modifier -= .025f;
    }
}

//****************    Background creation

//Function to render the image file for the background

void render_background(unsigned int VAO, unsigned int background_texture, int shaderProgram) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, background_texture);
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


//****************    Video frame creation

//Function to render the video frames

void render_video_frame(unsigned int video_VAO, unsigned int video_texture, int video_shaderProgram, std::map<double, uint8_t*>::iterator frame_iterator, std::map<double, uint8_t*>::iterator frame_iterator_end, double time, int width, int height, bool stream_ended) {

    //First case is when your frame should wait to be rendered, then waits the difference in time\
    //Second case is when your map iterator reaches the end of the map
    //Third case is everything in between
    if (frame_iterator->first > time) {
        //std::cout << "waiting " << frame_iterator->first - time << " seconds" << std::endl;
        glfwWaitEventsTimeout(frame_iterator->first - time);
        uint8_t* cur_frame = frame_iterator->second;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, video_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, cur_frame);
        glGenerateMipmap(GL_TEXTURE_2D);
        glUseProgram(video_shaderProgram);
        glBindVertexArray(video_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    else if (frame_iterator == frame_iterator_end) {
        stream_ended = true;
    } else {
        uint8_t* cur_frame = frame_iterator->second;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, video_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, cur_frame);
        glGenerateMipmap(GL_TEXTURE_2D);
        glUseProgram(video_shaderProgram);
        glBindVertexArray(video_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }
}




