#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <d3d9.h>
#include <dwmapi.h>

#include "classes/utils.h"
#include "memory/memory.hpp"
#include "classes/vector.hpp"
#include "hacks/reader.hpp"
#include "hacks/hack.hpp"
#include "classes/globals.hpp"
#include "classes/render.hpp"
#include "classes/auto_updater.hpp"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")

bool finish = false;
bool showMenu = true;

// D3D9 para o menu ImGui
LPDIRECT3D9       g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pDevice = NULL;
HWND              g_hMenu = NULL;

// ── GDI OVERLAY ──────────────────────────────────────────────────────────────
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        g::hdcBuffer = CreateCompatibleDC(NULL);
        g::hbmBuffer = CreateCompatibleBitmap(GetDC(hWnd), g::gameBounds.right, g::gameBounds.bottom);
        SelectObject(g::hdcBuffer, g::hbmBuffer);
        SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 0, LWA_COLORKEY);
        std::cout << "[overlay] Window created successfully" << std::endl;
        Beep(500, 100);
        break;
    }
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        FillRect(g::hdcBuffer, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
        if (GetForegroundWindow() == g_game.process->hwnd_ || GetForegroundWindow() == g_hMenu)
            hack::loop();
        BitBlt(hdc, 0, 0, g::gameBounds.right, g::gameBounds.bottom, g::hdcBuffer, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }
    case WM_DESTROY:
        DeleteDC(g::hdcBuffer);
        DeleteObject(g::hbmBuffer);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// ── MENU IMGUI (janela separada D3D9) ────────────────────────────────────────
LRESULT CALLBACK MenuWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool InitD3D(HWND hWnd, int w, int h)
{
    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_pD3D) return false;
    D3DPRESENT_PARAMETERS pp = {};
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.hDeviceWindow = hWnd;
    pp.BackBufferWidth = w;
    pp.BackBufferHeight = h;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    return SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &g_pDevice));
}

