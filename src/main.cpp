#include "raylib.h"
#include <iostream>

#pragma region imgui
#include "imgui.h"
#include "rlImGui.h"
#include "imguiThemes.h"
#pragma endregion

int main(void)
{
	// Initilization...

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 450, "window name");
	SetTargetFPS(60);

#pragma region imgui
	rlImGuiSetup(true);

	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2; // Font

	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable gamepad controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
	io.FontGlobalScale = 2.5;
#pragma endregion

	while (!WindowShouldClose())
	{
		// Update...
		
		BeginDrawing();
		ClearBackground(RAYWHITE);

#pragma region imgui
		rlImGuiBegin();

		// docking stuff...
		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::PopStyleColor(2);
#pragma endregion

		DrawText("Congrats I have just created my 4th window", 190, 200, 20, RED);

#pragma region imgui windows
		ImGui::Begin("test"); // First ImGui window
#pragma endregion
		ImGui::Text("Hello");
		if (ImGui::Button("Button"))
		{
			std::cout << "Button\n";
		}
		ImGui::SameLine();
		if (ImGui::Button("Button2"))
		{
			std::cout << "Second Button\n";
		}
#pragma region imgui windows
		ImGui::Begin("Second Window"); // Second ImGui window
#pragma endregion
		ImGui::Text("Hello");
		ImGui::Separator();
		ImGui::NewLine();
		static float a = 0;
		ImGui::SliderFloat("Slider", &a, 0, 1);

#pragma region imgui
		ImGui::End();
		ImGui::End();
		rlImGuiEnd();
#pragma endregion

		EndDrawing();
	}

#pragma region imgui
	rlImGuiShutdown();
#pragma endregion

	CloseWindow();
	return 0;
}