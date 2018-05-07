#include "MyApp.h"

#include <math.h>
#include <vector>

#include <array>
#include <list>
#include <tuple>

#include "imgui\imgui.h"

#include "ObjParser_OGL3.h"

CMyApp::CMyApp(void)
{
}


CMyApp::~CMyApp(void)
{
	
}

bool CMyApp::Init()
{
	// törlési szín legyen fehér
	glClearColor(0.9, 0.9, 0.8, 1);

	glEnable(GL_CULL_FACE); // kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST); // mélységi teszt bekapcsolása (takarás)
	//glDepthMask(GL_FALSE); // kikapcsolja a z-pufferbe történő írást - https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glDepthMask.xml

	glLineWidth(4.0f); // a vonalprimitívek vastagsága: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glLineWidth.xhtml
	//glEnable(GL_LINE_SMOOTH);

	// átlátszóság engedélyezése
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // meghatározza, hogy az átlátszó objektum az adott pixelben hogyan módosítsa a korábbi fragmentekből oda lerakott színt: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBlendFunc.xhtml

	// a raszterizált pontprimitívek mérete
	glPointSize(15.0f);

	//
	// shaderek betöltése
	//

	// így is létre lehetne hozni:

	// a shadereket tároló program létrehozása
	m_program.Init({ 
		{ GL_VERTEX_SHADER, "myVert.vert"},
		{ GL_FRAGMENT_SHADER, "myFrag.frag"}
	},
	{ 
		{ 0, "vs_in_pos" },					// VAO 0-as csatorna menjen a vs_in_pos-ba
		{ 1, "vs_in_normal" },				// VAO 1-es csatorna menjen a vs_in_normal-ba
		{ 2, "vs_out_tex0" },				// VAO 2-es csatorna menjen a vs_in_tex0-ba
	});

	m_program.LinkProgram();

	// tengelyeket kirajzoló shader rövid inicializálása
	m_axesProgram.Init({
		{GL_VERTEX_SHADER, "axes.vert"},
		{GL_FRAGMENT_SHADER, "axes.frag"}
	});
	// haladóknak, még rövidebb: 
	//m_axesProgram.Init({"axes.vert"_vs, "axes.frag"_fs});

	// kontrollpontokat kirajzoló program
	m_pointProgram.Init({
		{ GL_VERTEX_SHADER, "pointDrawer.vert" },
		{ GL_FRAGMENT_SHADER, "pointDrawer.frag" }
	});

	//
	// geometria definiálása (std::vector<...>) és GPU pufferekbe (m_buffer*) való feltöltése BufferData-val
	//

	// vertexek pozíciói:
	/*
	Az m_gpuBufferPos konstruktora már létrehozott egy GPU puffer azonosítót és a most következő BufferData hívás ezt 
		1. bind-olni fogja GL_ARRAY_BUFFER target-re (hiszen m_gpuBufferPos típusa ArrayBuffer) és
		2. glBufferData segítségével áttölti a GPU-ra az argumentumban adott tároló értékeit

	*/
	m_gpuBufferPos.BufferData( 
		std::vector<glm::vec3>{
			//		  X, Y, Z
			glm::vec3(-10,-1, -10),
			glm::vec3(-10,-1,  10), 
			glm::vec3( 10,-1, -10),
			glm::vec3( 10,-1,  10) 
		}
	);

	// normálisai
	m_gpuBufferNormal.BufferData(
		std::vector<glm::vec3>{
			// normal.X,.Y,.Z
			glm::vec3(0, 1, 0), // 0-ás csúcspont
			glm::vec3(0, 1, 0), // 1-es
			glm::vec3(0, 1, 0), // 2-es
			glm::vec3(0, 1, 0)  // 3-as
		}
	);

	// textúrakoordinátái
	m_gpuBufferTex.BufferData(
		std::vector<glm::vec2>{
			//        u, v
			glm::vec2(0, 0), // 0-ás csúcspont
			glm::vec2(1, 0), // 1-es
			glm::vec2(0, 1), // 2-es
			glm::vec2(1, 1)	 // 3-as
		}
	);

	// és a primitíveket alkotó csúcspontok indexei (az előző tömbökből) - triangle list-el való kirajzolásra felkészülve
	m_gpuBufferIndices.BufferData(
		std::vector<int>{
			0,1,2,	// 1. háromszög: 0-1-2 csúcspontokból
			2,1,3	// 2. háromszög: 2-1-3 csúcspontokból
		}
	);

	// geometria VAO-ban való regisztrálása
	m_vao.Init(
		{
			{ CreateAttribute<0, glm::vec3, 0, sizeof(glm::vec3)>, m_gpuBufferPos },	// 0-ás attribútum "lényegében" glm::vec3-ak sorozata és az adatok az m_gpuBufferPos GPU pufferben vannak
			{ CreateAttribute<1, glm::vec3, 0, sizeof(glm::vec3)>, m_gpuBufferNormal },	// 1-es attribútum "lényegében" glm::vec3-ak sorozata és az adatok az m_gpuBufferNormal GPU pufferben vannak
			{ CreateAttribute<2, glm::vec2, 0, sizeof(glm::vec2)>, m_gpuBufferTex }		// 2-es attribútum "lényegében" glm::vec2-ők sorozata és az adatok az m_gpuBufferTex GPU pufferben vannak
		},
		m_gpuBufferIndices
	);

	// textúra betöltése
	m_textureMetal.FromFile("texture.png");

	// mesh betöltése
	m_mesh = ObjParser::parse("suzanne.obj");

	// kamera
	m_camera.SetProj(45.0f, 1280.0f / 720.0f, 0.01f, 1000.0f);

	return true;
}

