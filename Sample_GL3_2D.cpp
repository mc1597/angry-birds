#include<iostream>
#include <cmath>
#include <cstdlib>
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
	//printf("Compiling shader : %s\n", vertex_file_path);
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
	//printf("Compiling shader : %s\n", fragment_file_path);
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
	//fprintf(stdout, "Linking program\n");
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

float gravity = 0.5,airDrag = 0.01,friction = 0.1,t=0,groundDrag = 0.5;
float camera_rotation_angle = 90;

class Border{

	char dir;
	public:
	VAO *bor[4];
	Border(){
		dir = 'I';
	}	
	char getDir(){
		return dir;
	}
	void setDir(char d){
		dir = d;
	}
	void create(int orient)
	{

		if(orient == 0 || orient == 2){
			static const GLfloat vertex_buffer_data [] = {
				0,4,0, // vertex 1
				0.25,4,0, // vertex 2
				0.25,-4,0, // vertex 3

				0.25,-4,0, // vertex 3
				0,-4,0, // vertex 4
				0,4,0, // vertex 1
			};

			static const GLfloat color_buffer_data [] = {
				0.5,0.35,0.05, // color 1
				0.5,0.35,0.05, // color 1
				0.5,0.35,0.05, // color 1

				1,0.5,0, // color 1
				1,0.5,0, // color 1
				1,0.5,0, // color 1
			};

			bor[orient] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

		}
		else{
			static const GLfloat vertex_buffer_data [] = {
				4,0,0, // vertex 1
				4,0.25,0, // vertex 2
				-4,0.25,0, // vertex 3

				-4,0.25,0, // vertex 3
				-4,0,0, // vertex 4
				4,0,0, // vertex 1
			};

			static const GLfloat color_buffer_data [] = {
				1,0.5,0, // color 1
				1,0.5,0, // color 1
				1,0.5,0, // color 1

				0.5,0.35,0.05, // color 1
				0.5,0.35,0.05, // color 1
				0.5,0.35,0.05, // color 1

			};

			bor[orient] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

		}
	}

	void draw(int orient){
		glUseProgram (programID);

		glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 1, 0);

		Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

		glm::mat4 VP = Matrices.projection * Matrices.view;
		glm::mat4 MVP;  // MVP = Projection * View * Model
		Matrices.model = glm::mat4(1.0f);

		Matrices.model = glm::mat4(1.0f);
		if(orient == 0){
			glm::mat4 translateBor0 = glm::translate (glm::vec3(3.75, 0, 0));
			Matrices.model *= (translateBor0);
		}
		else if(orient == 2){
			glm::mat4 translateBor2 = glm::translate (glm::vec3(-4, 0, 0));
			Matrices.model *= (translateBor2);
		}
		else if(orient == 1){
			glm::mat4 translateBor1 = glm::translate (glm::vec3(0, 3.75, 0));
			Matrices.model *= (translateBor1);
		}
		else if(orient == 3){
			glm::mat4 translateBor3 = glm::translate (glm::vec3(0, -4, 0));
			Matrices.model *= (translateBor3);
		}
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(bor[orient]);

	}


};

Border border[4];

class Target{
	float posx;
	float posy;
	float vel;
	float theta;
	float marks;
	float center[2];
	float radius;
	bool collided;
	public:
	VAO *tar;
	Target(){
		posx = 2;
		posy = 2;
		vel = 0;
		theta = 0;
		marks = 5;
		center[0] = 2;
		center[1] = 2;
		radius = 0.25;
		collided = false;
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
	float getMarks(){
		return marks;
	}
	float* getCenter(){
		return center;
	}
	float getRadius(){
		return radius;
	}
	bool getCollided(){
		return collided;
	}
	void setCollided(bool col){
		collided = col;
	}
	void setMarks(int m){
		marks = m;
	}
	void setCenter(float cx,float cy){
		center[0] = cx;
		center[1] = cy;
	}
	void setRadius(float r){
		radius = r;
	}

	void create()
	{

		int numVertices = 360;
		GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			vertex_buffer_data [3*i] = radius*cos(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 1] = radius*sin(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 2] = 0;
		}


		GLfloat* color_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			color_buffer_data [3*i] = 0;
			color_buffer_data [3*i + 1] = 1;
			color_buffer_data [3*i + 2] = 0;
		}


