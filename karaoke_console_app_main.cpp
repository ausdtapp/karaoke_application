//This file contains the "main" function of the karaoke console application

#include <thread>
#include <chrono>


#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"


//My created headers
#include "opengl_funcs.h"
#include "decoding_func.h"


//FFMPEG testing
//#define inline __inline
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


#include <inttypes.h>
#include <map>
#include <thread>

//States for the program, you either are in main menu, in a song search process, or playing a song
enum state {
    MAIN_MENU,
    SONG_SEARCH,
    SONG_RESULTS,
    SONG_PLAYING
};

//This class allows the tracking of the states during the program
class karaoke_session {
public:
    state program_state;

};

karaoke_session program;

//Processes window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);  

//Takes user's input characters and stores them as a string to use
void character_callback(GLFWwindow* window, unsigned int key);

//Callback for keys during program navigation
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//String to track user input and a mysql string object to plug into song search sql functions
std::string user_text_input;
string user_text_submission;

//Map of generated objects used in character generation
std::map<char, Character> characters;
int text_vertexShader;
int text_fragmentShader;
int text_shaderProgram;
unsigned int text_VAO, text_VBO;


int main()
{


    //Initializing of the window that everything will be rendered in and interacted with

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

   // glad: load all OpenGL function pointers
   //Keeps track of function pointers for OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


////****************    Text creation shaders and shader program
//
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int text_vertexShader = vertex_shader_creator(text_vertexShaderSource);
    int text_fragmentShader = fragment_shader_creator(text_fragmentShaderSource);
    int text_shaderProgram = shader_program_creator(text_vertexShader, text_fragmentShader);
    unsigned int text_VAO, text_VBO;

    std::map<char, Character> characters = text_component_generation(text_VAO, text_VBO, text_shaderProgram);

////////****************      Texture creation shaders and shader program

    int vertexShader = vertex_shader_creator(texture_vertexShaderSource);
    int fragmentShader = fragment_shader_creator(texture_fragmentShaderSource);
    int shaderProgram = shader_program_creator(vertexShader, fragmentShader);

    unsigned int VAO, VBO, EBO, background_texture;
    background_component_generation(VAO, VBO, EBO, shaderProgram, background_texture);

    ////////****************      Video Texture creation shaders and shader program

    int video_vertexShader = vertex_shader_creator(texture_vertexShaderSource);
    int video_fragmentShader = fragment_shader_creator(texture_fragmentShaderSource);
    int video_shaderProgram = shader_program_creator(video_vertexShader, video_fragmentShader);

    unsigned int video_VAO, video_VBO, video_EBO, video_texture;

    //Set up so the glfw window responds to character and key callback functions
    glfwSetCharCallback(window, character_callback);
    glfwSetKeyCallback(window, key_callback);

    //Bool objects to track if the video/audio stream has started/ended and gone through all frames
    bool stream_started = false;
    bool stream_ended = false;

    //Sets initial program state to the main menu
    program.program_state = MAIN_MENU;

    //Stream object that will be used to start and stop audio streams
    PaStream* stream;


    //// render loop
    while (!glfwWindowShouldClose(window))
    {

        processInput(window);

        //Sets background color to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //if program::state == MAIN_MENU, render main menu screen and text for buttons (artist/song search) , in mouse_callback thing during this state clicks the area where artist or song state is, then program::state = SONG_SEARCH

        if (program.program_state == MAIN_MENU) {
            render_background(VAO, background_texture, shaderProgram);
            render_text(characters, "Hit Enter to start song search", text_shaderProgram, text_VAO, text_VBO, 0.05f * SCR_WIDTH, 0.9f * SCR_HEIGHT, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f));
        }


        //if program::state == SONG_SEARCH, render user's input as typed, render "Type search term and hit enter", after enter is hit in this state program::state = SONG_RESULTS 

        if (program.program_state == SONG_SEARCH) {
            render_background(VAO, background_texture, shaderProgram);
            render_text(characters, "Type search text and hit Enter", text_shaderProgram, text_VAO, text_VBO, 0.05f * SCR_WIDTH, 0.9f * SCR_HEIGHT, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f));
            render_text(characters, "Input : " + user_text_input, text_shaderProgram, text_VAO, text_VBO, 0.05f * SCR_WIDTH, 0.8f * SCR_HEIGHT, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f));
        }


        //if program::state == SONG_RESULTS, render text "top 10 closest results", render print_songs(song_query(user_text_submission, my_session)), render a highlight over option mouse hovers (later)

        if (program.program_state == SONG_RESULTS) {
            render_background(VAO, background_texture, shaderProgram);
            render_text(characters, "Results for submission", text_shaderProgram, text_VAO, text_VBO, 0.05f * SCR_WIDTH, 0.9f * SCR_HEIGHT, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f));
            render_text(characters, user_text_submission, text_shaderProgram, text_VAO, text_VBO, 0.05f * SCR_WIDTH, 0.8f * SCR_HEIGHT, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f));
            print_songs(song_query(user_text_submission, my_session), SCR_WIDTH, SCR_HEIGHT, characters, text_shaderProgram, text_VAO, text_VBO);
        }

        //if program::state == SONG_PLAYING, it will inform the user that song is loading, decode the audio/video frames from the selected song, and renders the video while playing the audio 
        if (program.program_state == SONG_PLAYING) {

            //First statement marks that stream started, decodes audio/video frames, and plays first frame of video
            //Second statement ensures after stream ends that the state is reverted back to SONG_SEARCH for another search entry
            //Third statement renders every video frame before the  stream is marked as ended (last video_frame_map entry has been iterated through and rendered)
            if (!stream_started) {
                stream_started = true;
                const char* song = /*String of selected song*/;

                glfwSwapBuffers(window);
                render_background(VAO, background_texture, shaderProgram);
                render_text(characters, "Loading song", text_shaderProgram, text_VAO, text_VBO, 0.05f * SCR_WIDTH, 0.9f * SCR_HEIGHT, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f));

                //Video section
                decode_video(song);
                video_component_generation(video_VAO, video_VBO, video_EBO, video_shaderProgram, video_texture, video_width, video_height);

                //Audio section
                decode_audio(song);
                initialize_audio(&stream);

                //Renders first frame
                glfwSwapBuffers(window);
                glfwSetTime(0.0);
                double time = (int)(glfwGetTime() * 1000.0) / 1000.0;
                render_video_frame(video_VAO, video_texture, video_shaderProgram, video_frame_iterator, video_frame_map.end(), time, video_width, video_height, stream_ended);
                video_frame_iterator++;

            }
            else if (stream_ended == true) {
                stop_audio(&stream);
                program.program_state = SONG_SEARCH;
                stream_started = false;
                stream_ended = false;
            }
            else {
                double time = (int)(glfwGetTime() * 1000.0) / 1000.0;
                render_video_frame(video_VAO, video_texture, video_shaderProgram, video_frame_iterator, video_frame_map.end(), time, video_width, video_height, stream_ended);
                video_frame_iterator++;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Deleted existing buffers and arrays
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glDeleteVertexArrays(1, &text_VAO);
    glDeleteBuffers(1, &text_VBO);

    //End SQL and glfw session
    glfwTerminate();
    my_session.close();

    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //Changes window size
    glViewport(0, 0, width, height);
}


void character_callback(GLFWwindow* window, unsigned int key) {
    if (program.program_state == SONG_SEARCH) {
        user_text_input += (unsigned char)key;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (program.program_state == SONG_SEARCH && key == GLFW_KEY_BACKSPACE && user_text_input != "" && action == GLFW_PRESS) {
        user_text_input = user_text_input.substr(0, user_text_input.length() - 1);
    }
    if (program.program_state == MAIN_MENU && key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        std::cout << "entering song search";
        program.program_state = SONG_SEARCH;
    } else if (program.program_state == SONG_SEARCH && key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        std::cout << "entering song results";
        program.program_state = SONG_RESULTS;
        user_text_submission = user_text_input;
    }
    else if (program.program_state == SONG_RESULTS && key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        std::cout << "entering song playing " << std::endl;
        program.program_state = SONG_PLAYING;
    }
    else if (program.program_state == SONG_PLAYING && key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        std::cout << "entering main menu" << std::endl;
        program.program_state = MAIN_MENU;
    }

}




