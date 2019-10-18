#include "Raytracer.h"

Raytracer::Raytracer()
{
}

Raytracer::~Raytracer()
{
}

bool Raytracer::Init(std::string _WindowName, unsigned int _WindowWidth, unsigned int _WindowHeight, SDL_RendererFlags _RenderingFlag)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "WARNING: SDL could not initialize!" << std::endl;
		CheckSDLError(__LINE__);
		return 0;
	}

	CR_WindowWidth = _WindowWidth;
	CR_WindowHeight = _WindowHeight;
	CR_ScreenAspectRatio = CR_WindowWidth / CR_WindowHeight;

	CR_MainWindow = SDL_CreateWindow(_WindowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CR_WindowWidth, CR_WindowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (CR_MainWindow == nullptr) {
		std::cout << "WARNING: Window could not be created!" << std::endl;
		CheckSDLError(__LINE__);
		return 0;
	}

	CR_ScreenSurface = SDL_GetWindowSurface(CR_MainWindow);


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	RMask = 0xff000000;
	GMask = 0x00ff0000;
	BMask = 0x0000ff00;
	AMask = 0x000000ff;
#else
	RMask = 0x000000ff;
	GMask = 0x0000ff00;
	BMask = 0x00ff0000;
	AMask = 0xff000000;
#endif

	return 1;
}

void Raytracer::Start()
{
	CR_Active = true;
	Configure();
	Update();
}

void Raytracer::CheckSDLError(int line)
{
	std::string ErrorInfo = SDL_GetError();

	if (!ErrorInfo.empty()) {
		std::cout << "SDL Error: " << ErrorInfo << std::endl;

		if (line != -1) {
			std::cout << "Line: " << line << std::endl;
		}

		SDL_ClearError();
	}
}

void Raytracer::Configure()
{
	Sphere* TempRedSphere = new Sphere(glm::vec3(0.0f, 0.0f, -20.0f), 4.0f, glm::vec3(1.0f, 0.32f, 0.36f), glm::vec3(1.0f, 0.32f, 0.36f), glm::vec3(0.5f, 0.5f, 0.5f), 128.0f);
	Sphere* TempYellowSphere = new Sphere(glm::vec3(5.0f, -1.0f, -15.0f), 2.0f, glm::vec3(0.9f, 0.76f, 0.46f), glm::vec3(0.9f, 0.76f, 0.46f), glm::vec3(0.5f, 0.5f, 0.5f), 128.0f);
	Sphere* TempLightBlueSphere = new Sphere(glm::vec3(5.0f, 0.0f, -25.0f), 3.0f, glm::vec3(0.65f, 0.77f, 0.97f), glm::vec3(0.65f, 0.77f, 0.97f), glm::vec3(0.5f, 0.5f, 0.5f), 128.0f);
	Sphere* TempLightGreySphere = new Sphere(glm::vec3(-5.5f, 0.0f, -15.0f), 3.0f, glm::vec3(0.9f, 0.9f, 0.9f), glm::vec3(0.9f, 0.9f, 0.9f), glm::vec3(0.5f, 0.5f, 0.5f), 128.0f);

	Plane* TempPlane = new Plane(glm::vec3(0.0f, -4.0f, -20.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.5f, 0.5f, 0.5f), 128.0f);
	CR_ActiveObjects.push_back(TempRedSphere);
	CR_ActiveObjects.push_back(TempYellowSphere);
	CR_ActiveObjects.push_back(TempLightBlueSphere);
	CR_ActiveObjects.push_back(TempLightGreySphere);
	CR_ActiveObjects.push_back(TempPlane);

	CR_MainCamera = new Camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f), 1.0f, 0.25f);
	CR_PointLight = new Light(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	CR_AmbientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	CR_AreaLightSize = glm::vec3(9.0f, 0.1f, 9.0f);

	float CellsPerRow = glm::floor(glm::sqrt(CR_SoftShadowSamples));
	float CellSizeX = (CR_PointLight->Position.x - CR_AreaLightSize.x) / CellsPerRow;
	float CellSizeZ = (CR_PointLight->Position.z - CR_AreaLightSize.z) / CellsPerRow;
	for (int z = 0; z < CellsPerRow; ++z) {
		for (int x = 0; x < CellsPerRow; ++x) {
			CR_AreaLights.push_back(new Light(glm::vec3(CR_PointLight->Position.x + x * CellSizeX, CR_PointLight->Position.y, CR_PointLight->Position.z + z * CellSizeZ), CR_PointLight->ColorIntensity));
		}
	}
	//LoadMesh("../teapot_simple_smooth.obj", glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), 128.0f);
	//LoadMesh("../dragon.obj", glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), 128.0f);
	//LoadMesh("../bunny.obj", glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), 128.0f);
	//ToggleMesh();

	Render();
}

