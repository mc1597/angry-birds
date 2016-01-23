#include<iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <sstream>
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

float gravity = 0.6,airDrag = 0.005,friction = 0.1,t=0,groundDrag = 0.5;
float camera_rotation_angle = 90;
int counter,counter1;
bool twinkleOverride = false;
int level=1;
glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);


class Background{
	public:

		void draw(){
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram (programID);

		}
};
Background bg;

class Board{
	public:
	VAO *brd,*cir[2],*tri,*cross;
	bool levelUp;
	float radius;
	Board(){
		levelUp = false;
		radius = 0.6;
	}

	void createBoard(){
		static const GLfloat vertex_buffer_data [] = {
                                2,2,0, // vertex 1
                                2,-2,0, // vertex 2
                                -2,-2,0, // vertex 3

                                -2,-2,0, // vertex 3
                                -2,2,0, // vertex 4
                                2,2,0, // vertex 1
                        };

                        static const GLfloat color_buffer_data [] = {
				1,0.5,0,
				1,0.5,0,
				1,0.5,0,

				1,0.5,0,
				1,0.5,0,
				1,0.5,0,
                        };

                        brd = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

	}

	void createRedCircle(int index,float cx,float cy){

		 int numVertices = 360;
                        GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
                        for (int i=0; i<numVertices; i++) {
                                vertex_buffer_data [3*i] = cx + radius*cos(i*M_PI/180.0f);
                                vertex_buffer_data [3*i + 1] = cy + radius*sin(i*M_PI/180.0f);
                                vertex_buffer_data [3*i + 2] = 0;
                        }


                        GLfloat* color_buffer_data = new GLfloat [3*numVertices];
                        for (int i=0; i<numVertices; i++) {
                                color_buffer_data [3*i] = 1;
                                color_buffer_data [3*i + 1] = 0;
                                color_buffer_data [3*i + 2] = 0;
                        }


                        // create3DObject creates and returns a handle to a VAO that can be used later
                        cir[index] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

	}


	void createCross(){

       static const GLfloat vertex_buffer_data [] = {
			0.3,1.7,0,
			1.7,1.7,0,
			
			1.7,0.3,0,
			0.3,0.3,0,
                        };

                        static const GLfloat color_buffer_data [] = {
				1,1,1,
				1,1,1,
				1,1,1,
				1,1,1,

                        };

                        cross = create3DObject(GL_LINES, 4, vertex_buffer_data, color_buffer_data, GL_FILL);

	}
	void createTriangle(){

       static const GLfloat vertex_buffer_data [] = {
                                -0.4,0,0, // vertex 1
                                -1.2,0.58,0, // vertex 2
                                -1.2,-0.58,0, // vertex 3

                        };

                        static const GLfloat color_buffer_data [] = {
				1,1,1,
				1,1,1,
				1,1,1,

                        };

                        tri = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);


	}

	void draw(int index){
		Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
                glm::mat4 VP = Matrices.projection * Matrices.view;
                glm::mat4 MVP;  // MVP = Projection * View * Model
                Matrices.model = glm::mat4(1.0f);

		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		if(index==0)
			draw3DObject(brd);
		if(index==1)
			draw3DObject(cir[0]);
		if(index==2)
			draw3DObject(cir[1]);
		if(index==3)
			draw3DObject(tri);
		if(index==4)
			draw3DObject(cross);
			
			
	}
};

Board board;
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
		/*glUseProgram (programID);

		  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
		  glm::vec3 target (0, 0, 0);
		  glm::vec3 up (0, 1, 0);
		 */
		//Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

		Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
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

class Sun{
	public:
		VAO *sn[2];
		float posx;
		float posy;
		float radius;

		Sun(){
			posx=-3.75;
			posy=3.75;
			radius=1;
		}

		void createSun(){
			int numVertices = 360;
			GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				vertex_buffer_data [3*i] =  radius*cos(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 1] = radius*sin(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 2] = 0;
			}


			GLfloat* color_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				color_buffer_data [3*i] = 1;
				color_buffer_data [3*i + 1] = 0.65;
				color_buffer_data [3*i + 2] = 0;
			}


			// create3DObject creates and returns a handle to a VAO that can be used later
			sn[0] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

		}

		void createRays(){
			static const GLfloat vertex_buffer_data [] = {
				1,-0.02,0,
				2.1,-0.02,0,

				0.02,-1,0,
				0.02,-2.1,0,

				-1,0.02,0,
				-2.1,0.02,0,

				0.02,1,0,
				0.02,2.1,0,

				0.7,-0.7,0,
				1.2,-1.2,0,

				0.7,0.7,0,
				1.2,1.2,0,

				-0.7,-0.7,0,
				-1.2,-1.2,0,

				-0.7,0.7,0,
				-1.2,1.2,0,

				0.92,0.38,0,
				1.38,0.57,0,

				-0.92,0.38,0,
				-1.38,0.57,0,

				-0.92,-0.38,0,
				-1.38,-0.57,0,

				0.92,-0.38,0,
				1.38,-0.57,0,

				0.38,0.92,0,
				0.57,1.38,0,

				-0.38,0.92,0,
				-0.57,1.38,0,

				-0.38,-0.92,0,
				-0.57,-1.38,0,

				0.38,-0.92,0,
				0.57,-1.38,0,

			};

			static const GLfloat color_buffer_data [] = {
				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,

				1,0,0,
				1,1,0,
			};

			sn[1] = create3DObject(GL_LINES, 32, vertex_buffer_data, color_buffer_data, GL_FILL);

		}

		void draw(int index){
			Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateSn = glm::translate (glm::vec3(posx, posy, 0));
			Matrices.model *= (translateSn);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(sn[index]);


		}
};

