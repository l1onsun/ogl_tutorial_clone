#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ctime>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <common/shader.hpp>
//GLFWwindow* window;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

extern "C"
{
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

GLFWwindow* init_gl(int width = 1024, int height = 768) {
    GLFWwindow* window;

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        exit(-1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "Tutorial 02 - Red triangle", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        exit(-1);
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    return window;
}

glm::mat4 matrix_calc() {
    const float alpha = clock() / 1000.0;
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    const glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

    // Camera matrix
    glm::mat4 View       = glm::lookAt(
            glm::vec3(1.5*cos(alpha) ,1.5*sin(alpha), 1.5), // Camera is at (4,3,3), in World Space
            //glm::vec3(1.5 ,1.5*sin(alpha), 1.5*cos(alpha)),
            glm::vec3(0,0,0), // and looks at the origin
            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    // Model matrix : an identity matrix (model will be at the origin)
    const glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
    return MVP;
}

void print_fps(clock_t& last_time, int n = 500) {
    static int i = 0;
    if (i == 99) {
        clock_t new_time = clock();
        if (new_time - last_time == 0)
            return;
        std::cout <<  n * CLOCKS_PER_SEC / ((float) (new_time - last_time)) << std::endl;
        last_time = new_time;
    }
    i = (i + 1) % n;
}

int main( void )
{
    GLFWwindow* window = init_gl();

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    static const GLfloat g_vertex_buffer_data[] = {
            -0.4f, 0.4f, -0.1f,
            0.8f, 0.0f, -0.1f,
            -0.4f, -0.4f, -0.1f,

            0.4f, 0.4f, 0.1f,
            -0.8f, 0.0f, 0.1f,
            0.4f, -0.4f, 0.1f,
    };
//
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint programA = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShaderA.fragmentshader" );
    GLuint programB = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShaderB.fragmentshader" );
    GLuint MatrixA = glGetUniformLocation(programA, "MVP");
    GLuint MatrixB = glGetUniformLocation(programB, "MVP");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    int i = 0;
    clock_t last_time = clock();
    do{
        glm::mat4 MVP = matrix_calc();
        glClear(GL_COLOR_BUFFER_BIT);
//
        glUseProgram(programA);
        glUniformMatrix4fv(MatrixA, 1, GL_FALSE, &MVP[0][0]);
		// Draw nothing, see you in tutorial 2 !
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
        );

        glDrawArrays(GL_TRIANGLES, 0, 3);
        glUseProgram(programB);
        glUniformMatrix4fv(MatrixB, 1, GL_FALSE, &MVP[0][0]);
        glDrawArrays(GL_TRIANGLES, 3, 6);
        glDisableVertexAttribArray(0);
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		print_fps(last_time);
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programA);
    glDeleteProgram(programB);
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

