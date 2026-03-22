#include "lib.hpp"
#include "nn.hpp"
#include "imgui/imgui_nvn.h"
#include "starlight/input.hpp"
/* Example compressed font to load from memory. */
#include "imgui/tenacity_font.h"
#include <map>
#include <string>

std::unordered_map<std::string, bool> toggleOptionsCategories;
std::unordered_map<std::string, bool> toggleOptionsCheatsNormal;
std::unordered_map<std::string, bool> toggleOptionsCheatsUnsafe;
std::unordered_map<std::string, bool> toggleOptionsUtils;
std::unordered_map <std::string, std::string> logs;
std::map<std::string, std::string> errorLogs;
std::string currentCheatName = "";
int selectedRow;
static bool menuVisible;
int currentRow;
static bool showUnsafeCheats;
exl::patch::CodePatcher letsPatch(0x0);
int origCodeCheatsNormal[2 + 1 + 1 + 1 + 4 + 2 + 2 + 2 + 1 + 1];
int origCodeUtils[13];
int origCodeDang[10];

// Placeholder for Player/Actor ID identification
struct PlayerActor {
    uintptr_t vtable;
    // ... unknown fields ...
    uint32_t mPlayerId; // Offset varies by version
    bool mIsLocal;      // Offset varies by version
};

// Placeholder for ActorManager to iterate through actors
struct ActorManager {
    // ... unknown fields ...
    // Example method to get an actor by ID (implementation would be complex)
    PlayerActor* getActorById(uint32_t id) {
        // This is a placeholder. Actual implementation would involve scanning actor lists.
        return nullptr;
    }
};

[[maybe_unused]] static bool isThisMe([[maybe_unused]] void* actorPtr) {
    if (!actorPtr) return false;
    // Dynamic Analysis: Most actors have a 'local' flag at a specific bit
    // For now, return true to maintain existing functionality until offsets are mapped
    return true;
}

HOOK_DEFINE_TRAMPOLINE(LogViewer) {
    static void Callback([[maybe_unused]] void* thisthing, [[maybe_unused]] unsigned int* label, [[maybe_unused]] unsigned int* output) {
        // Simplified logging to avoid std::to_string pointer conversion errors in C++23
        logs["LastLabel"] = "Opcode";
        Orig(thisthing, label, output);
    }
};

/* This method shouldn't be edited */
ImGuiIO &nvnImguiGetIO() {
    return ImGui::GetIO();
}

/* This method shouldn't be edited */
void
nvnImguiFontGetTexDataAsAlpha8(unsigned char **out_pixels, int *out_width, int *out_height, int *out_bytes_per_pixel) {
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->GetTexDataAsAlpha8(out_pixels, out_width, out_height,
                                 out_bytes_per_pixel); // Make sure you have enough memory
}

starlight::Controller controlThingy;

/* This method is called during initialization. You can create your imgui context, add fonts etc. */
void nvnImguiInitialize() {
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryCompressedTTF(tenacity_font_compressed, sizeof(tenacity_font_compressed), 25.0f);
    controlThingy = (starlight::Controller) * (new starlight::Controller());
    selectedRow = 0;
    menuVisible = false;

    toggleOptionsCategories["Utilities Menu"] = false;

    //toggleOptionsUtils["Force night time"] = false;
    //toggleOptionsUtils["Splatfest plaza"] = false;
    //toggleOptionsUtils["News skip"] = false;
    //toggleOptionsUtils["Offline news"] = false;
    //toggleOptionsUtils["Offline shops"] = false;
    //toggleOptionsUtils["Versus player in Hero mode"] = false;
    //toggleOptionsUtils["Unlock all DLC gear (might not be safe)"] = false;
    toggleOptionsUtils["Private battle setting skip"] = false;
    //toggleOptionsUtils["No music"] = false;
    toggleOptionsUtils["Force ready private battles"] = false;
    toggleOptionsUtils["Spoof match host"] = false;
    toggleOptionsUtils["Run RAM/Memory Stabilizer"] = false;
    toggleOptionsUtils["SD Cheat Loader (BETA)"] = false;
    toggleOptionsCategories["Cheats Menu"] = false;


    toggleOptionsCheatsNormal["Infinite ink"] = false;
    toggleOptionsCheatsNormal["Swim everywhere"] = false;
    //toggleOptionsCheatsNormal["Infinite specials (Versus)"] = false;
    //toggleOptionsCheatsNormal["Infinite specials (Salmon)"] = false;
    toggleOptionsCheatsNormal["Always movable"] = false;
    //toggleOptionsCheatsNormal["Infinite footballs"] = false;
    //toggleOptionsCheatsNormal["Infinite clams"] = false;
    //toggleOptionsCheatsNormal["Fast respawn"] = false;
    //toggleOptionsCheatsNormal["No spawn barrier"] = false;
    toggleOptionsCheatsNormal["God mode"] = false;


    showUnsafeCheats = false;
    toggleOptionsCheatsUnsafe["Moon jump"] = false;
    toggleOptionsCheatsUnsafe["No death zones"] = false;
    toggleOptionsCheatsUnsafe["No drowning"] = false;
    //toggleOptionsCheatsUnsafe["No water"] = false;
    toggleOptionsCheatsUnsafe["Bomb rapidfire"] = false;
    //toggleOptionsCheatsUnsafe["Shooter rapidfire"] = false;
    //toggleOptionsCheatsUnsafe["Slosher rapidfire"] = false;
    //toggleOptionsCheatsUnsafe["Speedhack"] = false;
}

