#include "walkbot.h"
#define NOMINMAX
#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <string>

std::string szMapName;

#define M_PI 3.14159265358979323846


struct QAngle {
    float pitch;
    float yaw;
    float roll;
};



template <typename T>
T clamp(T value, T minVal, T maxVal) {
    return (value < minVal) ? minVal : (value > maxVal) ? maxVal : value;
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}
void normalize_angles(QAngle& angle) {
    if (angle.pitch > 89.0f) angle.pitch = 89.0f;
    if (angle.pitch < -89.0f) angle.pitch = -89.0f;

    while (angle.yaw > 180.0f) angle.yaw -= 360.0f;
    while (angle.yaw < -180.0f) angle.yaw += 360.0f;

    angle.roll = 0.0f;
}

float normalize_yaw(float yaw) {
    while (yaw > 180.0f) yaw -= 360.0f;
    while (yaw < -180.0f) yaw += 360.0f;
    return yaw;
}

float normalize_pitch(float pitch) {
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    return pitch;
}

float shortest_yaw_delta(float targetYaw, float currentYaw) {
    float delta = targetYaw - currentYaw;
    while (delta > 180.0f) delta -= 360.0f;
    while (delta < -180.0f) delta += 360.0f;
    return delta;
}
inline float RAD2DEG(float rad) {
    return rad * (180.0f / 3.14159265358979323846f);
}

void AimAtXYZ(const Vector3& targetWorldPos, float smooth) {
    Vector2 screenPos;
    bool onScreen = g_game.world_to_screen(targetWorldPos, screenPos, g_game.view_matrix);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    Vector2 screenCenter = { screenWidth / 2.0f, screenHeight / 2.0f };

    float dx = 0.0f, dy = 0.0f;

    if (onScreen) {
        dx = screenPos.x - screenCenter.x;
        dy = screenPos.y - screenCenter.y;

        float sm = std::clamp(smooth, 0.01f, 1.0f);
        dx *= sm;
        dy *= sm;
    }
    else {
        QAngle currentAngle = readmem<QAngle>(g_game.base_client + offsets::client_dll::dwViewAngles);
        Vector3 localPos = g_game.localOrigin;
        Vector3 viewOffset = readmem<Vector3>(g_game.localpCSPlayerPawn + offsets::client_dll::m_vecViewOffset);
        Vector3 eyePos = localPos + viewOffset;
        Vector3 delta = targetWorldPos - eyePos;
        float targetYaw = RAD2DEG(std::atan2(delta.y, delta.x));

        float currentYaw = std::fmod(currentAngle.yaw + 360.0f, 360.0f);
        float desiredYaw = std::fmod(targetYaw + 360.0f, 360.0f);

        float yawDiff = desiredYaw - currentYaw;

        if (yawDiff < 180.0f) yawDiff -= 360.0f;
        if (yawDiff > -180.0f) yawDiff += 360.0f;

        float turnSpeed = 400.0f; 
        dx = (yawDiff > 0 ? 1.0f : -1.0f) * turnSpeed * smooth;
        dy = 0.0f;
    }

    LONG dxInt = static_cast<LONG>(std::round(std::clamp(dx, -85.0f, 85.0f)));
    LONG dyInt = static_cast<LONG>(std::round(std::clamp(dy, -85.0f, 85.0f)));

    if (dxInt != 0 || dyInt != 0) {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.dx = dxInt;
        input.mi.dy = dyInt;
        SendInput(1, &input, sizeof(INPUT));
    }
}


void PressKey(WORD key) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    SendInput(1, &input, sizeof(INPUT));
}

void ReleaseKey(WORD key) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}


constexpr int VK_W = 0x57;
constexpr int VK_A = 0x41;
constexpr int VK_S = 0x53;
constexpr int VK_D = 0x44;


void StopMovement() {
    ReleaseKey(VK_W);
    ReleaseKey(VK_A);
    ReleaseKey(VK_S);
    ReleaseKey(VK_D);
}