Sun sun;

class Portal{

	public:
		VAO *por;
		float posx;
		float posy;
		float radius;
		float center[2];
		Portal(){
			posx = 3.2;
			posy = 1.5;	
			radius = 0.4;
			center[0]=posx;
			center[1]=posy;
		}

		void create(int index){
			int numVertices = 360;

			if(index==0){
				GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
				for (int i=0; i<numVertices; i++) {
					vertex_buffer_data [3*i] =  radius*cos(i*M_PI/180.0f);
					vertex_buffer_data [3*i + 1] =  radius*sin(i*M_PI/180.0f);
					vertex_buffer_data [3*i + 2] = 0;
				}


				GLfloat* color_buffer_data = new GLfloat [3*numVertices];
				for (int i=0; i<numVertices; i++) {
					color_buffer_data [3*i] = 1;
					color_buffer_data [3*i + 1] = 1;
					color_buffer_data [3*i + 2] = 1;
				}


				// create3DObject creates and returns a handle to a VAO that can be used later
				por = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);
			}
			else{
				GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
				for (int i=0; i<numVertices; i++) {
					vertex_buffer_data [3*i] =  radius*cos(i*M_PI/180.0f);
					vertex_buffer_data [3*i + 1] =  radius*sin(i*M_PI/180.0f);
					vertex_buffer_data [3*i + 2] = 0;
				}


				GLfloat* color_buffer_data = new GLfloat [3*numVertices];
				for (int i=0; i<numVertices; i++) {
					color_buffer_data [3*i] = 1;
					color_buffer_data [3*i + 1] = 0;
					color_buffer_data [3*i + 2] = 0;
				}


				// create3DObject creates and returns a handle to a VAO that can be used later
				por = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

			}
		}

		void draw(){
			Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translatePt = glm::translate (glm::vec3(posx, posy, 0));
			Matrices.model *= (translatePt);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			center[0]=posx;
			center[1]=posy;
			draw3DObject(por);


		}

};

Portal portal[4];
class Heart{

	public:
		VAO *hrt[3];
		float posx;
		float posy;
		float radius;

		Heart(){
			posx = 3.5;
			posy = 3.45;	
			radius = 0.062;
		}

		void createTriangle(int index){
			static const GLfloat vertex_buffer_data [] = {
				0,-0.062,0,
				-0.125,0.125,0,
				0.125,0.125,0,	
			};

			static const GLfloat color_buffer_data [] = {
				1,0,0, // color 0
				1,0,0, // color 0
				1,0,0, // color 0

			};
			hrt[index] = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);


		}

		void createLeft(int index){
			int numVertices = 190;
			GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				vertex_buffer_data [3*i] = -0.062 + radius*cos(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 1] = 0.125 + radius*sin(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 2] = 0;
			}


			GLfloat* color_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				color_buffer_data [3*i] = 1;
				color_buffer_data [3*i + 1] = 0;
				color_buffer_data [3*i + 2] = 0;
			}


			// create3DObject creates and returns a handle to a VAO that can be used later
			hrt[index] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

		}

		void createRight(int index){
			int numVertices = 190;
			GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				vertex_buffer_data [3*i] = 0.062 + radius*cos(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 1] = 0.125 + radius*sin(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 2] = 0;
			}


			GLfloat* color_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				color_buffer_data [3*i] = 1;
				color_buffer_data [3*i + 1] = 0;
				color_buffer_data [3*i + 2] = 0;
			}


			// create3DObject creates and returns a handle to a VAO that can be used later
			hrt[index] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

		}

		void draw(int index){
			Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateLt = glm::translate (glm::vec3(posx, posy, 0));
			Matrices.model *= (translateLt);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(hrt[index]);


		}


};

Heart heart[4];
class Light{

	public:
		VAO *li[90];
		float posx;
		float posy;
		float radius;

		Light(){
			posx=0;
			posy=0;
			radius=0.05;
		}

		void create(int index)
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
				color_buffer_data [3*i] = 1;
				color_buffer_data [3*i + 1] = 1;
				color_buffer_data [3*i + 2] = 1;
			}


			// create3DObject creates and returns a handle to a VAO that can be used later
			li[index] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);
		}

		void draw(int index){
			Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateLt = glm::translate (glm::vec3(posx, posy, 0));
			Matrices.model *= (translateLt);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(li[index]);


		}

};

Light light[30];

class Star{

	public:
		VAO *st[90];
		float posx;
		float posy;
		float radius;
		int twinkle;
		bool pause;