		// create3DObject creates and returns a handle to a VAO that can be used later
		tar = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	void draw(){
		glUseProgram (programID);

		glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 1, 0);

		Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

		glm::mat4 VP = Matrices.projection * Matrices.view;
		glm::mat4 MVP;  // MVP = Projection * View * Model
		Matrices.model = glm::mat4(1.0f);

		Matrices.model = glm::mat4(1.0f);
		
		glm::mat4 translateTar = glm::translate (glm::vec3(posx, posy, 0));
		Matrices.model *= (translateTar);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(tar);

	}

};

Target target[7];

float obstacle_rotation = 0;
class Obstacle{
	float posx;
	float posy;
	float marks;
	float center[2];
	float radius;
	bool collided;
	public:
	VAO *obs;
	Obstacle(){
		posx = 2;
		posy = 2;
		marks = 5;
		center[0] = posx;
		center[1] = posy;
		radius = 0.25;
		collided = false;
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
	float getMarks(){
		return marks;
	}
	float* getCenter(){
		return center;
	}
	float getRadius(){
		return radius;
	}
	bool getCollided(){
		return collided;
	}
	void setCollided(bool col){
		collided = col;
	}
	void setMarks(int m){
		marks = m;
	}
	void setCenter(float cx,float cy){
		center[0] = cx;
		center[1] = cy;
	}
	void setRadius(float r){
		radius = r;
	}

	void create()
	{

		int numVertices = 72;
		GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			vertex_buffer_data [3*i] = radius*cos((72*i)*M_PI/180.0f);
			vertex_buffer_data [3*i + 1] = radius*sin((72*i)*M_PI/180.0f);
			vertex_buffer_data [3*i + 2] = 0;
		}


		GLfloat* color_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			color_buffer_data [3*i] = 1;
			color_buffer_data [3*i + 1] = 0;
			color_buffer_data [3*i + 2] = 0;
		}


		// create3DObject creates and returns a handle to a VAO that can be used later
		obs = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	void draw(){
		glUseProgram (programID);

		glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 1, 0);

		Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

		glm::mat4 VP = Matrices.projection * Matrices.view;
		glm::mat4 MVP;  // MVP = Projection * View * Model
		Matrices.model = glm::mat4(1.0f);

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateObs = glm::translate (glm::vec3(posx, posy, 0));
		glm::mat4 rotateObs = glm::rotate((float)(obstacle_rotation*M_PI/180.0f), glm::vec3(0,0,1));
		Matrices.model *= (translateObs*rotateObs);
		obstacle_rotation+=0.5;
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(obs);

	}

};

Obstacle obstacle[7];
class Bird{
	int lives;
	int score;
	float posx;
	float posy;
	float vel;
	float theta;
	bool isMoving; 
	float radius;
	public:
	float center[2];
	float initX;
	float initY;
	float absy;
	float absx;
	float store;
	float newv;
	bool floor;
	bool flag;
	int dir;
	VAO *bird;
	Bird(){
		lives = 3;
		score = 0;
		isMoving = false;
		vel = 1.2;
		theta = 45;
		posx = 0;
		posy = 0;
		radius = 0.12;
		initX = -3.5;
		initY = 0;
		center[0] = 0.05 + initX;
		center[1] = 0 + initY; 
		absy = 0;
		absx = 4;
		store = 0;
		floor = false;
		flag = true;
		dir = 1;
	}

