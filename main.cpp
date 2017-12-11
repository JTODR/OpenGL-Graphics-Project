


#define _CRT_SECURE_NO_DEPRECATE
//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "maths_funcs.h"
#include "text.h"

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
#define MESH_NAME1 "../camaro_shell3.obj"
#define MESH_NAME2 "../camaro_windows_grill.obj"
#define MESH_NAME3 "../cactuses2.obj"
#define MESH_NAME4 "../sand3.obj"
#define MESH_NAME5 "../lizard6.obj"
#define MESH_NAME6 "../camaro_rim_origin.obj"
#define MESH_NAME7 "../camaro_tire_origin.obj"
#define MESH_NAME8 "../camaro_lights_exhaust.obj"
#define MESH_NAME9 "../road2.obj"
#define MESH_NAME10 "../sky_box3.obj"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

std::vector<float> g_vp, g_vn, g_vt;
int g_point_count = 0;
int point_count1 = 0;
int point_count2 = 0;
int point_count3 = 0;
int point_count4 = 0;
int point_count5 = 0;
int point_count6 = 0;
int point_count7 = 0;
int point_count8 = 0;
int point_count9 = 0;
int point_count10 = 0;




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
GLuint vao4;
GLuint vao5;
GLuint vao6;
GLuint vao7;
GLuint vao8;
GLuint vao9;
GLuint vao10;


GLfloat view_x = -8.0f;
GLfloat view_y = -5.0f;
GLfloat view_z = -6.0f;

GLfloat rotate_camera_x = 0.0f;
GLfloat rotate_camera_y = -26.0f;
GLfloat rotate_camera_z = 0.0f;
GLfloat model_rotate = 0.0f;//90.0f;

GLfloat trans_car_x = 3;
GLfloat trans_car_y = 0.4;
GLfloat trans_car_z = -40;
GLfloat rotate_y_car = 0;
//
GLfloat trans_liz_x = 20.0;
GLfloat trans_liz_y = 0.8;
GLfloat trans_liz_z = 20.0;
GLfloat rotate_liz_x = 0;
GLfloat rotate_liz_z = 0;

GLfloat rotate_wheel_deg = 0;

int rotate_count = 1;
string speed_to_print;
GLfloat speed_of_car = 0.1;


GLfloat offset = 1.0f;
GLfloat pitch;
GLfloat yaw;

//GLuint texture;
//GLuint texture0;
//GLuint texture1;
//GLuint texture2;
//GLuint texture3;

GLuint textures[7];

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

int generateObjectBufferMesh(GLuint vao) {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, g_point_count * 3 * sizeof(float), &g_vp[0], GL_STATIC_DRAW);

	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, g_point_count * 3 * sizeof(float), &g_vn[0], GL_STATIC_DRAW);

	unsigned int vt_vbo = 0;
	glGenBuffers(1, &vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, g_point_count * 2 * sizeof(float), &g_vt[0], GL_STATIC_DRAW);

	g_vp.clear();
	g_vn.clear();
	g_vt.clear();

	glBindVertexArray(vao);

	// vertices
	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// normals
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// textures
	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	return g_point_count;
}


#pragma endregion VBO_FUNCTIONS