void SetKeysForStraightLine(float yawDeg) {
    StopMovement();

    while (yawDeg > 180.0f) yawDeg -= 360.0f;
    while (yawDeg < -180.0f) yawDeg += 360.0f;


    if (yawDeg > -22.5f && yawDeg <= 22.5f) {
        PressKey('W');
        
    }
    else if (yawDeg > 22.5f && yawDeg <= 67.5f) {
        PressKey('W');
        PressKey('A');
        
    }
    else if (yawDeg > 67.5f && yawDeg <= 112.5f) {
        PressKey('A');
        
    }
    else if (yawDeg > 112.5f && yawDeg <= 157.5f) {
        PressKey('S');
        PressKey('A');
        
    }
    else if (yawDeg > 157.5f || yawDeg <= -157.5f) {
        PressKey('S');
        
    }
    else if (yawDeg > -157.5f && yawDeg <= -112.5f) {
        PressKey('S');
        PressKey('D');
       
    }
    else if (yawDeg > -112.5f && yawDeg <= -67.5f) {
        PressKey('D');
      
    }
    else if (yawDeg > -67.5f && yawDeg <= -22.5f) {
        PressKey('W');
        PressKey('D');

    }

}




nav_mesh::vec3_t ToNavVec3(const Vector3& v) {
    return { v.x, v.y, v.z };
}
Vector3 NavtoVec3(const nav_mesh::vec3_t& v) {
    return { v.x, v.y, v.z };
}


float YawToTarget(const Vector3& from, const Vector3& to) {
    Vector3 delta = to - from;
    return atan2f(delta.y, delta.x) * (180.0f / static_cast<float>(M_PI));
}

int RandomInt(int min, int max) {
    return min + (rand() % (max - min + 1));
}