		Star(){
			posx=0;
			posy=0;
			radius=0.05;
			twinkle=1;
			pause=false;
		}
		void createone(int index)
		{

			static const GLfloat vertex_buffer_data [] = {
				-0.12,-0.18,0,
				0,0.18,0,
				0.12,-0.18,0,

			};

			static const GLfloat color_buffer_data [] = {
				1,1,0, // color 0
				1,1,0, // color 0
				1,1,0, // color 0

			};
			st[index] = create3DObject(GL_LINE_STRIP, 3, vertex_buffer_data, color_buffer_data, GL_LINE);

		}
		void createtwo(int index)
		{

			static const GLfloat vertex_buffer_data [] = {

				-0.12,-0.18,0,
				0.2,0.07,0,
				-0.2,0.07,0, 

			};

			static const GLfloat color_buffer_data [] = {
				1,1,0, // color 0
				1,1,0, // color 0
				1,1,0, // color 0

			};
			st[index] = create3DObject(GL_LINE_STRIP, 3, vertex_buffer_data, color_buffer_data, GL_LINE);

		}
		void createthree(int index)
		{

			static const GLfloat vertex_buffer_data [] = {
				0.12,-0.18,0,
				-0.2,0.07,0, 
				0.2,0.07,0, 
			};

			static const GLfloat color_buffer_data [] = {
				1,1,0, // color 0
				1,1,0, // color 0
				1,1,0, // color 0

			};
			st[index] = create3DObject(GL_LINE_STRIP, 3, vertex_buffer_data, color_buffer_data, GL_LINE);

		}

		void draw(int index){
			/*glUseProgram (programID);

			  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
			  glm::vec3 target (0, 0, 0);
			  glm::vec3 up (0, 1, 0);
			 */
			//Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

			Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateStar = glm::translate (glm::vec3(posx, posy, 0));
			Matrices.model *= (translateStar);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(st[index]);

		}

};
Star star[180];

class Varys{

	public:
		VAO *var[2];
		float posx;
		float posy;
		float center[2];
		float radius;
		bool collided;
		int dir;
		int up;
		int count;
		bool pause;
		Varys(){

			count=0;
			posx=0;
			posy=-3.5;
			radius = 0.1;
			center[0] = posx;
			center[1] = posy;
			dir = 1;
			up = 1;
			pause=false;
		}



		void createBody(){
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
				color_buffer_data [3*i + 1] = 0;
				color_buffer_data [3*i + 2] = 0;
			}


			// create3DObject creates and returns a handle to a VAO that can be used later
			var[0] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

		}
		void createLegs(){
			static const GLfloat vertex_buffer_data [] = {
				0.1,0,0,
				0.2,0.05,0,

				0.2,0.05,0,
				0.25,0,0,

				0.05,0.086,0,
				0.1,0.1732,0,

				0.1,0.1732,0,
				0.15,0.043,0,

				-0.05,0.086,0,
				-0.1,0.1732,0,

				-0.1,0.1732,0,
				-0.15,0.043,0,

				-0.1,0,0,
				-0.2,0.05,0,

				-0.2,0.05,0,
				-0.25,0,0,

				-0.05,-0.086,0,
				-0.1,-0.1732,0,

				-0.1,-0.1732,0,  //
				-0.14,-0.12,0,

				0.05,-0.086,0,
				0.1,-0.1732,0,

				0.1,-0.1732,0,  //
				0.14,-0.12,0,
			};

			static const GLfloat color_buffer_data [] = {
				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0

				0,0,0, // color 0
				0,0,0, // color 0
			};
			var[1] = create3DObject(GL_LINES, 24, vertex_buffer_data, color_buffer_data, GL_LINE);

		}

		void draw(int index){
			Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateVarys = glm::translate (glm::vec3(posx, posy, 0));
			Matrices.model *= (translateVarys);
			MVP = VP * Matrices.model;
			if(!pause){
				posx += 0.02*dir;
				posy += 0.02*up;
			}
			center[0]=posx;
			center[1]=posy;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(var[index]);

		}

		void turn(){
			count++;
			if(count%25==0){
				dir=-1*dir;
			}
			if(center[1] > 3.5)
				up = -1;
			if(center[1] < -3.5)
				up = 1;
		}	
};
Varys varys[2];

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
	int dir;
	float scaleFactor;
	bool shrink;
	VAO *tar,*eye[2],*pupil[2],*mouth,*tooth;
	bool pause;
	int count;
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
		count = 0;
		dir=1;
		scaleFactor=1;
		shrink=false;
		pause=false;
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
			vertex_buffer_data [3*i] = 1.05*radius*cos(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 1] = 0.95*radius*sin(i*M_PI/180.0f);
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

	void createEye(int index,float startx,float starty)
	{

		int numVertices = 360;
		GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			vertex_buffer_data [3*i] = startx + 0.22*radius*cos(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 1] = starty + 0.44*radius*sin(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 2] = 0;
		}


		GLfloat* color_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			color_buffer_data [3*i] = 1;
			color_buffer_data [3*i + 1] = 1;
			color_buffer_data [3*i + 2] = 1;
		}


		// create3DObject creates and returns a handle to a VAO that can be used later
		eye[index] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	void createPupil(int index,float startx,float starty)
	{

		int numVertices = 360;
		GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			vertex_buffer_data [3*i] = startx + 0.11*radius*cos(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 1] = starty + 0.22*radius*sin(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 2] = 0;
		}


		GLfloat* color_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			color_buffer_data [3*i] = 0;
			color_buffer_data [3*i + 1] = 0;
			color_buffer_data [3*i + 2] = 0;
		}


		// create3DObject creates and returns a handle to a VAO that can be used later
		pupil[index] = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	void createMouth(float x,float y){
		int numVertices = 180;
		GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			vertex_buffer_data [3*i] = x + 0.4*radius*cos(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 1] = y + 0.4*radius*sin(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 2] = 0;
		}


		GLfloat* color_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			color_buffer_data [3*i] = 0;
			color_buffer_data [3*i + 1] = 0;
			color_buffer_data [3*i + 2] = 0;
		}


		// create3DObject creates and returns a handle to a VAO that can be used later
		mouth = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);


	}

	void createTooth(float x,float y){
		static const GLfloat vertex_buffer_data [] = {
			-0.15*radius,-1*y,0, // vertex 1
			0.15*radius,-1*y,0, // vertex 2
			0.15*radius,-1*y-(0.15*radius),0, // vertex 3

			0.15*radius,-1*y-(0.15*radius),0, // vertex 3
			-0.15*radius,-1*y-(0.15*radius),0, // vertex 4	
			-0.15*radius,-1*y,0, // vertex 1
		};

		static const GLfloat color_buffer_data [] = {
			1,1,1, // color 1
			1,1,1, // color 2
			1,1,1, // color 3

			1,1,1, // color 1
			1,1,1, // color 2
			1,1,1, // color 3
		};

		tooth = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

	}
	void draw(int i,int index,float angle){

		Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
		glm::mat4 VP = Matrices.projection * Matrices.view;
		glm::mat4 MVP;  // MVP = Projection * View * Model
		Matrices.model = glm::mat4(1.0f);

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateTar = glm::translate (glm::vec3(posx, posy, 0));
		glm::mat4 rotateTar = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1));
		glm::mat4 scaleTar = glm::scale (glm::vec3(scaleFactor, scaleFactor, 0));
		Matrices.model *= (translateTar * rotateTar * scaleTar);
		if(!pause){
			posy+=0.0005*dir;
			center[1] = posy;	
			count++;
			if(count%150==0)
				dir=-1*dir;

			if(scaleFactor>0&&shrink)
				scaleFactor-=0.001;
		}
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		if(i==0)
			draw3DObject(tar);
		if(i==1)
			draw3DObject(eye[index]);
		if(i==2)
			draw3DObject(pupil[index]);
		if(i==3)
			draw3DObject(mouth);
		if(i==4)
			draw3DObject(tooth);


	}

};

