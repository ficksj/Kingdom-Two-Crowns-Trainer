#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

// Forward exports so it can also act as version.dll if needed
#pragma comment(linker, "/export:GetFileVersionInfoA=C:\\Windows\\System32\\version.GetFileVersionInfoA")
#pragma comment(linker, "/export:GetFileVersionInfoByHandle=C:\\Windows\\System32\\version.GetFileVersionInfoByHandle")
#pragma comment(linker, "/export:GetFileVersionInfoExA=C:\\Windows\\System32\\version.GetFileVersionInfoExA")
#pragma comment(linker, "/export:GetFileVersionInfoExW=C:\\Windows\\System32\\version.GetFileVersionInfoExW")
#pragma comment(linker, "/export:GetFileVersionInfoSizeA=C:\\Windows\\System32\\version.GetFileVersionInfoSizeA")
#pragma comment(linker, "/export:GetFileVersionInfoSizeExA=C:\\Windows\\System32\\version.GetFileVersionInfoSizeExA")
#pragma comment(linker, "/export:GetFileVersionInfoSizeExW=C:\\Windows\\System32\\version.GetFileVersionInfoSizeExW")
#pragma comment(linker, "/export:GetFileVersionInfoSizeW=C:\\Windows\\System32\\version.GetFileVersionInfoSizeW")
#pragma comment(linker, "/export:GetFileVersionInfoW=C:\\Windows\\System32\\version.GetFileVersionInfoW")
#pragma comment(linker, "/export:VerFindFileA=C:\\Windows\\System32\\version.VerFindFileA")
#pragma comment(linker, "/export:VerFindFileW=C:\\Windows\\System32\\version.VerFindFileW")
#pragma comment(linker, "/export:VerInstallFileA=C:\\Windows\\System32\\version.VerInstallFileA")
#pragma comment(linker, "/export:VerInstallFileW=C:\\Windows\\System32\\version.VerInstallFileW")
#pragma comment(linker, "/export:VerLanguageNameA=C:\\Windows\\System32\\version.VerLanguageNameA")
#pragma comment(linker, "/export:VerLanguageNameW=C:\\Windows\\System32\\version.VerLanguageNameW")
#pragma comment(linker, "/export:VerQueryValueA=C:\\Windows\\System32\\version.VerQueryValueA")
#pragma comment(linker, "/export:VerQueryValueW=C:\\Windows\\System32\\version.VerQueryValueW")

// IL2CPP Types
typedef void* (*il2cpp_domain_get_t)();
typedef void* (*il2cpp_thread_attach_t)(void* domain);
typedef void* (*il2cpp_domain_assembly_open_t)(void* domain, const char* name);
typedef void* (*il2cpp_assembly_get_image_t)(void* assembly);
typedef void* (*il2cpp_class_from_name_t)(void* image, const char* namespaze, const char* name);
typedef void* (*il2cpp_class_get_method_from_name_t)(void* klass, const char* name, int argsCount);
typedef void* (*il2cpp_class_get_field_from_name_t)(void* klass, const char* name);
typedef void  (*il2cpp_field_static_get_value_t)(void* field, void* value);
typedef void  (*il2cpp_field_static_set_value_t)(void* field, void* value);
typedef void  (*il2cpp_field_get_value_t)(void* obj, void* field, void* value);
typedef void  (*il2cpp_field_set_value_t)(void* obj, void* field, void* value);
typedef void* (*il2cpp_runtime_invoke_t)(void* method, void* obj, void** params, void** exc);
typedef void* (*il2cpp_class_get_type_t)(void* klass);
typedef void* (*il2cpp_type_get_object_t)(void* type);

