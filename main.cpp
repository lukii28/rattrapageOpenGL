#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective
#include <unistd.h>

#include "shader_helper.h"
#include "_assimp_loader/assimp_loader.h"
#include "skybox.h"

#define DEG2RAD 0.01745329251f
#define RAD2DEG(rad) ((rad)*180.0f / (float)M_PI)
#define PI 3.14159

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);



// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 25.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX;
float lastY;
float fov = 45.0f;

float growSpeed = 0.05f;
float longueur_Bras = 5.0f; //attention scale * 2 pour 2 la longueur du bras vaux 1 on va compenser dans le scale
float longueur_Cylindre = 11.0f;

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

AssimpLoaded ARM;
AssimpLoaded Edge;
AssimpLoaded Arbre;
AssimpLoaded Cube;
AssimpLoaded Cylinder;

glm::mat4x4 projection_mat;
glm::mat4x4 sphere_model_mat;

glm::mat4x4 RAIL_BOYD_MAT;
glm::mat4x4 RAIL_LE_MAT;
glm::mat4x4 RAIL_RE_MAT;

glm::mat4x4 BRAS_ARM_MAT;
glm::mat4x4 BRAS_LE_MAT;
glm::mat4x4 BRAS_RE_MAT;

glm::mat4x4 CYLINDRE_ARM_MAT;
glm::mat4x4 CYLINDRE_LE_MAT;
glm::mat4x4 CYLINDRE_RE_MAT;

glm::mat4x4 ARBRE_MAT;

GLfloat BRAS_Angle = 90.0f*DEG2RAD;

GLuint pid_shader_misc;
GLuint pid_shader_skybox;
GLuint pid_shader_cubemap;

GLfloat light_position[3];
GLfloat camera_position[3];
GLfloat spot_dir[3];
GLuint light_mode = 0;
bool activate_specular = true;
bool activate_spot = true;
uint light_animation;

GLFWwindow *window;
const GLFWvidmode *mode;

//SkyboxLoaded Skybox;

float rotationSpeed = 360;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

std::string path;

void calculateBielle()
{
   RAIL_BOYD_MAT = glm::translate(glm::mat4(1.0), glm::vec3((longueur_Bras + longueur_Cylindre)/2, 0.0f, 0.0f));
   RAIL_BOYD_MAT = glm::scale(RAIL_BOYD_MAT, glm::vec3((longueur_Bras + longueur_Cylindre), 1.0f, 1.0f));
   
   RAIL_LE_MAT = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
   RAIL_RE_MAT = glm::translate(glm::mat4(1.0), glm::vec3((longueur_Bras + longueur_Cylindre), 0.0f, 0.0f));
   RAIL_RE_MAT = glm::rotate(RAIL_RE_MAT, 180 * DEG2RAD, glm::vec3(0.0f, 0.0f, 1.0f));
}

int main()
{  
   char cwd[PATH_MAX];

   getcwd(cwd, sizeof(cwd));
   for(int i=0;i<PATH_MAX;i++)
   {
      if(cwd[i] == '\0'){cwd[i-6]='\0';break;}
   }

   path =std::string(cwd);

   if (!glfwInit())
   {
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

   //window = glfwCreateWindow(1280, 720, "bielle_manivelle_V0", NULL, NULL);
   window = glfwCreateWindow(mode->width, mode->height, "bielle_manivelle_V0", glfwGetPrimaryMonitor(), NULL);
   glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

   lastX = mode->width / 2.0;
   lastY = mode->height / 2.0;

   if (!window)
   {
      std::cout << "failed to open window" << std::endl;
      return -1;
   }
   // tell GLFW to capture our mouse
   glfwMakeContextCurrent(window);
   glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
   glfwSetCursorPosCallback(window, mouse_callback);

   glewExperimental = GL_TRUE;
   if (glewInit() != GLEW_NO_ERROR)
   {
      std::cout << "glew error\n";
      return -1;
   }
   activate_spot = false;
   init();

   // mainloop with input control
   while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window))
   {

      glfwPollEvents();

      if (glfwGetKey(window, '0') == GLFW_PRESS)
      {
         light_mode = 0;
      }
      else if (glfwGetKey(window, '1') == GLFW_PRESS)
      {
         light_mode = 1;
      }
      else if (glfwGetKey(window, '2') == GLFW_PRESS)
      {
         light_mode = 2;
      }

      if (glfwGetKey(window, '3') == GLFW_PRESS)
      {
         activate_specular = true;
      }
      else if (glfwGetKey(window, '4') == GLFW_PRESS)
      {
         activate_specular = false;
      }

      if (glfwGetKey(window, '5') == GLFW_PRESS)
      {
         activate_spot = true;
      }
      else if (glfwGetKey(window, '6') == GLFW_PRESS)
      {
         activate_spot = false;
      }

      if (glfwGetKey(window, '7') == GLFW_PRESS)
      {
         light_animation = 0; // side light
      }
      else if (glfwGetKey(window, '8') == GLFW_PRESS)
      {
         light_animation = 1; // rotating light
      }

      if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
      {
         longueur_Cylindre += growSpeed;
         calculateBielle();
      }

      if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
      {
         if(longueur_Cylindre - growSpeed>2*longueur_Bras+0.75f)longueur_Cylindre -= growSpeed;
         calculateBielle();
      }

      if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
      {
         longueur_Bras += growSpeed;
         calculateBielle();
      }

      if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
      {
         longueur_Bras -= growSpeed;
         calculateBielle();
      }

      if (glfwGetKey(window, 'E') == GLFW_PRESS)
      {
         BRAS_Angle += (2 * PI) / rotationSpeed;
         if(fmod(BRAS_Angle,PI)>PI-(2 * PI) / rotationSpeed && fmod(BRAS_Angle,PI)<PI+(2 * PI) / rotationSpeed )BRAS_Angle += (2 * PI) / rotationSpeed;
       
      }

      if (glfwGetKey(window, 'R') == GLFW_PRESS)
      {
         if(rotationSpeed-1>=10)rotationSpeed-=1;
       
      }

      if (glfwGetKey(window, 'F') == GLFW_PRESS)
      {
         rotationSpeed+=1;
       
      }

      display();
      glfwSwapBuffers(window);
   }

   cleanup();

   return 0;
}

