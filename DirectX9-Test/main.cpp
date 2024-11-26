// include the basic windows header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"

#include "custom.h"
#include "font_awesome.h"
#include "font_awesome_data.h"
#include <vector>
#include <string>

// Declarations
LPDIRECT3D9 pD3D; // Pointer  to the Direct3D Interface
LPDIRECT3DDEVICE9 pD3DDevice; // Pointer to the device class
LPDIRECT3DVERTEXBUFFER9 v_buffer = nullptr; // Draw Here on the backbuffer

ImFont* icons_font = nullptr;
ImFont* menuFont;
ImFont* boldMenuFont;
ImFont* largerFont;

ImColor accentColor = ImColor(71, 20, 121, 255);
ImColor accentColorTrans = ImColor(71, 20, 121, 100);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct CUSTOMVERTEX
{
    FLOAT x, y, z; // D3DFVF_XYZR FLAG
    DWORD color; // D3DFVF_DIFFUSE FLAG
};

CUSTOMVERTEX t_vert[] =
{
    { 3.f, -3.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), },
    { 0.0f, 3.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 0), },
    { -3.f, -3.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 0), },
};

// define the screen resolution
#define SCREEN_WIDTH  2560
#define SCREEN_HEIGHT 1440

// Function prototype
void initD3D(HWND hWnd); // Setup and initialzition for Direct3D
void render_frame(); // Renders a single frame
void cleanD3D(); // Closes Direct3D and releases the memory
void init_graphics(); // Initialize vertexbuffer to draw vertices

// Create custom FLEXIBLE VERTEX FORMAT
#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

D3DXVECTOR3 vEye(0.0f, 0.0f, 10.f);
D3DXVECTOR3 vAt(0.0f, 0.0f, 0.0f);
D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f);

// the entry point for any Windows program
int WINAPI WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow)
{
    // the handle for the window, filled by a function
    HWND hWnd;
    // this struct holds information for the window class
    WNDCLASSEX wc;

    // clear out the window class for use
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    // fill in the struct with the needed information
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   // wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass1";

    // register the window class
    RegisterClassEx(&wc);

    // create the window and use the result as the handle
    hWnd = CreateWindowEx(NULL,
        L"WindowClass1",    // name of the window class
        L"Get hacked nerd lol.",    // title of the window
        WS_EX_TOPMOST | WS_POPUP,    // window style
        0,    // x-position of the window
        0,    // y-position of the window
        SCREEN_WIDTH,    // width of the window
        SCREEN_HEIGHT,    // height of the window
        NULL,    // we have no parent window, NULL
        NULL,    // we aren't using menus, NULL
        hInstance,    // application handle
        NULL);    // used with multiple windows, NULL

// display the window on the screen
    ShowWindow(hWnd, nCmdShow);

    // set up and initialize Direct3D
    initD3D(hWnd);

    // enter the main loop:

    // this struct holds Windows event messages
    MSG msg;

    // Enter the infinite message loop
    while (TRUE)
    {
        // Check to see if any messages are waiting in the queue
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // translate keystroke messages into the right format
            TranslateMessage(&msg);

            // send the message to the WindowProc function
            DispatchMessage(&msg);
        }

        // If the message is WM_QUIT, exit the while loop
        if (msg.message == WM_QUIT)
            break;

      //  vEye.y += 0.1f;
       // vAt.y += 0.1f;
        render_frame();

        Sleep(100);
    }

    // Clean up DirectX and COM
    cleanD3D();

    // return this part of the WM_QUIT message to Windows
    return msg.wParam;
}

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // sort through and find what code to run for the message given
    ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);


    switch (message)
    {
        // this message is read when the window is closed
    case WM_DESTROY:
    {
        // close the application entirely
        PostQuitMessage(0);
        return 0;
    } break;
    }

    // Handle any messages the switch statement didn't
    return DefWindowProc(hWnd, message, wParam, lParam);
}