// Function pointers
static il2cpp_domain_get_t                  p_il2cpp_domain_get = nullptr;
static il2cpp_thread_attach_t              p_il2cpp_thread_attach = nullptr;
static il2cpp_domain_assembly_open_t       p_il2cpp_domain_assembly_open = nullptr;
static il2cpp_assembly_get_image_t         p_il2cpp_assembly_get_image = nullptr;
static il2cpp_class_from_name_t            p_il2cpp_class_from_name = nullptr;
static il2cpp_class_get_method_from_name_t p_il2cpp_class_get_method_from_name = nullptr;
static il2cpp_class_get_field_from_name_t  p_il2cpp_class_get_field_from_name = nullptr;
static il2cpp_field_static_get_value_t     p_il2cpp_field_static_get_value = nullptr;
static il2cpp_field_static_set_value_t     p_il2cpp_field_static_set_value = nullptr;
static il2cpp_field_get_value_t            p_il2cpp_field_get_value = nullptr;
static il2cpp_field_set_value_t            p_il2cpp_field_set_value = nullptr;
static il2cpp_runtime_invoke_t             p_il2cpp_runtime_invoke = nullptr;
static il2cpp_class_get_type_t             p_il2cpp_class_get_type = nullptr;
static il2cpp_type_get_object_t            p_il2cpp_type_get_object = nullptr;

// IL2CPP Array layout
struct Il2CppArray {
    void* klass;
    void* monitor;
    void* bounds;
    uintptr_t max_length;
    void* vector[1];
};

static void* g_domain = nullptr;
static void* g_gameImage = nullptr;
static void* g_coreImage = nullptr;

// Cheat States
static bool g_cheatInfiniteStamina = false;
static bool g_cheatInvulnerable = false;
static bool g_cheatAutoBuild = false;
static bool g_cheatInfiniteWallet = false;
static bool g_cheatInvulnerableWalls = false;
static bool g_cheatDisableSpawning = false;

static std::ofstream g_log;

static void Log(const std::string& msg) {
    if (g_log.is_open()) {
        g_log << msg << std::endl;
        g_log.flush();
    }
}

static bool InitIl2Cpp() {
    HMODULE hGameAssembly = nullptr;
    while (!hGameAssembly) {
        hGameAssembly = GetModuleHandleA("GameAssembly.dll");
        if (!hGameAssembly) Sleep(200);
    }

    Log("[INIT] GameAssembly.dll detected!");

    p_il2cpp_domain_get = (il2cpp_domain_get_t)GetProcAddress(hGameAssembly, "il2cpp_domain_get");
    p_il2cpp_thread_attach = (il2cpp_thread_attach_t)GetProcAddress(hGameAssembly, "il2cpp_thread_attach");
    p_il2cpp_domain_assembly_open = (il2cpp_domain_assembly_open_t)GetProcAddress(hGameAssembly, "il2cpp_domain_assembly_open");
    p_il2cpp_assembly_get_image = (il2cpp_assembly_get_image_t)GetProcAddress(hGameAssembly, "il2cpp_assembly_get_image");
    p_il2cpp_class_from_name = (il2cpp_class_from_name_t)GetProcAddress(hGameAssembly, "il2cpp_class_from_name");
    p_il2cpp_class_get_method_from_name = (il2cpp_class_get_method_from_name_t)GetProcAddress(hGameAssembly, "il2cpp_class_get_method_from_name");
    p_il2cpp_class_get_field_from_name = (il2cpp_class_get_field_from_name_t)GetProcAddress(hGameAssembly, "il2cpp_class_get_field_from_name");
    p_il2cpp_field_static_get_value = (il2cpp_field_static_get_value_t)GetProcAddress(hGameAssembly, "il2cpp_field_static_get_value");
    p_il2cpp_field_static_set_value = (il2cpp_field_static_set_value_t)GetProcAddress(hGameAssembly, "il2cpp_field_static_set_value");
    p_il2cpp_field_get_value = (il2cpp_field_get_value_t)GetProcAddress(hGameAssembly, "il2cpp_field_get_value");
    p_il2cpp_field_set_value = (il2cpp_field_set_value_t)GetProcAddress(hGameAssembly, "il2cpp_field_set_value");
    p_il2cpp_runtime_invoke = (il2cpp_runtime_invoke_t)GetProcAddress(hGameAssembly, "il2cpp_runtime_invoke");
    p_il2cpp_class_get_type = (il2cpp_class_get_type_t)GetProcAddress(hGameAssembly, "il2cpp_class_get_type");
    p_il2cpp_type_get_object = (il2cpp_type_get_object_t)GetProcAddress(hGameAssembly, "il2cpp_type_get_object");

    if (!p_il2cpp_domain_get || !p_il2cpp_thread_attach || !p_il2cpp_runtime_invoke) {
        Log("[ERROR] Failed to find required IL2CPP exports!");
        return false;
    }

    while (!g_domain) {
        g_domain = p_il2cpp_domain_get();
        if (!g_domain) Sleep(100);
    }
    p_il2cpp_thread_attach(g_domain);

    while (!g_gameImage || !g_coreImage) {
        void* asmGame = p_il2cpp_domain_assembly_open(g_domain, "Assembly-CSharp");
        if (asmGame) g_gameImage = p_il2cpp_assembly_get_image(asmGame);

        void* asmCore = p_il2cpp_domain_assembly_open(g_domain, "UnityEngine.CoreModule");
        if (asmCore) g_coreImage = p_il2cpp_assembly_get_image(asmCore);

        if (!g_gameImage || !g_coreImage) Sleep(200);
    }

    Log("[INIT] Assembly-CSharp and UnityEngine.CoreModule loaded successfully!");
    return true;
}