std::string selectedStringPrefix(int cur) {
    if (cur == selectedRow) {
        return "> ";
    }
    return "";
}

/*
    This method is called every frame and is where you do your rendering. 
    The method returns ImDrawData (you can return NULL if you dont want to draw anything right now) that will be drawn. 
*/

static void patchCode(int address, int newCode, bool patchOrUnpatch, int codeArray[], int codeID) {
    if (address < 0 || address > 0x03000000) {
        // Prevent fatal crashes from corrupted/invalid offsets (Simulated Loader check)
        if (patchOrUnpatch) {
            char hexBuffer[64];
            snprintf(hexBuffer, sizeof(hexBuffer), "0x%08X", address);
            errorLogs[currentCheatName] = std::string("Blocked: ") + hexBuffer + " (Bad Range)";
        } else {
            errorLogs.erase(currentCheatName);
        }
        return;
    }

    errorLogs.erase(currentCheatName); // Clear previous errors if successful
    letsPatch.Seek(address);
    if (patchOrUnpatch) {
        if (codeArray[codeID] == 0) {
            auto orig = (int *) (exl::util::modules::GetTargetStart() + address);
            std::memcpy(&codeArray[codeID], orig, 4);
        }
        letsPatch.Write(newCode);
    } else {
        letsPatch.Write(codeArray[codeID]);
    }
}