void init()
{
   glEnable(GL_DEPTH_TEST);

   glEnable(GL_DEPTH_CLAMP);
   glDepthFunc(GL_LESS);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glViewport(0, 0, mode->width, mode->height);
   projection_mat = glm::perspective(45.0f, (float)mode->width / (float)mode->height, 0.01f, 1000.0f);

   light_animation = 0;

   light_position[0] = 0.0; // x
   light_position[1] = 0.0; // up
   light_position[2] = 5.0; // z

   pid_shader_misc = load_shaders((path+std::string("/glsl/base_vshader.glsl")).c_str(), (path+std::string("/glsl/base_fshader.glsl")).c_str());
   pid_shader_skybox = load_shaders((path+std::string("/glsl/skybox_vshader.glsl")).c_str(), (path+std::string("/glsl/skybox_fshader.glsl")).c_str());
   pid_shader_cubemap = load_shaders((path+std::string("/glsl/cubemap_vshader.glsl")).c_str(), (path+std::string("/glsl/cubemap_fshader.glsl")).c_str());

   ARM.init(pid_shader_misc, (path+std::string("/3d/arm.obj")).c_str());
   Edge.init(pid_shader_misc, (path+std::string("/3d/edges.obj")).c_str());
   Arbre.init(pid_shader_misc, (path+std::string("/3d/arbre.obj")).c_str());
   Cube.init(pid_shader_misc, (path+std::string("/3d/cube.obj")).c_str());
   Cylinder.init(pid_shader_misc, (path+std::string("/3d/cylinder.obj")).c_str());
   //Skybox.init(pid_shader_skybox, pid_shader_cubemap);
   Cylinder.set_color(1.0f, 1.0f, 1.0f);
   Cube.set_color(1.0f, 1.0f, 1.0f);
   sphere_model_mat = glm::scale(sphere_model_mat, glm::vec3(0.1, 0.1, 0.1));

   calculateBielle();
}

