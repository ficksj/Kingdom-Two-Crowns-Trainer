#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

// Forward exports to system version.dll
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

static bool g_cheatInfiniteStamina = false;
static bool g_cheatInvulnerable = false;
static bool g_cheatAutoBuild = false;

static std::ofstream g_log;

static void Log(const std::string& msg) {
    if (g_log.is_open()) {
        g_log << msg << std::endl;
        g_log.flush();
    }
    std::cout << msg << std::endl;
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

    // Wait until domain is ready
    while (!g_domain) {
        g_domain = p_il2cpp_domain_get();
        if (!g_domain) Sleep(100);
    }
    p_il2cpp_thread_attach(g_domain);

    // Wait until Assembly-CSharp and UnityEngine.CoreModule are loaded
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
        Log("[INFO] Kingdom.inst.debugToolsEnabled = true");
    }
}

// Invoke a method on DebugTools
static bool CallDebugToolsMethod(const char* methodName) {
    void* dt = GetDebugToolsInstance();
    if (!dt) {
        Log("[WARN] DebugTools instance not found! Make sure you are in a loaded game/island.");
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
    if (exc) {
        Log(std::string("[ERROR] Exception calling method: ") + methodName);
        return false;
    }

    return true;
}

// Activate DebugTools GameObject and toggle panel
static void ToggleDebugMenu() {
    void* dt = GetDebugToolsInstance();
    if (!dt) {
        Log("[WARN] DebugTools instance not found!");
        return;
    }

    EnableKingdomDebugTools();

    // 1. Set GameObject active
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

    // 2. Call OpenButtonPressed / OnInGameButton
    void* dtClass = p_il2cpp_class_from_name(g_gameImage, "", "DebugTools");
    void* openMethod = p_il2cpp_class_get_method_from_name(dtClass, "OpenButtonPressed", 0);
    if (!openMethod) openMethod = p_il2cpp_class_get_method_from_name(dtClass, "OnInGameButton", 0);

    if (openMethod) {
        p_il2cpp_runtime_invoke(openMethod, dt, nullptr, &exc);
        Log("[CHEAT] Toggled in-game Debug Menu!");
        MessageBeep(MB_OK);
    }
}

static void PrintBanner() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << R"(
======================================================================
     KINGDOM: TWO CROWNS - IN-GAME CHEAT ENGINE & TRAINER
======================================================================
)" << std::endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << " [HOTKEYS]:\n"
              << "   [F1]  Toggle In-Game Debug Menu (Открыть внутриигровое меню)\n"
              << "   [F2]  Add Coins (+20 монет)\n"
              << "   [F3]  Add Gems (+5 камней)\n"
              << "   [F4]  Toggle Infinite Mount Stamina (Бесконечная стамина)\n"
              << "   [F5]  Toggle Invulnerable Rulers & Crown (Бессмертие и корона)\n"
              << "   [F6]  Increase Game Speed / Timescale (Ускорить игру)\n"
              << "   [F7]  Defeat All Greed Now (Уничтожить всех жадин)\n"
              << "   [F8]  Destroy All Portals (Уничтожить все порталы)\n"
              << "   [F9]  Toggle Auto-Build (Мгновенное строительство)\n"
              << "   [F10] Skip Day (Пропустить день)\n"
              << "   [F11] Pause / Resume Day-Night Cycle (Пауза времени суток)\n"
              << "   [F12] Start Blood Moon (Запустить Кровавую Луну)\n"
              << "======================================================================\n"
              << " [STATUS] Cheat loaded and ready! Press hotkeys in-game anytime.\n"
              << "======================================================================\n"
              << std::endl;
}

