#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <ao/ao.h>
#include <mpg123.h>
#include <unistd.h>
#include <cstdlib>

// #include <GL/glew.h>
// #include <GL/gl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define BITS 8

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

int do_rot, floor_rel;;
GLuint programID;
double last_update_time, current_time;
glm::vec3 rect_pos, floor_pos;
float rectangle_rotation = 0;

glm::vec3 floorPos;
glm::vec3 floorRot = glm::vec3(0,0,0);

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
	{
	    std::string Line = "";
	    while(getline(VertexShaderStream, Line))
		VertexShaderCode += "\n" + Line;
	    VertexShaderStream.close();
	}

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
	std::string Line = "";
	while(getline(FragmentShaderStream, Line))
	    FragmentShaderCode += "\n" + Line;
	FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    //    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    //    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    //    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

// void initGLEW(void){
//     glewExperimental = GL_TRUE;
//     if(glewInit()!=GLEW_OK){
// 	fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
//     }
//     if(!GLEW_VERSION_3_3)
// 	fprintf(stderr, "3.3 version not available\n");
// }



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}



/**************************
 * Customizable functions *
 **************************/

 struct faceColor{
  float R;
  float G;
  float B;
};

struct tile{
  int type;
  glm::vec3 T;
  glm::vec3 R;
  glm::vec3 S;
};

struct Block
{
  int state,move,angle,tempAngle;
  int speed;//Animation speed
  int x,y,z;//Which axis to rotate around
  glm::mat4 memoryMat;//Stores all prev transformations of block
  float cx,cy,cz;//Current center of block
  float tx,ty,tz;//Which edge of the block to rotate around
  int sx,sy,sz;//Scale factor
};

vector< vector<tile> > tileGrid1;
vector< vector<tile> > tileGrid2;

vector<Block> block;

float X1[4] = {-0.5,0,0,0.5};
float Y1[4] = {-0.5,-0.5,-0.5,-0.5};
float Z1[4] = {0,0.5,-0.5,0};
int p = 1;
int i = 0;
float divY = 1,divX = 1,divZ = 1;
glm::ivec2 blockTile[2];
int x1 = 7,x2,ya = 3,yb;
int lastMove=0;
int teleXa,teleYa;
vector<int> fragX,fragY;
int teleXb,teleYb,pathX,pathY,goalX,goalY;
vector<int> bridgeX,bridgeY;

Block initBlock(float cx,float cy,float cz,int sx,int sy,int sz)
{
  Block bloc;
  bloc.move = 0;
  bloc.angle = 0;
  bloc.tempAngle = 0;
  bloc.x = bloc.y = bloc.z = 0;
  bloc.state = 1;
  bloc.speed = 11;
  bloc.tx = bloc.ty = bloc.tz = 0;
  bloc.cx = cx;
  bloc.cy = cy;
  bloc.cz = cz;
  bloc.sx = sx;
  bloc.sz = sz;
  bloc.sy = sy;
  return bloc;
}



GLfloat floorColor[] =
{
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  
};

GLfloat blockColor[] =
{
 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,

 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,

 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,

 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,


 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,

 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,


 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,

 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,


 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,

 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,


 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,

 0.4,0.2,0,
 0.4,0.2,0,
 0.4,0.2,0,

};

GLfloat floorColorRed[] =
{
  1,0,0,
  1,0,1,
  1,0,0,

  1,0,0,
  1,0,0,
  1,0,1,

  1,0,0,
  1,0,1,
  1,0,0,

  1,0,0,
  1,0,1,
  1,0,0,

  1,0,0,
  1,0,0,
  1,0,0,

  1,0,0,
  1,0,0,
  1,0,0,

  1,0,0,
  1,0,0,
  1,0,0,

  1,0,0,
  1,0,0,
  1,0,0,

  1,0,0,
  1,0,0,
  1,0,0,

  1,0,0,
  1,0,1,
  1,0,0,

  1,0,0,
  1,0,0,
  1,0,0,

  1,0,0,
  1,0,0,
  1,0,0,
};

GLfloat floorColorGreen[] =
{
  0,1,0,
  1,1,0,
  0,1,0,

  0,1,0,
  0,1,0,
  1,1,0,

  0,1,0,
  1,1,0,
  0,1,0,

  0,1,0,
  1,1,0,
  0,1,0,

  0,1,0,
  0,1,0,
  0,1,0,

  0,1,0,
  0,1,0,
  0,1,0,

  0,1,0,
  0,1,0,
  0,1,0,

  0,1,0,
  0,1,0,
  0,1,0,

  0,1,0,
  0,1,0,
  0,1,0,

  0,1,0,
  1,1,0,
  0,1,0,

  0,1,0,
  0,1,0,
  0,1,0,

  0,1,0,
  0,1,0,
  0,1,0,
};
GLfloat floorColorBlue[] =
{
  0,0,1,
  1,0,1,
  0,0,1,

  0,0,1,
  0,0,1,
  1,0,1,

  0,0,1,
  1,0,1,
  0,0,1,

  0,0,1,
  1,0,1,
  0,0,1,

  0,0,1,
  0,0,1,
  0,0,1,

  0,0,1,
  0,0,1,
  0,0,1,

  0,0,1,
  0,0,1,
  0,0,1,

  0,0,1,
  0,0,1,
  0,0,1,

  0,0,1,
  0,0,1,
  0,0,1,

  0,0,1,
  1,0,1,
  0,0,1,

  0,0,1,
  0,0,1,
  0,0,1,

  0,0,1,
  0,0,1,
  0,0,1,
};

GLfloat borderColor[] =
{
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0,0,0,
  0,0,0,
  0,0,0,
  
  0,0,0,
  0.75,0.75,0.75,
  0,0,0,


  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  
  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,

  0.75,0.75,0.75,
  0.75,0.75,0.75,
  0.75,0.75,0.75,
};

