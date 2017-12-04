


#define _CRT_SECURE_NO_DEPRECATE
//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "maths_funcs.h"

// Assimp includes

#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.


/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME1 "../camaro_shell.obj"
#define MESH_NAME2 "../camaro_wheel3.obj"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

std::vector<float> g_vp, g_vn, g_vt;
int g_point_count = 0;
int point_count1 = 0;
int point_count2 = 0;


// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
GLuint shaderProgramID;


unsigned int mesh_vao = 0;
int width = 800;
int height = 600;

GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;
GLuint vao1;
GLuint vao2;


#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

bool load_mesh(const char* file_name) {
	const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate); // TRIANGLES!

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return false;
	}
	printf("  %i animations\n", scene->mNumAnimations);
	printf("  %i cameras\n", scene->mNumCameras);
	printf("  %i lights\n", scene->mNumLights);
	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		g_point_count = mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				//printf ("      vp %i (%f,%f,%f)\n", v_i, vp->x, vp->y, vp->z);
				g_vp.push_back(vp->x);
				g_vp.push_back(vp->y);
				g_vp.push_back(vp->z);
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				//printf ("      vn %i (%f,%f,%f)\n", v_i, vn->x, vn->y, vn->z);
				g_vn.push_back(vn->x);
				g_vn.push_back(vn->y);
				g_vn.push_back(vn->z);
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				//printf ("      vt %i (%f,%f)\n", v_i, vt->x, vt->y);
				g_vt.push_back(vt->x);
				g_vt.push_back(vt->y);
			}
			if (mesh->HasTangentsAndBitangents()) {
				// NB: could store/print tangents here
			}
		}
	}



	aiReleaseImport(scene);
	return true;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile) {
	FILE* fp = fopen(shaderFile, "rb"); //!->Why does binary flag "RB" work and not "R"... wierd msvc thing?

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		getchar();
		getchar();
		exit(0);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		getchar();
		getchar();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		getchar();
		getchar();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "../Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "../Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		getchar();
		getchar();

		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);

		getchar();
		getchar();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS


// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS

void generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	////////////////////////// MESH 1 /////////////////////////////////////

	load_mesh(MESH_NAME1);

	point_count1 = g_point_count;
	g_point_count = 0;

	//cout << "POINT COUNT1 = " << point_count1 << endl;

	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, point_count1 * 3 * sizeof(float), &g_vp[0], GL_STATIC_DRAW);

	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, point_count1 * 3 * sizeof(float), &g_vn[0], GL_STATIC_DRAW);

	//	This is for texture coordinates which you don't currently need, so I have commented it out
	//	unsigned int vt_vbo = 0;
	//	glGenBuffers (1, &vt_vbo);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glBufferData (GL_ARRAY_BUFFER, g_point_count * 2 * sizeof (float), &g_vt[0], GL_STATIC_DRAW);

	g_vp.clear();
	g_vn.clear();
	g_vt.clear();

	//GLuint vao1;	
	glGenVertexArrays(1, &vao1);
	glBindVertexArray(vao1);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//	This is for texture coordinates which you don't currently need, so I have commented it out
	//	glEnableVertexAttribArray (loc3);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	//////////////////////////// MESH 2 /////////////////////////////////////
	//g_point_count = 0;
	load_mesh(MESH_NAME2);
	point_count2 = g_point_count;
	g_point_count = 0;
	//unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
	
	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, point_count2 * 3 * sizeof(float), &g_vp[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, point_count2 * 3 * sizeof(float), &g_vn[0], GL_STATIC_DRAW);

	//	This is for texture coordinates which you don't currently need, so I have commented it out
		unsigned int vt_vbo = 0;
		glGenBuffers (1, &vt_vbo);
		glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
		glBufferData (GL_ARRAY_BUFFER, g_point_count * 2 * sizeof (float), &g_vt[0], GL_STATIC_DRAW);

	g_vp.clear();
	g_vn.clear();
	g_vt.clear();


	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	////	This is for texture coordinates which you don't currently need, so I have commented it out
	//	glEnableVertexAttribArray (loc3);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

}


#pragma endregion VBO_FUNCTIONS

//GLfloat view_x = -38.0f;
//GLfloat view_y = -6.0f;
//GLfloat view_z = -3.8f;
//GLfloat rotate_camera_x = -90.0f;
//GLfloat rotate_camera_y = -90.0f;
GLfloat view_x = -24.5f;
GLfloat view_y = -4.5f;
GLfloat view_z = -4.5f;
GLfloat rotate_camera_x = 0.0f;
GLfloat rotate_camera_y = -84.0f;
GLfloat rotate_camera_z = 0.0f;
GLfloat model_rotate = 0.0f;//90.0f;