float distTo(const nav_mesh::vec3_t& a, const nav_mesh::vec3_t& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

float vecDist(const Vector3& a, const Vector3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

float vecDistFlat(const Vector3& a, const Vector3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}
Vector3 Lerpvec(const Vector3& a, const Vector3& b, float t) {
    return a + (b - a) * t;
}
static std::chrono::steady_clock::time_point areaStartTime;



uintptr_t WalkBot::runcheck() {
    std::map<uintptr_t, bool> entityMap;

    for (int i = 1; i < 32; i++)
    {
        uintptr_t EntityList = readmem<uintptr_t>(g_game.base_client + offsets::client_dll::dwEntityList);
        if (!EntityList) continue;
        uintptr_t listEntityController = readmem<uintptr_t>(EntityList + ((8 * (i & 0x7FFF) >> 9) + 16));
        if (!listEntityController) continue;
        uintptr_t entityController = readmem<uintptr_t>(listEntityController + (120) * (i & 0x1FF));
        if (!entityController) continue;
        uintptr_t entityControllerPawn = readmem<uintptr_t>(entityController + offsets::client_dll::m_hPlayerPawn);
        if (!entityControllerPawn) continue;
        uintptr_t listEntity = readmem<uintptr_t>(EntityList + (0x8 * ((entityControllerPawn & 0x7FFF) >> 9) + 16));
        if (!listEntity) continue;
        uintptr_t entity = readmem<uintptr_t>(listEntity + (120) * (entityControllerPawn & 0x1FF));
        if (!entity) continue;
        if (!entity || entity == g_game.localpCSPlayerPawn || entityMap.find(entity) != entityMap.end())
            continue;

        

        Vector3g origin = readmem<Vector3g>(entity + offsets::client_dll::m_vOldOrigin);
        
        Vector3 localPos = g_game.localOrigin;
        Vector3 viewOffset = readmem<Vector3>(g_game.localpCSPlayerPawn + offsets::client_dll::m_vecViewOffset);
        Vector3 eyePos = localPos + viewOffset;

        uintptr_t entity_pGameSceneNode = readmem<uintptr_t>(entity + offsets::client_dll::m_pGameSceneNode);

        uintptr_t boneArray = readmem<uintptr_t>(entity_pGameSceneNode + 0x1F0);
        uintptr_t headPosAddr = boneArray + 6 * 32;
       // uintptr_t chestPosAddr = boneArray + 92 * 32;

        Vector3 headPos = readmem<Vector3>(headPosAddr);
       // Vector3 chestPos = readmem<Vector3>(chestPosAddr);
        bool immune = readmem<bool>(entity + 0x3F24);
        if (immune) continue;
        if (visCheck->IsPointVisible({ eyePos.x, eyePos.y, eyePos.z }, { headPos.x,headPos.y,headPos.z })) {
            int health = readmem<int>(entity + 0x350);
            std::cout << health << std::endl;

            return entity;
        }
        
    }
    return NULL;
}

Vector3 AngleToForwardVector(const QAngle& angles) {
    float pitch = angles.yaw * (M_PI / 180.0f);
    float yaw = angles.pitch * (M_PI / 180.0f);

    float cp = cos(pitch);
    float sp = sin(pitch);
    float cy = cos(yaw);
    float sy = sin(yaw);

    return Vector3(cp * cy, cp * sy, -sp);
}

Vector3 Normalize(const Vector3& v) {
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length == 0) return Vector3(0, 0, 0);
    return Vector3(v.x / length, v.y / length, v.z / length);
}

float DotProduct(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
Vector3 AngleToVector(const QAngle& angles) {
    float pitch = angles.pitch * (M_PI / 180.0f);
    float yaw = angles.yaw * (M_PI / 180.0f);

    float cp = std::cos(pitch);
    float sp = std::sin(pitch);
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);

    return Vector3{
        cp * cy,  
        cp * sy,  
        -sp      
    };
}

bool IsLookingAt(Vector3 targetPos, float toleranceDegrees = 0.1f) {
    QAngle viewAngles = readmem<QAngle>(g_game.base_client + offsets::client_dll::dwViewAngles);
    Vector3 localPos = g_game.localOrigin;
    Vector3 viewOffset = readmem<Vector3>(g_game.localpCSPlayerPawn + offsets::client_dll::m_vecViewOffset);
    Vector3 eyePos = localPos + viewOffset;


    Vector3 forward = AngleToVector(viewAngles);

    Vector3 toTarget = (targetPos - eyePos).normalized();

    float dot = forward.Dot(toTarget);
    dot = std::clamp(dot, -1.0f, 1.0f); 
    float angleBetween = std::acos(dot) * (180.0f / M_PI); 

   
    return angleBetween <= toleranceDegrees;
}


std::wstring GetActiveWindowTitle() {
    HWND hwnd = GetForegroundWindow(); 
    if (hwnd == nullptr) return L"";

    wchar_t title[256];
    GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
    return std::wstring(title);
}
void MouseClick() {
    INPUT inputDown = { 0 };
    inputDown.type = INPUT_MOUSE;
    inputDown.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    INPUT inputUp = { 0 };
    inputUp.type = INPUT_MOUSE;
    inputUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    
    SendInput(1, &inputDown, sizeof(INPUT));
    SendInput(1, &inputUp, sizeof(INPUT));
}
void WalkBot::Run() {
    if (!LevelCheck()) return;
    uintptr_t Enemy = runcheck(); 
    int health = readmem<int>(Enemy + 0x350);
    if (Enemy != NULL && health == 256) {
        
        m_vAreasPath.clear();
        StopMovement();
        uintptr_t entity_pGameSceneNode = readmem<uintptr_t>(Enemy + offsets::client_dll::m_pGameSceneNode);

        uintptr_t boneArray = readmem<uintptr_t>(entity_pGameSceneNode + 0x1F0);
        uintptr_t headPosAddr = boneArray + 33 * 32;
        Vector3 position = readmem<Vector3>(headPosAddr);
        Vector3 localPos = g_game.localOrigin;
        Vector3 viewOffset = readmem<Vector3>(g_game.localpCSPlayerPawn + offsets::client_dll::m_vecViewOffset);
        Vector3 eyePos = localPos + viewOffset;
        if (visCheck->IsPointVisible({ eyePos.x, eyePos.y, eyePos.z }, { position.x, position.y, position.z })) {
            AimAtXYZ(position,0.65f);
            if (IsLookingAt(position, 0.25)) {
                MouseClick();
            }
            return;
        }
        return;
    }
    if (GetActiveWindowTitle() != L"Counter-Strike 2") return;
    if (DoesNeedNewPath()) {

        m_pCurrentTarget = GetAreaNearEnemies();
        auto pAreaClosestToPlayer = GetAreaNearPlayer(g_game.localOrigin);
        std::cout << "Trying to find path from #" << pAreaClosestToPlayer->get_id() << " to #"
            << m_pCurrentTarget->get_id() << std::endl;

        bool bWasPathSuccessful = true;
        try {
            m_vAreasPath = m_pNavFile->find_path(nav_mesh::vec3_t(pAreaClosestToPlayer->get_center()), nav_mesh::vec3_t(m_pCurrentTarget->get_center()));
        }
        catch (const std::exception&) {
            bWasPathSuccessful = false;
        }

        if (bWasPathSuccessful) {
            std::cout << "Path to #"
                << m_pCurrentTarget->get_id()
                << " found." << std::endl;
            std::cout << "Path contents:" << std::endl;
            for (const auto& point : m_vAreasPath) {
                std::cout << "vec3_t(" << point.x << ", " << point.y << ", " << point.z << ")" << std::endl;
            }
        }

    }
    else {
        static int lastAreaIndex = -1;
        static size_t currentPathIndex = 0;

        if (!m_vAreasPath.empty() && currentPathIndex < m_vAreasPath.size()) {
            auto targetPoint = m_vAreasPath[currentPathIndex];
            QAngle currentAngle = readmem<QAngle>(g_game.base_client + offsets::client_dll::dwViewAngles);
            if (lastAreaIndex != currentPathIndex) {
                areaStartTime = std::chrono::steady_clock::now();
                lastAreaIndex = currentPathIndex;
            }

            auto now = std::chrono::steady_clock::now();
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - areaStartTime).count();

            Vector3 aimTarget = NavtoVec3(targetPoint);
            if (currentPathIndex + 1 < m_vAreasPath.size()) {
                Vector3 nextPoint = NavtoVec3(m_vAreasPath[currentPathIndex + 1]);
                aimTarget = Lerpvec(aimTarget, nextPoint, 0.3f); 
            }
            float distance = distTo(ToNavVec3(g_game.localOrigin), targetPoint);
            float distanceFlat = vecDistFlat(g_game.localOrigin, NavtoVec3(targetPoint));
            float verticalDiff = aimTarget.z - g_game.localOrigin.z;
            
           // float smooth = clamp(0.3f - (distanceFlat / 1000.f), 0.06f, 0.2f);
            Vector3 localPos = g_game.localOrigin;
            Vector3 viewOffset = readmem<Vector3>(g_game.localpCSPlayerPawn + offsets::client_dll::m_vecViewOffset);
            Vector3 eyePos = localPos + viewOffset;

            aimTarget.z = eyePos.z;

            AimAtXYZ(aimTarget, 0.35);

            Vector3 delta = aimTarget - eyePos;

            float targetYaw = RAD2DEG(std::atan2(delta.y, delta.x));

            float currentYaw = std::fmod(currentAngle.yaw + 360.0f, 360.0f);
            float desiredYaw = std::fmod(targetYaw + 360.0f, 360.0f);

            float yawDiff = desiredYaw - currentYaw;
            if (yawDiff > 180.0f) yawDiff -= 360.0f;
            if (yawDiff < -180.0f) yawDiff += 360.0f;

            
            bool shouldJump = false;
            std::cout << "elapsedMs: " << elapsedMs << "ms, distflat: " << distanceFlat << "m, dist: " << distance << "m, vdiff: " << verticalDiff << ", Angle: " << yawDiff << std::endl;
            if (distanceFlat > 40.f && verticalDiff < -14.5f && elapsedMs >= 1400 && elapsedMs < 5000) {

                shouldJump = true;
            }
            else if (verticalDiff > 35.f && distanceFlat < 80.f && elapsedMs >= 1400 && elapsedMs < 5000) {

                shouldJump = true;
            }

            if (elapsedMs >= 3800) shouldJump = true;

            if (shouldJump) {
                PressKey(VK_SPACE); 
            }
            else {
                ReleaseKey(VK_SPACE);
            }
            
            if (elapsedMs >= 6000) {
                std::cout << "[!] Stuck too long at area " << currentPathIndex << ", resetting path." << std::endl;
                now = std::chrono::steady_clock::now();
                m_vAreasPath.clear();
                currentPathIndex = 0;
                StopMovement();
                return;
            }
            if (distanceFlat < 55.f) { 
                std::cout << "At checkPoint!" << std::endl;
                currentPathIndex++;
            }
            else {
                SetKeysForStraightLine(yawDiff);
            }
        }
        else {
            std::cout << "Finished!" << std::endl;
            m_vAreasPath.clear();
            StopMovement();
            currentPathIndex = 0; 
        }
        
    }
    

}