double speed;
void display() {


	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model_matrix");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view_matrix");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj_matrix");
	int texture_num_loc = glGetUniformLocation(shaderProgramID, "texture_num");

	//glBindTexture(GL_TEXTURE_2D, 0);
	int x = 20, y = 560;
	float r = 1.0, g = 1.0, b = 1.0;

	
	if (trans_car_z < 60)
		speed = 100 + trans_car_z;
	else
		speed += 0.01;
	
	speed_to_print = to_string(speed);
	speed_to_print = speed_to_print.substr(0, 5);
	//speed_to_print = 
	speed_to_print = "Speed: " + speed_to_print +" km/h";

	glUniform1i(texture_num_loc, -1);
	//glColor3f(r, g, b);
	glWindowPos2f(x, y);
	int len, i;
	len = speed_to_print.size();
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, speed_to_print[i]);
	}



	///////////////////////////////////// CAR SHELL, WINDOWS/ GRILL AND LIGHTS/ EXHAUST /////////////////////////////////////

	// Root of the Hierarchy - Car shell
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 100.0);
	mat4 model1 = identity_mat4();
	model1 = rotate_y_deg(model1, rotate_y_car);
	model1 = translate(model1, vec3(trans_car_x, trans_car_y, trans_car_z));
	
	view = translate(view, vec3(view_x, view_y, view_z));
	view = rotate_x_deg(view, rotate_camera_x);
	view = rotate_y_deg(view, rotate_camera_y);

	mat4 global1 = model1;

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1.m);
	glUniform1i(texture_num_loc, 0);
	glBindVertexArray(vao1);		
	glBindTexture(GL_TEXTURE_2D, textures[0]);							
	glDrawArrays(GL_TRIANGLES, 0, point_count1);

	// windows and grill
	mat4 modelWindowsGrill = identity_mat4();
	modelWindowsGrill = translate(modelWindowsGrill, vec3(0, -2, 0));

	mat4 global1a = global1 * modelWindowsGrill;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1a.m);
	glUniform1i(texture_num_loc, 1);
	glBindVertexArray(vao2);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 0, point_count2);

	// lights and exhaust of car
	mat4 modelLightsExhaust = identity_mat4();
	modelLightsExhaust = translate(modelLightsExhaust, vec3(0, 0, 0));

	mat4 global1b = global1 * modelLightsExhaust;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global1b.m);
	glUniform1i(texture_num_loc, 2);
	glBindVertexArray(vao8);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDrawArrays(GL_TRIANGLES, 0, point_count8);


	//////////////////////////////////// FRONT LEFT WHEEL /////////////////////////////////////

	// front left rim
	mat4 model_fl_rim = identity_mat4();
	model_fl_rim = rotate_x_deg(model_fl_rim, rotate_wheel_deg);
	model_fl_rim = translate(model_fl_rim, vec3(0.9, 0.65, 1.4));

	mat4 global_fl_rim = global1 * model_fl_rim;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global_fl_rim.m);
	glUniform1i(texture_num_loc, 2);
	glBindVertexArray(vao6);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDrawArrays(GL_TRIANGLES, 0, point_count6);


	// front left tire
	mat4 model_fl_tire = identity_mat4();
	model_fl_tire = rotate_x_deg(model_fl_tire, rotate_wheel_deg);
	model_fl_tire = translate(model_fl_tire, vec3(0.9, 0.65, 1.4));

	mat4 global_fl_tire = global1 * model_fl_tire;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global_fl_tire.m);
	glUniform1i(texture_num_loc, 1);
	glBindVertexArray(vao7);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 0, point_count7);

	//////////////////////////////////// FRONT RIGHT WHEEL /////////////////////////////////////

	// front right rim
	mat4 model_fr_rim = identity_mat4();
	model_fr_rim = rotate_y_deg(model_fr_rim, 180.0);
	model_fr_rim = rotate_x_deg(model_fr_rim, rotate_wheel_deg);
	model_fr_rim = translate(model_fr_rim, vec3(-0.7, 0.65, 1.4));

	mat4 global_fr_rim = global1 * model_fr_rim;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global_fr_rim.m);
	glUniform1i(texture_num_loc, 2);
	glBindVertexArray(vao6);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDrawArrays(GL_TRIANGLES, 0, point_count6);


	// front right tire
	mat4 model_fr_tire = identity_mat4();
	model_fr_tire = rotate_x_deg(model_fr_tire, rotate_wheel_deg);
	model_fr_tire = translate(model_fr_tire, vec3(-0.7, 0.65, 1.4));

	mat4 global_fr_tire = global1 * model_fr_tire;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global_fr_tire.m);
	glUniform1i(texture_num_loc, 1);
	glBindVertexArray(vao7);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 0, point_count7);

	//////////////////////////////////// BACK LEFT WHEEL /////////////////////////////////////

	// back left rim
	mat4 model_bl_rim = identity_mat4();
	model_bl_rim = rotate_x_deg(model_bl_rim, rotate_wheel_deg);
	model_bl_rim = translate(model_bl_rim, vec3(0.9, 0.65, -1.2));

	mat4 global_bl_rim = global1 * model_bl_rim;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global_bl_rim.m);
	glUniform1i(texture_num_loc, 2);
	glBindVertexArray(vao6);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDrawArrays(GL_TRIANGLES, 0, point_count6);


	// back left tire
	mat4 model_bl_tire = identity_mat4();
	model_bl_tire = rotate_x_deg(model_bl_tire, rotate_wheel_deg);
	model_bl_tire = translate(model_bl_tire, vec3(0.9, 0.65, -1.2));

	mat4 global_bl_tire = global1 * model_bl_tire;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global_bl_tire.m);
	glUniform1i(texture_num_loc, 1);
	glBindVertexArray(vao7);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 0, point_count7);

	//////////////////////////////////// BACK RIGHT WHEEL /////////////////////////////////////

	// back right rim
	mat4 model_br_rim = identity_mat4();
	model_br_rim = rotate_y_deg(model_br_rim, 180.0);
	model_br_rim = rotate_x_deg(model_br_rim, rotate_wheel_deg);
	model_br_rim = translate(model_br_rim, vec3(-0.7, 0.65, -1.2));

	mat4 global_br_rim = global1 * model_br_rim;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global_br_rim.m);
	glUniform1i(texture_num_loc, 2);
	glBindVertexArray(vao6);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDrawArrays(GL_TRIANGLES, 0, point_count6);


	// back right tire
	mat4 model_br_tire = identity_mat4();
	model_br_tire = rotate_x_deg(model_br_tire, rotate_wheel_deg);
	model_br_tire = translate(model_br_tire, vec3(-0.7, 0.65, -1.2));

	mat4 global_br_tire = global1 * model_br_tire;

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global_br_tire.m);
	glUniform1i(texture_num_loc, 1);
	glBindVertexArray(vao7);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 0, point_count7);

	////////////////////////////////////////// CACTUSES ////////////////////////////////////////

	// Cactuses mesh
	mat4 model_cactuses = identity_mat4();
	model_cactuses = translate(model_cactuses, vec3(0, 0, 0));
	model_cactuses = rotate_y_deg(model_cactuses, 90.0f);

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model_cactuses.m);
	glUniform1i(texture_num_loc, 3);
	glBindVertexArray(vao3);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glDrawArrays(GL_TRIANGLES, 0, point_count3);

	////////////////////////////////////////// ROAD ////////////////////////////////////////

	// Road mesh
	mat4 road_model = identity_mat4();
	road_model = translate(road_model, vec3(0, 0, 0));
	road_model = rotate_y_deg(road_model, 90.0f);

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, road_model.m);
	glBindVertexArray(vao9);
	glUniform1i(texture_num_loc, 5);

	glBindTexture(GL_TEXTURE_2D, textures[5]);
	glDrawArrays(GL_TRIANGLES, 0, point_count9);

	////////////////////////////////////////// SAND ////////////////////////////////////////

	// Sand mesh
	mat4 sand_model = identity_mat4();
	sand_model = translate(sand_model, vec3(0, 0, 0));
	sand_model = rotate_y_deg(sand_model, 90.0f);

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, sand_model.m);
	glBindVertexArray(vao4);
	glUniform1i(texture_num_loc, 4);

	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glDrawArrays(GL_TRIANGLES, 0, point_count4);

	////////////////////////////////////////// SKY BOX ////////////////////////////////////////

	// sky box mesh
	mat4 sky_model = identity_mat4();
	sky_model = translate(sky_model, vec3(0, 0, 0));
	sky_model = rotate_y_deg(sky_model, 90.0f);

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, sky_model.m);
	glBindVertexArray(vao10);
	glUniform1i(texture_num_loc, 6);

	glBindTexture(GL_TEXTURE_2D, textures[6]);
	glDrawArrays(GL_TRIANGLES, 0, point_count10);



	//////////////////////////////////////////// LIZARD /////////////////////////////////////////
	
	// Lizard mesh
	mat4 lizard_model = identity_mat4();
	lizard_model = rotate_z_deg(lizard_model, rotate_liz_z);
	lizard_model = rotate_y_deg(lizard_model, 270);
	lizard_model = translate(lizard_model, vec3(trans_liz_x, trans_liz_y, trans_liz_z));

	// update uniforms & draw
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, lizard_model.m);
	glUniform1i(texture_num_loc, 3);
	glBindVertexArray(vao5);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glDrawArrays(GL_TRIANGLES, 0, point_count5);

	glutSwapBuffers();
}

