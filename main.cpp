


#define _CRT_SECURE_NO_DEPRECATE
//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
//#include "SOIL.h"
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME1 "../camaro_shell.obj"
#define MESH_NAME2 "../camaro_wheel3.obj"
#define MESH_NAME3 "../road_mesh7.obj"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

std::vector<float> g_vp, g_vn, g_vt;
int g_point_count = 0;
int point_count1 = 0;
int point_count2 = 0;
int point_count3 = 0;




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
GLuint vao3;

GLuint texture;
GLuint texture0;

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

	unsigned int vt_vbo = 0;
	glGenBuffers(1, &vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, point_count1 * 2 * sizeof(float), &g_vt[0], GL_STATIC_DRAW);



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
	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);




	//stbi_image_free(image);




	//////////////////////////// MESH 2 /////////////////////////////////////
	////g_point_count = 0;
	//load_mesh(MESH_NAME2);
	//point_count2 = g_point_count;
	//g_point_count = 0;
	////unsigned int vp_vbo = 0;
	//loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	//loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	//loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
	//
	//glGenBuffers(1, &vp_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	//glBufferData(GL_ARRAY_BUFFER, point_count2 * 3 * sizeof(float), &g_vp[0], GL_STATIC_DRAW);

	//glGenBuffers(1, &vn_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	//glBufferData(GL_ARRAY_BUFFER, point_count2 * 3 * sizeof(float), &g_vn[0], GL_STATIC_DRAW);

	////	This is for texture coordinates which you don't currently need, so I have commented it out
	///*	unsigned int vt_vbo = 0;
	//	glGenBuffers (1, &vt_vbo);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glBufferData (GL_ARRAY_BUFFER, g_point_count * 2 * sizeof (float), &g_vt[0], GL_STATIC_DRAW);*/

	//g_vp.clear();
	//g_vn.clear();
	//g_vt.clear();


	//glGenVertexArrays(1, &vao2);
	//glBindVertexArray(vao2);

	//glEnableVertexAttribArray(loc1);
	//glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	//glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//glEnableVertexAttribArray(loc2);
	//glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	//glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//////	This is for texture coordinates which you don't currently need, so I have commented it out
	////	glEnableVertexAttribArray (loc3);
	////	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	////	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);


	////////////////////////////// MESH 3 /////////////////////////////////////
	////g_point_count = 0;
	//load_mesh(MESH_NAME3);
	//point_count3 = g_point_count;
	//g_point_count = 0;
	////unsigned int vp_vbo = 0;
	//loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	//loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	//loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	//glGenBuffers(1, &vp_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	//glBufferData(GL_ARRAY_BUFFER, point_count3 * 3 * sizeof(float), &g_vp[0], GL_STATIC_DRAW);

	//glGenBuffers(1, &vn_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	//glBufferData(GL_ARRAY_BUFFER, point_count3 * 3 * sizeof(float), &g_vn[0], GL_STATIC_DRAW);

	////	This is for texture coordinates which you don't currently need, so I have commented it out
	///*unsigned int vt_vbo = 0;
	//glGenBuffers(1, &vt_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	//glBufferData(GL_ARRAY_BUFFER, g_point_count * 2 * sizeof(float), &g_vt[0], GL_STATIC_DRAW);*/

	//g_vp.clear();
	//g_vn.clear();
	//g_vt.clear();


	//glGenVertexArrays(1, &vao3);
	//glBindVertexArray(vao3);

	//glEnableVertexAttribArray(loc1);
	//glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	//glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//glEnableVertexAttribArray(loc2);
	//glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	//glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

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
GLfloat trans_car_z = -20;

GLfloat rotate_wheel_deg = 0;

//void loadTexture() {
//	
//	int width1, height1, bpp;
//	//unsigned char* image = SOIL_load_image("C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\seaguls.jpeg", &width1, &height1, 0, SOIL_LOAD_RGB);
//	unsigned char* image = stbi_load("C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\seaguls.jpeg", &width1, &height1, &bpp, STBI_rgb);
//	if (!image) {
//		cout << "COULD NOT LOAD IMAGE" << endl;
//	}
//	//GLuint texture;
//	glGenTextures(1, &texture);
//	glBindTexture(GL_TEXTURE_2D, texture);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	
//	//return false;
//	//glEnable(GL_TEXTURE_2D);
//	//glActiveTexture(texture);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
//	glGenerateMipmap(GL_TEXTURE_2D);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	stbi_image_free(image);
//
//	
//	//glUniform1i(glGetUniformLocation(shaderProgramID, "basic_texture"), 0);
//
//	cout << "TEXTURE LOADED" << endl;
//	//stbi_image_free(image);
//
//	//return texture;
//}