GLfloat trans_car_x = 0;
GLfloat trans_car_y = 0;
GLfloat trans_car_z = 0;

GLfloat rotate_wheel_deg = 0;


void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	glBindVertexArray(vao1);		//NB: This will allow us to select the first object

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model_matrix");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view_matrix");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj_matrix");


	// Root of the Hierarchy - Car shell
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 100.0);
	mat4 model1 = identity_mat4();
	model1 = translate(model1, vec3(0, 0, trans_car_z));
	view = translate(view, vec3(view_x, view_y, view_z));
	view = rotate_x_deg(view, rotate_camera_x);
	view = rotate_y_deg(view, rotate_camera_y);

	mat4 global1 = model1;

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1.m);

	glDrawArrays(GL_TRIANGLES, 0, point_count1);

	glBindVertexArray(vao2);		//NB: This will allow us to select the second object

	// Front left wheel
	mat4 model2 = identity_mat4();
	model2 = rotate_x_deg(model2, rotate_wheel_deg);
	model2 = translate(model2, vec3(0.9,0.65,1.4));

	mat4 global2 = global1 * model2;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global2.m);


	glDrawArrays(GL_TRIANGLES, 0, point_count2);

	// Front right wheel
	mat4 model3 = identity_mat4();
	model3 = rotate_y_deg(model3, 180.0);
	model3 = rotate_x_deg(model3, rotate_wheel_deg);
	model3 = translate(model3, vec3(-0.7, 0.65, 1.4));

	mat4 global3 = global1 * model3;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global3.m);


	glDrawArrays(GL_TRIANGLES, 0, point_count2);

	// Back left wheel
	mat4 model4 = identity_mat4();
	model4 = rotate_x_deg(model4, rotate_wheel_deg);
	model4 = translate(model4, vec3(0.9, 0.65, -1.2));

	mat4 global4 = global1 * model4;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global4.m);


	glDrawArrays(GL_TRIANGLES, 0, point_count2);

	// Back right wheel
	mat4 model5 = identity_mat4();
	model5 = rotate_y_deg(model5, 180.0);
	model5 = rotate_x_deg(model5, rotate_wheel_deg);
	model5 = translate(model5, vec3(-0.7, 0.65, -1.2));

	mat4 global5 = global1 * model5;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global5.m);

	glDrawArrays(GL_TRIANGLES, 0, point_count2);

	glutSwapBuffers();
}


void updateScene() {

	// Placeholder code, if you want to work with framerate
	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	//model_rotate += 0.01f;
	trans_car_z += 0.01f;
	// rotate the model slowly around the y axis
	rotate_wheel_deg += 0.5f;
	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// load mesh into a vertex buffer array
	generateObjectBufferMesh();

}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {

	switch (key) {
	case 27:
		exit(0);
		break;

	case 'w':
		view_x = view_x + 0.5f;
		cout << "view_x: " << view_x << endl;
		break;
	case 's':
		view_x = view_x - 0.5f;
		cout << "view_x: " << view_x << endl;
		break;

	case 'x':
		view_y = view_y - 0.5f;
		cout << "view_y: " << view_y << endl;
		break;
	case 'z':
		view_y = view_y + 0.5f;
		cout << "view_y: " << view_y << endl;
		break;

	case 'a':
		view_z = view_z - 0.5f;
		cout << "view_z: " << view_z << endl;
		break;
	case 'd':
		view_z = view_z + 0.5f;
		cout << "view_z: " << view_z << endl;
		break;

	}



}

void specialKeypress(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_LEFT:
		rotate_camera_y = rotate_camera_y - 2;
		cout << "rotate_camera_y: " << rotate_camera_y << endl;
		break;

	case GLUT_KEY_RIGHT:
		rotate_camera_y = rotate_camera_y + 2;
		cout << "rotate_camera_y: " << rotate_camera_y << endl;
		break;

	case GLUT_KEY_UP:
		rotate_camera_x = rotate_camera_x - 2;
		cout << "rotate_camera_x: " << rotate_camera_x << endl;
		//rotate_camera_z = rotate_camera_z - 2;
		//cout << "rotate_camera_z: " << rotate_camera_z << endl;
		break;

	case GLUT_KEY_DOWN:
		rotate_camera_x = rotate_camera_x + 2;
		cout << "rotate_camera_x: " << rotate_camera_x << endl;
		//rotate_camera_z = rotate_camera_z + 2;
		//cout << "rotate_camera_z: " << rotate_camera_z << endl;
		break;
	}



}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutSpecialFunc(specialKeypress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	//cout << "HELLO THERE" << endl;
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