int currColor = 1;
int floorWidth = 20;
int floorHeight = 10;
int drop[2] = {0,0};
int kill[2] = {0,0};
float rectangle_rot_dir = 1;
int c = 0;
glm::vec3 blockScale = glm::vec3(1,1,1);
//Block block;
bool rotLock = false;

bool rectangle_rot_status = true, keys[1024], zoomin = false, zoomout = false, panr = false, panl = false;
GLfloat lastFrame = 0.0f;   // Time of last frame
GLfloat deltaTime = 0.0f;   // Time between current frame and last frame
GLfloat currentFrame = glfwGetTime();
glm::vec3 eye = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

int moves = 0;
int splitz = 0;
int lives = 3;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
/*void split(int k)
{
  
  if(c == 1)
    c = 0;
  else if(c == 0)
    c = 1;
  if(p == 1 && k == 1)
  {
    splitz = 1;
    p++;
    c = 1;
    if(block[0].state == 1)
    {
      block[0].cy -= 0.5;
      x2 = x1;
      yb = ya;
    }
    else if(block[0].state == 2)
      block[0].cx += 0.5;
    else if(block[0].state == 3)
      block[0].cz += 0.5;
    glm::mat4 translateRectangleToAxis1 = glm::translate(glm::vec3(block[0].cx,block[0].cy,block[0].cz));        // glTranslatef
    block[0].memoryMat = translateRectangleToAxis1;

    if(block[0].state == 1)
      block.push_back(initBlock(block[0].cx,-3.3,block[0].cz,1,1,1));
    else if(block[0].state == 2)
      block.push_back(initBlock(block[0].cx - 1,-3.3,block[0].cz,1,1,1));
    else if(block[0].state == 3)
      block.push_back(initBlock(block[0].cx,-3.3,block[0].cz - 1,1,1,1));
    glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[1].cx,block[1].cy,block[1].cz));        // glTranslatef
    block[1].memoryMat = translateRectangleToAxis;

  }
}*/

/*void join()
{
  int j = 0;
  if(block[0].cx == block[1].cx)
  {
    
    if(block[0].cz - block[1].cz == -1)
    {
      block[0].state = 3;
      block[0].sx = 1;
      block[0].sy = 1;
      block[0].sz = 2;

      block[0].cz += 0.5;
      j = 1;
      splitz = 0;
    }
    else if(block[0].cz - block[1].cz == 1)
    {
      block[0].state = 3;
      block[0].sx = 1;
      block[0].sy = 1;
      block[0].sz = 2;

      block[0].cz -= 0.5;
      j = 1;
      splitz = 0;

    }
  }
  else if(block[0].cz == block[1].cz)
  {
    
    if(block[0].cx - block[1].cx == -1)
    {
      block[0].state = 2;
      block[0].sx = 2;
      block[0].sy = 1;
      block[0].sz = 1;

      block[0].cx += 0.5;
      j = 1;
      splitz = 0;

    }
    else if(block[0].cx - block[1].cx == 1)
    {
      block[0].state = 2;
      block[0].sx = 2;
      block[0].sy = 1;
      block[0].sz = 1;

      block[0].cx -= 0.5;
      j = 1;
      splitz = 0;

    }
  }
  if(j == 1)
  {
    c = 0;
    p = 1;
    glm::mat4 translateRectangleToAxis1 = glm::translate(glm::vec3(block[0].cx,block[0].cy,block[0].cz));        // glTranslatef
    block[0].memoryMat = translateRectangleToAxis1;
    block.erase(block.end());
  }
}*/