// Find instance of DebugTools
static void* GetDebugToolsInstance() {
    if (!g_domain || !g_gameImage || !g_coreImage) return nullptr;
    p_il2cpp_thread_attach(g_domain);

    void* dtClass = p_il2cpp_class_from_name(g_gameImage, "", "DebugTools");
    if (!dtClass) return nullptr;

    void* dtType = p_il2cpp_class_get_type(dtClass);
    if (!dtType) return nullptr;

    void* dtTypeObj = p_il2cpp_type_get_object(dtType);
    if (!dtTypeObj) return nullptr;

    void* resClass = p_il2cpp_class_from_name(g_coreImage, "UnityEngine", "Resources");
    if (!resClass) return nullptr;

    void* findMethod = p_il2cpp_class_get_method_from_name(resClass, "FindObjectsOfTypeAll", 1);
    if (!findMethod) return nullptr;

    void* args[1] = { dtTypeObj };
    void* exc = nullptr;
    Il2CppArray* arr = (Il2CppArray*)p_il2cpp_runtime_invoke(findMethod, nullptr, args, &exc);
    if (!arr || exc || arr->max_length == 0) {
        return nullptr;
    }

    return arr->vector[0];
}

// Enable debugToolsEnabled flag on Kingdom.inst
static void EnableKingdomDebugTools() {
    if (!g_domain || !g_gameImage) return;
    p_il2cpp_thread_attach(g_domain);

    void* kingdomClass = p_il2cpp_class_from_name(g_gameImage, "", "Kingdom");
    if (!kingdomClass) return;

    void* instField = p_il2cpp_class_get_field_from_name(kingdomClass, "inst");
    if (!instField) return;

    void* kingdomInst = nullptr;
    p_il2cpp_field_static_get_value(instField, &kingdomInst);
    if (!kingdomInst) return;

    void* debugField = p_il2cpp_class_get_field_from_name(kingdomClass, "debugToolsEnabled");
    if (debugField) {
        bool enabled = true;
        p_il2cpp_field_set_value(kingdomInst, debugField, &enabled);
    }
}

// Invoke a method on DebugTools
static bool CallDebugToolsMethod(const char* methodName) {
    void* dt = GetDebugToolsInstance();
    if (!dt) {
        return false;
    }

    EnableKingdomDebugTools();

    void* dtClass = p_il2cpp_class_from_name(g_gameImage, "", "DebugTools");
    void* method = p_il2cpp_class_get_method_from_name(dtClass, methodName, 0);
    if (!method) {
        Log(std::string("[ERROR] Method not found: ") + methodName);
        return false;
    }

    void* exc = nullptr;
    p_il2cpp_runtime_invoke(method, dt, nullptr, &exc);
    return !exc;
}

