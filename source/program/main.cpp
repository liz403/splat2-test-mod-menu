#include "lib.hpp"
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
int selectedRow;
static bool menuVisible;
int currentRow;
static bool showUnsafeCheats;
exl::patch::CodePatcher letsPatch(0x0);
int origCodeCheatsNormal[2 + 1 + 1 + 1 + 4 + 2 + 2 + 2 + 1 + 1];
int origCodeUtils[13];
int origCodeDang[7];
HOOK_DEFINE_TRAMPOLINE(LogViewer) {

        /* Define the callback for when the function is called. Don't forget to make it static and name it Callback. */
        static void Callback(void* thisthing, unsigned int* label, unsigned int* output) {
            std::string s = std::to_string((unsigned long)label);
            std::string s2 = std::to_string((unsigned long)output);
            logs[s] = s2;
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
    if (!menuVisible && controlThingy.isPressed(starlight::Controller::Buttons::L) &&
        controlThingy.isPressed(starlight::Controller::Buttons::R)) {
        menuVisible = true;
    }
    if (menuVisible) {
        if (controlThingy.isPressed(starlight::Controller::Buttons::DownDpad)) {
            selectedRow++;
        }
        if (controlThingy.isPressed(starlight::Controller::Buttons::UpDpad)) {
            selectedRow--;
            if (selectedRow < 0)
                selectedRow = 0;
        }


        ImGui::NewFrame();
        ImGui::Begin("Splatoon 2 mod menu test! this shit sucks");
        ImVec2 res = ImVec2(450.0f, 1200.0f);
        ImGui::SetWindowSize(res);
        ImVec2 pos = ImVec2(0.0f, selectedRow * -20.0f);
        ImGui::SetWindowPos(pos);
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
                        ImGui::Text(((selectedStringPrefix(currentRow) + std::string(myPairCheats.first) +
                                      std::string(myPairCheats.second == false ? status[0] : status[1])).c_str()));

                        if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
                            if (currentRow == selectedRow) {
                                toggleOptionsCheatsNormal[myPairCheats.first] = !myPairCheats.second;
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
                            ImGui::Text(((selectedStringPrefix(currentRow) + std::string(myPairDangCheats.first) +
                                          std::string(
                                                  myPairDangCheats.second == false ? status[0] : status[1])).c_str()));

                            if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
                                if (currentRow == selectedRow) {
                                    toggleOptionsCheatsUnsafe[myPairDangCheats.first] = !myPairDangCheats.second;
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
                                    patchCode(0x024BB9B4, 0x3D2D2D0A, myPairDangCheats.second, origCodeDang, 6);
                                }
                            }
                        }
                    }
                }
                if (myPairMenu.first.compare("Utilities Menu") == 0) {
                    for (const auto &myPairUtils: toggleOptionsUtils) {
                        currentRow++;
                        std::string base = "";
                        ImGui::Text(((selectedStringPrefix(currentRow) + std::string(myPairUtils.first) +
                                      std::string(myPairUtils.second == false ? status[0] : status[1])).c_str()));

                        if (controlThingy.isPressed(starlight::Controller::Buttons::A)) {
                            if (currentRow == selectedRow) {
                                toggleOptionsUtils[myPairUtils.first] = !myPairUtils.second;
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
                                }
                            }
                        }
                    }
                }
            }
        }
        ImGui::End();

        ImGui::Render();
    }

    controlThingy.update();
    return menuVisible ? ImGui::GetDrawData() : nullptr;
}


extern "C" void exl_main(void *x0, void *x1) {





    /* Setup hooking enviroment. */
    envSetOwnProcessHandle(exl::util::proc_handle::Get());
    exl::hook::Initialize();
   // LogViewer::InstallAtOffset(0x01b091d0);


}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}