int score = 0;

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
  int b = c;
  float shiftSide = 1.5;
  float shiftDown = 0.5;
  if(p == 2)
  {
    shiftSide = 1;
    shiftDown = 0;
  }  
  GLfloat cameraSpeed = 0.10f;
  if(key == GLFW_KEY_W)
      eye += cameraSpeed * front;
  if(key == GLFW_KEY_S)
      eye -= cameraSpeed * front;
  if(key == GLFW_KEY_A)
      eye -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
  if(key == GLFW_KEY_D)
      eye += glm::normalize(glm::cross(front, up)) * cameraSpeed;
   if(key == GLFW_KEY_T)
  {
    eye = glm::vec3(0.0f, 5.0f, -floorHeight/2);
    front = glm::vec3(0.0f, -1.0f, 0.0f);
    up = glm::vec3(0.0f, 0.0f, -1.0f);
  }  
  if(key == GLFW_KEY_N)
  {
    eye = glm::vec3(0.0f, 0.0f, 3.0f);
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
  }     
    // Function is called first on GLFW_PRESS.
  if (action == GLFW_PRESS) {
        switch (key) {
  /*case GLFW_KEY_P:
      split(1);
      break;
  case GLFW_KEY_J:
      join();
      break;*/
  case GLFW_KEY_LEFT:
      score++;
      system("echo -e \"\a\"");
      moves = 1;
      block[b].move = 1;
      block[b].angle = 88;
      i = 0;
      if(p == 1)
      {

        if(block[b].state == 1)
        {
          block[b].state = 2;
          block[b].cy -= shiftDown;
          block[b].cx -= shiftSide;
          divY = 0.5;
          x2=x1-2;
          yb=ya;
          x1--;
        }
        else if(block[b].state == 2)
        {
          block[b].state = 1;
          block[b].cx -= shiftSide;
          block[b].cy += shiftDown;
          divX = 0.5;
          x2=x1-1;
          yb=ya;
          x1-=2;
        }
        else if(block[b].state == 3)
        {
          block[b].cx -= 1;
          x2=x2-1;
          x1--;
        }
      }
      else
      {
        if(b == 0)
        {
          x1--;
        }
        else
          x2--;
        block[b].cx -= shiftSide;
      }
      block[b].x = 0;block[b].y = 0;block[b].z = 1;
      break;
  case GLFW_KEY_RIGHT:
      score++;
      system("echo -e \"\a\"");
      moves = 2;
      block[b].move = 1;
      block[b].angle = -88;
      i = 3;
      if(p == 1)
      {
      if(block[b].state == 1)
      {
        block[b].state = 2;
        block[b].cx += shiftSide;
        block[b].cy -= shiftDown;
        divY = 0.5;
        x2=x1+1;
        yb=ya;
        x1 = x1 + 2;
      }
      else if(block[b].state == 2)
      {
        block[b].state = 1;
        block[b].cx += shiftSide;
        block[b].cy += shiftDown;
        divX = 0.5;
        x2=x2+2;
        yb=ya;
        x1 += 1;
      }
      else if(block[b].state == 3)
      {
        block[b].cx += 1;
        x2=x2+1;
        x1 += 1;
      }
      }
      else
      {
        if(b == 0)
        {
          x1++;
        }
        else
          x2++;
        block[b].cx += shiftSide;
      }
      block[b].x = 0;block[b].y = 0;block[b].z = 1;
      break;
  case GLFW_KEY_UP:
      score++;
      system("echo -e \"\a\"");
      moves = 3;
      block[b].move = -1;
      block[b].angle = -88;
      i = 2;
      if(p == 1)
      {
      if(block[b].state == 1)
      {
        block[b].state = 3;
        block[b].cz -= shiftSide;
        block[b].cy -= shiftDown;
        divY = 0.5;
        yb=ya+2;
        x2=x1;
        ya++;
      }
      else if(block[b].state == 2)
      {
        block[b].cz -= 1;
        yb++;
        ya++;
      }
      else if(block[b].state == 3)
      {
        block[b].state = 1;
        block[b].cz -= shiftSide;
        block[b].cy += shiftDown;
        divZ = 0.5;
        yb=ya+1;
        ya+=2;
      }
      }
      else
      {
        if(b == 0)
        {
          ya++;
        }
        else
          yb++;
        block[b].cz -= shiftSide;
      }
      block[b].x = 1;block[b].y = 0;block[b].z = 0;
      break;
  case GLFW_KEY_DOWN:
      score++;
      system("echo -e \"\a\"");
      moves = 4;
      block[b].move = -1;
      block[b].angle = 88;
      i = 1;
      if(p == 1)
      {
      if(block[b].state == 1)
      {
        block[b].state = 3;
        block[b].cz += shiftSide;
        block[b].cy -= shiftDown;
        divY = 0.5;
        yb=ya-1;
        x2=x1;
        ya-=2;
      }
      else if(block[b].state == 2)
      {
        block[b].cz += 1;
        yb--;
        ya--;
      }
      else if(block[b].state == 3)
      {
        block[b].state = 1;
        block[b].cz += shiftSide;
        block[b].cy += shiftDown;
        divZ = 0.5;
        yb=ya-2;
        ya-=1;
      }
      }
      else
      {
        if(b == 0)
        {
          ya--;
        }
        else
          yb--;
        block[b].cz += shiftSide;
      }
      block[b].x = 1;block[b].y = 0;block[b].z = 0;
      break;      
  default:
      break;
        }
        // cout << x1 << " " << ya << " "<< x2 << " " << yb << endl;
    }
}


void do_movement()
{
    GLfloat cameraSpeed = 5.0f * deltaTime;
    if(keys[GLFW_KEY_W])
        eye += cameraSpeed * front;
    if(keys[GLFW_KEY_S])
        eye -= cameraSpeed * front;
    if(keys[GLFW_KEY_A])
        eye -= glm::normalize(glm::cross(front, up))*cameraSpeed;
    if(keys[GLFW_KEY_D])
        eye += glm::normalize(glm::cross(front, up))*cameraSpeed;
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
    case 'Q':
    case 'q':
	quit(window);
	break;
    
    default:
	break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        
    default:
	break;
    }
}



/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
GLfloat fov = 90.0f;
double yaw, pitch;
GLfloat lastX = 683, lastY = 384;
bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
  
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    GLfloat sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 ffront;
    ffront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    ffront.y = sin(glm::radians(pitch));
    ffront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(ffront);
}  

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  //eye -= yoffset * front;
  // if(fov >= 1.0f && fov <= 90.0f)
  //   fov -= yoffset;
  // // if(fov <= 1.0f)
  // //   fov = 1.0f;
  // // if(fov >= 45.0f)
  // //   fov = 45.0f;
}
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    
    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views

    Matrices.projection = glm::perspective(fov, (GLfloat)fbwidth/(GLfloat)fbheight, 0.1f, 100.0f);

    // Ortho projection for 2D views
    //Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *rectangle, *cam, *floor_vao,*sevenSeg;

void createSevenSeg()
{
  static const GLfloat vertex_buffer_data [] = {
    -0.02,-0.02,0, // vertex 1
    0.02,-0.02,0, // vertex 2
    0.02, 0.2,0, // vertex 3

    0.02, 0.2,0, // vertex 3
    -0.02, 0.2,0, // vertex 4
    -0.02,-0.02,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  sevenSeg = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
	-0.5, 0.5, 0.5, 
	-0.5, -0.5, 0.5, 
	0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5, 
	0.5, -0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, -0.5,
	0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	-0.5, 0.5, -0.5,
	-0.5, 0.5, -0.5,
	-0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5, 
	-0.5, 0.5, -0.5,
	-0.5, -0.5, 0.5, 
	-0.5, 0.5, 0.5, 
	-0.5, 0.5, -0.5,
	-0.5, 0.5, 0.5, 
	0.5, 0.5, 0.5,
	-0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	-0.5, -0.5, 0.5, 
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	-0.5, -0.5, 0.5, 
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,
	-0.5, 0.5, 0.5,
	0.5, 0.5, -0.5,
	0.5, 0.75, -0.5,
    };

    if(currColor == 1)
    rectangle = create3DObject(GL_TRIANGLES, 12*3 , vertex_buffer_data, floorColor, GL_FILL);
    else if(currColor == 2)
    rectangle = create3DObject(GL_TRIANGLES, 12*3 , vertex_buffer_data, floorColorRed, GL_FILL);
    else if(currColor == 3)
    rectangle = create3DObject(GL_TRIANGLES, 12*3 , vertex_buffer_data, floorColorGreen, GL_FILL);
    else if(currColor == 4)
    rectangle = create3DObject(GL_TRIANGLES, 12*3 , vertex_buffer_data, floorColorBlue, GL_FILL);
else if(currColor == 5)
    rectangle = create3DObject(GL_TRIANGLES, 12*3 , vertex_buffer_data, blockColor, GL_FILL);

 
}