void display()
{

   //---------------------CAMERA---------------------//
   glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
   // per-frame time logic
   // --------------------
   float currentFrame = static_cast<float>(glfwGetTime());
   deltaTime = currentFrame - lastFrame;
   lastFrame = currentFrame;

   // input GLFW
   // -----
   processInput(window);

   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //---------------------BRAS---------------------// dans la loop car non statique
   BRAS_ARM_MAT = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.5f));
   BRAS_ARM_MAT = glm::rotate(BRAS_ARM_MAT, BRAS_Angle, glm::vec3(0.0f, 0.0f, 1.0f));
   BRAS_ARM_MAT = glm::scale(BRAS_ARM_MAT, glm::vec3(longueur_Bras*2, 1.0f, 1.0f)); //on multiplie par 2 pour equilibrer le scale

   //---------------------CALCUL ANGLE ET POSITIONS---------------------//
   // Angle bras
   float RAIL_Angle=asinf((longueur_Bras*sinf(BRAS_Angle))/longueur_Cylindre);
   float CYLINDRE_Angle=PI-RAIL_Angle-BRAS_Angle;
   float TREE_Distance = longueur_Cylindre*(sinf(CYLINDRE_Angle)/sinf(BRAS_Angle));

   ARBRE_MAT = glm::translate(glm::mat4(1.0), glm::vec3(TREE_Distance, 0.0f, 0.25f));
   

   CYLINDRE_LE_MAT = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 1.0f));
   CYLINDRE_LE_MAT = glm::rotate(CYLINDRE_LE_MAT, BRAS_Angle, glm::vec3(0.0f, 0.0f, 1.0f));
   CYLINDRE_LE_MAT = glm::translate(CYLINDRE_LE_MAT, glm::vec3(longueur_Bras, 0.0f, 0.0f));
   CYLINDRE_LE_MAT = glm::rotate(CYLINDRE_LE_MAT, CYLINDRE_Angle, glm::vec3(0.0f, 0.0f, 1.0f));

   CYLINDRE_RE_MAT = glm::translate(glm::mat4(1.0), glm::vec3(TREE_Distance, 0.0f, 1.0f));
   CYLINDRE_RE_MAT = glm::rotate(CYLINDRE_RE_MAT, RAIL_Angle, glm::vec3(0.0f, 0.0f, -1.0f));

   glm::vec3 middlePos = (glm::vec3(CYLINDRE_LE_MAT[3]) + glm::vec3(ARBRE_MAT[3])) * 0.5f;
   CYLINDRE_ARM_MAT = glm::translate(glm::mat4(1.0), glm::vec3(middlePos[0],middlePos[1],1.0f));
   CYLINDRE_ARM_MAT = glm::rotate(CYLINDRE_ARM_MAT, RAIL_Angle, glm::vec3(0.0f, 0.0f, -1.0f));
   CYLINDRE_ARM_MAT = glm::scale(CYLINDRE_ARM_MAT, glm::vec3(longueur_Cylindre, 1.0f, 1.0f));


   // move light up and down with time make the coord y of light go from 0 to 10
   if (light_animation == 0)
   {
      light_position[0] = 0.0;
      light_position[1] = 10.0 - fabs(10.0 - fmod(2 * glfwGetTime(), 20.0));
      light_position[2] = 5.0;

      // have the spot directed vaguely to the sphere
      spot_dir[0] = 0.0;
      spot_dir[1] = -1.0 / 1.4142;
      spot_dir[2] = -1.0 / 1.4142;
   }
   else
   {
      // circle rotating above the sphere
      light_position[0] = 2 * sin(glfwGetTime() * 0.5);
      light_position[1] = 3.0;
      light_position[2] = 2 * cos(glfwGetTime() * 0.5);

      // spot centered on the sphere (0,0)
      // will be normalized by the shader
      // division on x and z is to off-center the spot (more natural)
      spot_dir[0] = -light_position[0] / 1.2;
      spot_dir[1] = -light_position[1];
      spot_dir[2] = -light_position[2] / 1.2;
   }

   // update positions and options to shaders
   glUseProgram(pid_shader_misc);
   glUniform3fv(glGetUniformLocation(pid_shader_misc, "light_position"), 1, light_position);
   glUniform3fv(glGetUniformLocation(pid_shader_misc, "camera_position"), 1, camera_position);
   glUniform1ui(glGetUniformLocation(pid_shader_misc, "lighting_mode"), light_mode);
   glUniform1ui(glGetUniformLocation(pid_shader_misc, "activate_specular"), activate_specular);
   glUniform3fv(glGetUniformLocation(pid_shader_misc, "spot_direction"), 1, spot_dir);
   glUniform1ui(glGetUniformLocation(pid_shader_misc, "activate_spot"), activate_spot);
   glUseProgram(0);

   //---------------------DRAW---------------------//
   // rail:
   ARM.set_color(0.8f, 0.0f, 0.0f);
   Edge.set_color(0.8f, 0.0f, 0.0f);
   ARM.draw(RAIL_BOYD_MAT, view, projection_mat);
   Edge.draw(RAIL_LE_MAT, view, projection_mat);
   Edge.draw(RAIL_RE_MAT, view, projection_mat);

   // bras:
   ARM.set_color(0.8f, 0.8f, 0.0f);
   ARM.draw(BRAS_ARM_MAT, view, projection_mat);

   // cube(bout de cylindre):
   Cube.draw(CYLINDRE_LE_MAT, view, projection_mat);
   Cube.draw(CYLINDRE_RE_MAT, view, projection_mat);
   

   // arbre:
   Arbre.set_color(0.8f, 0.8f, 0.0f);
   Arbre.draw(ARBRE_MAT, view, projection_mat);
   

   Cylinder.draw(CYLINDRE_ARM_MAT, view, projection_mat);

   // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        //Skybox.draw(view, projection_mat);
        glDepthFunc(GL_LESS); // set depth function back to default
}

void cleanup()
{
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

   float cameraSpeed = static_cast<float>(25 * deltaTime);
   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      cameraPos += cameraSpeed * cameraFront;
   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      cameraPos -= cameraSpeed * cameraFront;
   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
   // make sure the viewport matches the new window dimensions; note that width and
   // height will be significantly larger than specified on retina displays.
   glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
   float xpos = static_cast<float>(xposIn);
   float ypos = static_cast<float>(yposIn);

   if (firstMouse)
   {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
   }

   float xoffset = xpos - lastX;
   float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
   lastX = xpos;
   lastY = ypos;

   float sensitivity = 0.1f; // change this value to your liking
   xoffset *= sensitivity;
   yoffset *= sensitivity;

   yaw += xoffset;
   pitch += yoffset;

   // make sure that when pitch is out of bounds, screen doesn't get flipped
   if (pitch > 89.0f)
      pitch = 89.0f;
   if (pitch < -89.0f)
      pitch = -89.0f;

   glm::vec3 front;
   front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
   front.y = sin(glm::radians(pitch));
   front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
   cameraFront = glm::normalize(front);
}