void initD3D(HWND hWnd)
{
    pD3D = Direct3DCreate9(D3D_SDK_VERSION); // Create Direct3D Interface

    D3DPRESENT_PARAMETERS d3dpp; // Struct to hold various device information

    ZeroMemory(&d3dpp, sizeof(d3dpp)); // Clear struct
    d3dpp.Windowed = true;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; // Discard old frames
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8; // BackBuffer format to 32-bit
    d3dpp.BackBufferWidth = SCREEN_WIDTH;
    d3dpp.BackBufferHeight = SCREEN_HEIGHT;

    // Create Device class from the d3dpp struct
    pD3D->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &pD3DDevice);

    init_graphics();

    pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE); // Turn off 3D Lighting
    // Draws the back of the triangle
    pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    ImGui::CreateContext();

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX9_Init(pD3DDevice);

    // Styling Start
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

    io.LogFilename = nullptr;
    io.IniFilename = nullptr;

    // doing this because i need to merge awesoem font, but it you cant merge the first font
    ImFontConfig CustomFont;
    CustomFont.FontDataOwnedByAtlas = false;

    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 3;
    icons_config.OversampleV = 3;

    // load a custom font to be able to merge awesome font
    io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 21.f, &CustomFont);
    // icon font
    icons_font = io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 19.5f, &icons_config, icons_ranges);
    // consolas font
    menuFont = io.Fonts->AddFontFromFileTTF(("C:\\Windows\\Fonts\\Consola.ttf"), 14.f);
    largerFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Consola.ttf", 32.0f); // Change the size to your preference

    boldMenuFont = io.Fonts->AddFontFromFileTTF(("C:\\Windows\\Fonts\\Consola.ttf"), 14.f);
    io.Fonts->AddFontDefault();

    auto& colors = style.Colors;

    style.ScrollbarRounding = 0;
    style.WindowRounding = 4.0f;

    colors[ImGuiCol_WindowBg] = ImColor(5, 5, 5, 255);
    colors[ImGuiCol_ChildBg] = ImColor(8, 8, 8, 255);

    colors[ImGuiCol_TextDisabled] = ImColor(102, 59, 148, 135);

    colors[ImGuiCol_Button] = ImColor(6, 6, 6, 255);
    colors[ImGuiCol_ButtonActive] = accentColor;
    colors[ImGuiCol_ButtonHovered] = accentColorTrans;

    colors[ImGuiCol_CheckMark] = accentColor;

    colors[ImGuiCol_FrameBg] = ImColor(0, 0, 0, 255);
    colors[ImGuiCol_FrameBgActive] = ImColor(0, 0, 0, 255);
    colors[ImGuiCol_FrameBgHovered] = ImColor(0, 0, 0, 255);

    colors[ImGuiCol_SliderGrab] = accentColorTrans;
    colors[ImGuiCol_SliderGrabActive] = accentColor;

    colors[ImGuiCol_Header] = accentColor;
    colors[ImGuiCol_HeaderHovered] = accentColorTrans;
    colors[ImGuiCol_HeaderActive] = accentColor;

    colors[ImGuiCol_Border] = accentColor;
    // Styling End
}

void TopLeftButtons(std::vector<std::string> names, std::vector<int> indexes, int &selected_index)
{
    std::vector<ImVec2> sizes = {};
    float total_area = 0.0f;
    float max_button_width = 0.0f;
    float max_button_height = 0.0f;

    const auto& style = ImGui::GetStyle();

    for (std::string& name : names)
    {
        const ImVec2 label_size = ImGui::CalcTextSize(name.c_str(), nullptr, true);
        ImVec2 size = ImGui::CalcItemSize(ImVec2(), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
        size.x += 45.5f;
        size.y += 15.f;

        sizes.push_back(size);
        total_area += size.x;

        max_button_width = max(max_button_width, size.x);
        max_button_height = max(max_button_height, size.y);
    }

    float fWidth = 80; // half of the parent window width

    ImVec2 button_size = ImVec2(max_button_width, max_button_height);

    for (int i = 0; i < names.size(); i++)
    {
        ImGui::SetCursorPosX(fWidth - max_button_width / 2);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 12.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);

        if (selected_index == indexes[i])
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImColor(71, 20, 121, 255).Value);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(71, 20, 121, 255).Value);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(71, 20, 121, 255).Value);

            if (ImGui::Button(names[i].c_str(), button_size))
            {
                selected_index = indexes[i];
            }
            ImGui::PopStyleColor(3);
        }
        else
        {
            if (ImGui::Button(names[i].c_str(), button_size))
            {
                selected_index = indexes[i];
            }
        }

        ImGui::PopStyleVar();
    }

}
float value = 0.5f;
void render_frame()
{
    // Clear Window to Blue
    pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(140,40, 0), 1.0f, 0);

    pD3DDevice->BeginScene(); // Begin 3D Scene (Locks the Memory for us)

    // Select which vertex format is being used
    pD3DDevice->SetFVF(CUSTOMFVF);

    // SET UP THE PIPELINE

    D3DXMATRIX matRotateY;    // a matrix to store the rotation information

    static float index = 0.0f; index += 0.05f;    // an ever-increasing float value

    // build a matrix to rotate the model based on the increasing float value
    D3DXMatrixRotationY(&matRotateY, index);

    // tell Direct3D about our matrix
    //pD3DDevice->SetTransform(D3DTS_WORLD, &(matRotateY));

    D3DXMATRIX matRotateX;    // a matrix to store the rotation information

    static float indexX = 0.0f; indexX += 0.05f;    // an ever-increasing float value

    // build a matrix to rotate the model based on the increasing float value
    D3DXMatrixRotationX(&matRotateX, 5.f);

    D3DXMATRIX matRotateXY = matRotateX * matRotateY;

    // tell Direct3D about our matrix
    pD3DDevice->SetTransform(D3DTS_WORLD, &matRotateXY);
    // tell Direct3D about our matrix

    D3DXMATRIX matView; // Set view transform matrix