glm::vec3 Raytracer::Raytrace(glm::vec3 _RayOrigin, glm::vec3 _RayDirection, HitInfo& _Hit, int _CurrentDepth, int _MaxDepth)
{
	glm::vec3 CombinedColor = CR_AmbientColor;
	for (int i = 0; i < CR_ActiveObjects.size(); ++i) {
		CR_ActiveObjects[i]->CheckIntersection(_RayOrigin, _RayDirection, _Hit);
	}

	if (_Hit.HitStatus) {
		glm::vec3 AmbientColor = _Hit.AmbientC * CR_AmbientColor;
		glm::vec3 LightDirFull = CR_PointLight->Position - _Hit.IntPoint;
		glm::vec3 LightDir = glm::normalize(LightDirFull);

		float DiffuseScalar = glm::dot(LightDir, _Hit.Normal);
		glm::vec3 DiffuseColor = _Hit.DiffuseC * CR_PointLight->ColorIntensity * glm::max(0.0f, DiffuseScalar);
		glm::vec3 LightReflDir = glm::normalize(2 * -DiffuseScalar * _Hit.Normal + LightDir);
		float SpecularScalar = glm::dot(LightReflDir, _RayDirection);
		glm::vec3 SpecularColor = _Hit.SpecularC * CR_PointLight->ColorIntensity * glm::pow(glm::max(0.0f, SpecularScalar), _Hit.Shininess);
		CombinedColor = AmbientColor + DiffuseColor + SpecularColor;

		if (CR_Effects_Hard_Shadows) {
			HitInfo LightRayHit;
			glm::vec3 LightRayOrigin = _Hit.IntPoint + _Hit.Normal * 1e-4f;
			for (int i = 0; i < CR_ActiveObjects.size(); ++i) {
				CR_ActiveObjects[i]->CheckIntersection(LightRayOrigin, LightDir, LightRayHit);
			}

			if (LightRayHit.HitStatus && LightRayHit.Distance > 0.0f && LightRayHit.Distance < glm::length(LightDirFull)) {
				CombinedColor = AmbientColor;
			}
		}

		if (CR_Effects_Soft_Shadows) {
			std::vector<HitInfo> AreaLightHits;
			glm::vec3 LightRayOrigin = _Hit.IntPoint + _Hit.Normal * 1e-4f;
			for (int s = 0; s < CR_AreaLights.size(); ++s) {
				HitInfo LightRayHit;
				
				LightDirFull = CR_AreaLights[s]->Position - _Hit.IntPoint;
				LightDir = glm::normalize(LightDirFull);

				for (int i = 0; i < CR_ActiveObjects.size(); ++i) {
					CR_ActiveObjects[i]->CheckIntersection(LightRayOrigin, LightDir, LightRayHit);
				}
				if (LightRayHit.HitStatus && LightRayHit.Distance > 0 && LightRayHit.Distance < glm::length(LightDirFull)) {
					AreaLightHits.push_back(LightRayHit);
				}
			}

			float DiffuseScalar = glm::dot(LightDir, _Hit.Normal);
			glm::vec3 DiffuseColor = _Hit.DiffuseC * CR_PointLight->ColorIntensity * glm::max(0.0f, DiffuseScalar);
			glm::vec3 LightReflDir = glm::normalize(2 * -DiffuseScalar * _Hit.Normal + LightDir);
			float SpecularScalar = glm::dot(LightReflDir, _RayDirection);
			glm::vec3 SpecularColor = _Hit.SpecularC * CR_PointLight->ColorIntensity * glm::pow(glm::max(0.0f, SpecularScalar), _Hit.Shininess);

			glm::vec3 ShadingColor = AmbientColor + DiffuseColor + SpecularColor;

			CombinedColor = (float)AreaLightHits.size() * AmbientColor + (float)(CR_AreaLights.size() - AreaLightHits.size()) * ShadingColor;

			CombinedColor *= 1.0f / CR_AreaLights.size();
		}
		if (CR_Effects_Reflections && _CurrentDepth < _MaxDepth) {
			HitInfo ReflRayHit;
			return CombinedColor + _Hit.SpecularC * Raytrace(_Hit.ReflRayOrigin, _Hit.ReflRayDir, ReflRayHit, _CurrentDepth += 1, _MaxDepth);
		}
	}
	
	return CombinedColor;
}