// Toggle on-screen In-Game Debug Menu
static void ToggleDebugMenu() {
    void* dt = GetDebugToolsInstance();
    if (!dt) {
        return;
    }

    EnableKingdomDebugTools();

    // Set GameObject active
    void* compClass = p_il2cpp_class_from_name(g_coreImage, "UnityEngine", "Component");
    void* getGOMethod = p_il2cpp_class_get_method_from_name(compClass, "get_gameObject", 0);
    void* exc = nullptr;
    void* go = p_il2cpp_runtime_invoke(getGOMethod, dt, nullptr, &exc);
    if (go && !exc) {
        void* goClass = p_il2cpp_class_from_name(g_coreImage, "UnityEngine", "GameObject");
        void* setActiveMethod = p_il2cpp_class_get_method_from_name(goClass, "SetActive", 1);
        bool active = true;
        void* args[1] = { &active };
        p_il2cpp_runtime_invoke(setActiveMethod, go, args, &exc);
    }

    // Call OpenButtonPressed or ShowPanel or OnInGameButton
    void* dtClass = p_il2cpp_class_from_name(g_gameImage, "", "DebugTools");
    void* openMethod = p_il2cpp_class_get_method_from_name(dtClass, "OpenButtonPressed", 0);
    if (!openMethod) openMethod = p_il2cpp_class_get_method_from_name(dtClass, "OnInGameButton", 0);
    if (!openMethod) openMethod = p_il2cpp_class_get_method_from_name(dtClass, "ShowPanel", 0);

    if (openMethod) {
        p_il2cpp_runtime_invoke(openMethod, dt, nullptr, &exc);
        MessageBeep(MB_OK);
    }

    void* actionsMethod = p_il2cpp_class_get_method_from_name(dtClass, "OnButtonActions", 0);
    if (actionsMethod) {
        p_il2cpp_runtime_invoke(actionsMethod, dt, nullptr, &exc);
    }
}

