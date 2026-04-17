#include "raylib.h"
#include <iostream>
#include <gameLayer/gameMain.h>
#include <platform/asserts.h>

#pragma region imgui
#include "imgui.h"
#include "rlImGui.h"
#include "imguiThemes.h"
#include "ImGuiStyle.h"
#pragma endregion

int main(void)
{
	// Initilization...

#if PRODUCTION_BUILD == 1
	SetTraceLogLevel(LOG_NONE); // no log output to the console by raylib
#endif

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 450, "window name");
	SetExitKey(KEY_NULL); // Disable Esc from closing window
	SetTargetFPS(144);

#pragma region imgui
	rlImGuiSetup(true);
	SetupImGuiStyle();
	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig config;
	config.OversampleH = 3;
	config.OversampleV = 3;
	config.PixelSnapH = true;

	ImFont* roboto = io.Fonts->AddFontFromFileTTF(RESOURCES_PATH "Roboto-Regular.ttf", 24.0f, &config);
	io.FontDefault = roboto;

	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable gamepad controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
	rlImGuiReloadFonts();
#pragma endregion

	if (!initGame())
	{
		return 0;
	}

	while (!WindowShouldClose())
	{
		// Update...
		
		BeginDrawing();
		ClearBackground(BLACK);


#pragma region imgui
		rlImGuiBegin();

		// docking stuff...
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::PopStyleColor(2);
#pragma endregion

		if (!updateGame())
		{
			CloseWindow();
		}

#pragma region imgui
		rlImGuiEnd();
#pragma endregion

		EndDrawing();
	}

	closeGame();

#pragma region imgui
	rlImGuiShutdown();
#pragma endregion

	return 0;
};