void Raytracer::Render()
{
	auto StartTime = std::chrono::high_resolution_clock::now();

	CR_ScreenSurface = SDL_GetWindowSurface(CR_MainWindow);
	CR_ScreenAspectRatio = CR_ScreenSurface->w / CR_ScreenSurface->h;
	SDL_Surface* BufferSurface = SDL_CreateRGBSurface(0, CR_ScreenSurface->w, CR_ScreenSurface->h, 32, RMask, GMask, BMask, AMask);

	if (BufferSurface != NULL) {
		uint32_t* PixelAddress = (uint32_t*)BufferSurface->pixels;
		if (SDL_MUSTLOCK(BufferSurface)) {
			SDL_LockSurface(BufferSurface);
		}
		int OffsetMod = BufferSurface->pitch * 0.25f;
		float FOV_Angle = glm::tan(glm::radians(90.0f * 0.5f));
		for (int y = 0; y < CR_ScreenSurface->h; ++y) {

			int LineOffset = y * OffsetMod;

			float PixelNormalizedy = (y + 0.5f) * (1.0f / CR_ScreenSurface->h);
			float PixelRemappedy = 1.0f - 2.0f * PixelNormalizedy;
			float PixelCameray = PixelRemappedy * FOV_Angle;

			for (int x = 0; x < CR_ScreenSurface->w; ++x) {

				/*Uint8* PixelAddress = (Uint8*)ImageSurface->pixels + y * ImageSurface->pitch + x * ImageSurface->format->BytesPerPixel;*/
				/*int LineOffset = y * (ImageSurface->pitch / sizeof(uint32_t));*/				

				float PixelNormalizedx = (x + 0.5f) / CR_ScreenSurface->w;
				float PixelRemappedx = (2.0f * PixelNormalizedx - 1.0f) * CR_ScreenAspectRatio;
				float PixelCamerax = PixelRemappedx * FOV_Angle;

				glm::vec3 PixelCameraSpacePos(PixelCamerax + CR_MainCamera->Position.x, PixelCameray + CR_MainCamera->Position.y, CR_MainCamera->Position.z - 1.0f);
				glm::vec3 RayDirection = glm::normalize(PixelCameraSpacePos - CR_MainCamera->Position);

				Uint32 ColorBitValue = 0;
				HitInfo Hit;
				glm::vec3 CombinedColor = Raytrace(CR_MainCamera->Position, RayDirection, Hit, 1, 4);
				ColorBitValue = SDL_MapRGB(BufferSurface->format, glm::clamp(CombinedColor.r * 255.0f, 0.0f, 255.0f), glm::clamp(CombinedColor.g * 255.0f, 0.0f, 255.0f), glm::clamp(CombinedColor.b * 255.0f, 0.0f, 255.0f));
				PixelAddress[LineOffset + x] = ColorBitValue;
			}
		}

		if (SDL_MUSTLOCK(BufferSurface)) {
			SDL_UnlockSurface(BufferSurface);
		}

		SDL_Surface* OptimizedSurface = SDL_ConvertSurface(BufferSurface, CR_ScreenSurface->format, 0);
		SDL_BlitSurface(OptimizedSurface, NULL, CR_ScreenSurface, NULL);
		SDL_FreeSurface(OptimizedSurface);
		OptimizedSurface = NULL;
		delete OptimizedSurface;
		SDL_FreeSurface(BufferSurface);
		BufferSurface = NULL;
		delete BufferSurface;
		SDL_UpdateWindowSurface(CR_MainWindow);
		//std::cout << "Unoptimized Format: " << SDL_GetPixelFormatName(CR_BufferSurface->format->format) << std::endl;
		//std::cout << "Optmized Format: " << SDL_GetPixelFormatName(OptimizedSurface->format->format) << std::endl;
	}
	else {
		std::cout << "WARNING: Unable to render surface! " << std::endl;
		CheckSDLError(__LINE__);
	}

	auto EndTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed_seconds = EndTime - StartTime;
	std::cout << "Elapsed time: " << elapsed_seconds.count() << " seconds." << std::endl;
}

