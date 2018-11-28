// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <wrl/client.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = nullptr; } }
#define SAFE_DELETE(p) { if(p) { delete (p); (p) = nullptr; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[](p); (p) = nullptr; } }

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib") 

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11_2.h>
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include <wincodec.h>
#include <ScreenGrab.h>

#include <WICTextureLoader.h>

#include <string>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>
#include <thread>
#include <algorithm>
#include "vecquat.h"

extern char key_pressed;
extern int control;
extern float universe_scale;
extern bool switched_shadows;
extern bool switched_object;
extern float dt_a;
extern float dt_b;
extern int time_acceleration;
extern int star_object;
extern float update_messages_interval;
extern float seconds_since_last_messages_update;
extern int total_objects;
extern vec3 aether_velocity;

extern ID3D11Device* D3D_device;
extern ID3D11DeviceContext* D3D_context;
extern HWND m_hwnd;

struct Config_
{
	Config_()
	{
		total_initial_setup_objects = 0;
		draw_stars = false;
		draw_orbits = false;
		draw_constellations = false;
		draw_shadows = true;
		caption_on = false;
		changed_scale = false;
		camera_locked = true;
		camera_changed = false;
		changed_attitude = false;
		create_projected_sentences = false;
		create_screen_sentences = false;
		system_initialized = false;
		bullet_initialized = false;
		kill_auto_grapple = false;
		command = "";
		display_info = "N/A";
	};

	int total_initial_setup_objects;
	bool draw_stars;
	bool changed_scale;
	bool caption_on;
	bool draw_orbits;
	bool draw_constellations;
	bool draw_shadows;
	bool create_projected_sentences;
	bool create_screen_sentences;
	bool system_initialized;
	bool bullet_initialized;
	bool changed_attitude;
	char* command;
	bool camera_locked;
	bool camera_changed;
	bool kill_auto_grapple;
	std::string display_info;
};

struct robotic_arm_struct
{
	robotic_arm_struct()
	{
		total_combos = 0;
		handled_part = 0;
		size = 0;
	};

	std::vector<int> members;
	std::vector<bool> second_runs;
	std::vector<int> combos;
	int total_combos;
	int handled_part;
	int size;
};

extern std::vector<robotic_arm_struct> robotic_arm;


extern Config_ Config;