void createRectangleBorder ()
{
GLfloat vertex_buffer_data [] = {
    -0.5, 0.5, 0.5,
    -0.5, -0.5, 0.5,
    0.5, -0.5, 0.5,

    -0.5, 0.5, 0.5,
    0.5, -0.5, 0.5,
    0.5, 0.5, 0.5,

    0.5, 0.5, 0.5,
    0.5, -0.5, 0.5,
    0.5, -0.5, -0.5,

    0.5, 0.5, 0.5,
    0.5, -0.5, -0.5,
    0.5, 0.5, -0.5,

    0.5, 0.5, -0.5,
    0.5, -0.5, -0.5,
    -0.5, -0.5, -0.5,

    0.5, 0.5, -0.5,
    -0.5, -0.5, -0.5,
    -0.5, 0.5, -0.5,

    -0.5, 0.5, -0.5,
    -0.5, -0.5, -0.5,
    -0.5, -0.5, 0.5,

    -0.5, 0.5, -0.5,
    -0.5, -0.5, 0.5,
    -0.5, 0.5, 0.5,

    -0.5, 0.5, -0.5,
    -0.5, 0.5, 0.5,
    0.5, 0.5, 0.5,

    -0.5, 0.5, -0.5,
    0.5, 0.5, 0.5,
    0.5, 0.5, -0.5,

    -0.5, -0.5, 0.5,
    -0.5, -0.5, -0.5,
    0.5, -0.5, -0.5,

    -0.5, -0.5, 0.5,
    0.5, -0.5, -0.5,
    0.5, -0.5, 0.5,

    };


    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle = create3DObject(GL_TRIANGLES, 12*3 , vertex_buffer_data, borderColor, GL_LINE);
}

