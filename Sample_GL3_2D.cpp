#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

GLuint programID;

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
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

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

float gravity = 0.5,airDrag = 0.1,friction = 0.1,t=0;
float camera_rotation_angle = 90;
class Bird{
	int lives;
	float posx;
	float posy;
	float vel;
	float theta;
	bool isMoving; 
	public:
	VAO *bird;
	Bird(){
		lives = 3;
		isMoving = false;
		vel = 1.2;
		theta = 45;
		posx = 0;
		posy = 0;
	}

	float getX(){
		return posx;
	}
	float getY(){
		return posy;
	}
	float getVel(){
		return vel;
	}
	float getAngle(){
		return theta;
	}
	float getStatus(){
		return isMoving;
	}

	void setStatus(bool st){
		isMoving = st;
	}

	void setX(float x){
		posx = x;
	}
	void setY(float y){
		posy = y;
	}
	void setVel(float v){
		vel = v;
	}
	void setAngle(float ang){
		theta = ang;
	}

	void create(){
		// GL3 accepts only Triangles. Quads are not supported
		static const GLfloat vertex_buffer_data [] = {
			0.3,0,0, // vertex 1
			0,0.2,0, // vertex 2
			0,-0.2,0, // vertex 3

			//0.6, 0.6,0, // vertex 3
			//-0.6, 0.6,0, // vertex 4
			//-0.6,-0.6,0  // vertex 1
		};

		static const GLfloat color_buffer_data [] = {
			1,1,0, // color 1
			1,1,0, // color 2
			1,1,0, // color 3

			//1,1,0.4, // color 3
			//0.5,0.5,0.5, // color 4
			//1,1,0  // color 1
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		bird = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	void draw()

	{
		// clear the color and depth in the frame buffer
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Don't change unless you know what you are doing
		glUseProgram (programID);

		// Eye - Location of camera. Don't change unless you are sure!!
		glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 1, 0);

		//  Don't change unless you are sure!!
		Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

		//  Don't change unless you are sure!!
		glm::mat4 VP = Matrices.projection * Matrices.view;

		glm::mat4 MVP;	// MVP = Projection * View * Model

		Matrices.model = glm::mat4(1.0f);


		// Render your scene 

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateSquare = glm::translate (glm::vec3(-2, 0, 0));        // glTranslatef
		glm::mat4 moveSquare = glm::translate(glm::vec3((float)(-2.0f+posx),float(0.0f+posy),0.0f)); 
		Matrices.model *= (translateSquare * moveSquare);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(bird);
		if(isMoving){
			posx = vel*cos(theta*M_PI/180.0f)*t - (0.5*airDrag*cos(theta*M_PI/180.0f)*t*t);
			posy = vel*sin(theta*M_PI/180.0f)*t - (0.5*(gravity+(airDrag*sin(theta*M_PI/180.0f)))*t*t);
		}
	}
};
Bird angryBird;

class Point{
	float posx;
	float posy;
	public:
	VAO *point;
	Point(){
		posx = 0;
		posy = 0;
	}

	float getX(){
		return posx;
	}
	float getY(){
		return posy;
	}
	void setX(int x){
		posx = x;
	}
	void setY(int y){
		posy = y;
	}

	void create()
	{
		static const GLfloat vertex_buffer_data [] = {
			-2,0,0, // vertex 1
			-2,0.03,0, // vertex 2
			-2.08,0.03,0, // vertex 3

			-2.08,0.03,0, // vertex 3
			-2.08,0,0, // vertex 4
			-2,0,0, // vertex 1
		};

		static const GLfloat color_buffer_data [] = {
			1,1,1, // color 1
			1,1,1, // color 2
			1,1,1, // color 3

			1,1,1, // color 3
			1,1,1, // color 4
			1,1,1, // color 1
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		point = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	}
	void draw (float time)
	{
		// clear the color and depth in the frame buffer
		//glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram (programID);

		glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 1, 0);

		Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

		glm::mat4 VP = Matrices.projection * Matrices.view;
		glm::mat4 MVP;	// MVP = Projection * View * Model
		Matrices.model = glm::mat4(1.0f);

		// Render your scene 
		Matrices.model = glm::mat4(1.0f);

		time/=2;
		//posx = angryBird.getVel()*cos(angryBird.getAngle()*M_PI/180.0f)*time;
		//posy = angryBird.getVel()*sin(angryBird.getAngle()*M_PI/180.0f)*time - (0.5*gravity*time*time);

			posx = angryBird.getVel()*cos(angryBird.getAngle()*M_PI/180.0f)*time - (0.5*airDrag*cos(angryBird.getAngle()*M_PI/180.0f)*time*time);
			posy = angryBird.getVel()*sin(angryBird.getAngle()*M_PI/180.0f)*time - (0.5*(gravity+(airDrag*sin(angryBird.getAngle()*M_PI/180.0f)))*time*time);
		glm::mat4 translateInit = glm::translate (glm::vec3(-1.0, 0, 0));     

		glm::mat4 translatePoint = glm::translate(glm::vec3((float)(-1.0f + posx),(float)(posy), 0.0f));        
		Matrices.model *= (translateInit*translatePoint);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(point);

	}
};

Point path[20];
/**************************
 * Customizable functions *
 **************************/

/*float square_rot_dir = 1;
  float square_dir = 1;
  bool square_rot_status = false;*/
//bool start_movement = false;
//float theta = 45,v=1.2;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_W:
				angryBird.setVel(angryBird.getVel()+0.2); 
				break;
			case GLFW_KEY_S:
				angryBird.setVel(angryBird.getVel()-0.2); 
				break;
			case GLFW_KEY_A:
				angryBird.setAngle(angryBird.getAngle()-5); 
				break;
			case GLFW_KEY_D:
				angryBird.setAngle(angryBird.getAngle()+5); 
				break;
			case GLFW_KEY_SPACE:
				angryBird.setStatus(!angryBird.getStatus()); 
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;
		}
	}
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
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				//triangle_rot_dir *= -1;
				break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				//rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