//float speed_of_car = 0.1;


void updateScene() {

	// Placeholder code, if you want to work with framerate
	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	// car movement 
	if (trans_car_z < -20 && trans_car_z > -30) {
		speed_of_car = 0.1;
	}
	else if (trans_car_z < -10 && trans_car_z > -20) {
		speed_of_car = 0.15;
	}
	else if (trans_car_z < 0 && trans_car_z > -10) {
		speed_of_car = 0.2;
	}
	else
		speed_of_car = 0.22;
	
	
	if (trans_car_z < 200) {
		trans_car_z += speed_of_car;
		rotate_wheel_deg += (speed_of_car * 200);
		cout << "trans_car_z = " << trans_car_z << endl;
	}
	/*else if (trans_car_x > -3) {
		trans_car_x -= 0.5;
		trans_car_z += 0.5;
		rotate_y_car += 15.0f;
	}
	else if (trans_car_z < -3){
		trans_car_z += speed_of_car;
		cout << "trans_car_z = " << trans_car_z << endl;
		rotate_y_car = 0.0f;
	}*/


	// lizard movement
	if (rotate_count % 4 == 0 && trans_liz_z < 3) {
		rotate_liz_z = 5.0f;
	}
	else {
		rotate_liz_z = -5.0f;
	}
	
	rotate_count++;

	if (trans_car_z > -10 && trans_liz_x > 3) {
		trans_liz_x -= 0.05f;
	}
	// Draw the next frame
	glutPostRedisplay();
}