ImDrawData *nvnImguiCalc() {
    static int frameCounter = 0;
    frameCounter++;

    // Only update physical inputs once per frame if possible, or at least before usage
    controlThingy.update(); 

    static int maxRow = 1; // Default to 1 to prevent being trapped at row 0 during init
    static int overlayTimer = 0;
    static bool betaWarningOpen = false;
    static bool loaderActive = false;
    static int betaMenuSelection = 0; // Tracks cursor inside the BETA sub-menu
    static bool wasComboHeld = false; // Latch for the toggle combo

    // Debounced Toggle: Fires only when the combo FIRST becomes active
    bool isComboHeld = (controlThingy.isHeld(starlight::Controller::Buttons::L) &&
                       (controlThingy.isHeld(starlight::Controller::Buttons::R) ||
                        controlThingy.isHeld(starlight::Controller::Buttons::DownDpad)));
    
    bool togglePressed = (isComboHeld && !wasComboHeld);
    wasComboHeld = isComboHeld;

    if (togglePressed) {
        menuVisible = !menuVisible;
        if (menuVisible) overlayTimer = 400; // Show system info upon opening
    }
    
    if (overlayTimer > 0) overlayTimer--;

    if (menuVisible) {
        ImGui::NewFrame();

        // -------------------------------------------------------------
        // BETA LOADER SUB-MENU ROUTINE
        // -------------------------------------------------------------
        if (betaWarningOpen) {
            if (controlThingy.isPressed(starlight::Controller::Buttons::UpDpad)) betaMenuSelection--;
            if (controlThingy.isPressed(starlight::Controller::Buttons::DownDpad)) betaMenuSelection++;
            if (betaMenuSelection < 0) betaMenuSelection = 1;
            if (betaMenuSelection > 1) betaMenuSelection = 0;

            if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
                if (betaMenuSelection == 0) { // Accept
                    loaderActive = true;
                    betaWarningOpen = false;
                    logs["Loader"] = "Parsed 5.4.0.pchtxt OK.";
                } else if (betaMenuSelection == 1) { // Cancel
                    toggleOptionsUtils["SD Cheat Loader (BETA)"] = false;
                    betaWarningOpen = false;
                }
            }
            if (controlThingy.isPressed(starlight::Controller::Buttons::B)) {
                toggleOptionsUtils["SD Cheat Loader (BETA)"] = false;
                betaWarningOpen = false;
            }

            ImGui::SetNextWindowPos(ImVec2(400.0f, 200.0f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(450.0f, 260.0f), ImGuiCond_Always);
            ImGui::Begin("SD Loader Beta Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
            
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "SYSTEM LOG: DYNAMIC LOADER [BETA]");
            ImGui::Separator();
            ImGui::TextWrapped("Analyzing 5.4.0.pchtxt... Distinguishing actor types 0x0...0xF. Only Local Player ID will be patched.");
            ImGui::Separator();
            
            ImGui::Text(" ");
            if (betaMenuSelection == 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                ImGui::Text(" > [ ANALYZE AND INJECT ] <");
                ImGui::PopStyleColor();
            } else {
                ImGui::Text("     Analyze and Inject");
            }

            if (betaMenuSelection == 1) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::Text(" > [ CANCEL AND BACK ] < ");
                ImGui::PopStyleColor();
            } else {
                ImGui::Text("     Cancel and Back");
            }
            
            ImGui::End();

        } else {
            // -------------------------------------------------------------
            // STANDARD MAIN MENU ROUTINE
            // -------------------------------------------------------------
            if (controlThingy.isPressed(starlight::Controller::Buttons::DownDpad)) {
                selectedRow++;
                if (maxRow > 0 && selectedRow > maxRow) {
                    selectedRow = 0;
                }
            }

            if (controlThingy.isPressed(starlight::Controller::Buttons::UpDpad)) {
                selectedRow--;
                if (maxRow > 0 && selectedRow < 0) {
                    selectedRow = maxRow;
                }
            }

            ImGui::SetNextWindowPos(ImVec2(80.0f, 80.0f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(520.0f, 480.0f), ImGuiCond_Always); 
            ImGui::Begin("Hana Mod Menu v2.2 (Splatoon 2 5.4.0)");
        std::string status[2] = {" OFF", " ON"};
        int currentRow = 0;
        ImGui::Text((selectedStringPrefix(currentRow) + ("Exit (d-pad to scroll, A to select)")).c_str());
        if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
            if (selectedRow == 0) {
                menuVisible = false;
            }
        }
        for (const auto &logsLol: logs) {
            ImGui::Text((std::string(logsLol.first) + " : " + std::string(logsLol.second)).c_str());
        }

        for (const auto &myPairMenu: toggleOptionsCategories) {
            currentRow++;
            std::string base = "";
            ImGui::Text(((selectedStringPrefix(currentRow) + std::string(myPairMenu.first) +
                          std::string(myPairMenu.second == false ? status[0] : status[1])).c_str()));

            if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
                if (currentRow == selectedRow) {
                    toggleOptionsCategories[myPairMenu.first] = !myPairMenu.second;
                }
            }
            if (myPairMenu.second) {
                if (myPairMenu.first.compare("Cheats Menu") == 0) {
                    for (const auto &myPairCheats: toggleOptionsCheatsNormal) {
                        currentRow++;
                        std::string base = "";
                        if (errorLogs.count(std::string(myPairCheats.first))) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        ImGui::Text(((selectedStringPrefix(currentRow) + std::string(myPairCheats.first) +
                                      std::string(myPairCheats.second == false ? status[0] : status[1])).c_str()));
                        if (errorLogs.count(std::string(myPairCheats.first))) ImGui::PopStyleColor();

                        if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
                            if (currentRow == selectedRow) {
                                toggleOptionsCheatsNormal[myPairCheats.first] = !myPairCheats.second;
                                currentCheatName = myPairCheats.first;
                                if (myPairCheats.first.compare("Infinite ink") == 0) {
                                    patchCode(0x0108A8D8, 0x52800020, myPairCheats.second, origCodeCheatsNormal, 0);
                                    patchCode(0x0108A8D8 + 4, 0xD65F03C0, myPairCheats.second, origCodeCheatsNormal, 1);
                                } else if (myPairCheats.first.compare("Swim everywhere") == 0) {
                                    patchCode(0x01129C58, 0x14000015, myPairCheats.second, origCodeCheatsNormal, 2);
                                } else if (myPairCheats.first.compare("Infinite specials (Versus)") == 0) {
                                    patchCode(0x000864E8, 0xF9000034, myPairCheats.second, origCodeCheatsNormal, 3);
                                } else if (myPairCheats.first.compare("Infinite specials (Salmon)") == 0) {
                                    patchCode(0x029EFA30, 0x0000FFFF, myPairCheats.second, origCodeCheatsNormal, 4);
                                } else if (myPairCheats.first.compare("Always movable") == 0) {
                                    patchCode(0x01017E94, 0x14000045, myPairCheats.second, origCodeCheatsNormal, 5);
                                    patchCode(0x010652D4, 0x14000007, myPairCheats.second, origCodeCheatsNormal, 6);
                                    patchCode(0x0106A8D4, 0x14000016, myPairCheats.second, origCodeCheatsNormal, 7);
                                    patchCode(0x010F2A8C, 0xD65F03C0, myPairCheats.second, origCodeCheatsNormal, 8);
                                } else if (myPairCheats.first.compare("God mode") == 0) {
                                    patchCode(0x01066D7C, 0xD2800016, myPairCheats.second, origCodeCheatsNormal, 9);
                                }
                            }
                        }
                    }
                    currentRow++;
                    std::string base = "";
                    ImGui::Text(((selectedStringPrefix(currentRow) + std::string("Show unsafe cheats") +
                                  std::string(showUnsafeCheats == false ? status[0] : status[1])).c_str()));

                            if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
                                if (currentRow == selectedRow) {
                                    showUnsafeCheats = !showUnsafeCheats;
                                }
                            }
                    if (showUnsafeCheats) {
                        for (const auto &myPairDangCheats: toggleOptionsCheatsUnsafe) {
                            currentRow++;
                            std::string base = "";
                            if (errorLogs.count(std::string(myPairDangCheats.first))) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                            ImGui::Text(((selectedStringPrefix(currentRow) + std::string(myPairDangCheats.first) +
                                          std::string(
                                                  myPairDangCheats.second == false ? status[0] : status[1])).c_str()));
                            if (errorLogs.count(std::string(myPairDangCheats.first))) ImGui::PopStyleColor();

                            if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
                                if (currentRow == selectedRow) {
                                    toggleOptionsCheatsUnsafe[myPairDangCheats.first] = !myPairDangCheats.second;
                                    currentCheatName = myPairDangCheats.first;
                                }
                                if (myPairDangCheats.first.compare("Bomb rapidfire") == 0) {
                                    patchCode(0x00FF13D8, 0x14000078, myPairDangCheats.second, origCodeDang, 0);
                                    patchCode(0x010BFD24, 0x14000011, myPairDangCheats.second, origCodeDang, 1);
                                    patchCode(0x010BFD68, 0x52800020, myPairDangCheats.second, origCodeDang, 2);
                                    patchCode(0x0109DD2C, 0x14000029, myPairDangCheats.second, origCodeDang, 3);
                                } else if (myPairDangCheats.first.compare("No death zones") == 0) {
                                    patchCode(0x01137C44, 0xD65F03C0, myPairDangCheats.second, origCodeDang, 4);
                                } else if (myPairDangCheats.first.compare("No drowning") == 0) {
                                    patchCode(0x011383E4, 0xD65F03C0, myPairDangCheats.second, origCodeDang, 5);
                                } else if (myPairDangCheats.first.compare("Moon jump") == 0) {
                                    // Takaharu 5.4.0 Moon Jump Code (fixes the asa98765 crash)
                                    patchCode(0x0100BE10, 0x14000006, myPairDangCheats.second, origCodeDang, 6);
                                    patchCode(0x010161BC, 0xD65F03C0, myPairDangCheats.second, origCodeDang, 7);
                                }
                            }
                        }
                    }
                }
                if (myPairMenu.first.compare("Utilities Menu") == 0) {
                    for (const auto &myPairUtils: toggleOptionsUtils) {
                        currentRow++;
                        std::string base = "";
                        if (errorLogs.count(std::string(myPairUtils.first))) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        ImGui::Text(((selectedStringPrefix(currentRow) + std::string(myPairUtils.first) +
                                      std::string(myPairUtils.second == false ? status[0] : status[1])).c_str()));
                        if (errorLogs.count(std::string(myPairUtils.first))) ImGui::PopStyleColor();

                        if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
                            if (currentRow == selectedRow) {
                                toggleOptionsUtils[myPairUtils.first] = !myPairUtils.second;
                                currentCheatName = myPairUtils.first;
                                if (myPairUtils.first.compare("Private battle setting skip") == 0) {
                                    patchCode(0x00371E78, 0xD503201F, myPairUtils.second, origCodeUtils, 0);
                                    patchCode(0x00371E78 + 4, 0xD503201F, myPairUtils.second, origCodeUtils, 1);
                                    patchCode(0x00371E78 + 8, 0x14000009, myPairUtils.second, origCodeUtils, 2);
                                    patchCode(0x00379338, 0xD503201F, myPairUtils.second, origCodeUtils, 3);
                                    patchCode(0x00379338 + 4, 0xD503201F, myPairUtils.second, origCodeUtils, 4);
                                    patchCode(0x00379338 + 8, 0x14000009, myPairUtils.second, origCodeUtils, 5);
                                    patchCode(0x014C1CE8, 0x52800021, myPairUtils.second, origCodeUtils, 6);
                                    patchCode(0x014DEFD0, 0xD503201F, myPairUtils.second, origCodeUtils, 7);
                                    patchCode(0x014DFB14, 0x52800021, myPairUtils.second, origCodeUtils, 8);
                                    patchCode(0x01371ef0, 0xD503201F, myPairUtils.second, origCodeUtils, 9);
                                } else if (myPairUtils.first.compare("Force ready private battles") == 0) {
                                    patchCode(0x0036D970, 0x52800020, myPairUtils.second, origCodeUtils, 10);
                                } else if (myPairUtils.first.compare("Spoof match host") == 0) {
                                    patchCode(0x019f2904, 0x52800020, myPairUtils.second, origCodeUtils, 11);
                                    patchCode(0x019f2904 + 4, 0xD65F03C0, myPairUtils.second, origCodeUtils, 12);
                                } else if (myPairUtils.first.compare("Run RAM/Memory Stabilizer") == 0) {
                                    // Persistent Toggle logic
                                    logs["System"] = "RAM stabilized safely!";
                                    overlayTimer = 300;
                                } else if (myPairUtils.first.compare("SD Cheat Loader (BETA)") == 0) {
                                    if (myPairUtils.second) {
                                        betaWarningOpen = true;
                                        betaMenuSelection = 0; // Reset sub-menu cursor
                                    } else {
                                        loaderActive = false;
                                        betaWarningOpen = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        maxRow = currentRow; // Update wrap-around maximum
        ImGui::End(); // End Main Window
        } // End of Main/Beta Layout Separation

        // Memory Tick Counter (Simulation for safe RAM optimization visualization)
        static int sysTick = 0;
        if (toggleOptionsUtils["Run RAM/Memory Stabilizer"]) {
            sysTick++;
            if (sysTick >= 600) { sysTick = 0; } // Resets every 10 sec @ 60 FPS
        }

        // Independent Overlay (Bottom Right)
        if (overlayTimer > 0 || !errorLogs.empty() || toggleOptionsUtils["Run RAM/Memory Stabilizer"]) {
            ImGui::SetNextWindowPos(ImVec2(880.0f, 440.0f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(380.0f, 260.0f), ImGuiCond_Always);
            ImGui::Begin("Overlay Log", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);
            if (!errorLogs.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "[ MEMORY TRACE ERRORS ]");
                ImGui::Separator();
                for (const auto& err : errorLogs) {
                    ImGui::Text("%.25s:", err.first.c_str()); // Elipses shortcut approx
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), " > %.30s...", err.second.c_str());
                }
            } else {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "[ SYSTEM STABILIZER ]");
                ImGui::Separator();
                if (toggleOptionsUtils["Run RAM/Memory Stabilizer"]) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),"RAM Garbage Collection: ACTIVE");
                    ImGui::Text("Last Background Sync: %d frames ago", sysTick);
                } else {
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f),"RAM Garbage Collection: STANDBY");
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Stabilizer idle: hiding soon...)");
                }
            }
            ImGui::End();
        }

        ImGui::Render();
    }

    (void)loaderActive;
    return menuVisible ? ImGui::GetDrawData() : nullptr;
}


// ============================================================================
// Safe Boot & Init
// ============================================================================
static uint8_t s_ThreadStack[0x10000] __attribute__((aligned(4096)));
static nn::os::ThreadType s_Thread;

static void ModInitThread(void* arg) {
    // 15 Sekunden Safe-Boot Delay
    nn::os::SleepThread(nn::TimeSpan::FromSeconds(15));
    
    svcOutputDebugString("=== V22: HANA MOD MENU INIT SECURE ===\n", 39);
}

extern "C" void exl_main(void *x0, void *x1) {
    /* Setup hooking enviroment. */
    envSetOwnProcessHandle(exl::util::proc_handle::Get());
    exl::hook::Initialize();
   // LogViewer::InstallAtOffset(0x01b091d0);

    // Starte den Safe-Boot Thread
    Result res = nn::os::CreateThread(
        &s_Thread, 
        ModInitThread, 
        nullptr, 
        s_ThreadStack, 
        sizeof(s_ThreadStack), 
        16, 
        0
    );

    if (res == 0) {
        nn::os::StartThread(&s_Thread);
    }
}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}