//VAO *bird, *point[20];


/*

   void createPoint(int i)
   {
// GL3 accepts only Triangles. Quads are not supported
static const GLfloat vertex_buffer_data [] = {
-2,0,0, // vertex 1
-2,0.03,0, // vertex 2
-2.08,0.03,0, // vertex 3

-2.08,0.03,0, // vertex 3
-2.08,0,0, // vertex 4
-2,0,0, // vertex 1
};

static const GLfloat color_buffer_data [] = {
1,1,1, // color 1
1,1,1, // color 2
1,1,1, // color 3

1,1,1, // color 3
1,1,1, // color 4
1,1,1, // color 1
};

// create3DObject creates and returns a handle to a VAO that can be used later
point[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float acc = 0.05;
float square_rotation = 0;
float square_movement = 0;
float p_x=0,p_y=0,t=0,g=0.5;

// Render the scene with openGL 
// Edit this function according to your assignment 
void draw ()
{
// clear the color and depth in the frame buffer
glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// use the loaded shader program
// Don't change unless you know what you are doing
glUseProgram (programID);

// Eye - Location of camera. Don't change unless you are sure!!
glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
// Target - Where is the camera looking at.  Don't change unless you are sure!!
glm::vec3 target (0, 0, 0);
// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
glm::vec3 up (0, 1, 0);

// Compute Camera matrix (view)
// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
//  Don't change unless you are sure!!
Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
//  Don't change unless you are sure!!
glm::mat4 VP = Matrices.projection * Matrices.view;

// Send our transformation to the currently bound shader, in the "MVP" uniform
// For each model you render, since the MVP will be different (at least the M part)
//  Don't change unless you are sure!!
glm::mat4 MVP;	// MVP = Projection * View * Model

// Load identity to model matrix
Matrices.model = glm::mat4(1.0f);


// Render your scene 

Matrices.model = glm::mat4(1.0f);

glm::mat4 translateSquare = glm::translate (glm::vec3(-2, 0, 0));        // glTranslatef
glm::mat4 moveSquare = glm::translate(glm::vec3((float)(-2.0f+p_x),float(0.0f+p_y),0.0f)); 
Matrices.model *= (translateSquare * moveSquare);
MVP = VP * Matrices.model;
glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

// draw3DObject draws the VAO given to it using current MVP matrix
draw3DObject(square);
if(start_movement){
	p_x = v*cos(theta*M_PI/180.0f)*t;
	p_y = v*sin(theta*M_PI/180.0f)*t - (0.5*g*t*t);
}

//-------------- End of Projectile --------------------
int i = 0;
float rect_x=0,rect_y=0,time;
for(i=1;i<15;i++){
	Matrices.model = glm::mat4(1.0f);

	time = 0.5*i;
	rect_x = v*cos(theta*M_PI/180.0f)*time;
	rect_y = v*sin(theta*M_PI/180.0f)*time - (0.5*g*time*time);

	glm::mat4 translateInit = glm::translate (glm::vec3(-1.0, 0, 0));     

	glm::mat4 translatePoint = glm::translate(glm::vec3((float)(-1.0f + rect_x),(float)(rect_y), 0.0f));        
	Matrices.model *= (translateInit*translatePoint);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(point[i]);
}

//------------End of path code--------------------------
}
*/
/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	int i =0;
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//createRectangle ();
	angryBird.create ();
	for(i=1;i<20;i++){
		path[i].create();
	}
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
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
	int width = 600;
	int height = 600;
	int i;
	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		angryBird.draw();
		//path.draw();
		for(i=1;i<20;i++){
			path[i].draw(i);
		}

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.025) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			if(angryBird.getStatus())
				t+=0.025;
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
