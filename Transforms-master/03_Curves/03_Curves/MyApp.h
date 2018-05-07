#pragma once

// C++ includes
#include <memory>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "ProgramObject.h"
#include "BufferObject.h"
#include "VertexArrayObject.h"
#include "TextureObject.h"

#include "Mesh_OGL3.h"
#include "gCamera.h"

#include <vector>

class CMyApp
{
public:
	CMyApp(void);
	~CMyApp(void);

	bool Init();
	void Clean();

	void Update();
	void Render();

	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);
protected:
	// görbénk kiértékelése
	glm::vec3 Eval(float);
	float	m_currentParam{ 0 };

	// shaderekhez szükséges változók
	ProgramObject		m_program;		// shaderek programja
	ProgramObject		m_axesProgram;
	ProgramObject		m_pointProgram;

	Texture2D			m_textureMetal;

	VertexArrayObject	m_vao;			// VAO objektum
	IndexBuffer			m_gpuBufferIndices;		// indexek
	ArrayBuffer			m_gpuBufferPos;	// pozíciók tömbje
	ArrayBuffer			m_gpuBufferNormal;	// normálisok tömbje
	ArrayBuffer			m_gpuBufferTex;	// textúrakoordináták tömbje

	std::unique_ptr<Mesh>	m_mesh;

	gCamera				m_camera;

	const int kMaxPointCount = 10;

	float translation[3] = { 0.0f, 0.0f, 0.0f };

	float rotation[3]    = { 0.0f, 0.0f, 0.0f };
	float rotation_center[3]    = { 0.0f, 0.0f, 0.0f };	

	float scaling[3] = {1.0f, 1.0f , 1.0f };

	float skew_xy[2] = { 0.0f, 0.0f };
	float skew_xz[2] = { 0.0f, 0.0f };
	float skew_yz[2] = { 0.0f, 0.0f };

	glm::mat4 skew_xy_mat = glm::mat4(1, 0, 0, 0,
									  0, 1, 0, 0,
									  0, 0, 1, 0,
									  0, 0, 0, 1 );

	glm::mat4 skew_xz_mat = glm::mat4(1, 0, 0, 0,
									  0, 1, 0, 0,
									  0, 0, 1, 0,
									  0, 0, 0, 1);

	glm::mat4 skew_yz_mat = glm::mat4(1, 0, 0, 0,
									  0, 1, 0, 0,
									  0, 0, 1, 0,
									  0, 0, 0, 1);

	float central_s[3] = { 0.0f, 0.0f, 0.0f };

	glm::mat4 proj_cx_mat = glm::mat4(0, 0, 0, -1.0f,
									  0, 1, 0, 0,
									  0, 0, 1, 0,
									  0, 0, 0, 1);

	glm::mat4 proj_cy_mat = glm::mat4(1, 0, 0, 0,
									  0, 0, 0, -1.0f,
									  0, 0, 1, 0,
									  0, 0, 0, 1);

	glm::mat4 proj_cz_mat = glm::mat4(1, 0, 0, 0,
									  0, 1, 0, 0,
									  0, 0, 0, -1.0f,
									  0, 0, 0, 1);

	bool wipe_x = false;
	bool wipe_y = false;
	bool wipe_z = false;

	int e = 0;
	bool parallerl_proj_mode = false;

	glm::mat4 parallel_proj_x_mat = glm::mat4(0, 0, 0, 0,
											  0, 1, 0, 0,
											  0, 0, 1, 0,
											  0, 0, 0, 1);

	glm::mat4 parallel_proj_y_mat = glm::mat4(1, 0, 0, 0,
											  0, 0, 0, 0,
											  0, 0, 1, 0,
											  0, 0, 0, 1);

	glm::mat4 parallel_proj_z_mat = glm::mat4(1, 0, 0, 0,
											  0, 1, 0, 0,
											  0, 0, 0, 0,
											  0, 0, 0, 1);

	std::vector<glm::vec3>		m_controlPoints{ {-10,0,-10}, {10,0,-10} };
	std::vector<std::string>	m_pointName{ "Point 1", "Point 2" };
};