void createCam ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
	-0.1, 0, 0,
	0.1, 0, 0, 
	0, 0.1, 0,
    };

    static const GLfloat color_buffer_data [] = {
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    cam = create3DObject(GL_TRIANGLES, 1*3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createFloor ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
	-1, -0.5, 1,
	1, -0.5, 1, 
	-1, -0.5, -1,
	-1, -0.5, -1,
	1, -0.5, 1, 
	1, -0.5, -1,
    };

    static const GLfloat color_buffer_data [] = {
	0.65, 0.165, 0.165,
	0.65, 0.165, 0.165,
  
	0.65, 0.165, 0.165,
	0.65, 0.165, 0.165,
	0.65, 0.165, 0.165,
	0.65, 0.165, 0.165,
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    floor_vao = create3DObject(GL_TRIANGLES, 2*3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

int level = 1;

int lvlone[10][20] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,2,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,3,1,1,1,0,0,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,5,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
  };

int lvltwo[10][20] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,4,1,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0},
    {1,1,1,0,0,1,1,1,1,4,0,0,1,5,1,1,0,0,0,0},
    {1,1,1,0,0,1,1,1,1,1,0,0,1,1,2,1,0,0,0,0},
    {3,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
  };

float camera_rotation_angle = 90;


glm::mat4 genModelMatrix(glm::vec3 translate, glm::vec3 rotation , glm::vec3 scale)
{
  glm::mat4 tempScale = glm::scale(glm::mat4(1.0f),scale);
  glm::mat4 tempTranslate = glm::translate(translate);
  glm::mat4 tempRotateZ = glm::rotate((float)(rotation.z * M_PI/180.0f ),glm::vec3(0,0,1));
  glm::mat4 tempRotateY = glm::rotate((float)(rotation.y * M_PI/180.0f) ,glm::vec3(0,1,0));
  glm::mat4 tempRotateX = glm::rotate((float)(rotation.x * M_PI/180.0f),glm::vec3(1,0,0));
  return tempTranslate*tempRotateX*tempRotateY*tempRotateZ*tempScale;
  // glm::mat4 T = glm::rotate((float)((90 - camera_rotation_angle)*M_PI/180.0f), glm::vec3(0,1,0));

  // return tempTranslate * tempScale;
}
int done = 0;


float ssx[7] = {3.5,3.5,3.5,3.5,3.5,3.7,3.7};
float ssy[7] = {3.5,3.2,3.45,3.15,3.75,3.5,3.2};
float ssa[7] = {0,0,-90,-90,-90,0,0};
float vis[7] = {1,1,1,1,1,1,1};

void drawPrintScore(glm::mat4 VP, glm::mat4 MVP)
{
  int first = score % 10;
  int temp = score/10;
  int sec = temp % 100;
  int firstP = lives % 10;

  switch(firstP)
  {
    case 0:
          vis[2] = 0;
          break;
    case 1:
          vis[2] = vis[3] = vis[4] = vis[5] = vis[6] = 0;
          break;
    case 2:
          vis[0] = vis[6] = 0;
          break;
    case 3:
          vis[0] = vis[1] = 0;
          break;
    case 4:
          vis[1] = vis[4] = vis[3] = 0;
          break;
    case 5:
          vis[1] = vis[5] = 0;
          break;
    case 6:
          vis[5] = 0;
          break;
    case 7:
          vis[0] = vis[1] = vis[2] = vis[3] = 0;
          break;
    case 8:
          break;
    case 9:
          vis[1] = 0;
  }
  for(int i = 0;i<7;i++)
  {
    if(vis[i] == 1)
    {
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 translateRectangle = glm::translate (glm::vec3(ssx[i] - 7, ssy[i], 0));        // glTranslatef
      glm::mat4 rotateRectangle = glm::rotate((float)(ssa[i]*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)    Matrices.model *= (translateRectangle);
      Matrices.model *= (translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(sevenSeg);
    }
  }
  vis[0] = vis[1] = vis[2] = vis[3] = vis[4] = vis[5] = vis[6] = 1;

  switch(first)
  {
    case 0:
          vis[2] = 0;
          break;
    case 1:
          vis[2] = vis[3] = vis[4] = vis[5] = vis[6] = 0;
          break;
    case 2:
          vis[0] = vis[6] = 0;
          break;
    case 3:
          vis[0] = vis[1] = 0;
          break;
    case 4:
          vis[1] = vis[4] = vis[3] = 0;
          break;
    case 5:
          vis[1] = vis[5] = 0;
          break;
    case 6:
          vis[5] = 0;
          break;
    case 7:
          vis[0] = vis[1] = vis[2] = vis[3] = 0;
          break;
    case 8:
          break;
    case 9:
          vis[1] = 0;
  }
  for(int i = 0;i<7;i++)
  {
    if(vis[i] == 1)
    {
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 translateRectangle = glm::translate (glm::vec3(ssx[i], ssy[i], 0));        // glTranslatef
      glm::mat4 rotateRectangle = glm::rotate((float)(ssa[i]*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)    Matrices.model *= (translateRectangle);
      Matrices.model *= (translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(sevenSeg);
    }
  }
  vis[0] = vis[1] = vis[2] = vis[3] = vis[4] = vis[5] = vis[6] = 1;

  switch(sec)
  {
    case 0:
          vis[0] = vis[1] = vis[2] = vis[3] = vis[4] = vis[5] = vis[6] = 0;
          break;
    case 1:
          vis[2] = vis[3] = vis[4] = vis[5] = vis[6] = 0;
          break;
    case 2:
          vis[0] = vis[6] = 0;
          break;
    case 3:
          vis[0] = vis[1] = 0;
          break;
    case 4:
          vis[1] = vis[4] = vis[3] = 0;
          break;
    case 5:
          vis[1] = vis[5] = 0;
          break;
    case 6:
          vis[5] = 0;
          break;
    case 7:
          vis[0] = vis[1] = vis[2] = vis[3] = 0;
          break;
    case 8:
          break;
    case 9:
          vis[1] = 0;
  }
  for(int i = 0;i<7;i++)
  {
    if(vis[i] == 1)
    {
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 translateRectangle = glm::translate (glm::vec3(ssx[i] - 0.5, ssy[i], 0));        // glTranslatef
      glm::mat4 rotateRectangle = glm::rotate((float)(ssa[i]*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)    Matrices.model *= (translateRectangle);
      Matrices.model *= (translateRectangle * rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(sevenSeg);
    }
  }
  vis[0] = vis[1] = vis[2] = vis[3] = vis[4] = vis[5] = vis[6] = 1;
}

int checkTile(int b)
{
  if(level == 1)
  {
    if(block[b].state == 1 && p == 1)
    {
        if(tileGrid1[ya][x1].type == 0)
        {
          if(drop[b] == 1)
            kill[b] = 1;
          drop[b] = 1;
          return 0; 
        }
    }
    else if(p == 1)
    {
      if(tileGrid1[ya][x1].type == 0 && tileGrid1[yb][x2].type == 0)
      {
        if(drop[b] == 1)
          kill[b] = 1;
        drop[b] = 1;
        return 0; 
      }
    }
    else
    {
      if(tileGrid1[ya][x1].type == 0 && b == 0)
      {
        if(drop[b] == 1)
          kill[b] = 1;
        drop[b] = 1;
        return 0; 
      }
      else if(tileGrid1[yb][x2].type == 0 && b == 1)
      {
        if(drop[b] == 1)
          kill[b] = 1;
        drop[b] = 1;
        return 0;
      }
    }
    if(block[b].state == 1 && p == 1)
    {
      if(x1 == goalX && ya == goalY)
      {
        level = 2;
        //Change Special Blocks
        drop[0] = 0;
        drop[1] = 0;
        kill[0] = 0;
        kill[1] = 0;
        teleXa = 9;
        teleYa = 3;
        teleXb = 1;
        teleYb = 2;

        goalX = 13;
        goalY = 3;

        fragX[0] = 14;
        fragY[0] = 4;
        //fragX.push_back(7);
        //fragY.push_back(2);
        //fragX.push_back(7);
        //fragY.push_back(4);
        //fragX.push_back(14);
        //fragY.push_back(3);

        pathX = 0;
        pathY = 5;

        bridgeX.clear();
        bridgeY.clear();
        bridgeX.push_back(10);
        bridgeX.push_back(11);
        //bridgeX.push_back(13);
        //bridgeX.push_back(14);
        //bridgeX.push_back(15);
        //bridgeX.push_back(16);
        //bridgeX.push_back(16);
        //bridgeX.push_back(15);
        bridgeY.push_back(3);
        bridgeY.push_back(3);
        //bridgeY.push_back(6);
        //bridgeY.push_back(6);
        //bridgeY.push_back(6);
        //bridgeY.push_back(7);
        //bridgeY.push_back(7);
        //bridgeY.push_back(5);


        glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(0,-2.8,-3));        // glTranslatef
        // glm::mat4 translateRectangleFromAxis = glm::translate(glm::vec3(-tx,-ty,-tz));        // glTranslatef
        block[0].memoryMat = translateRectangleToAxis;
        block[0].cx = 0;
        block[0].cy = -2.8;
        block[0].cz = -3;
        // block[1].memoryMat = translateRectangleToAxis;
        x1 = 7;
        ya = 3;
        x2 = 7;
        yb = 3;
      }
      else if(x1 == fragX[0] && ya == fragY[0])
      {
        tileGrid1[fragY[0]][fragX[0]].type = 0;
        if(drop[0] == 1)
          kill[0] = 1;
        drop[0] = 1;
        if(drop[1] == 1)
          kill[1] = 1;
        drop[1] = 1;
        return 0; 
      }
      else if(x1 == teleXa && ya == teleYa && done == 0)
      {
        done = 1;
        int x = -8;
        int z = 1;
        block[0].cx += x;
        block[0].cz += z;
        x1 = teleXb;
        ya = teleYb;
        x2 = x1;
        yb = ya;
        glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[b].cx,block[b].cy,block[b].cz));        // glTranslatef
        // glm::mat4 translateRectangleFromAxis = glm::translate(glm::vec3(-tx,-ty,-tz));        // glTranslatef
        block[b].memoryMat = translateRectangleToAxis; 
      }
      else if(x1 == teleXb && ya == teleYb && done == 0)
      {
        done = 1;
        int x = 8;
        int z = -1;
        block[0].cx += x;
        block[0].cz += z;
        x1 = teleXa;
        ya = teleYa;
        x2 = x1;
        yb = ya;
        glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[b].cx,block[b].cy,block[b].cz));        // glTranslatef
        // glm::mat4 translateRectangleFromAxis = glm::translate(glm::vec3(-tx,-ty,-tz));        // glTranslatef
        block[b].memoryMat = translateRectangleToAxis; 
      }
    }
  }
  else
  {
    if(block[b].state == 1 && p == 1)
    {
        if(tileGrid2[ya][x1].type == 0)
        {
          if(drop[b] == 1)
            kill[b] = 1;
          drop[b] = 1;
          return 0; 
        }
    }
    else if(p == 1)
    {
      if(tileGrid2[ya][x1].type == 0 && tileGrid2[yb][x2].type == 0)
      {
        if(drop[b] == 1)
          kill[b] = 1;
        drop[b] = 1;
        return 0; 
      }
    }
    else
    {
      if(tileGrid2[ya][x1].type == 0 && b == 0)
      {
        if(drop[b] == 1)
          kill[b] = 1;
        drop[b] = 1;
        return 0; 
      }
      else if(tileGrid2[yb][x2].type == 0 && b == 1)
      {
        if(drop[b] == 1)
          kill[b] = 1;
        drop[b] = 1;
        return 0; 
      }
    }
    if(block[b].state == 1 && p == 1)
    {
      if(x1 == goalX && ya == goalY)
      {
        cout << "Congrats you win" << endl;
        exit(0);
      }
      // cout << "Hekki" << endl;
      for(int j = 0;j<fragX.size();j++)
      {
        if(x1 == fragX[j] && ya == fragY[j])
        {
          tileGrid2[fragY[j]][fragX[j]].type = 0;
          if(drop[0] == 1)
            kill[0] = 1;
          drop[0] = 1;
          if(drop[1] == 1)
            kill[1] = 1;
          drop[1] = 1;
          return 0; 
        }
      }
      if(x1 == teleXa && ya == teleYa && done == 0)
      {
        done = 1;
        int x = -8;
        int z = 1;
        block[0].cx += x;
        block[0].cz += z;
        x1 = teleXb;
        ya = teleYb;
        x2 = x1;
        // cout << x1 << " " << ya << "asf"<<endl;
        yb = ya;
        glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[b].cx,block[b].cy,block[b].cz));        // glTranslatef
        // glm::mat4 translateRectangleFromAxis = glm::translate(glm::vec3(-tx,-ty,-tz));        // glTranslatef
        block[b].memoryMat = translateRectangleToAxis; 
      }
      else if(x1 == teleXb && ya == teleYb && done == 0)
      {
        done = 1;
        int x = 8;
        int z = -1;
        block[0].cx += x;
        block[0].cz += z;
        x1 = teleXa;
        ya = teleYa;
        x2 = x1;
        yb = ya;
        // cout << x1 << " " << ya << endl;

        glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[b].cx,block[b].cy,block[b].cz));        // glTranslatef
        // glm::mat4 translateRectangleFromAxis = glm::translate(glm::vec3(-tx,-ty,-tz));        // glTranslatef
        block[b].memoryMat = translateRectangleToAxis; 
      }
    }
  }
  return 1;
}

