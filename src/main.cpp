#include "raylib.h"
#include <iostream>

#pragma region imgui
#include "imgui.h"
#include "rlImGui.h"
#include "imguiThemes.h"
#include "ImGuiStyle.h"
#pragma endregion

int main(void)
{
	// Initilization...

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 450, "window name");
	SetTargetFPS(60);
	bool myOption = false;
	int selectionOption = 0;
	std::string resultMessage = "";
	bool confirmed = false;
	int integer = 0;
	float position[3] = { 0.0f, 0.0f, 0.0f };
	ImVec2 myVec = ImVec2(20.0f, 100.0f);
	float position2 = 0.0f;
	float radius = 0.0f;
	float color[3] = { 1.0f, 0.0f, 0.0f };
	float colorPicker[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	const char* youDone = "";

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
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Second button hovered");
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
		ImGui::Checkbox("Check", &myOption);
		ImGui::Separator();
		ImGui::Text("Choose between 2 options");
		ImGui::RadioButton("Suicide", &selectionOption, 0);
		ImGui::RadioButton("Kill Hank", &selectionOption, 1);
		ImGui::BeginDisabled(confirmed);
		if (ImGui::Button("Confirm") && !confirmed)
		{
			confirmed = true;
			resultMessage = selectionOption == 0
				? "You sacrificed yourself for the future!"
				: "You killed Hank and destroyed all the human trust!";
		}
		ImGui::EndDisabled();
		if (!resultMessage.empty())
			ImGui::Text(resultMessage.c_str());
#pragma region imgui windows
		ImGui::Begin("Second Window"); // Second ImGui window
#pragma endregion
		ImGui::Text("Hello");
		ImGui::Separator();
		ImGui::NewLine();
		static float a = 0;
		ImGui::SliderFloat("Slider", &a, 0, 1);
		ImGui::Separator();
		ImGui::InputInt("InputInt", &integer, 1, 5); // Create Input int
		if (integer > 100) integer = 100; // Value can't be over 100
		else if (integer < 0) integer = 0; // Value can't be less than 0
		ImGui::Separator();
		ImGui::DragFloat3("Drag Float", position, 0.1f); // 3 Float values for x,y,z
		ImGui::Separator();
		ImGui::Text("Position");
		ImGui::VSliderFloat("##vslider", myVec, &position2, 0.0f, 100.0f, "");
		ImGui::SameLine();
		ImGui::Text("%.1f", position2); // shows current value next to it
		ImGui::Separator();
		ImGui::SliderAngle("SliderAngle", &radius);
		ImGui::Separator();
		ImGui::ColorEdit3("Color Edit 3", color);
		ImGui::Separator();
		ImGui::ProgressBar(0.65f, { 300,50 });
		ImGui::SameLine();
		ImGui::Text("Percentage");
		ImGui::NewLine();
		if (ImGui::CollapsingHeader("Collapsing header"))
		{
			if (ImGui::Button("Don't press me daddy"))
			{
				youDone = "You're done lil bro I'm calling a cops";
			}
		ImGui::Text(youDone);
		}
		ImGui::Separator();
		if (ImGui::TreeNode("Tree Node"))
		{
			ImGui::Text("I'm inside the tree!");
			ImGui::Button("A button inside tree");

			ImGui::TreePop(); // this closes the TreeNode, always needed!
		}
		ImGui::Separator();
		ImGui::BeginChild("ChildID", { 500,100 }, true);
		ImGui::Text("This is inside child");
		ImGui::Button("Button inside child");
		ImGui::EndChild();
		ImGui::SameLine(); // Same line
		ImGui::BeginChild("Right panel", { 500,100 }, true);
		ImGui::Text("Right Panel");
		ImGui::EndChild();
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
};