#pragma once
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

#include "Sphere.h"
#include "Plane.h"
#include "Triangle.h"
#include "objloader.h"
#include "Raytracer.h"
#include "Camera.h"
#include "Mesh.h"
#include "HitInfo.h"
#include "Light.h"

enum LocalState {
	STARTING,
	ACTIVE,
	INACTIVE
};

enum Effect {
	LIGHTING,
	SHADOWS,
	REFLECTIONS
};

class Raytracer
{
public:
	Raytracer();
	~Raytracer();

	bool Init(std::string _WindowName, unsigned int _WindowWidth, unsigned int _WindowHeight, SDL_RendererFlags _RenderingFlag);
	void Start();
	void CheckSDLError(int line);
	void Configure();
	void Render();
	void Update();
	void Deactivate();

	unsigned int CR_WindowWidth;
	unsigned int CR_WindowHeight;

	uint32_t RMask, GMask, BMask, AMask;

	glm::vec3 CR_AmbientColor;
	
	Light* CR_PointLight = NULL;
	float CR_SoftShadowSamples = 64;
	glm::vec3 CR_AreaLightSize;

	std::vector<Shape*> CR_ActiveObjects;
	std::vector<Shape*> CR_InactiveObjects;

	SDL_Window* CR_MainWindow = NULL;
	SDL_Surface* CR_ScreenSurface = NULL;

	Camera* CR_MainCamera = NULL;

	SDL_GLContext CR_OGL_Context;
	LocalState CR_CurrentState;

	float CR_ScreenAspectRatio;

	glm::vec3 Raytrace(glm::vec3 _RayOrigin, glm::vec3 _RayDirection, HitInfo& _HitInfo, int _CurrentDepth, int _MaxDepth);

	bool CR_Render_Meshes = false;

	bool CR_Effects_Hard_Shadows = true;
	bool CR_Effects_Soft_Shadows = true;
	bool CR_Effects_Reflections = true;

	void ToggleHardShadows();
	void ToggleSoftShadows();
	void ToggleReflections();

	bool LoadMesh(const char* _FilePath, glm::vec3 _AmbienctC, glm::vec3 _DiffuseC, glm::vec3 _SpecularC, float _Shininess);
	bool ToggleMesh();
};