void checkTrigger(int b)
{
  if(level == 1)
  {
    if((x1 == pathX && ya == pathY) || (x2 == pathX && yb == pathY))
    {
      for(int i = 0;i<2;i++)
      {
          if(tileGrid1[bridgeY[i]][bridgeX[i]].type == 0)
            tileGrid1[bridgeY[i]][bridgeX[i]].type = 1;
          else
            tileGrid1[bridgeY[i]][bridgeX[i]].type = 0;
      }
    }
  }
  else
  {
    if((x1 == pathX && ya == pathY) || (x2 == pathX && yb == pathY))
    {
      for(int i = 0;i<2;i++)
      {
          // cout << i << " " << j << endl;
          if(tileGrid2[bridgeY[i]][bridgeX[i]].type == 0)
            tileGrid2[bridgeY[i]][bridgeX[i]].type = 1;
          else
            tileGrid2[bridgeY[i]][bridgeX[i]].type = 0;
      }
    } 
  }
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void drawBlock(glm::mat4 VP,glm::mat4 MVP, GLFWwindow * window,int doM,int b)
{
  // MVP *= prev;
  if(block[0].move != 0 && done == 1)
    done = 0;
  if(block[b].move == 0)
    checkTile(b);
  // checkTrigger(b);
  if(kill[b] == 1)
  {
    if(block[b].cy > -12)
    {
      // cout << block[0].cy << endl;
      block[b].cy -= 0.2;
      glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[b].cx,block[b].cy,block[b].cz));        // glTranslatef
      // glm::mat4 translateRectangleFromAxis = glm::translate(glm::vec3(-tx,-ty,-tz));        // glTranslatef
      block[b].memoryMat = translateRectangleToAxis; 
    }
    else
    {
      drop[0] = 0;
        drop[1] = 0;
        kill[0] = 0;
        kill[1] = 0;
      cout << "Oops" << endl;
      score = 0;
      lives--;
      glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(0,-2.8,-3));        // glTranslatef
        // glm::mat4 translateRectangleFromAxis = glm::translate(glm::vec3(-tx,-ty,-tz));        // glTranslatef
        block[0].memoryMat = translateRectangleToAxis;
        block[0].cx = 0;
        block[0].cy = -2.8;
        block[0].cz = -3;
        block[0].state = 1;
        block[0].sx = 1;
        block[0].sy = 2;
        block[0].sz = 1;
        // block[1].memoryMat = translateRectangleToAxis;
        x1 = 7;
        ya = 3;
        x2 = 7;
        yb = 3;
      if(lives == -1)
        exit(0);
    }
  }
  else if(block[b].move == 1)
  {
    if(block[b].tempAngle != block[b].angle)
    {
      if(block[b].angle < 0)
      {
        block[b].tempAngle -= 1 * block[b].speed;
      }
      else
      {
        block[b].tempAngle += 1 * block[b].speed;
      }
    }
    else
    {
      if(p == 1)
      {
        if(block[b].state == 1)
        {
          block[b].sx = 1;
          block[b].sy = 2;
          block[b].sz = 1;
        }
        if(block[b].state == 2)
        {
          block[b].sx = 2;
          block[b].sy = 1;
          block[b].sz = 1;
        }
        if(block[b].state == 3)
        {
          block[b].sx = 1;
          block[b].sy = 1;
          block[b].sz = 2;
        }
      }
      else
      {
        block[b].sx = 1;
        block[b].sy = 1;
        block[b].sz = 1;
      }
      glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[b].cx,block[b].cy,block[b].cz));        // glTranslatef
      block[b].memoryMat = translateRectangleToAxis; 
      block[b].x = block[b].y = block[b].z = 0;
      block[b].angle = block[b].tempAngle = 0;
      block[b].move = 0;
      block[b].tx = block[b].ty = block[b].tz = 0;
      divY = 1;
      divX = 1;
      divZ = 1;
      checkTrigger(b);
      // cout << x1 << " " << ya << " " << x2 << " " << yb << endl;
      // ch = 0;
    }
  }
  else if(block[b].move == -1)
  {
    if(block[b].tempAngle != block[b].angle)
    {
      if(block[b].angle < 0)
      {
        block[b].tempAngle -= 1 * block[b].speed;
      }
      else
      {
        block[b].tempAngle += 1 * block[b].speed;
      }
    }
    else
    {
      if(p == 1)
      {
        if(block[b].state == 1)
        {
          block[b].sx = 1;
          block[b].sy = 2;
          block[b].sz = 1;
        }
        if(block[b].state == 2)
        {
          block[b].sx = 2;
          block[b].sy = 1;
          block[b].sz = 1;
        }
        if(block[b].state == 3)
        {
          block[b].sx = 1;
          block[b].sy = 1;
          block[b].sz = 2;
        }
      }
      else
      {
        block[b].sx = 1;
        block[b].sy = 1;
        block[b].sz = 1;
      }
      glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[b].cx,block[b].cy,block[b].cz));        // glTranslatef
      // glm::mat4 translateRectangleFromAxis = glm::translate(glm::vec3(-tx,-ty,-tz));        // glTranslatef
      block[b].memoryMat = translateRectangleToAxis; 
      
      block[b].angle = block[b].tempAngle = 0;
      block[b].x = block[b].y = block[b].z = 0;
      block[b].tx = block[b].ty = block[b].tz = 0;
      block[b].move = 0;
      divY = 1;
      divX = 1;
      divZ = 1;
      checkTrigger(b);
      // cout << x1 << " " << ya << " " << x2 << " " << yb << endl;
    }
    
  }
  else
  {
    block[b].tx = block[b].ty = block[b].tz = 0;
    block[b].x = 0;
    block[b].z = 0;
    block[b].y = 1;
  }
  if(splitz == 1)
  {
    for(int i = 0;i<p;i++)
    {
      block[i].sx = block[i].sy = block[i].sz = 1;
    }  
  }
  Matrices.model = block[b].memoryMat;
  glm::mat4 scaleCube = glm::scale(glm::vec3(block[b].sx,block[b].sy,block[b].sz));
  if(block[b].tempAngle != 0)
  {
      if(p == 2)
      {
        divX = divY = divZ = 1;
      }
      block[b].tx = float(X1[i]/divX);
      block[b].ty = float(Y1[i]/divY);
      block[b].tz = float(Z1[i]/divZ);
      glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[b].tx,block[b].ty,block[b].tz));        // glTranslatef
      glm::mat4 rotateRectangle = glm::rotate((float)(block[b].tempAngle*M_PI/180.0f), glm::vec3(block[b].x,block[b].y,block[b].z));
      glm::mat4 translateRectangleFromAxis = glm::translate(glm::vec3(-block[b].tx,-block[b].ty,-block[b].tz));        // glTranslatef
      Matrices.model *= translateRectangleToAxis * rotateRectangle * translateRectangleFromAxis;
      // memoryMat = Matrices.model;
  }
  // glm::mat4 translateRectangle = glm::translate(glm::vec3(0,-1,0));        // glTranslate
  Matrices.model *= scaleCube;

  if(floor_rel)
    Matrices.model = Matrices.model * glm::translate(floor_pos);
  if(doM)
    MVP *= VP * Matrices.model;
  else
    MVP *= VP;
  if(drop[b] == 1)
    kill[b] = 1;

  // prev *= MVP;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    // draw3DObject draws the VAO given to it using current MVP matrix
  // draw3DObject(rectangle);
  currColor = 5;
  createRectangle();
  draw3DObject(rectangle);
  createRectangleBorder();
  draw3DObject(rectangle);
}