void menu_thread()
{
    const int MW = 500, MH = 450;

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MenuWndProc, 0, 0,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "JohnMenuClass", NULL };
    RegisterClassEx(&wc);

    g_hMenu = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        "JohnMenuClass", "John.exe DEV",
        WS_POPUP,
        100, 100, MW, MH,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    MARGINS m = { -1 };
    DwmExtendFrameIntoClientArea(g_hMenu, &m);
    SetLayeredWindowAttributes(g_hMenu, RGB(0, 0, 0), 0, LWA_COLORKEY);

    if (!InitD3D(g_hMenu, MW, MH)) return;

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;

    // Estilo
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.f;
    style.FrameRounding = 4.f;
    style.ItemSpacing = ImVec2(8, 6);
    style.Colors[ImGuiCol_WindowBg] = ImColor(15, 15, 15, 245);
    style.Colors[ImGuiCol_Border] = ImColor(50, 50, 50, 200);
    style.Colors[ImGuiCol_FrameBg] = ImColor(30, 30, 30, 255);
    style.Colors[ImGuiCol_FrameBgHovered] = ImColor(45, 45, 45, 255);
    style.Colors[ImGuiCol_CheckMark] = ImColor(0, 200, 80, 255);
    style.Colors[ImGuiCol_SliderGrab] = ImColor(0, 200, 80, 255);
    style.Colors[ImGuiCol_Button] = ImColor(35, 35, 35, 255);
    style.Colors[ImGuiCol_ButtonHovered] = ImColor(55, 55, 55, 255);
    style.Colors[ImGuiCol_Header] = ImColor(0, 150, 60, 180);
    style.Colors[ImGuiCol_HeaderHovered] = ImColor(0, 180, 70, 200);
    style.Colors[ImGuiCol_Tab] = ImColor(25, 25, 25, 255);
    style.Colors[ImGuiCol_TabActive] = ImColor(0, 160, 65, 255);
    style.Colors[ImGuiCol_TabHovered] = ImColor(0, 180, 70, 200);
    style.Colors[ImGuiCol_TitleBgActive] = ImColor(10, 10, 10, 255);

    ImGui_ImplWin32_Init(g_hMenu);
    ImGui_ImplDX9_Init(g_pDevice);

    ShowWindow(g_hMenu, SW_SHOW);

    // ✅ VARIÁVEIS LOCAIS DO MENU - SINCRONIZADAS COM CONFIG
    bool menu_show_box_esp = config::show_box_esp;
    bool menu_show_skeleton_esp = config::show_skeleton_esp;
    bool menu_show_head_tracker = config::show_head_tracker;
    bool menu_team_esp = config::team_esp;
    bool menu_show_extra_flags = config::show_extra_flags;
    bool menu_show_console = config::show_console;
    bool menu_streamproof = config::streamproof;
    bool menu_show_health_bar = config::show_health_bar;
    bool menu_show_armor_bar = config::show_armor_bar;

    // ✅ FLAGS INDIVIDUAIS DO MENU
    bool menu_show_flag_name = true;
    bool menu_show_flag_health = true;
    bool menu_show_flag_armor = true;
    bool menu_show_flag_weapon = true;
    bool menu_show_flag_distance = true;
    bool menu_show_flag_money = true;
    bool menu_show_flag_flashed = true;
    bool menu_show_flag_defusing = true;

    // Cores do ESP como float[3] para o ColorEdit
    float colEnemy[3] = { GetRValue(config::esp_box_color_enemy) / 255.f, GetGValue(config::esp_box_color_enemy) / 255.f, GetBValue(config::esp_box_color_enemy) / 255.f };
    float colTeam[3] = { GetRValue(config::esp_box_color_team) / 255.f,  GetGValue(config::esp_box_color_team) / 255.f,  GetBValue(config::esp_box_color_team) / 255.f };

    MSG msg;
    while (!finish)
    {
        while (PeekMessage(&msg, g_hMenu, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
            showMenu = !showMenu;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (showMenu)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2((float)MW, (float)MH), ImGuiCond_Always);
            ImGui::Begin("##main", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

            // Título
            float tw = ImGui::CalcTextSize("John.exe  DEV").x;
            ImGui::SetCursorPosX((MW - tw) * 0.5f - 10);
            ImGui::TextColored(ImColor(0, 230, 80), "John.exe ");
            ImGui::SameLine();
            ImGui::TextColored(ImColor(180, 180, 180), "DEV");
            ImGui::Separator();
            ImGui::Spacing();

            // Tabs
            if (ImGui::BeginTabBar("tabs"))
            {
                // ── VISUALS ──────────────────────────────
                if (ImGui::BeginTabItem("Visuals"))
                {
                    ImGui::Spacing();
                    ImGui::Checkbox("Box ESP", &menu_show_box_esp);
                    ImGui::Checkbox("Skeleton", &menu_show_skeleton_esp);
                    ImGui::Checkbox("Head Tracker", &menu_show_head_tracker);
                    ImGui::Checkbox("Show Team", &menu_team_esp);
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    ImGui::Text("ESP Visible:");
                    ImGui::Checkbox("Health Bar", &menu_show_health_bar);
                    ImGui::Checkbox("Armor Bar", &menu_show_armor_bar);
                    ImGui::Spacing();
                    ImGui::Text("Box Colors:");
                    if (ImGui::ColorEdit3("Enemy##col", colEnemy, ImGuiColorEditFlags_NoInputs))
                        config::esp_box_color_enemy = RGB((BYTE)(colEnemy[0] * 255), (BYTE)(colEnemy[1] * 255), (BYTE)(colEnemy[2] * 255));
                    if (ImGui::ColorEdit3("Team##col", colTeam, ImGuiColorEditFlags_NoInputs))
                        config::esp_box_color_team = RGB((BYTE)(colTeam[0] * 255), (BYTE)(colTeam[1] * 255), (BYTE)(colTeam[2] * 255));
                    ImGui::EndTabItem();
                }

                // ── FLAGS ────────────────────────────────
                if (ImGui::BeginTabItem("Flags"))
                {
                    ImGui::Spacing();
                    ImGui::Checkbox("Extra Flags", &menu_show_extra_flags);
                    ImGui::Spacing();
                    ImGui::Text("Enabled flags:");
                    ImGui::Indent();
                    ImGui::Checkbox("Name##flag", &menu_show_flag_name);
                    ImGui::Checkbox("Health (hp)##flag", &menu_show_flag_health);
                    ImGui::Checkbox("Armor##flag", &menu_show_flag_armor);
                    ImGui::Checkbox("Weapon##flag", &menu_show_flag_weapon);
                    ImGui::Checkbox("Distance (m)##flag", &menu_show_flag_distance);
                    ImGui::Checkbox("Money ($)##flag", &menu_show_flag_money);
                    ImGui::Checkbox("Flashed##flag", &menu_show_flag_flashed);
                    ImGui::Checkbox("Defusing##flag", &menu_show_flag_defusing);
                    ImGui::Unindent();
                    ImGui::Spacing();
                    ImGui::Text("Flag render distance:");
                    ImGui::SliderInt("##dist", &config::flag_render_distance, 5, 100, "%dm");
                    ImGui::EndTabItem();
                }

                // ── SETTINGS ─────────────────────────────
                if (ImGui::BeginTabItem("Settings"))
                {
                    ImGui::Spacing();
                    ImGui::Checkbox("Console", &menu_show_console);
                    ImGui::Checkbox("Streamproof", &menu_streamproof);
#ifndef _UC
                    ImGui::Checkbox("Auto Update", &config::automatic_update);
#endif
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    ImGui::Text("Cache Refresh Rate:");
                    ImGui::SliderInt("##cache", &config::cache_refresh_rate, 1, 20, "%dms");
                    ImGui::Spacing();
                    if (ImGui::Button("Save Config", ImVec2(-1, 28)))
                        config::save();
                    ImGui::Spacing();
                    if (ImGui::Button("Unload ESP", ImVec2(-1, 28)))
                        finish = true;
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            // Rodapé
            ImGui::SetCursorPosY(MH - 28);
            ImGui::Separator();
            ImGui::TextColored(ImColor(80, 80, 80), "INSERT = toggle menu");

            ImGui::End();

            // Drag para mover a janela
            static POINT lastMouse = {};
            static bool dragging = false;
            if (ImGui::IsMouseClicked(0)) {
                GetCursorPos(&lastMouse);
                dragging = true;
            }
            if (!ImGui::IsMouseDown(0)) dragging = false;
            if (dragging && !ImGui::IsAnyItemHovered()) {
                POINT cur; GetCursorPos(&cur);
                RECT wr; GetWindowRect(g_hMenu, &wr);
                SetWindowPos(g_hMenu, NULL,
                    wr.left + (cur.x - lastMouse.x),
                    wr.top + (cur.y - lastMouse.y),
                    0, 0, SWP_NOSIZE | SWP_NOZORDER);
                lastMouse = cur;
            }
        }

        // ✅ SINCRONIZA OS VALORES DO MENU COM O CONFIG
        config::show_box_esp = menu_show_box_esp;
        config::show_skeleton_esp = menu_show_skeleton_esp;
        config::show_head_tracker = menu_show_head_tracker;
        config::team_esp = menu_team_esp;
        config::show_extra_flags = menu_show_extra_flags;
        config::show_console = menu_show_console;
        config::streamproof = menu_streamproof;
        config::show_health_bar = menu_show_health_bar;
        config::show_armor_bar = menu_show_armor_bar;
        config::show_flag_name = menu_show_flag_name;
        config::show_flag_health = menu_show_flag_health;
        config::show_flag_armor = menu_show_flag_armor;
        config::show_flag_weapon = menu_show_flag_weapon;
        config::show_flag_distance = menu_show_flag_distance;
        config::show_flag_money = menu_show_flag_money;
        config::show_flag_flashed = menu_show_flag_flashed;
        config::show_flag_defusing = menu_show_flag_defusing;

        ImGui::EndFrame();

        g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.f, 0);
        if (SUCCEEDED(g_pDevice->BeginScene())) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pDevice->EndScene();
        }
        g_pDevice->Present(NULL, NULL, NULL, NULL);

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (g_pDevice) g_pDevice->Release();
    if (g_pD3D)    g_pD3D->Release();
    DestroyWindow(g_hMenu);
    UnregisterClass("JohnMenuClass", GetModuleHandle(NULL));
}

// ── READ THREAD ───────────────────────────────────────────────────────────────
void read_thread() {
    while (!finish) {
        g_game.loop();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

// ── MAIN ──────────────────────────────────────────────────────────────────────
int main() {
    utils.update_console_title();

    std::cout << "[info] John.exe DEV - CS2 External ESP\n" << std::endl;

    std::cout << "[config] Reading configuration." << std::endl;
    if (config::read())
        std::cout << "[updater] Successfully read configuration file\n" << std::endl;
    else
        std::cout << "[updater] Error reading config file, resetting to the default state\n" << std::endl;

#ifndef _UC
    try { updater::check_and_update(config::automatic_update); }
    catch (std::exception& e) { std::cout << "Exception: " << e.what() << std::endl; }
#endif

    std::cout << "[updater] Reading offsets from file offsets.json." << std::endl;
    if (updater::read())
        std::cout << "[updater] Successfully read offsets file\n" << std::endl;
    else
        std::cout << "[updater] Error reading offsets file, resetting to the default state\n" << std::endl;

    g_game.init();

    if (g_game.buildNumber != updater::build_number) {
        std::cout << "[cs2] Build number doesnt match, offsets may be outdated." << std::endl;
        std::cout << "[cs2] Press any key to continue" << std::endl;
        std::cin.get();
    }
    else {
        std::cout << "[cs2] Offsets seem to be up to date! have fun!" << std::endl;
    }

    std::cout << "[overlay] Waiting to focus game..." << std::endl;
    while (GetForegroundWindow() != g_game.process->hwnd_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        g_game.process->UpdateHWND();
        ShowWindow(g_game.process->hwnd_, TRUE);
    }
    std::cout << "[overlay] Creating overlay..." << std::endl;

    // Inicia menu ImGui em thread separada
    std::thread menuThread(menu_thread);
    menuThread.detach();

    // GDI overlay
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = WHITE_BRUSH;
    wc.hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongA(g_game.process->hwnd_, -6));
    wc.lpszMenuName = " ";
    wc.lpszClassName = " ";
    RegisterClassExA(&wc);

    GetClientRect(g_game.process->hwnd_, &g::gameBounds);

    HWND hWnd = CreateWindowExA(
        WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        " ", "cs2-external-esp", WS_POPUP,
        g::gameBounds.left, g::gameBounds.top,
        g::gameBounds.right - g::gameBounds.left,
        g::gameBounds.bottom + g::gameBounds.left,
        NULL, NULL, NULL, NULL);

    if (!hWnd) return 0;

    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ShowWindow(hWnd, TRUE);

    std::thread read(read_thread);

    std::cout << "[menu] Press INSERT to toggle menu" << std::endl;
    std::cout << "[menu] Press END to unload" << std::endl;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && !finish)
    {
        if (GetAsyncKeyState(VK_END) & 0x8000) finish = true;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    read.detach();
    Beep(700, 100); Beep(700, 100);
    std::cout << "[overlay] Destroying overlay." << std::endl;
    DeleteDC(g::hdcBuffer);
    DeleteObject(g::hbmBuffer);
    DestroyWindow(hWnd);
    g_game.close();

#ifdef NDEBUG
    std::cout << "[cs2] Press any key to close" << std::endl;
    std::cin.get();
#endif

    return 1;
}