Target target[7];

class Comet{
	public:
		VAO *com,*train;
		float posx;
		float posy;
		bool show;
		float center[2];
		float radius;
		bool pause;
		Comet(){
			posx = 5;
			posy = 2;
			center[0] = posx;
			center[1] = posy;
			radius = 0.1;
			show = false;	
			pause=false;
		}
		void create(){
			int numVertices = 360;
			GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				vertex_buffer_data [3*i] = radius*cos(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 1] = radius*sin(i*M_PI/180.0f);
				vertex_buffer_data [3*i + 2] = 0;
			}


			GLfloat* color_buffer_data = new GLfloat [3*numVertices];
			for (int i=0; i<numVertices; i++) {
				color_buffer_data [3*i] = 0.21;
				color_buffer_data [3*i + 1] = 0.97;
				color_buffer_data [3*i + 2] = 0.69;
			}


			// create3DObject creates and returns a handle to a VAO that can be used later
			com = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

		}

		void createTrain(){
			static const GLfloat vertex_buffer_data [] = {
				0,0.1,0,
				0.2,0.1,0,
				0.3,0.05,0,

				0.3,0.05,0,
				0,0.05,0,
				0,0.1,0,

				0,0.05,0,
				0.3,0.05,0,
				0.4,0,0,

				0.4,0,0,
				0,0,0,
				0,0.05,0,

				0,0,0,	
				0.4,0,0,
				0.3,-0.05,0,

				0.3,-0.05,0,
				0,-0.05,0,
				0,0,0,

				0,-0.05,0,
				0.3,-0.05,0,
				0.2,-0.1,0,

				0.2,-0.1,0,
				0,-0.1,0,
				0,-0.05,0,

			};

			static const GLfloat color_buffer_data [] = {

				1,1,1,
				1,1,1,
				1,1,1,

				1,1,1,
				1,1,1,
				1,1,1,

				1,1,1,
				1,1,1,
				1,1,1,

				1,1,1,
				1,1,1,
				1,1,1,

				1,1,1,
				1,1,1,
				1,1,1,

				1,1,1,
				1,1,1,
				1,1,1,

				1,1,1,
				1,1,1,
				1,1,1,

				1,1,1,
				1,1,1,
				1,1,1,
			};

			train = create3DObject(GL_TRIANGLES, 24, vertex_buffer_data, color_buffer_data, GL_FILL);


		}
		void draw(int index){
			Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
			glm::mat4 VP = Matrices.projection * Matrices.view;
			glm::mat4 MVP;  // MVP = Projection * View * Model
			Matrices.model = glm::mat4(1.0f);

			Matrices.model = glm::mat4(1.0f);

			glm::mat4 translateTar = glm::translate (glm::vec3(posx, posy, 0));
			Matrices.model *= (translateTar);
			if(!pause){
				posx-=0.03;
				center[0]=posx;
			}
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			if(index==0)
				draw3DObject(com);
			if(index==1)
				draw3DObject(train);


		}

		void stop(){
			if(center[0] < -4.5){
				show = false;
				posx = 5;
				center[0] = posx;
			}
		}

};