// Handle command from hotkey or IPC pipe
static void ExecuteCommand(const std::string& cmd) {
    if (cmd == "TOGGLE_MENU") {
        ToggleDebugMenu();
    } else if (cmd == "GIVE_COINS") {
        CallDebugToolsMethod("OnButtonGiveCoins");
        CallDebugToolsMethod("OnButtonGiveCoins");
        MessageBeep(MB_OK);
    } else if (cmd == "GIVE_GEMS") {
        CallDebugToolsMethod("OnButtonGiveGems");
        MessageBeep(MB_OK);
    } else if (cmd == "INFINITE_STAMINA") {
        g_cheatInfiniteStamina = !g_cheatInfiniteStamina;
        CallDebugToolsMethod("OnButtonInfiniteStamina");
        MessageBeep(MB_OK);
    } else if (cmd == "INVULNERABLE") {
        g_cheatInvulnerable = !g_cheatInvulnerable;
        CallDebugToolsMethod("OnButtonInvulnerableRulers");
        MessageBeep(MB_OK);
    } else if (cmd == "SPEED") {
        CallDebugToolsMethod("OnButtonTimescaleIncrement");
        MessageBeep(MB_OK);
    } else if (cmd == "DEFEAT_GREED") {
        CallDebugToolsMethod("OnButtonDefeatGreedNow");
        MessageBeep(MB_OK);
    } else if (cmd == "DESTROY_PORTALS") {
        CallDebugToolsMethod("OnButtonDestroyAllPortals");
        MessageBeep(MB_OK);
    } else if (cmd == "AUTO_BUILD") {
        g_cheatAutoBuild = !g_cheatAutoBuild;
        CallDebugToolsMethod("OnButtonAutoBuildEnabled");
        MessageBeep(MB_OK);
    } else if (cmd == "SKIP_DAY") {
        CallDebugToolsMethod("OnButtonSkipDay");
        MessageBeep(MB_OK);
    } else if (cmd == "DAY_NIGHT_PAUSE") {
        CallDebugToolsMethod("OnButtonDayNightPause");
        MessageBeep(MB_OK);
    } else if (cmd == "BLOOD_MOON") {
        CallDebugToolsMethod("OnButtonStartRedMoon");
        MessageBeep(MB_OK);
    }
    // --- NEW CHEATS ---
    else if (cmd == "INFINITE_WALLET") {
        g_cheatInfiniteWallet = !g_cheatInfiniteWallet;
        MessageBeep(MB_OK);
        Log(std::string("[CHEAT] Infinite Wallet (No Spend): ") + (g_cheatInfiniteWallet ? "ENABLED" : "DISABLED"));
    } else if (cmd == "INVULNERABLE_WALLS") {
        g_cheatInvulnerableWalls = !g_cheatInvulnerableWalls;
        CallDebugToolsMethod("OnButtonInvunerableWalls");
        MessageBeep(MB_OK);
        Log("[CHEAT] Invulnerable Walls toggled!");
    } else if (cmd == "CUT_ALL_TREES") {
        CallDebugToolsMethod("OnButtonCutdownAllTrees");
        MessageBeep(MB_OK);
        Log("[CHEAT] Cut down all trees on the island!");
    } else if (cmd == "MELT_SNOW") {
        CallDebugToolsMethod("OnButtonTriggerSnowMelt");
        MessageBeep(MB_OK);
        Log("[CHEAT] Snow melted / Spring season restored!");
    } else if (cmd == "SET_TECH_STONE") {
        CallDebugToolsMethod("OnButtonSetTechAgeStone");
        MessageBeep(MB_OK);
        Log("[CHEAT] Tech Age upgraded to Stone Age!");
    } else if (cmd == "SET_TECH_IRON") {
        CallDebugToolsMethod("OnButtonSetTechAgeIron");
        MessageBeep(MB_OK);
        Log("[CHEAT] Tech Age upgraded to Iron Age!");
    } else if (cmd == "RETREAT_ENEMIES") {
        CallDebugToolsMethod("OnButtonRetreatEnemies");
        MessageBeep(MB_OK);
        Log("[CHEAT] All enemies forced to retreat!");
    } else if (cmd == "DESTROY_CLOSEST_PORTAL") {
        CallDebugToolsMethod("OnButtonDestroyClosestPortal");
        MessageBeep(MB_OK);
        Log("[CHEAT] Destroyed closest portal!");
    } else if (cmd == "DISABLE_SPAWNING") {
        g_cheatDisableSpawning = !g_cheatDisableSpawning;
        CallDebugToolsMethod("OnButtonSpawningEnabled");
        MessageBeep(MB_OK);
        Log("[CHEAT] Spawning of Greed toggled!");
    } else if (cmd == "UNLOCK_ALL_LANDS") {
        CallDebugToolsMethod("OnButtonUnlockAllLands");
        MessageBeep(MB_OK);
        Log("[CHEAT] Unlocked all islands (1-5)!");
    } else if (cmd == "UNLOCK_ALL_ITEMS") {
        CallDebugToolsMethod("OnButtonUnlockAllItems");
        CallDebugToolsMethod("OnButtonUnlockAllDieties");
        MessageBeep(MB_OK);
        Log("[CHEAT] Unlocked all items and god statues!");
    } else if (cmd == "CHANGE_MOUNT") {
        CallDebugToolsMethod("OnButtonRandomizeAppearanceThisMount");
        MessageBeep(MB_OK);
        Log("[CHEAT] Mount changed / randomized!");
    } else if (cmd == "DISTRIBUTE_ARCHERS") {
        CallDebugToolsMethod("OnButtonDistributeArchers");
        MessageBeep(MB_OK);
        Log("[CHEAT] Distributed archers evenly across defenses!");
    }
}

