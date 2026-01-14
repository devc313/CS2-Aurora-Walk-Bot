#pragma once
#include "nav_file.h"
#include <deque>

#include <iostream>
#include "VisCheck.h"
#include "reader.hpp"
#include <vector>
#include <cstdlib> 
#include <ctime>  


class WalkBot
{
public:
	void Run();
	bool DoesNeedNewPath();
	bool LevelCheck();
	void OptimizePath(Vector3 vPlayerPos);
	
	nav_mesh::nav_area* GetAreaNearEnemies();
	size_t m_iPathIndex = 0;
	uintptr_t runcheck();
	std::unique_ptr<VisCheck> visCheck;
	nav_mesh::nav_file* m_pNavFile = nullptr;
	std::vector<nav_mesh::vec3_t> m_vAreasPath{};
	nav_mesh::nav_area* m_pCurrentTarget{};
	std::deque<float> m_vDeltaAnglesHist{ 30, 999.0f };
	nav_mesh::nav_area* GetAreaNearPlayer(Vector3 origin);

private:

};