	float getLives(){
		return lives;
	}
	float getScore(){
		return score;
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
	float *getCenter(){
		return center;
	}
	float getRadius(){
		return radius;
	}

	void setStatus(bool st){
		isMoving = st;
	}

	void setLives(float l){
		lives = l;
	}
	void setScore(float s){
		score = s;
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
	void setCenter(float cx,float cy){
		center[0] = cx;
		center[1] = cy;
	}
	void setRadius(float r){
		radius = r;
	}

	void reset(){
		initX = -3.5;
		initY = 0;
		posx = 0;
		posy = 0;
		center[0] = 0.05 + initX; 
		center[1] = 0 + initY;
		theta = 45;
		vel = 1.2;
		lives--;
		isMoving = !isMoving;
		floor = false;
		flag = true;
		dir = 1;
	}
	void create(){
		static const GLfloat vertex_buffer_data [] = {
			0.17,0,0, // vertex 1
			0,0.1,0, // vertex 2
			0,-0.1,0, // vertex 3

		};

		static const GLfloat color_buffer_data [] = {
			1,1,0, // color 1
			1,1,0, // color 2
			1,1,0, // color 3

		};

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

		//glm::mat4 translateSquare = glm::translate (glm::vec3(initX, initY, 0));        // glTranslatef
		glm::mat4 moveSquare = glm::translate(glm::vec3((float)(initX+posx),float(initY+posy),0.0f)); 
		Matrices.model *= (moveSquare);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		draw3DObject(bird);
		if(floor){
			if(flag){	
				vel = vel - groundDrag*t;
				posx = posx + dir*(vel*t - 0.5*groundDrag*t*t);
				center[0] = initX + posx + (0.17/3);
			}
			if(vel <= 0 || center[0] > 3.25 || center[0] < -3.75){
				flag = false;
				//floor = false;
				reset();
			}
		}
		else if(isMoving){
			float vel_old,theta_old,vel_new,theta_new;

			posx = vel*cos(theta*M_PI/180.0f)*t - 0.5*airDrag*t*t;
			posy = vel*sin(theta*M_PI/180.0f)*t - (0.5*(gravity+(airDrag*sin(theta*M_PI/180.0f)))*t*t);
			center[0] = initX + posx + (0.17/3);
			center[1] = initY + posy;

		}
	}

	bool checkCollision(int index){
		int cx,cy;
		float r;
		cx = target[index].getCenter()[0];
		cy = target[index].getCenter()[1];
		r = target[index].getRadius();
		if(sqrt(pow((center[0]-cx),2)+pow((center[1]-cy),2))<=(radius + target[index].getRadius())){
			target[index].setCollided(true);
			setScore(getScore() + (int)((1/r)*10 + target[index].getX()*10));	
			//cout << "Score: " << getScore() << endl;
			return true;
		}
		else{
			return false;	
		}
	}
	void checkObstacle(int i){
		int cx,cy;
		float theta_old,vel_old,vel_new,theta_new,alpha=0.8;
		cx = obstacle[i].getCenter()[0];
		cy = obstacle[i].getCenter()[1];
		if(sqrt(pow((center[0]-cx),2)+pow((center[1]-cy),2))<=(radius + obstacle[i].getRadius())){
			if(!obstacle[i].getCollided()){
				initX = initX + posx;
				initY = initY + posy;
				theta_old = theta;
				vel_old = vel;

				vel_new = sqrt(pow(alpha*vel_old*cos(theta_old*M_PI/180.0f),2)+pow(vel_old*sin(theta_old*M_PI/180.0f),2));

				if(dir == 1 && t== 0){	
					theta_new = 180 - atan(sin(theta_old*M_PI/180.0f)/(alpha*cos(theta*M_PI/180.0f)));
					dir = -1;
					obstacle[i].setCollided(true);
				}
				else if(dir == -1 && t== 0){
					theta_new = atan(sin(theta_old*M_PI/180.0f)/(alpha*cos(theta*M_PI/180.0f)));
					dir = 1;
					obstacle[i].setCollided(true);
				}		
				t=0;
				vel = vel_new;
				theta = theta_new;
			}

		}
		else
			obstacle[i].setCollided(false);

	}

	void checkRoof(){
		float theta_old,vel_old,vel_new,theta_new,alpha=0.8;
		if(center[1] > 3.6){
			initX = initX + posx;
			initY = initY + posy;
			t=0;
			theta_old = theta;
			vel_old = vel;
			vel_new = sqrt(((alpha*vel_old*cos(theta_old*M_PI/180.0f))*(alpha*vel_old*cos(theta_old*M_PI/180.0f)))+(vel_old*sin(theta_old*M_PI/180.0f)*(vel_old*sin(theta_old*M_PI/180.0f))));
			theta_new = -1*(atan(sin(theta_old*M_PI/180.0f)/(alpha*cos(theta*M_PI/180.0f))));
			vel = vel_new;
			theta = theta_new;
			//cout << "roof " << theta_old << " " << theta_new << endl;
		}
	}
	void checkWall(){
		float theta_old,vel_old,vel_new,theta_new,alpha=0.8;
		if(center[0] > 3.65){
			initX = initX + posx;
			initY = initY + posy;
			t=0;
			theta_old = theta;
			vel_old = vel;
			vel_new = sqrt(((alpha*vel_old*cos(theta_old*M_PI/180.0f))*(alpha*vel_old*cos(theta_old*M_PI/180.0f)))+(vel_old*sin(theta_old*M_PI/180.0f)*(vel_old*sin(theta_old*M_PI/180.0f))));
			theta_new = 180 - atan(sin(theta_old*M_PI/180.0f)/(alpha*cos(theta*M_PI/180.0f)));
			vel = vel_new;
			theta = theta_new;
			if(theta < 90)
				dir = 1;
			else
				dir = -1;
			//cout << "theta: " << theta << endl;

		}
		else if(center[0] < -3.65){
			initX = initX + posx;
			initY = initY + posy;
			t=0;
			theta_old = theta;
			vel_old = vel;
			vel_new = sqrt(((alpha*vel_old*cos(theta_old*M_PI/180.0f))*(alpha*vel_old*cos(theta_old*M_PI/180.0f)))+(vel_old*sin(theta_old*M_PI/180.0f)*(vel_old*sin(theta_old*M_PI/180.0f))));
			theta_new = atan(sin(theta_old*M_PI/180.0f)/(alpha*cos(theta*M_PI/180.0f)));
			vel = vel_new;
			theta = theta_new;
			if(theta < 90)
				dir = 1;
			else
				dir = -1;

			//cout << "theta: " << theta << endl;
		}

	}

	void checkFloor(){
		float theta_old,vel_old,vel_new,theta_new,alpha=0.8;
		if(center[1] <= -3.6&&!floor){
			theta_old = theta;
			vel_old = vel;
			//cout << vel << endl;
			vel_new = sqrt(((alpha*vel_old*cos(theta_old*M_PI/180.0f))*(alpha*vel_old*cos(theta_old*M_PI/180.0f)))+(vel_old*sin(theta_old*M_PI/180.0f)*(vel_old*sin(theta_old*M_PI/180.0f))));
			theta_new = atan(sin(theta_old*M_PI/180.0f)/(alpha*cos(theta*M_PI/180.0f)));
			vel = vel_new;	
			theta = theta_new;
			t = 0;
			vel = vel*cos(theta*M_PI/180.0f);
			//cout << vel << endl;
			floor = true;	
		}
	}

};
Bird angryBird;

class Point{
	float posx;
	float posy;
	public:
	float initX;
	float initY;
	VAO *point;
	Point(){
		posx = 0;
		posy = 0;
		initX = 0;
		initY = 0;
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
			0,0,0, // vertex 1
			0,0.03,0, // vertex 2
			0.08,0.03,0, // vertex 3

			0.08,0.03,0, // vertex 3
			0.08,0,0, // vertex 4
			0,0,0, // vertex 1
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

		posx = angryBird.getVel()*cos(angryBird.getAngle()*M_PI/180.0f)*time - (0.5*airDrag*cos(angryBird.getAngle()*M_PI/180.0f)*time*time);
		posy = angryBird.getVel()*sin(angryBird.getAngle()*M_PI/180.0f)*time - (0.5*(gravity+(airDrag*sin(angryBird.getAngle()*M_PI/180.0f)))*time*time);

		//glm::mat4 translateInit = glm::translate (glm::vec3(0, 0, 0));     
		initX = angryBird.initX;
		initY = angryBird.initY;
		glm::mat4 translatePoint = glm::translate(glm::vec3((float)(initX + posx),(float)(initY + posy), 0.0f));        
		Matrices.model *= (translatePoint);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(point);

	}
};

Point path[20];

/**************************
 * Customizable functions *
 **************************/

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE && !angryBird.getStatus()) {
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
			case GLFW_KEY_UP:
				if(angryBird.initY < 3.5)
					angryBird.initY += 0.2; 
				angryBird.center[1] = angryBird.initY;			
				break;
			case GLFW_KEY_DOWN:
				if(angryBird.initY > -3.5)
					angryBird.initY -= 0.2; 
				angryBird.center[1] = angryBird.initY;			
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

GLfloat fov=89.8f;
void scroll(GLFWwindow* window,double x,double y){

        if(fov<89)
                fov=89;
        if(fov>91)
                fov=91;
        if(fov>=89&&fov<=91)
                fov+=y*0.1;
        cout << fov << " " << y << endl;


}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	//GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	   /*glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); 
	*/
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	//Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

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
	glfwSetScrollCallback(window, scroll);

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	int i =0,j=0;
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//createRectangle ();
	border[0].create(0);
	border[1].create(2);
	border[2].create(1);
	border[3].create(3);
	angryBird.create ();
	for(i=1;i<20;i++){
		path[i].create();
	}
	for(j=0;j<7;j++){

		target[j].setX((float)((float)(rand()%7) + (-3.2f)));
		target[j].setY((float)((float)(rand()%7) + (-3.2f)));
		target[j].setCenter(target[j].getX(),target[j].getY());
		target[j].setRadius((float)((float)((rand()%25)+15)/100));
		target[j].create();		
	}
	for(j=0;j<7;j++){

		obstacle[j].setX((float)((float)(rand()%7) + (-3.2f)));
		obstacle[j].setY((float)((float)(rand()%7) + (-3.2f)));
		obstacle[j].setCenter(obstacle[j].getX(),obstacle[j].getY());
		obstacle[j].setRadius((float)((float)((rand()%25)+10)/100));
		obstacle[j].create();		
	}
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.0f, 0.2f, 0.4f, 1.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	//cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	//cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	//cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	//cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
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
	for(i=0;i<7;i++)
		target[i].draw();
	for(i=0;i<7;i++)
		obstacle[i].draw();
	while (!glfwWindowShouldClose(window)) {
		// OpenGL Draw commands
		reshapeWindow (window, width, height);
		angryBird.draw();
		border[0].draw(0);
		border[1].draw(2);
		border[2].draw(1);
		border[3].draw(3);
		//path.draw();
		if(!angryBird.floor){
			for(i=1;i<20;i++){
				path[i].draw(i);
			}
		}

		
		for(i=0;i<7;i++){
			if(!target[i].getCollided()&&!angryBird.checkCollision(i)){
				target[i].draw();
			}
			else{
				;//cout << "collided!  " << endl;
			}
		}
		for(i=0;i<7;i++)
			obstacle[i].draw();
		if(angryBird.getStatus()){
			angryBird.checkWall();
			for(i=0;i<7;i++)
				angryBird.checkObstacle(i);
			angryBird.checkFloor();
			angryBird.checkRoof();
		}
		if(angryBird.getLives() <= 0){
			cout << "GAME OVER!" << endl;
			cout << "Score: " << angryBird.getScore() << endl;
			quit(window);
		}
		//if(angryBird.checkCollision(target)){
		//	delete target;
		//}
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