// IPC Named Pipe Server Thread for Launcher communication
static DWORD WINAPI PipeServerThread(LPVOID lpParam) {
    while (true) {
        HANDLE hPipe = CreateNamedPipeA(
            "\\\\.\\pipe\\CheatKTCPipe",
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, 1024, 1024, 0, nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            Sleep(1000);
            continue;
        }

        if (ConnectNamedPipe(hPipe, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)) {
            char buffer[256];
            DWORD bytesRead = 0;
            if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
                buffer[bytesRead] = '\0';
                ExecuteCommand(std::string(buffer));
            }
        }
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
    return 0;
}

// Main cheat loop
static DWORD WINAPI CheatThread(LPVOID lpParam) {
    g_log.open("cheat_log.txt", std::ios::out | std::ios::trunc);
    Log("[START] CheatKTC injected into Kingdom Two Crowns!");

    if (!InitIl2Cpp()) {
        Log("[FATAL] InitIl2Cpp failed!");
        return 1;
    }

    Log("[READY] IL2CPP initialized. Hotkeys & IPC pipe active!");

    CreateThread(nullptr, 0, PipeServerThread, nullptr, 0, nullptr);

    int walletLoopTick = 0;

    // Polling loop for hotkeys
    while (true) {
        Sleep(50);
        walletLoopTick++;

        // Keep coins and gems perpetually full if "Never Decrease" is active
        if (g_cheatInfiniteWallet && (walletLoopTick % 6 == 0)) {
            CallDebugToolsMethod("OnButtonGiveCoins");
            CallDebugToolsMethod("OnButtonGiveGems");
        }

        bool ctrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

        // F1-F12 Hotkeys
        if (GetAsyncKeyState(VK_F1) & 1)  ExecuteCommand("TOGGLE_MENU");
        if (GetAsyncKeyState(VK_F2) & 1)  ExecuteCommand("GIVE_COINS");
        if (GetAsyncKeyState(VK_F3) & 1)  ExecuteCommand("GIVE_GEMS");
        if (GetAsyncKeyState(VK_F4) & 1)  ExecuteCommand("INFINITE_STAMINA");
        if (GetAsyncKeyState(VK_F5) & 1)  ExecuteCommand("INVULNERABLE");
        if (GetAsyncKeyState(VK_F6) & 1)  ExecuteCommand("SPEED");
        if (GetAsyncKeyState(VK_F7) & 1)  ExecuteCommand("DEFEAT_GREED");
        if (GetAsyncKeyState(VK_F8) & 1)  ExecuteCommand("DESTROY_PORTALS");
        if (GetAsyncKeyState(VK_F9) & 1)  ExecuteCommand("AUTO_BUILD");
        if (GetAsyncKeyState(VK_F10) & 1) ExecuteCommand("SKIP_DAY");
        if (GetAsyncKeyState(VK_F11) & 1) ExecuteCommand("DAY_NIGHT_PAUSE");
        if (GetAsyncKeyState(VK_F12) & 1) ExecuteCommand("BLOOD_MOON");

        // Ctrl + 1..9 Hotkeys for new features
        if (ctrlPressed) {
            if (GetAsyncKeyState('1') & 1) ExecuteCommand("INVULNERABLE_WALLS");
            if (GetAsyncKeyState('2') & 1) ExecuteCommand("INFINITE_WALLET");
            if (GetAsyncKeyState('3') & 1) ExecuteCommand("CUT_ALL_TREES");
            if (GetAsyncKeyState('4') & 1) ExecuteCommand("MELT_SNOW");
            if (GetAsyncKeyState('5') & 1) ExecuteCommand("SET_TECH_STONE");
            if (GetAsyncKeyState('6') & 1) ExecuteCommand("SET_TECH_IRON");
            if (GetAsyncKeyState('7') & 1) ExecuteCommand("RETREAT_ENEMIES");
            if (GetAsyncKeyState('8') & 1) ExecuteCommand("DESTROY_CLOSEST_PORTAL");
            if (GetAsyncKeyState('9') & 1) ExecuteCommand("CHANGE_MOUNT");
            if (GetAsyncKeyState('0') & 1) ExecuteCommand("UNLOCK_ALL_LANDS");
        }
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, CheatThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
