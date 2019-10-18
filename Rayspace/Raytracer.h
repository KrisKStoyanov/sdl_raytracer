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

class Raytracer
{
public:
	Raytracer();
	~Raytracer();

	bool CR_Active = false;

	bool Init(const char* _WindowName, int _WindowWidth, int _WindowHeight);
	void Start();
	void CheckSDLError(int line);
	void Configure();
	void Render();
	void Update();
	void Deactivate();

	uint32_t RMask, GMask, BMask, AMask;

	glm::vec3 CR_AmbientColor;
	
	Light* CR_PointLight = NULL;
	float CR_SoftShadowSamples = 64;
	glm::vec3 CR_AreaLightSize;
	std::vector<Light*> CR_AreaLights;

	std::vector<Shape*> CR_ActiveObjects;

	SDL_Window* CR_MainWindow = NULL;
	Camera* CR_MainCamera = NULL;

	glm::vec3 Raytrace(glm::vec3 _RayOrigin, glm::vec3 _RayDirection, int _CurrentDepth, int _MaxDepth);

	bool CR_Mesh_Rendering = false;
	bool CR_Effects_Hard_Shadows = true;
	bool CR_Effects_Soft_Shadows = true;
	bool CR_Effects_Reflections = true;

	void ToggleMeshRendering();
	void ToggleHardShadows();
	void ToggleSoftShadows();
	void ToggleReflections();

	bool LoadMesh(const char* _FilePath, glm::vec3 _AmbienctC, glm::vec3 _DiffuseC, glm::vec3 _SpecularC, float _Shininess);
};