void Raytracer::Update()
{
	while (CR_Active) {
		SDL_Event CurrentEvent;
		while (SDL_PollEvent(&CurrentEvent)) {
			if (CurrentEvent.type == SDL_QUIT) {
				CR_Active = false;
				break;
			}
			if (CurrentEvent.type == SDL_WINDOWEVENT) {
				switch (CurrentEvent.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					Render();
					break;

				case SDL_WINDOWEVENT_MAXIMIZED:
					Render();
					break;
				default:
					break;
				}
			}
			if (CurrentEvent.type == SDL_KEYDOWN) {
				switch (CurrentEvent.key.keysym.sym) {
				case SDLK_ESCAPE:
					std::cout << "Hotkey Pressed: [ESC]" << std::endl;
					CR_Active = false;
					break;
				case SDLK_w:
					CR_MainCamera->SetPosition(CR_MainCamera->Position + glm::vec3(0.0f, 0.0f, -1.0f));
					std::cout << "Hotkey Pressed: [W]" << std::endl;
					Render();
					break;
				case SDLK_a:
					CR_MainCamera->SetPosition(CR_MainCamera->Position + glm::vec3(-1.0f, 0.0f, 0.0f));
					std::cout << "Hotkey Pressed: [A]" << std::endl;
					Render();	
					break;
				case SDLK_d:
					CR_MainCamera->SetPosition(CR_MainCamera->Position + glm::vec3(1.0f, 0.0f, 0.0f));
					std::cout << "Hotkey Pressed: [D]" << std::endl;
					Render();
					break;
				case SDLK_s:
					CR_MainCamera->SetPosition(CR_MainCamera->Position + glm::vec3(0.0f, 0.0f, 1.0f));
					std::cout << "Hotkey Pressed: [S]" << std::endl;
					Render();
					break;
				case SDLK_1:
					std::cout << "Hotkey Pressed: [1]" << std::endl;
					ToggleHardShadows();
					std::cout << "Hard Shadows: " << CR_Effects_Hard_Shadows << std::endl;
					break;
				case SDLK_2:
					std::cout << "Hotkey Pressed: [2]" << std::endl;
					ToggleSoftShadows();
					std::cout << "Soft Shadows: " << CR_Effects_Soft_Shadows << std::endl;
					break;
				case SDLK_3:
					std::cout << "Hotkey Pressed: [2]" << std::endl;
					ToggleReflections();
					std::cout << "Reflections: " << CR_Effects_Reflections << std::endl;
					break;
				default:
					break;
				}
			}
		}
		SDL_UpdateWindowSurface(CR_MainWindow);
	}
}

void Raytracer::ToggleHardShadows()
{
	CR_Effects_Hard_Shadows = !CR_Effects_Hard_Shadows;
	CR_Effects_Soft_Shadows = false;
	Render();
}

void Raytracer::ToggleSoftShadows()
{
	CR_Effects_Soft_Shadows = !CR_Effects_Soft_Shadows;
	CR_Effects_Hard_Shadows = false;
	Render();
}

void Raytracer::ToggleReflections()
{
	CR_Effects_Reflections = !CR_Effects_Reflections;
	Render();
}

bool Raytracer::LoadMesh(const char* _FilePath, glm::vec3 _AmbienctC, glm::vec3 _DiffuseC, glm::vec3 _SpecularC, float _Shininess)
{
	std::vector<glm::vec3> MeshVertices;
	std::vector<glm::vec3> MeshNormals;
	if (!loadOBJ(_FilePath, MeshVertices, MeshNormals)) {
		return false;
	}
	Mesh* TempMesh = new Mesh(MeshVertices, MeshNormals, _AmbienctC, _DiffuseC, _SpecularC, _Shininess);
	CR_ActiveObjects.push_back(TempMesh);
	
	return true;
}

bool Raytracer::ToggleMesh()
{
	CR_Render_Meshes = !CR_Render_Meshes;
	Mesh* TempMesh = new Mesh();
	//std::vector<Shape*>::iterator startIt = CR_Shapes.begin();
	//for (; startIt != CR_Shapes.end(); ++startIt) {
	//	if (*startIt == TempMesh) {
	//		CR_InactiveObjects.push_back(*startIt);
	//		CR_Shapes.erase(startIt, CR_Shapes.end());
	//	}
	//}

	//auto e = std::find(CR_Shapes.begin(), CR_Shapes.end(), (Mesh*))
	//CR_InactiveObjects.push_back();
	return CR_Render_Meshes;
}

void Raytracer::Deactivate()
{
	for (Light* L : CR_AreaLights) {
		delete L;
	}

	for (Shape* S : CR_ActiveObjects) {
		delete S;
	}

	for (Shape* S : CR_InactiveObjects) {
		delete S;
	}

	CR_PointLight = NULL;
	delete CR_PointLight;
	CR_MainCamera = NULL;
	delete CR_MainCamera;
	SDL_FreeSurface(CR_ScreenSurface);
	CR_ScreenSurface = NULL;
	delete CR_ScreenSurface;
	SDL_DestroyWindow(CR_MainWindow);
	CR_MainWindow = NULL;
	delete CR_MainWindow;

	SDL_Quit();
}