void loadTextures(GLuint texture, const char* filepath, int active_arg, const GLchar* texString, int texNum) {
	int x, y, n;
	int force_channels = 4;
	unsigned char *image_data = stbi_load(filepath, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", filepath);

	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n",
			filepath);
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
	
	glActiveTexture(active_arg);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		image_data);
	glUniform1i(glGetUniformLocation(shaderProgramID, texString), texNum);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_REPEAT);
	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	//return true;
}

void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	//GLuint texture0;

	load_mesh(MESH_NAME1);
	glGenVertexArrays(1, &vao1);
	point_count1 = generateObjectBufferMesh(vao1);

	load_mesh(MESH_NAME2);
	glGenVertexArrays(1, &vao2);
	point_count2 = generateObjectBufferMesh(vao2);

	load_mesh(MESH_NAME3);
	glGenVertexArrays(1, &vao3);
	point_count3 = generateObjectBufferMesh(vao3);

	load_mesh(MESH_NAME4);
	glGenVertexArrays(1, &vao4);
	point_count4 = generateObjectBufferMesh(vao4);

	load_mesh(MESH_NAME5);
	glGenVertexArrays(1, &vao5);
	point_count5 = generateObjectBufferMesh(vao5);

	load_mesh(MESH_NAME6);
	glGenVertexArrays(1, &vao6);
	point_count6 = generateObjectBufferMesh(vao6);

	load_mesh(MESH_NAME7);
	glGenVertexArrays(1, &vao7);
	point_count7 = generateObjectBufferMesh(vao7);

	load_mesh(MESH_NAME8);
	glGenVertexArrays(1, &vao8);
	point_count8 = generateObjectBufferMesh(vao8);

	load_mesh(MESH_NAME9);
	glGenVertexArrays(1, &vao9);
	point_count9 = generateObjectBufferMesh(vao9);

	load_mesh(MESH_NAME10);
	glGenVertexArrays(1, &vao10);
	point_count10 = generateObjectBufferMesh(vao10);


	glGenTextures(7, textures);

	loadTextures(textures[0], "C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\red.jpg", GL_TEXTURE0, "redTexture", 0);

	loadTextures(textures[1], "C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\black1.jpg", GL_TEXTURE1, "blackTexture", 1);

	loadTextures(textures[2], "C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\grey.jpg", GL_TEXTURE2, "greyTexture", 2);

	loadTextures(textures[3], "C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\cactus_texture3.jpg", GL_TEXTURE3, "cactusTexture", 3);

	loadTextures(textures[4], "C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\sand_texture.jpg", GL_TEXTURE4, "sandTexture", 4);

	loadTextures(textures[5], "C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\road_texture.jpg", GL_TEXTURE5, "roadTexture", 5);

	loadTextures(textures[6], "C:\\Users\\Joseph\\Documents\\College\\4th Year\\CS4052_Graphics\\Lab5_Scene\\sky_texture.jpg", GL_TEXTURE6, "skyTexture", 6);

	//init_text_rendering("freemono.png", "freemono.meta", width, height);
	//// x and y are -1 to 1
	//// size_px is the maximum glyph size in pixels (try 100.0f)
	//// r,g,b,a are red,blue,green,opacity values between 0.0 and 1.0
	//// if you want to change the text later you will use the returned integer as a parameter
	//int hello_id = add_text("Hello world!", 0, 0, 100.0f, 1, 1, 1, 1);

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

	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}