void CMyApp::Clean()
{
}

bool cucc(glm::mat4 &transform)
{
	glm::mat4 t = glm::transpose(transform);
	bool changed = ImGui::DragFloat4("t1", &t[0].x, 0.2f);
	changed |= ImGui::DragFloat4("t2", &t[1].x, 0.2f);
	changed |= ImGui::DragFloat4("t3", &t[2].x, 0.2f);
	changed |= ImGui::DragFloat4("t4", &t[3].x, 0.2f);
	if (changed) transform = glm::transpose(t); 
	return changed;
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time) / 1000.0f;

	m_camera.Update(delta_time);

	last_time = SDL_GetTicks();
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT) és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// tengelyek
	m_axesProgram.Use();
	m_axesProgram.SetUniform("mvp", m_camera.GetViewProj()*glm::translate(glm::vec3(0,0.1f,0)));

	glDrawArrays(GL_LINES, 0, 6);

	// kontrollpontok
	m_pointProgram.Use();
	
	m_pointProgram.SetUniform("mvp", m_camera.GetViewProj());
	m_pointProgram.SetUniform("points", m_controlPoints);
	m_pointProgram.SetUniform("color", glm::vec4(1,0,1,1));

	glDrawArrays(GL_POINTS, 0, (GLsizei)m_controlPoints.size());

	// kontrollpontokat összekötő szakaszok
	m_pointProgram.SetUniform("color", glm::vec4(0.5, 0.5, 0.5, 1));
	glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)m_controlPoints.size());

	// mindegyik geometria ugyanazt a shader programot hasznalja: ne kapcsolgassuk most ki-be
	m_program.Use();

	m_program.SetTexture("texImage", 0, m_textureMetal);

	// talaj
	glm::mat4 wallWorld = glm::translate(glm::vec3(0, 0, 0));
	m_program.SetUniform("world", wallWorld);
	m_program.SetUniform("worldIT", glm::transpose(glm::inverse(wallWorld)));
	m_program.SetUniform("MVP", m_camera.GetViewProj()*wallWorld);
	m_program.SetUniform("Kd", glm::vec4(1,1,1, 1));

	m_vao.Bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	m_vao.Unbind(); // nem feltétlen szükséges: a m_mesh->draw beállítja a saját VAO-ját

	skew_xy_mat[2][0] = skew_xy[0];
	skew_xy_mat[2][1] = skew_xy[1];

	skew_xz_mat[1][0] = skew_xz[0];
	skew_xz_mat[1][2] = skew_xz[1];

	skew_yz_mat[0][1] = skew_yz[0];
	skew_yz_mat[0][2] = skew_yz[1];

	// Suzanne 1
	

	if (central_s[0] > 0.005f || central_s[0] < -0.005f)
	{
		proj_cx_mat[0][3] = -1.0f / central_s[0];
	}
	if (central_s[1] > 0.005f || central_s[1] < -0.005f)
	{
		proj_cy_mat[1][3] = -1.0f / central_s[1];
	}
	if (central_s[2] > 0.005f || central_s[2] < -0.005f)
	{
		proj_cz_mat[2][3] = -1.0f / central_s[2];
	}


	glm::mat4 suzanne1World =
		glm::scale(glm::vec3(scaling[0], scaling[1], scaling[2]))
		*
		glm::translate(glm::vec3(translation[0], translation[1], translation[2]))
		*
		glm::rotate(rotation[0], glm::vec3(1, 0, 0))
		*
		glm::rotate(rotation[1], glm::vec3(0, 1, 0))
		*
		glm::rotate(rotation[2], glm::vec3(0, 0, 1))
		*
		glm::translate(glm::vec3(rotation_center[0], rotation_center[1], rotation_center[2]))
		*
		skew_yz_mat
		*
		skew_xz_mat
		*
		skew_xy_mat
		;

	if (!(central_s[0] < 0.005f && central_s[0] > -0.005f))
	{
		suzanne1World *= proj_cx_mat;
	}
	if (!(central_s[1] < 0.005f && central_s[1] > -0.005f))
	{
		suzanne1World *= proj_cy_mat;
	}
	if (!(central_s[2] < 0.005f && central_s[2] > -0.005f))
	{
		suzanne1World *= proj_cz_mat;
	}

	if (wipe_x)
		suzanne1World[0] = glm::vec4(glm::vec3(0.0f), 0);
	if (wipe_y)
		suzanne1World[1] = glm::vec4(glm::vec3(0.0f), 0);
	if (wipe_z)
		suzanne1World[2] = glm::vec4(glm::vec3(0.0f), 0);

	glm::vec3 lookat_norm = glm::normalize(m_camera.GetAt() - m_camera.GetEye());
	ImGui::InputFloat3("asdf", &lookat_norm[0]);
	if (parallerl_proj_mode && e == 0)
	{

		parallel_proj_x_mat[0][1] = -lookat_norm.y / lookat_norm.x;
		parallel_proj_x_mat[0][2] = -lookat_norm.z / lookat_norm.x;
		suzanne1World *= (parallel_proj_x_mat);
		cucc(suzanne1World);

	}
	else if (parallerl_proj_mode && e == 1)
	{
		parallel_proj_y_mat[1][0] = -lookat_norm.x / lookat_norm.y;
		parallel_proj_y_mat[1][2] = -lookat_norm.z / lookat_norm.y;
		suzanne1World *= (parallel_proj_y_mat);
	}
	else if (parallerl_proj_mode && e == 2)
	{
		parallel_proj_z_mat[2][0] = -lookat_norm.x / lookat_norm.z;
		parallel_proj_z_mat[2][1] = -lookat_norm.y / lookat_norm.z;
		suzanne1World *= (parallel_proj_z_mat);
	}


	m_program.SetUniform("world", suzanne1World);
	m_program.SetUniform("worldIT", glm::transpose(glm::inverse(suzanne1World)));
	m_program.SetUniform("MVP", m_camera.GetViewProj()*suzanne1World);
	m_program.SetUniform("Kd", glm::vec4(1, 0.3, 0.3, 1));
	
	m_mesh->draw();

	// végetért a 3D színtér rajzolása
	m_program.Unuse();

	//
	// UI
	//
	// A következő parancs megnyit egy ImGui tesztablakot és így látszik mit tud az ImGui.
	//ImGui::ShowTestWindow();
	// A ImGui::ShowTestWindow implementációja az imgui_demo.cpp-ben található
	// Érdemes még az imgui.h-t böngészni, valamint az imgui.cpp elején a FAQ-ot elolvasni.
	// Rendes dokumentáció nincs, de a fentiek elegendőek kell legyenek.
	
	ImGui::SetNextWindowPos(ImVec2(2, 2), ImGuiSetCond_FirstUseEver);
	// csak akkor lépjünk be, hogy ha az ablak nincs csíkká lekicsinyítve...
	if (ImGui::Begin("Transformations:"))
	{
		/*
		static size_t currentItem = 0;

		ImGui::ListBoxHeader("List", 5);
		for (size_t i = 0; i < m_pointName.size(); ++i)
		{
			if (ImGui::Selectable(m_pointName[i].c_str(), (i==currentItem))  )
				currentItem = i;
		}
		ImGui::ListBoxFooter();

		if (ImGui::Button("Add") && m_pointName.size() < kMaxPointCount)
		{
			m_pointName.push_back("Point " + std::to_string(m_pointName.size()+1));
			m_controlPoints.push_back({ 0,0,0 });
			currentItem = m_pointName.size() - 1;
		}

		ImGui::SameLine();

		if (ImGui::Button("Delete") && m_pointName.size() > 0 && currentItem < m_pointName.size() )
		{
			m_pointName.erase(m_pointName.begin() + currentItem);
			m_controlPoints.erase(m_controlPoints.begin() + currentItem);

			size_t number = currentItem+1;
			for (auto& it = m_pointName.begin()+ currentItem; it != m_pointName.end(); ++it)
			{
				*it = "Point " + std::to_string(number);
				++number;
			}
			if (m_pointName.size() == 0)
				currentItem = 0;
			else
				if (currentItem >= m_pointName.size())
					currentItem = m_pointName.size() - 1;
		}

		if (m_controlPoints.size() > 0)
			ImGui::SliderFloat3("Coordinates", &(m_controlPoints[currentItem][0]), -10, 10);

		ImGui::SliderFloat("Parameter", &m_currentParam, 0, (float)(m_controlPoints.size()-1));
		*/

		ImGui::SliderFloat3("Scaling", &scaling[0], -10, 10);
		ImGui::SliderFloat3("Translation", &translation[0], -10, 10);
		ImGui::SliderAngle("Rotate x", &rotation[0]);
		ImGui::SliderAngle("Rotate y", &rotation[1]);
		ImGui::SliderAngle("Rotate z", &rotation[2]);
		ImGui::SliderFloat3("Rotation center", &rotation_center[0], -10, 10);
		ImGui::SliderFloat2("Skew x-y", &skew_xy[0], -10, 10);
		ImGui::SliderFloat2("Skew x-z", &skew_xz[0], -10, 10);
		ImGui::SliderFloat2("Skew y-z", &skew_yz[0], -10, 10);

		ImGui::Checkbox("Proj to x", &wipe_x);
		ImGui::SameLine();
		ImGui::Checkbox("Proj to y", &wipe_y);
		ImGui::SameLine();
		ImGui::Checkbox("Proj to z", &wipe_z);

		ImGui::DragFloat3("Central projection values", &central_s[0], 0.005f, -10, 10);
		ImGui::Checkbox("Parallel projection", &parallerl_proj_mode);

		if(parallerl_proj_mode)
		{
			ImGui::RadioButton("Onto x plane", &e, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Onto y plane", &e, 1);
			ImGui::SameLine();
			ImGui::RadioButton("Onto z plane", &e, 2);
		}

		if (ImGui::Button("Reset", ImVec2(-1,0)))
		{
			for (int i = 0; i < 3; i++)
			{
				translation[i] = 0.0f;
				rotation[i] = 0.0f;
				rotation_center[i] = 0.0f;
				scaling[i] = 1.0f;
				central_s[i] = 0.0f;
			}

			for (int i = 0; i < 2; i++)
			{
				skew_xy[i] = 0.0f;
				skew_xz[i] = 0.0f;
				skew_yz[i] = 0.0f;
			}

			wipe_x = false;
			wipe_y = false;
			wipe_z = false;

			parallerl_proj_mode = false;

		}


		// 1. feladat: Suzanne feje mindig forduljon a menetirány felé! Először ezt elég síkban (=XZ) megoldani!
		// 2. feladat: valósíts meg egy Animate gombot! Amíg nyomva van az m_currentParameter periodikusan változzon a [0,m_controlPoints.size()-1] intervallumon belül!
		// 3. feladat: egyenközű Catmull-Rom spline-nal valósítsd meg Suzanne görbéjét a mostani törött vonal helyett!

	}
	ImGui::End(); // ...de még ha le is volt, End()-et hívnunk kell
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h );

	m_camera.Resize(_w, _h);
}

glm::vec3 CMyApp::Eval(float t)
{
	if (m_controlPoints.size() == 0)
		return glm::vec3(0);

	int interval = (int)t;

	if (interval < 0)
		return m_controlPoints[0];
		
	if (interval >= m_controlPoints.size()-1)
		return m_controlPoints[m_controlPoints.size()-1];

	float localT = t - interval;
	return (1- localT)*m_controlPoints[interval] + localT*m_controlPoints[interval+1];
}
