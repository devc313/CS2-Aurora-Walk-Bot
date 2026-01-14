#pragma once
#include "vector.hpp"
#include <map>
#include <string>
#include <mutex>
#include <vector>
#include <thread>
#include <map>
#include <cmath>
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include "driver.h"
#include "offsets.h"
struct Vector2 {
	float x, y;

	Vector2() : x(0), y(0) {}
	Vector2(float _x, float _y) : x(_x), y(_y) {}
};
struct view_matrix_t
{
	float matrix[4][4];
};


class CBones {
public:
	std::map<std::string, Vector3> bonePositions;
};



class CC4 {
public:
	uintptr_t get_planted();
	uintptr_t get_node();
	Vector3 get_origin();
};

class CPlayer {
public:
	uintptr_t entity;
	int team;
	uintptr_t pCSPlayerPawn;
	uintptr_t gameSceneNode;
	uintptr_t boneArray;
	uintptr_t spottedState;
	int health;
	int armor;
	std::string name;
	Vector3 origin;
	Vector3 head;
	CBones bones;
	bool is_defusing;
	bool visible;
	int32_t money;
	float flashAlpha;
	std::string weapon;
	void ReadBones();
	void ReadHead();
};

class CGame
{
public:
	DWORD_PTR base_client;
	DWORD_PTR base_engine;
	uintptr_t buildNumber;
	bool inGame;
	Vector3 localOrigin;
	std::string mapName;
	bool isC4Planted;
	int localTeam;
	uintptr_t entity_list;
	CC4 c4;
	std::vector<CPlayer> players = {};
	void init();
	void loop();
	void close();
	bool world_to_screen(const Vector3& worldPos, Vector2& screenPos, const view_matrix_t& viewMatrix);
	std::uint32_t localPlayerPawn;
	view_matrix_t view_matrix;
	uintptr_t localPlayer;
	uintptr_t localpCSPlayerPawn;
private:

	uintptr_t localList_entry2;
};

inline CGame g_game;

inline std::mutex reader_mutex;