// Background thread loop
static DWORD WINAPI CheatThread(LPVOID lpParam) {
    // Open log file in game directory
    g_log.open("cheat_log.txt", std::ios::out | std::ios::trunc);
    Log("[START] Kingdom Two Crowns Cheat Mod initializing...");

    // Allocate Console for instant visual feedback
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    SetConsoleTitleA("Kingdom Two Crowns - Cheat Console");

    PrintBanner();

    if (!InitIl2Cpp()) {
        Log("[FATAL] Could not initialize IL2CPP hooks!");
        return 1;
    }

    Log("[READY] Ready for inputs! Switch to the game window.");

    // Polling loop for hotkeys
    while (true) {
        Sleep(50);

        // F1: Toggle Debug Menu
        if (GetAsyncKeyState(VK_F1) & 1) {
            ToggleDebugMenu();
        }

        // F2: Give Coins
        if (GetAsyncKeyState(VK_F2) & 1) {
            // Call 2 times for ~20 coins
            CallDebugToolsMethod("OnButtonGiveCoins");
            CallDebugToolsMethod("OnButtonGiveCoins");
            Log("[CHEAT] +Coins added to wallet!");
            MessageBeep(MB_OK);
        }

        // F3: Give Gems
        if (GetAsyncKeyState(VK_F3) & 1) {
            CallDebugToolsMethod("OnButtonGiveGems");
            Log("[CHEAT] +Gems added to pouch!");
            MessageBeep(MB_OK);
        }

        // F4: Infinite Mount Stamina
        if (GetAsyncKeyState(VK_F4) & 1) {
            g_cheatInfiniteStamina = !g_cheatInfiniteStamina;
            CallDebugToolsMethod("OnButtonInfiniteStamina");
            Log(std::string("[CHEAT] Infinite Mount Stamina: ") + (g_cheatInfiniteStamina ? "ENABLED" : "DISABLED"));
            MessageBeep(MB_OK);
        }

        // F5: Invulnerable Rulers & Crown
        if (GetAsyncKeyState(VK_F5) & 1) {
            g_cheatInvulnerable = !g_cheatInvulnerable;
            CallDebugToolsMethod("OnButtonInvulnerableRulers");
            Log(std::string("[CHEAT] Invulnerable Ruler & Crown (God Mode): ") + (g_cheatInvulnerable ? "ENABLED" : "DISABLED"));
            MessageBeep(MB_OK);
        }

        // F6: Game Speed Timescale
        if (GetAsyncKeyState(VK_F6) & 1) {
            CallDebugToolsMethod("OnButtonTimescaleIncrement");
            Log("[CHEAT] Game Speed increased (Timescale +)!");
            MessageBeep(MB_OK);
        }

        // F7: Defeat Greed Now
        if (GetAsyncKeyState(VK_F7) & 1) {
            CallDebugToolsMethod("OnButtonDefeatGreedNow");
            Log("[CHEAT] Defeated all Greed monsters on the island!");
            MessageBeep(MB_OK);
        }

        // F8: Destroy All Portals
        if (GetAsyncKeyState(VK_F8) & 1) {
            CallDebugToolsMethod("OnButtonDestroyAllPortals");
            Log("[CHEAT] Destroyed all enemy portals!");
            MessageBeep(MB_OK);
        }

        // F9: Auto-Build
        if (GetAsyncKeyState(VK_F9) & 1) {
            g_cheatAutoBuild = !g_cheatAutoBuild;
            CallDebugToolsMethod("OnButtonAutoBuildEnabled");
            Log(std::string("[CHEAT] Instant Auto-Build: ") + (g_cheatAutoBuild ? "ENABLED" : "DISABLED"));
            MessageBeep(MB_OK);
        }

        // F10: Skip Day
        if (GetAsyncKeyState(VK_F10) & 1) {
            CallDebugToolsMethod("OnButtonSkipDay");
            Log("[CHEAT] Skipped to next day!");
            MessageBeep(MB_OK);
        }

        // F11: Pause / Resume Day-Night
        if (GetAsyncKeyState(VK_F11) & 1) {
            CallDebugToolsMethod("OnButtonDayNightPause");
            Log("[CHEAT] Day/Night Cycle Paused/Resumed!");
            MessageBeep(MB_OK);
        }

        // F12: Blood Moon
        if (GetAsyncKeyState(VK_F12) & 1) {
            CallDebugToolsMethod("OnButtonStartRedMoon");
            Log("[CHEAT] Blood Moon (Red Moon) triggered!");
            MessageBeep(MB_OK);
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

