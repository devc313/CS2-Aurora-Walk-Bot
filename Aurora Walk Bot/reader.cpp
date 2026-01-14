#include "reader.hpp"


DWORD_PTR GetModule(DWORD pid, const char* moduleName) {
    std::wstring wideModule;
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, moduleName, -1, nullptr, 0);
    if (wideCharLength > 0) {
        wideModule.resize(wideCharLength);
        MultiByteToWideChar(CP_UTF8, 0, moduleName, -1, &wideModule[0], wideCharLength);
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MODULEENTRY32W moduleEntry{};
    moduleEntry.dwSize = sizeof(MODULEENTRY32W);

    if (Module32FirstW(snapshot, &moduleEntry)) {
        do {
            if (!wcscmp(moduleEntry.szModule, wideModule.c_str())) {
                CloseHandle(snapshot);
                return reinterpret_cast<DWORD_PTR>(moduleEntry.modBaseAddr);
            }
        } while (Module32NextW(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);
    return 0;
}




void CGame::init() {
    
    base_client = GetModule(mem::process_id, "client.dll");
    base_engine = GetModule(mem::process_id, "engine2.dll");

    buildNumber = readmem<uintptr_t>(base_engine + offsets::engine2_dll::dwBuildNumber);

    std::cout << "Build Number Found: " << buildNumber << std::endl;
}


void CGame::loop() {
    std::lock_guard<std::mutex> lock(reader_mutex);

    inGame = false;

    localPlayer = readmem<uintptr_t>(base_client + offsets::client_dll::dwLocalPlayerController);
    if (!localPlayer) return;

    localPlayerPawn = readmem<std::uint32_t>(localPlayer + offsets::client_dll::m_hPlayerPawn);
    if (!localPlayerPawn) return;

    entity_list = readmem<uintptr_t>(base_client + offsets::client_dll::dwEntityList);

    localList_entry2 = readmem<uintptr_t>(entity_list + 0x8 * ((localPlayerPawn & 0x7FFF) >> 9) + 16);
    localpCSPlayerPawn = readmem<uintptr_t>(localList_entry2 + 120 * (localPlayerPawn & 0x1FF));
    if (!localpCSPlayerPawn) return;

    view_matrix = readmem<view_matrix_t>(base_client + offsets::client_dll::dwViewMatrix);

    localOrigin = readmem<Vector3>(localpCSPlayerPawn + offsets::client_dll::m_vOldOrigin);
    
        

}
bool CGame::world_to_screen(const Vector3& worldPos, Vector2& screenPos, const view_matrix_t& viewMatrix) {
    float clip_x = worldPos.x * viewMatrix.matrix[0][0] + worldPos.y * viewMatrix.matrix[0][1] + worldPos.z * viewMatrix.matrix[0][2] + viewMatrix.matrix[0][3];
    float clip_y = worldPos.x * viewMatrix.matrix[1][0] + worldPos.y * viewMatrix.matrix[1][1] + worldPos.z * viewMatrix.matrix[1][2] + viewMatrix.matrix[1][3];
    float clip_z = worldPos.x * viewMatrix.matrix[2][0] + worldPos.y * viewMatrix.matrix[2][1] + worldPos.z * viewMatrix.matrix[2][2] + viewMatrix.matrix[2][3];
    float clip_w = worldPos.x * viewMatrix.matrix[3][0] + worldPos.y * viewMatrix.matrix[3][1] + worldPos.z * viewMatrix.matrix[3][2] + viewMatrix.matrix[3][3];

    if (clip_w < 0.1f) // To prevent division by zero and culling objects behind the camera
        return false;

    // Normalize to NDC (Normalized Device Coordinates)
    float ndc_x = clip_x / clip_w;
    float ndc_y = clip_y / clip_w;

    // Convert to screen coordinates
    screenPos.x = (ndc_x + 1.0f) * 0.5f * 1920.0f;
    screenPos.y = (1.0f - ndc_y) * 0.5f * 1080.0f;


    return true;
}