Comet comet;
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
	bool pause;
	Obstacle(){
		posx = 2;
		posy = 2;
		marks = 5;
		center[0] = posx;
		center[1] = posy;
		radius = 0.25;
		collided = false;
		pause=false;
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
			color_buffer_data [3*i] = 0.5;
			color_buffer_data [3*i + 1] = 0.5;
			color_buffer_data [3*i + 2] = 0.5;
		}


		// create3DObject creates and returns a handle to a VAO that can be used later
		obs = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	void draw(){

		Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
		glm::mat4 VP = Matrices.projection * Matrices.view;
		glm::mat4 MVP;  // MVP = Projection * View * Model
		Matrices.model = glm::mat4(1.0f);

		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateObs = glm::translate (glm::vec3(posx, posy, 0));
		glm::mat4 rotateObs = glm::rotate((float)(obstacle_rotation*M_PI/180.0f), glm::vec3(0,0,1));
		Matrices.model *= (translateObs*rotateObs);
		if(!pause)
			obstacle_rotation+=0.5;
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(obs);

	}

};

Obstacle obstacle[7];


float bird_rotation = 0;
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
	bool pause;
	int hit;
	bool allowed;
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
	VAO *bird,*saucer;
	Bird(){
		lives = 3;
		score = 0;
		allowed=true;
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
		hit=0;
		pause=false;
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
		t=0;
		allowed=true;
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

	void createSaucer(){
		int numVertices = 360;
		GLfloat* vertex_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			vertex_buffer_data [3*i] =  0.05 + radius*cos(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 1] = radius*sin(i*M_PI/180.0f);
			vertex_buffer_data [3*i + 2] = 0;
		}


		GLfloat* color_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			color_buffer_data [3*i] = 1;
			color_buffer_data [3*i + 1] = 0;
			color_buffer_data [3*i + 2] = 0;
		}


		// create3DObject creates and returns a handle to a VAO that can be used later
		saucer = create3DObject(GL_TRIANGLE_FAN, numVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

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

	void draw(int index)

	{

		Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
		glm::mat4 VP = Matrices.projection * Matrices.view;
		glm::mat4 MVP;	// MVP = Projection * View * Model
		Matrices.model = glm::mat4(1.0f);
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 moveBird = glm::translate(glm::vec3((float)(initX+posx),float(initY+posy),0.0f)); 
		glm::mat4 rotateBird = glm::rotate((float)(bird_rotation*M_PI/180.0f), glm::vec3(0,0,1));
		Matrices.model *= (moveBird*rotateBird);
		if(!pause)
			bird_rotation+=0.5;
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		if(index==0)
			draw3DObject(bird);
		else
			draw3DObject(saucer);
		if(floor&&!pause){
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
		else if(isMoving&&!pause){
			float vel_old,theta_old,vel_new,theta_new;
			if(t>3)
				allowed=true;	
			posx = vel*cos(theta*M_PI/180.0f)*t - 0.5*airDrag*t*t;
			posy = vel*sin(theta*M_PI/180.0f)*t - (0.5*(gravity+(airDrag*sin(theta*M_PI/180.0f)))*t*t);
			center[0] = initX + posx + (0.17/3);
			center[1] = initY + posy;

		}
	}

	void checkCollision(int index){
		float cx,cy;
		float r;
		cx = target[index].getCenter()[0];
		cy = target[index].getCenter()[1];
		r = target[index].getRadius();
		if(sqrt(pow((center[0]-cx),2)+pow((center[1]-cy),2))<=(radius + target[index].getRadius()) && !target[index].shrink){
			target[index].setCollided(true);
			setScore(getScore() + (int)((1/r)*4 + abs(target[index].getX()*4) + level*5));	
			target[index].shrink=true;
			target[index].setRadius(0);
			hit++;
			//cout << "Hits: " << hit << endl;	
		}
	}

	void checkComet(){
		float cx,cy;
		cx = comet.center[0];
		cy = comet.center[1];
		if(sqrt(pow((center[0]-cx),2)+pow((center[1]-cy),2))<=(radius + 0.1)){
			setLives(getLives() + 1);
			comet.show=false;
			comet.posx = 5;
			comet.center[0] = comet.posx;
		}

	}

	void checkPortal(){
		float cx1,cy1,cx2,cy2;

		cx1 = portal[0].center[0];
		cy1 = portal[0].center[1];
		cx2 = portal[1].center[0];
		cy2 = portal[1].center[1];
		if(center[1]>0&&allowed){
			if(sqrt(pow((center[0]-cx1),2)+pow((center[1]-cy1),2))<=(radius + portal[0].radius)){

				t = 0;
				initX = cx2;
				initY = cy2;
				posx = 0;
				posy = 0;
				center[0] = 0.05 + initX; 
				center[1] = 0 + initY;
				allowed=false;
				dir = -1*dir;
				isMoving=true;
				if(dir==-1)
					theta = 135;
				else
					theta = 45;
			}
		}
		else if(center[1]<0&&allowed){
			if(sqrt(pow((center[0]-cx2),2)+pow((center[1]-cy2),2))<=(radius + portal[1].radius)){

				t = 0;
				initX = cx1;
				initY = cy1;
				posx = 0;
				posy = 0;
				allowed=false;
				center[0] = 0.05 + initX; 
				center[1] = 0 + initY;
				isMoving=true;
				dir = -1*dir;
				if(dir==-1)
					theta = 135;
				else
					theta = 45;
			}
		}

	}
	void checkVarys(){
		float cx,cy;
		float r;
		cx = varys[0].center[0];
		cy = varys[0].center[1];
		if(sqrt(pow((center[0]-cx),2)+pow((center[1]-cy),2))<=(radius + 0.2)){
			setScore(getScore() - (10*level));	
			reset();

		}

	}
	void checkObstacle(int i){
		float cx,cy;
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
			1.0,0.325,0.28,	
			1.0,0.325,0.28,	
			1.0,0.325,0.28,	
			1.0,0.325,0.28,	
			1.0,0.325,0.28,	
			1.0,0.325,0.28,	
		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		point = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	}
	void draw (float time)
	{
		// clear the color and depth in the frame buffer
		//glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*glUseProgram (programID);

		  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
		  glm::vec3 target (0, 0, 0);
		  glm::vec3 up (0, 1, 0);
		 */
		//	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

		Matrices.view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);
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


void pauseGame(bool play){
	int i;
	if(!play){
		angryBird.pause=true;
		varys[0].pause=true;
		comet.pause=true;
		for(i=0;i<7;i++){
			obstacle[i].pause=true;
			target[i].pause=true;
		}
	}
	else{
		angryBird.pause=false;
		varys[0].pause=false;
		comet.pause=false;
		for(i=0;i<7;i++){
			obstacle[i].pause=false;
			target[i].pause=false;
		}
	}
}
/**************************
 * Customizable functions *
 **************************/
GLfloat fov=89.8f;
GLfloat deltaTime = 0.0f;
GLfloat factor = 0.01f;
GLfloat cameraSpeed = 3.0f * deltaTime;
bool pressNext=false;
bool goNext=false;
bool on=true;
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE && !angryBird.getStatus()&&!angryBird.pause) {
		switch (key) {
			case GLFW_KEY_F:
				angryBird.setVel(angryBird.getVel()+0.2); 
				break;
			case GLFW_KEY_S:
				angryBird.setVel(angryBird.getVel()-0.2); 
				break;
			case GLFW_KEY_B:
				angryBird.setAngle(angryBird.getAngle()-5); 
				break;
			case GLFW_KEY_A:
				angryBird.setAngle(angryBird.getAngle()+5); 
				break;
			case GLFW_KEY_T:
				twinkleOverride = !twinkleOverride; 
				break;
			case GLFW_KEY_SPACE:
				angryBird.setStatus(!angryBird.getStatus()); 
				break;
			case GLFW_KEY_K:
				if(angryBird.initY < 3.5)
					angryBird.initY += 0.2; 
				angryBird.center[1] = angryBird.initY;			
				break;
			case GLFW_KEY_M:
				if(angryBird.initY > -3.5)
					angryBird.initY -= 0.2; 
				angryBird.center[1] = angryBird.initY;			
				break;

			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		cameraSpeed = 3.0f * deltaTime;
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_LEFT:
				cameraPos += glm::normalize(glm::cross(cameraFront,cameraUp))*cameraSpeed*factor;
				break;
			case GLFW_KEY_RIGHT:
				cameraPos -= glm::normalize(glm::cross(cameraFront,cameraUp))*cameraSpeed*factor;
				break;
			case GLFW_KEY_UP:
				if(fov<89)
					fov=89;
				if(fov>91)
					fov=91;
				if(fov>=89&&fov<=91)
					fov-=0.1;
				break;
			case GLFW_KEY_DOWN:
				if(fov<89)
					fov=89;
				if(fov>91)
					fov=91;
				if(fov>=89&&fov<=91)
					fov+=0.1;
				break;
			case GLFW_KEY_P:
				on=!on;
				pauseGame(on);
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
			cout << "Your score: " << angryBird.getScore() << endl;
			cout << "LEVEL: " << level << endl;
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
double slope;
void mouse_callback(GLFWwindow* window,double x,double y){
	if(!angryBird.pause){
	double bird_x=38.0f,bird_y=300.0f;
	slope = atan((y-bird_y)/(x-bird_x));
	slope = (-1*slope*180.0/M_PI)+20;
	}
	if(pressNext&&x>180&&x<270&&y>250&&y<340)
				goNext = true;
	if(pressNext&&x>330&&x<420&&y>250&&y<340){
			cout << "Your score: " << angryBird.getScore() << endl;
			cout << "LEVEL: " << level << endl;
			quit(window);
		}

}
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS){
				pressNext=true;
			}
			break;
			if (action == GLFW_RELEASE)
				pressNext=false;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_PRESS) {
				if(!angryBird.pause)
					angryBird.setAngle(slope);
				//rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}



void scroll(GLFWwindow* window,double x,double y){

	if(fov<89)
		fov=89;
	if(fov>91)
		fov=91;
	if(fov>=89&&fov<=91)
		fov-=y*0.1;
	cameraSpeed = 3.0f * deltaTime;
	if(x==-1)
		cameraPos -= glm::normalize(glm::cross(cameraFront,cameraUp))*cameraSpeed*factor;
	if(x==1)
		cameraPos += glm::normalize(glm::cross(cameraFront,cameraUp))*cameraSpeed*factor;


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
	glfwSetCursorPosCallback(window,mouse_callback);
	/*stringstream ss;
	  ss << angryBird.getScore();
	  string str = ss.str();	
	  string t = "Angry Birds: Star Wars Edition!!!\t\t\t\t\t\t  Score: " + str;
	  const char *some = t.c_str();
	  glfwSetWindowTitle(window,some);*/
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


	border[0].create(0);
	border[1].create(2);
	border[2].create(1);
	border[3].create(3);
	angryBird.createSaucer();
	angryBird.create();
	for(i=1;i<20;i++){
		path[i].create();
	}

	board.createBoard();
	board.createRedCircle(0,-1,0);
	board.createRedCircle(1,1,0);
	board.createTriangle();
	board.createCross();
	for(j=0;j<7;j++){

		target[j].setX((float)((float)(rand()%7) + (-3.2f)));
		target[j].setY((float)((float)(rand()%7) + (-3.2f)));
		target[j].setCenter(target[j].getX(),target[j].getY());
		target[j].setRadius((float)((float)((rand()%20)+35)/100));
		target[j].create();		
		target[j].createEye(0,(target[j].getRadius()/2),target[j].getRadius()/4);
		target[j].createEye(1,-1*(target[j].getRadius()/2),target[j].getRadius()/4);
		target[j].createPupil(0,(target[j].getRadius()/2),target[j].getRadius()/8);
		target[j].createPupil(1,-1*(target[j].getRadius()/2),target[j].getRadius()/8);
		target[j].createMouth(0,(target[j].getRadius()/3));
		target[j].dir=pow(-1,j%2);
		//target[j].createTooth(0,(target[j].getRadius()/3));
	}
	for(j=0;j<7;j++){

		obstacle[j].setX((float)((float)(rand()%7) + (-3.2f)));
		obstacle[j].setY((float)((float)(rand()%7) + (-3.2f)));
		obstacle[j].setCenter(obstacle[j].getX(),obstacle[j].getY());
		obstacle[j].setRadius((float)((float)((rand()%25)+10)/100));
		obstacle[j].create();		
	}

	for(j=0;j<39;j++){
		if(j<3){
			star[j].posx = -3.0;
			star[j].posy = 2.6;
			star[j].twinkle=1;
		}
		else if(j<6){
			star[j].posx = -1.0;
			star[j].posy = 1.1;
			star[j].twinkle=0;
		}
		else if(j<9){
			star[j].posx = 1.0;
			star[j].posy = 2.75;
			star[j].twinkle=1;
		}
		else if(j<12){
			star[j].posx = 2.0;
			star[j].posy = 1.75;
			star[j].twinkle=0;
		}
		else if(j<15){
			star[j].posx = 3.0;
			star[j].posy = 2.25;
			star[j].twinkle=1;
		}
		else if(j<18){
			star[j].posx = 1.0;
			star[j].posy = -2.75;
			star[j].twinkle=0;
		}
		else if(j<21){
			star[j].posx = -2.8;
			star[j].posy = -3.1;
			star[j].twinkle=1;
		}
		else if(j<24){
			star[j].posx = 1.4;
			star[j].posy = 2.5;
			star[j].twinkle=1;
		}
		else if(j<27){
			star[j].posx = 3.05;
			star[j].posy = -2.15;
			star[j].twinkle=0;
		}
		else if(j<30){
			star[j].posx = -1.5;
			star[j].posy = -0.5;
			star[j].twinkle=1;
		}
		else if(j<33){
			star[j].posx = 3.25;
			star[j].posy = -3.15;
			star[j].twinkle=1;
		}
		else if(j<36){
			star[j].posx = -1.05;
			star[j].posy = 3.35;
			star[j].twinkle=0;
		}
		else if(j<39){
			star[j].posx = -0.75;
			star[j].posy = -2.15;
			star[j].twinkle=1;
		}
		if(j%3==0){
			star[j].createone(j);
			//cout << j << " " << star[j].posx << " " << star[j].posy << " " << star[j].twinkle << endl;
		}
		if(j%3==1){
			star[j].createtwo(j);
		}
		if(j%3==2){
			star[j].createthree(j);
		}
	}

	/*for(j=0;j<29;j++){
	//light[j].posx = (((j)-15)*0.25) + 0.25;
	//light[j].posy = 3.0 + pow(-1,(j)%2)*0.25;
	light[j].posx = ((float)((float)(rand()%7) + (-3.2f)));
	light[j].posy = ((float)((float)(rand()%7) + (-3.2f)));
	light[j].create(j);
	}*/

	varys[0].createBody();
	varys[0].createLegs();

	heart[3].posx = 2.60;
	heart[3].posy = 3.45;
	heart[2].posx = 2.90;
	heart[2].posy = 3.45;
	heart[1].posx = 3.20;
	heart[1].posy = 3.45;
	heart[0].posx = 3.5;
	heart[0].posy = 3.45;
	for(i=0;i<4;i++){
		heart[i].createTriangle(0);
		heart[i].createLeft(1);
		heart[i].createRight(2);
	}
	sun.createSun();
	sun.createRays();
	comet.show = false;
	comet.posy = rand()%5 - 2;
	comet.center[1] = comet.posy;
	comet.createTrain();
	comet.create();
	portal[0].posx = 2.8;
	portal[0].posy = 1.5;
	portal[0].create(0);
	portal[1].posx = -2.0;
	portal[1].posy = -2.8;
	portal[1].create(0);
	portal[2].posx = 2.8;
	portal[2].posy = 1.5;
	portal[2].create(1);
	portal[3].posx = -2.0;
	portal[3].posy = -2.8;
	portal[3].create(1);

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

void next_level(GLFWwindow* window, int width, int height){
	int i;
	goNext=false;
	angryBird.setScore(angryBird.getScore() + angryBird.getLives()*50);
	angryBird.reset();
	angryBird.setStatus(false);
	angryBird.setLives(3);
	angryBird.hit=0;
	level++;
	initGL (window, width, height);
	for(i=0;i<7;i++){
		target[i].shrink=false;
		target[i].scaleFactor=1;

	}
	counter1=0;
	pauseGame(true);
	board.levelUp=false;
	goNext=false;

}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;
	int i,j,k,num;
	stringstream ss1,ss2;
	string convStr1,convStr2,concatStr;
	num = rand()%400 + 200;
	//cout << num << endl;
	GLFWwindow* window = initGLFW(width, height);
	initGL (window, width, height);
	bool blink = true;
	double last_update_time = glfwGetTime(), current_time;
	double last_blink_time = glfwGetTime(), current_blink_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {
		// OpenGL Draw commands
		reshapeWindow (window, width, height);
		ss1.str("");	
		ss2.str("");	
		ss1 << angryBird.getScore();
		ss2 << level;
		convStr1 = ss1.str();	
		convStr2 = ss2.str();	
		concatStr = "Angry Birds: Star Wars Edition!!!\t\t\t Level: " +  convStr2 + "\t\tScore: " + convStr1;
		const char *gameTitle = concatStr.c_str();
		glfwSetWindowTitle(window,gameTitle);
		bg.draw();
		sun.draw(0);
		sun.draw(1);
		border[0].draw(0);
		border[1].draw(2);
		border[2].draw(1);
		border[3].draw(3);
		portal[0].draw();
		portal[1].draw();
		if(!angryBird.allowed){
			portal[2].draw();
			portal[3].draw();
		}
		angryBird.draw(1);
		angryBird.draw(0);
		if(!angryBird.floor){
			for(i=1;i<10;i++){
				path[i].draw(i);
			}
		}

		for(i=0;i<39;i++){
			if(star[i].twinkle||twinkleOverride)
				star[i].draw(i);
		}

		//	for(i=0;i<29;i++)
		//		light[i].draw(i);
		for(i=0;i<7;i++){
			//if(!target[i].getCollided()&&!angryBird.checkCollision(i)){
			target[i].draw(0,0,0);
			target[i].draw(1,0,-10);
			target[i].draw(1,1,10);
			target[i].draw(2,0,-10);
			target[i].draw(2,1,10);
			target[i].draw(3,0,180);
			//target[i].draw(4,0,0);
			//}
			//else{
			//	;//cout << "collided!  " << endl;
			//}
		}
		for(i=0;i<7;i++)
			obstacle[i].draw();
		varys[0].turn();	
		varys[0].draw(0);
		varys[0].draw(1);

		for(i=0;i<angryBird.getLives();i++){
			heart[i].draw(0);
			heart[i].draw(1);
			heart[i].draw(2);
		}
		if(comet.show){
			comet.draw(1);
			comet.draw(0);
			angryBird.checkComet();
		}
		comet.stop();
		if(angryBird.getStatus()){
			angryBird.checkWall();
			angryBird.checkFloor();
			for(i=0;i<7;i++)
				angryBird.checkCollision(i);
			for(i=0;i<7;i++)
				angryBird.checkObstacle(i);
			angryBird.checkVarys();
			angryBird.checkRoof();
			angryBird.checkPortal();
		}
		if(angryBird.getLives() <= 0){
			cout << "GAME OVER!" << endl;
			cout << "LEVEL: " << level << endl;
			cout << "Score: " << angryBird.getScore() << endl;
			quit(window);
		}
		if(angryBird.hit == 1){
			pauseGame(false);
			board.levelUp=true;
			board.draw(0);
			board.draw(1);
			board.draw(2);
			board.draw(3);
			board.draw(4);
			if(goNext&&board.levelUp){
				goNext=false;
				next_level(window, width, height);
			}
		}
		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.025) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			deltaTime+=0.025;
			if(!angryBird.pause){
				counter++;
				counter1++;
			}
			//cout << "ct" << counter1 << endl;
			if(angryBird.getStatus()&&!angryBird.pause)
				t+=0.025;
			last_update_time = current_time;
			if(counter%10==0&&!angryBird.pause){
				for(k=0;k<39;k++)
					star[k].twinkle=!star[k].twinkle;
			}
			if(counter1 == num){
				comet.show=true;
				comet.posx = 5;
				comet.center[0] = comet.posx;
			}
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
