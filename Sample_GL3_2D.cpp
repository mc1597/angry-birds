#include<iostream>
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

float gravity = 0.5,airDrag = 0.1,friction = 0.1,t=0,groundDrag = 0.5;
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
			vertex_buffer_data [3*i] = 0.25*cos(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 1] = 0.25*sin(i*M_PI/180.0f);
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
		//center[0]+=posx;
		//center[1]+=posy;
		//cout << center[0] << " " << center[1] << endl;
		//glm::mat4 translatePoint = glm::translate(glm::vec3((float)(-1.0f + posx),(float)(posy), 0.0f));
		Matrices.model *= (translateTar);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(tar);

	}

};
Target target;
class Bird{
	int lives;
	float posx;
	float posy;
	float vel;
	float theta;
	bool isMoving; 
	float center[2];
	float radius;
	public:
	float initX;
	float initY;
	float absy;
	float absx;
	float store;
	float newv;
	bool floor;
	bool flag;
	VAO *bird;
	Bird(){
		lives = 3;
		isMoving = false;
		vel = 1.2;
		theta = 45;
		posx = 0;
		posy = 0;
		center[0] = 0.05;
		center[1] = 0; 
		radius = 0.12;
		initX = -3.5;
		initY = 0;
		absy = 0;
		absx = 4;
		store = 0;
		floor = false;
		flag = true;
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
				posx = posx + vel*t - 0.5*groundDrag*t*t;
				vel = vel - groundDrag*t;
				center[0] = initX + posx + (0.17/3);
				//cout << "Vel: " << vel << endl;
			}
			if(vel <= 0 || center[0] > 3.25 || center[0] < -3.75)
				flag = false;
			//cout << "centerx: " << center[0] << endl;
		}
		else if(isMoving){
			float oldx,oldy;
			oldx = posx;
			oldy = posy;
			posx = vel*cos(theta*M_PI/180.0f)*t - (0.5*airDrag*cos(theta*M_PI/180.0f)*t*t);
			posy = vel*sin(theta*M_PI/180.0f)*t - (0.5*(gravity+(airDrag*sin(theta*M_PI/180.0f)))*t*t);
			center[0] = initX + posx + (0.17/3);
			center[1] = initY + posy;
			//cout << "posx: " << posx << " posy: " << posy << endl;
			//cout << "centerx: " << center[0] << " centery: " << center[1] << " absy: "<< absy + center[1] << endl;

		}
	}

	bool checkCollision(){
		int cx,cy;
		cx = target.getCenter()[0];
		cy = target.getCenter()[1];
		//cout << "1.  "<< target.getCollided() << endl;
		if(sqrt(((center[0]-cx)*(center[0]-cx))+((center[1]-cy)*(center[1]-cy)))<=(radius + target.getRadius())){
			target.setCollided(true);		
			//smash = true;
			//cout << "2.  "<< target.getCollided() << endl;
			return true;
		}
		else{
			//	cout << "3.  "<< target.getCollided() << endl;
			return false;	
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

		}

	}

	void checkFloor(){
		float theta_old,vel_old,vel_new,theta_new,alpha=0.8;
		if(center[1] <= -3.6&&!floor){
			theta_old = theta;
			vel_old = vel;
			vel_new = sqrt(((alpha*vel_old*cos(theta_old*M_PI/180.0f))*(alpha*vel_old*cos(theta_old*M_PI/180.0f)))+(vel_old*sin(theta_old*M_PI/180.0f)*(vel_old*sin(theta_old*M_PI/180.0f))));
			theta_new = atan(sin(theta_old*M_PI/180.0f)/(alpha*cos(theta*M_PI/180.0f)));
			vel = vel_new;	
			theta = theta_new;
			t = 0;
			vel = vel*cos(theta*M_PI/180.0f);
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
		//posx = angryBird.getVel()*cos(angryBird.getAngle()*M_PI/180.0f)*time;
		//posy = angryBird.getVel()*sin(angryBird.getAngle()*M_PI/180.0f)*time - (0.5*gravity*time*time);

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
	border[0].create(0);
	border[1].create(2);
	border[2].create(1);
	border[3].create(3);
	angryBird.create ();
	for(i=1;i<20;i++){
		path[i].create();
	}
	target.create();
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
	target.draw();
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		angryBird.draw();
		border[0].draw(0);
		border[1].draw(2);
		border[2].draw(1);
		border[3].draw(3);
		//path.draw();
		if(!angryBird.floor){
			for(i=1;i<6;i++){
				path[i].draw(i);
			}
		}
		if(!target.getCollided()&&!angryBird.checkCollision()){
			target.draw();
		}
		else{
			;//cout << "collided!  " << endl;
		}
		angryBird.checkWall();
		angryBird.checkFloor();
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