void display() {


	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);



	//glBindTexture(GL_TEXTURE_2D, texture0);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model_matrix");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view_matrix");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj_matrix");



	// Root of the Hierarchy - Car shell
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 100.0);
	mat4 model1 = identity_mat4();
	//model1 = translate(model1, vec3(0, 0.4, trans_car_z));
	view = translate(view, vec3(view_x, view_y, view_z));
	view = rotate_x_deg(view, rotate_camera_x);
	view = rotate_y_deg(view, rotate_camera_y);

	mat4 global1 = model1;


	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1.m);
	//glUniform1i(glGetUniformLocation(shaderProgramID, "tex"), 4);

	glBindVertexArray(vao1);		//NB: This will allow us to select the first object
	//glBindTexture(GL_TEXTURE_2D, texture0);
	glDrawArrays(GL_TRIANGLES, 0, point_count1);

	//glBindVertexArray(vao2);		//NB: This will allow us to select the second object

	//// Front left wheel
	//mat4 model2 = identity_mat4();
	//model2 = rotate_x_deg(model2, rotate_wheel_deg);
	//model2 = translate(model2, vec3(0.9,0.65,1.4));

	//mat4 global2 = global1 * model2;

	//// update uniforms & draw
	//glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global2.m);


	//glDrawArrays(GL_TRIANGLES, 0, point_count2);

	//// Front right wheel
	//mat4 model3 = identity_mat4();
	//model3 = rotate_y_deg(model3, 180.0);
	//model3 = rotate_x_deg(model3, rotate_wheel_deg);
	//model3 = translate(model3, vec3(-0.7, 0.65, 1.4));

	//mat4 global3 = global1 * model3;

	//// update uniforms & draw
	//glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global3.m);


	//glDrawArrays(GL_TRIANGLES, 0, point_count2);

	//// Back left wheel
	//mat4 model4 = identity_mat4();
	//model4 = rotate_x_deg(model4, rotate_wheel_deg);
	//model4 = translate(model4, vec3(0.9, 0.65, -1.2));

	//mat4 global4 = global1 * model4;

	//// update uniforms & draw
	//glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global4.m);


	//glDrawArrays(GL_TRIANGLES, 0, point_count2);

	//// Back right wheel
	//mat4 model5 = identity_mat4();
	//model5 = rotate_y_deg(model5, 180.0);
	//model5 = rotate_x_deg(model5, rotate_wheel_deg);
	//model5 = translate(model5, vec3(-0.7, 0.65, -1.2));

	//mat4 global5 = global1 * model5;

	//// update uniforms & draw
	//glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global5.m);

	//glDrawArrays(GL_TRIANGLES, 0, point_count2);

	//glBindVertexArray(vao3);

	//// Road mesh
	//mat4 road_model = identity_mat4();
	//road_model = translate(road_model, vec3(0, 0, 0));
	//road_model = rotate_y_deg(road_model, 90.0f);
	//view = translate(view, vec3(view_x, view_y, view_z));
	//view = rotate_x_deg(view, rotate_camera_x);
	//view = rotate_y_deg(view, rotate_camera_y);

	//// update uniforms & draw
	//glUniformMatrix4fv(matrix_location, 1, GL_FALSE, road_model.m);

	//glDrawArrays(GL_TRIANGLES, 0, point_count3);

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
	trans_car_z += 0.05f;
	// rotate the model slowly around the y axis
	rotate_wheel_deg += 10.0f;
	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	GLuint texture0;

	// load mesh into a vertex buffer array
	generateObjectBufferMesh();
	//int width1, height1, bpp;
	////unsigned char* image = SOIL_load_image("C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\seaguls.jpeg", &width1, &height1, 0, SOIL_LOAD_RGB);
	//unsigned char* image = stbi_load("C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\seaguls1.jpeg", &width1, &height1, &bpp, STBI_rgb);
	//if (!image) {
	//	cout << "COULD NOT LOAD IMAGE" << endl;
	//}
	////GLuint texture;
	//glGenTextures(1, &texture0);
	//glBindTexture(GL_TEXTURE_2D, texture0);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	////glEnable(GL_TEXTURE_2D);
	////glActiveTexture(texture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width1, height1, 0, GL_RGB8, GL_UNSIGNED_BYTE, image);

	//glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//stbi_image_free(image);


	////glUniform1i(glGetUniformLocation(shaderProgramID, "basic_texture"), 0);

	//cout << "TEXTURE LOADED" << endl;
	
	int x, y, n;
	int force_channels = 4;
	unsigned char *image_data = stbi_load("C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\seaguls1.jpg", &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", "C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\seaguls1.jpg");
		
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n",
			"C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\seaguls1.jpg");
	}
	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
	glGenTextures(1, &texture0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		image_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	//return true;

}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {

	switch (key) {
	case 27:
		exit(0);
		break;

	case 'w':
		view_x = view_x + 1.0f;
		cout << "view_x: " << view_x << endl;
		break;
	case 's':
		view_x = view_x - 1.0f;
		cout << "view_x: " << view_x << endl;
		break;

	case 'x':
		view_y = view_y - 1.0f;
		cout << "view_y: " << view_y << endl;
		break;
	case 'z':
		view_y = view_y + 1.0f;
		cout << "view_y: " << view_y << endl;
		break;

	case 'a':
		view_z = view_z - 1.0f;
		cout << "view_z: " << view_z << endl;
		break;
	case 'd':
		view_z = view_z + 1.0f;
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

void initialise_texture(GLuint* id, int x, int y, unsigned char* data) {
	//glGenTextures(1, id);
	//glActiveTexture(*id);
	//glBindTexture(GL_TEXTURE_2D, *id);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	/*glGenTextures(1, id);
	glActiveTexture(*id);
	glBindTexture(GL_TEXTURE_2D, *id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

}


int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");

	/*int width1, height1;
	unsigned char* image = SOIL_load_image("ball.png", &width1, &height1, 0, SOIL_LOAD_RGB);
	glGenTextures(1, &texture0);
	glActiveTexture(texture0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SOIL_free_image_data(image);*/

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

//
//// Link statically with GLEW
//#define GLEW_STATIC
//
//// Headers
//#include <GL/glew.h>
//#include <SOIL.h>
//#include <SFML/Window.hpp>
//
//// Shader sources
//const GLchar* vertexSource = R"glsl(
//    #version 150 core
//    in vec2 position;
//    in vec3 color;
//    in vec2 texcoord;
//    out vec3 Color;
//    out vec2 Texcoord;
//    void main()
//    {
//        Color = color;
//        Texcoord = texcoord;
//        gl_Position = vec4(position, 0.0, 1.0);
//    }
//)glsl";
//const GLchar* fragmentSource = R"glsl(
//    #version 150 core
//    in vec3 Color;
//    in vec2 Texcoord;
//    out vec4 outColor;
//    uniform sampler2D texKitten;
//    uniform sampler2D texPuppy;
//    void main()
//    {
//        outColor = mix(texture(texKitten, Texcoord), texture(texPuppy, Texcoord), 0.5);
//    }
//)glsl";
//
//int main()
//{
//	sf::ContextSettings settings;
//	settings.depthBits = 24;
//	settings.stencilBits = 8;
//
//	sf::Window window(sf::VideoMode(800, 600, 32), "OpenGL", sf::Style::Titlebar | sf::Style::Close, settings);
//
//	// Initialize GLEW
//	glewExperimental = GL_TRUE;
//	glewInit();
//
//	// Create Vertex Array Object
//	GLuint vao;
//	glGenVertexArrays(1, &vao);
//	glBindVertexArray(vao);
//
//	// Create a Vertex Buffer Object and copy the vertex data to it
//	GLuint vbo;
//	glGenBuffers(1, &vbo);
//
//	GLfloat vertices[] = {
//		//  Position      Color             Texcoords
//		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
//		0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
//		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
//		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
//	};
//
//	glBindBuffer(GL_ARRAY_BUFFER, vbo);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//	// Create an element array
//	GLuint ebo;
//	glGenBuffers(1, &ebo);
//
//	GLuint elements[] = {
//		0, 1, 2,
//		2, 3, 0
//	};
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
//
//	// Create and compile the vertex shader
//	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//	glShaderSource(vertexShader, 1, &vertexSource, NULL);
//	glCompileShader(vertexShader);
//
//	// Create and compile the fragment shader
//	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
//	glCompileShader(fragmentShader);
//
//	// Link the vertex and fragment shader into a shader program
//	GLuint shaderProgram = glCreateProgram();
//	glAttachShader(shaderProgram, vertexShader);
//	glAttachShader(shaderProgram, fragmentShader);
//	glBindFragDataLocation(shaderProgram, 0, "outColor");
//	glLinkProgram(shaderProgram);
//	glUseProgram(shaderProgram);
//
//	// Specify the layout of the vertex data
//	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
//	glEnableVertexAttribArray(posAttrib);
//	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);
//
//	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
//	glEnableVertexAttribArray(colAttrib);
//	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
//
//	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
//	glEnableVertexAttribArray(texAttrib);
//	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
//
//	// Load textures
//	GLuint textures[2];
//	glGenTextures(2, textures);
//
//	int width, height;
//	unsigned char* image;
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, textures[0]);
//	image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
//	SOIL_free_image_data(image);
//	glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, textures[1]);
//	image = SOIL_load_image("sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
//	SOIL_free_image_data(image);
//	glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	bool running = true;
//	while (running)
//	{
//		sf::Event windowEvent;
//		while (window.pollEvent(windowEvent))
//		{
//			switch (windowEvent.type)
//			{
//			case sf::Event::Closed:
//				running = false;
//				break;
//			}
//		}
//
//		// Clear the screen to black
//		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT);
//
//		// Draw a rectangle from the 2 triangles using 6 indices
//		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//
//		// Swap buffers
//		window.display();
//	}
//
//	glDeleteTextures(2, textures);
//
//	glDeleteProgram(shaderProgram);
//	glDeleteShader(fragmentShader);
//	glDeleteShader(vertexShader);
//
//	glDeleteBuffers(1, &ebo);
//	glDeleteBuffers(1, &vbo);
//
//	glDeleteVertexArrays(1, &vao);
//
//	window.close();
//
//	return 0;
//}