// VECTORS

    D3DXMatrixLookAtLH(&matView, 
        &vEye,    // the camera position
        &vAt,    // the look-at position
        &vUp);    // the up direction)

    // Set the view transform to matView
    pD3DDevice->SetTransform(D3DTS_VIEW, &matView);

    D3DXMATRIX matProjection; // projection transform matrix

    D3DXMatrixPerspectiveFovLH(&matProjection, D3DXToRadian(45), (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, 1.0f, 100.f);

    pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProjection); // Set Projection Matrix

    // Select the vertex buffer to display
    pD3DDevice->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));

    // COpy vertexbuffer to backbuffer
    pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 1);

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(850, 600));
    ImGui::SetNextWindowPos(ImVec2(855, 420));

    // Begin Window
    bool b = true;
    static int indxex = 0;

    ImGui::Begin(("Pandemonium"), &b, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize );
    {
        ImGui::BeginChild(("Sidebar"), ImVec2(160, 0), true);
        {
            float window_width = ImGui::GetWindowWidth();
            ImGui::PushFont(largerFont);
            // Set the cursor position to center the text
            ImGui::SetCursorPosX((window_width - ImGui::CalcTextSize("Zenith").x) * 0.5f);
            
            ImGui::TextColored(ImColor(71, 20, 121, 255), "Zenith");
            ImGui::PopFont();
            TopLeftButtons({ (ICON_FA_CROSSHAIRS" Legitbot"), (ICON_FA_HAND_POINT_UP" Ragebot"), (ICON_FA_SHIELD" Anti Aim"), (ICON_FA_USERS" Visuals"), (ICON_FA_BOMB" Exploits"),
                (ICON_FA_COGS" Misc") , (ICON_FA_PEN_SQUARE" Lua"), (ICON_FA_TAG" Settings") }, { 0, 1, 2,3, 4, 5, 6, 7 }, indxex);
        }
        ImGui::EndChild();

        // draw next to it!
        ImGui::SameLine();

        ImGui::PushFont(menuFont);
        ImGui::BeginChild(("setting_area"), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Border);
        {
            switch (indxex)
            {
            case 0:
                ImGui::BeginChild("TopSpacer", ImVec2(210.f, 18.f), false);  ImGui::EndChild();
                ImGui::Columns(2, NULL, false);
                {
                    ImGui::BeginGroup();
                    {
                        bool isChecked = false; // Initialize the checkbox state
                        
                        float color[3];
                        // Create the checkbox with the label "Enable Hello"
                        ImGui::Checkbox("Enable Hello", &isChecked);
                        ImGui::Checkbox("Enable Hello2", &isChecked);
                        ImGui::Checkbox("Enable Hello23", &isChecked);
                        ImGui::SliderFloat("Slider", &value, 0.0f, 1.0f);
                        ImGui::SameLine();

                        // Draw the input field
                        ImGui::SetNextItemWidth(50);
                        ImGui::InputFloat("##Input", &value, 0.0f, 1.0f, "%.3f");
                        ImGui::Combo("Combo", &indxex, "Hello\0Hello2\0Hello23\0");
                        ImGui::ColorEdit3("Color", color);

                        
                    }
                    ImGui::EndGroup();
                }

                break;
            case 1:

                break;
            case 3:

                break;
            case 5:
 
                break;
            default:
                // Code to handle other cases
                break;
            }

        }
        ImGui::EndChild();
        ImGui::PopFont();

    }

    ImVec2 window_pos{ ImGui::GetWindowPos() };
    ImVec2 window_size{ ImGui::GetWindowSize() };
    ImVec2 cursor_pos{ ImGui::GetCursorPos() };
    // Border Animation
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    draw_list->AddRect({ window_pos.x - 1, window_pos.y - 1 }, { window_pos.x + window_size.x + 1, window_pos.y + window_size.y + 1 }, ImColor(71, 20, 121,255), 4.f, 0, 3.f);

    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    pD3DDevice->EndScene(); // End 3D Scene (Unlock Memory for other)

    pD3DDevice->Present(0, 0, 0, 0); // Display the created frame
}

// Memory clean up!
void cleanD3D(void)
{
    v_buffer->Release();
    pD3DDevice->Release(); // Close the 3D Device
    pD3D->Release(); // Close the Direct3D Interface
}

void init_graphics()
{
    // create a vertex buffer interface called v_buffer
    pD3DDevice->CreateVertexBuffer(
        3 * sizeof(CUSTOMVERTEX), // Buffer Size
        0, 
        CUSTOMFVF, // OUR FLAGS
        D3DPOOL_MANAGED,
        &v_buffer, // COPY BUFFER
        NULL);

    VOID* pVoid;
    v_buffer->Lock(0, 0, (void**)&pVoid, 0); // Locks v_buffer
    memcpy(pVoid, t_vert, sizeof(t_vert));
    v_buffer->Unlock(); // Unlocks v_buffer
}