void WalkBot::OptimizePath(Vector3 vPlayerPos) {
    if (m_vAreasPath.size() > 1) {
        const float flDistToFirst = distTo(m_vAreasPath.back(), ToNavVec3(vPlayerPos));
        const float flDistToSecond = distTo(m_vAreasPath[m_vAreasPath.size() - 2], ToNavVec3(vPlayerPos));

        if (flDistToFirst > flDistToSecond) {
            m_vAreasPath.pop_back();
        }
    }
}



nav_mesh::nav_area* WalkBot::GetAreaNearPlayer(Vector3 origin) {
    nav_mesh::vec3_t position = ToNavVec3(origin);
    nav_mesh::nav_area* closest = nullptr;

    float closest_dist_sq = FLT_MAX;
    
    auto& areas = WalkBot::m_pNavFile->get_areas();
    for (auto& area : areas) {
        const nav_mesh::vec3_t& center = area.get_center();
        float dx = center.x - position.x;
        float dy = center.y - position.y;
        float dz = center.z - position.z;

        float dist_sq = dx * dx + dy * dy + dz * dz;

        if (dist_sq < closest_dist_sq) {
            closest_dist_sq = dist_sq;
            closest = &area;
        }
    }

    return closest;
}


nav_mesh::nav_area* WalkBot::GetAreaNearEnemies() {
    nav_mesh::nav_area* pCurrentTarget = nullptr;
    std::vector<CPlayer> list;
    CPlayer player;
    uintptr_t list_entry, list_entry2, playerPawn, playerMoneyServices, clippingWeapon, weaponData, playerNameData;

    CPlayer pFurthestPlayer;
    float flBestDistance = 0;
    for (int i = 1; i < 32; i++)
    {
        uintptr_t EntityList = readmem<uintptr_t>(g_game.base_client + offsets::client_dll::dwEntityList);
        if (!EntityList) continue;
        uintptr_t listEntityController = readmem<uintptr_t>(EntityList + ((8 * (i & 0x7FFF) >> 9) + 16));
        if (!listEntityController) continue;
        uintptr_t entityController = readmem<uintptr_t>(listEntityController + (120) * (i & 0x1FF));
        if (!entityController) continue;
        uintptr_t entityControllerPawn = readmem<uintptr_t>(entityController + offsets::client_dll::m_hPlayerPawn);
        if (!entityControllerPawn) continue;
        uintptr_t listEntity = readmem<uintptr_t>(EntityList + (0x8 * ((entityControllerPawn & 0x7FFF) >> 9) + 16));
        if (!listEntity) continue;
        uintptr_t entity = readmem<uintptr_t>(listEntity + (120) * (entityControllerPawn & 0x1FF));
        if (!entity || entity == g_game.localPlayer )
            continue;
        player.entity = entity;
        
        Vector3 origin = readmem<Vector3>(entity + offsets::client_dll::m_vOldOrigin);
        player.origin = origin;
        std::cout << "Got origin: " << player.origin.x << std::endl;
        if (vecDist(player.origin, g_game.localOrigin) > flBestDistance) {
            flBestDistance = vecDist(player.origin, g_game.localOrigin);
            pFurthestPlayer = player;
        }

    }

    if (pFurthestPlayer.entity) {
        nav_mesh::nav_area* pNavArea = GetAreaNearPlayer(pFurthestPlayer.origin);
        if (pNavArea) {
            pCurrentTarget = pNavArea;
            
        }
    }

    if (!pCurrentTarget) {
        
        int iRandomIdx = RandomInt(0, m_pNavFile->get_areas().size() - 1);
        pCurrentTarget = &(m_pNavFile->get_areas()[iRandomIdx]);
    }


    return pCurrentTarget;
}
std::string readString(uint64_t address, size_t maxLength = 256) {
    char buffer[256] = { 0 }; // make sure the buffer is zero-initialized
    for (size_t i = 0; i < maxLength - 1; ++i) {
        buffer[i] = readmem<char>(address + i);
        if (buffer[i] == '\0') break;
    }
    return std::string(buffer);
}

bool WalkBot::LevelCheck() {
    uint64_t globalVars = readmem<uint64_t>(g_game.base_client + offsets::client_dll::dwGlobalVars);
    uint64_t cur_map_name = readmem<uint64_t>(globalVars + 0x180);
    std::string mapName = readString(cur_map_name);
    if (cur_map_name) {
        if (szMapName != mapName) {
            szMapName = mapName;

            WalkBot::m_pNavFile = new nav_mesh::nav_file();
            WalkBot::m_pNavFile->load("C:\\Users\\adass\\Desktop\\cs-nav\\" + mapName + ".nav");

            visCheck = std::make_unique<VisCheck>("C:\\Users\\adass\\Desktop\\cs-nav\\" + mapName + ".opt");
            return true;
        }
        else {
            return true;
        }
        
    }
    else {
        return false;
    }
}





bool WalkBot::DoesNeedNewPath() {
    if (m_vAreasPath.size() == 0) {
        return true;
    }



    return false;
}