void draw (GLFWwindow* window, float x, float y, float w, float h, int doM, int doV, int doP)
{
    int fbwidth, fbheight;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));


    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram(programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraDirection = glm::normalize(eye - target);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
   
    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

    glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

    
    GLfloat radius = 10.0f;    
    GLfloat camX = sin(glfwGetTime()) * radius;
    GLfloat camZ = cos(glfwGetTime()) * radius;
    glm::mat4 view;
    view = glm::lookAt(eye, eye + front, up); 
    Matrices.view = view; 

    // Compute Camera matrix (view)
    // if(doV)
	// Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane
    // else
	// Matrices.view = glm::mat4(1.0f);

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    glm::mat4 VP;
    if (doP)
	VP = Matrices.projection * Matrices.view;
    else
	VP = Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    glm::mat4 MVP;	// MVP = Projection * View * Model

    // Load identity to model matrix

 //Create cube  
    // Matrices.model = glm::mat4(1.0f);
    // glm::mat4 translateRectangle = glm::translate (glm::vec3(1, 0, 0));        // glTranslatef
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    // Matrices.model *= (translateRectangle * rotateRectangle);
    // MVP = VP * Matrices.model;
    // glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    // draw3DObject(rectangle);
    for(int i = 0;i<p;i++)
      drawBlock(VP,MVP,window,doM,i);

    // cout << "Hello" << endl;
    for(int j = 0 ; j<floorHeight ; j++)
    {
      for(int i = 0 ; i<floorWidth ; i++)
      {
        if(level == 1)
        {
          if(tileGrid1[j][i].type != 0)
          {
          Matrices.model = genModelMatrix(tileGrid1[j][i].T, floorRot , glm::vec3(1,0.5,1));
          MVP = VP * Matrices.model;
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          currColor = tileGrid1[j][i].type;
          createRectangle();
          draw3DObject(rectangle);
          createRectangleBorder();
          draw3DObject(rectangle);
          }
        }
        else
        {
          if(tileGrid2[j][i].type != 0)
          {
          Matrices.model = genModelMatrix(tileGrid2[j][i].T, floorRot , glm::vec3(1,0.5,1));
          MVP = VP * Matrices.model;
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          currColor = tileGrid2[j][i].type;
          createRectangle();
          draw3DObject(rectangle);
          createRectangleBorder();
          draw3DObject(rectangle);
          }
        }
      }
    }
    drawPrintScore(VP,MVP);
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Bloxorz", NULL, NULL);

    if (!window) {
	exit(EXIT_FAILURE);
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    createRectangle ();
    createCam();
    createFloor();
    createSevenSeg();
	
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.8f, 0.2f, 0.0f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
    int width = 1366;
    int height = 768;
    rect_pos = glm::vec3(0, 0, 0);
    floor_pos = glm::vec3(0, 0, 0);
    do_rot = 0;
    floor_rel = 1;

    floorPos = glm::vec3(-7, -4, 0);
    floor_pos = glm::vec3(0, 0, 0);

    //Audio
     mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

//    if(argc < 2)
  //      exit(0);

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = 3000;
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, "mario.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    // /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

    GLFWwindow* window = initGLFW(width, height);
    // initGLEW();
    initGL (window, width, height);

    last_update_time = glfwGetTime();

    //Level Design
    for(int i = 0; i < floorHeight; i++)
    {
      vector<tile> tempRow;
      for(int j = 0; j <floorWidth; j++)
      {
        tile tempTile;
        tempTile.type = lvlone[i][j];
        tempTile.T = floorPos + glm::vec3(j,0,-i);
        tempTile.S = glm::vec3(1,0.5,1);
        tempRow.push_back(tempTile);
      }
      tileGrid1.push_back(tempRow);
    }

    for(int i = 0; i < floorHeight; i++)
    {
      vector<tile> tempRow;
      for(int j = 0; j <floorWidth; j++)
      {
        tile tempTile;
        tempTile.type = lvltwo[i][j];
        tempTile.T = floorPos + glm::vec3(j,0,-i);
        tempTile.S = glm::vec3(1,0.5,1);
        tempRow.push_back(tempTile);
      }
      tileGrid2.push_back(tempRow);
    }

    //bridgeY.push_back(3);
    bridgeY.push_back(4);
    bridgeY.push_back(4);
    //bridgeX.push_back(10);
    bridgeX.push_back(12);
    bridgeX.push_back(13);

    pathY = 4;
    pathX = 8;

    fragY.push_back(3);
    fragX.push_back(10);

    goalX = 15;
    goalY = 6;


    block.push_back(initBlock(0,-2.8,-3,1,2,1));
    glm::mat4 translateRectangleToAxis = glm::translate(glm::vec3(block[0].cx,block[0].cy,block[0].cz));        // glTranslatef
    block[0].memoryMat = translateRectangleToAxis; 

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
    
         if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
            ao_play(dev, (char *)buffer, done);
        else 
            mpg123_seek(mh, 0, SEEK_SET); // loop audio from start again if ended


	// clear the color and depth in the frame buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // OpenGL Draw commands
	current_time = glfwGetTime();

	if(do_rot)
	    camera_rotation_angle += 90*(current_time - last_update_time); // Simulating camera rotation
	if(camera_rotation_angle > 720)
	    camera_rotation_angle -= 720;
	last_update_time = current_time;
     

	draw(window, 0, 0, 1, 1, 1, 1, 1);
    

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();
        
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;    
        do_movement ();
    }

    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

    glfwTerminate();
    return 0;
    
    //    exit(EXIT_SUCCESS);
}
