// ImGui - standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "imgui\imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>

#include "Point.h"
#include "KdTree.h"

#include <random>
#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <string>

KdTree g_kdtree;
std::vector<Point> g_points;

ImVec4 g_canvas_color = ImVec4(0.225f, 0.275f, 0.3f, 1.00f);

ImVec2 g_translation;
ImVec2 g_canvas_pos;
ImVec2 g_canvas_offset;

ImVec2 g_windowSize;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

void drawKdTree(ImDrawList* draw_list, KdTreeNode* node, AABB aabb)
{
	if (node->isLeaf())
		return;

	if (node->axis == 0)
		draw_list->AddLine(ImVec2(node->value + g_translation.x, aabb.min.m_y + g_translation.y), ImVec2(node->value + g_translation.x, aabb.max.m_y + g_translation.y), ImColor(0.1f, 0.7f, 0.4f, 2.0f));
	else
		draw_list->AddLine(ImVec2(aabb.min.m_x + g_translation.x, node->value + g_translation.y), ImVec2(aabb.max.m_x + g_translation.x, node->value + g_translation.y), ImColor(0.1f, 0.7f, 0.4f, 2.0f));
	
	AABB aabbLeft = aabb;
	AABB aabbRight = aabb;

	if (node->axis == 0)
		aabbLeft.max.m_x = aabbRight.min.m_x = node->value;
	else
		aabbLeft.max.m_y = aabbRight.min.m_y = node->value;

	drawKdTree(draw_list, node->left, aabbLeft);
	drawKdTree(draw_list, node->right, aabbRight);
}

static void ShowExampleAppFixedOverlay(bool* p_open)
{
	const float DISTANCE = 10.0f;
	static int corner = 0;
	ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
	ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
	if (ImGui::Begin("Example: Fixed Overlay", p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::Text("Drag with mouse wheel to move the canvas.");
		ImGui::Text("Left click to query nearest point.");
		ImGui::Separator();
		ImGui::Text("Mouse position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x - g_translation.x, ImGui::GetIO().MousePos.y - g_translation.y);
		ImGui::End();
	}
}

void ShowExampleAppCustomRendering(bool* p_open)
{
	ImGui::SetNextWindowSize(g_windowSize, ImGuiCond_::ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_::ImGuiCond_Always);
	if (!ImGui::Begin("Example: Custom rendering", p_open, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	g_canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
	ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
	if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
	if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
	draw_list->AddRectFilled(g_canvas_pos, ImVec2(g_canvas_pos.x + canvas_size.x, g_canvas_pos.y + canvas_size.y), ImColor(g_canvas_color.x, g_canvas_color.y, g_canvas_color.z, g_canvas_color.w));

	ImGui::InvisibleButton("canvas", canvas_size);
		
	static Point query;
	static Point nearest;
	if (ImGui::IsItemHovered())
	{
		if(ImGui::GetIO().MouseDown[2])
			g_canvas_offset = ImVec2(g_canvas_offset.x + ImGui::GetIO().MouseDelta.x, g_canvas_offset.y + ImGui::GetIO().MouseDelta.y);
		else if (ImGui::GetIO().MouseDown[0])
		{
			query = Point((int32_t)ImGui::GetIO().MousePos.x - (int32_t)g_translation.x, (int32_t)ImGui::GetIO().MousePos.y - (int32_t)g_translation.y);
			nearest = g_kdtree.nearestNeighbor(query);
		}
	}
	g_translation = ImVec2(g_canvas_pos.x + g_canvas_offset.x, g_canvas_pos.y + g_canvas_offset.y);



	draw_list->PushClipRect(g_canvas_pos, ImVec2(g_canvas_pos.x + canvas_size.x, g_canvas_pos.y + canvas_size.y), true);      // clip lines within the canvas (if we resize it, etc.)

	draw_list->AddLine(ImVec2(-99999 + g_translation.x, g_translation.y), ImVec2(99999 + g_translation.x, g_translation.y), ImColor(0.4f, 0.4f, 0.4f, 1.0f), 1.5f);
	draw_list->AddLine(ImVec2(g_translation.x, -99999 + g_translation.y), ImVec2(g_translation.x, 99999 + g_translation.y), ImColor(0.4f, 0.4f, 0.4f, 1.0f), 1.5f);

	if (g_kdtree.m_Root != nullptr)
	{			
		draw_list->AddLine(ImVec2(g_kdtree.m_AABB.min.m_x + g_translation.x, g_kdtree.m_AABB.min.m_y + g_translation.y), ImVec2(g_kdtree.m_AABB.min.m_x + g_translation.x, g_kdtree.m_AABB.max.m_y + g_translation.y), ImColor(0.1f, 0.7f, 0.4f, 2.0f));
		draw_list->AddLine(ImVec2(g_kdtree.m_AABB.min.m_x + g_translation.x, g_kdtree.m_AABB.min.m_y + g_translation.y), ImVec2(g_kdtree.m_AABB.max.m_x + g_translation.x, g_kdtree.m_AABB.min.m_y + g_translation.y), ImColor(0.1f, 0.7f, 0.4f, 2.0f));
		draw_list->AddLine(ImVec2(g_kdtree.m_AABB.max.m_x + g_translation.x, g_kdtree.m_AABB.max.m_y + g_translation.y), ImVec2(g_kdtree.m_AABB.min.m_x + g_translation.x, g_kdtree.m_AABB.max.m_y + g_translation.y), ImColor(0.1f, 0.7f, 0.4f, 2.0f));
		draw_list->AddLine(ImVec2(g_kdtree.m_AABB.max.m_x + g_translation.x, g_kdtree.m_AABB.max.m_y + g_translation.y), ImVec2(g_kdtree.m_AABB.max.m_x + g_translation.x, g_kdtree.m_AABB.min.m_y + g_translation.y), ImColor(0.1f, 0.7f, 0.4f, 2.0f));

		drawKdTree(draw_list, g_kdtree.m_Root, g_kdtree.m_AABB);
	}

	for (size_t i = 0; i < g_points.size(); i++)
	{
		ImVec2 pos = ImVec2(g_points[i].m_x + g_translation.x, g_points[i].m_y + g_translation.y);
		draw_list->AddCircleFilled(pos, 2.0f, IM_COL32(255, 255, 255, 255), 5);
	}

	draw_list->AddCircleFilled(ImVec2(nearest.m_x + g_translation.x, nearest.m_y + g_translation.y), 3.5f, IM_COL32(255, 0, 0, 255), 10);
	draw_list->AddCircleFilled(ImVec2(query.m_x + g_translation.x, query.m_y + g_translation.y), 3.5f, IM_COL32(0, 255, 0, 255), 10);
	
	draw_list->PopClipRect();

	ImGui::End();
}

void genRandomPoints(std::vector<Point>& points, int n)
{
	points.reserve(points.size() + n);
	
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(50, 750);

	for (size_t i = 0; i < n; i++)
		points.emplace_back(dis(gen), dis(gen));
}

void window_size_callback(GLFWwindow*, int width, int height)
{
	if (width == 0 && height == 0)
		return;

	g_windowSize = ImVec2((float)width, (float)height);
}

int main(int, char**)
{
	genRandomPoints(g_points, 1000);

	g_kdtree.build(10, g_points);

    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui OpenGL3 example", NULL, NULL);

	glfwSetWindowSizeCallback(window, window_size_callback);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	window_size_callback(window, width, height);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    gl3wInit();

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL3_Init(window, true);

    // Setup style
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 0.0f;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

		static bool showCanvas = true;
		static bool showOverlay = true;
		ShowExampleAppCustomRendering(&showCanvas);
		ShowExampleAppFixedOverlay(&showOverlay);

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(g_canvas_color.x, g_canvas_color.y, g_canvas_color.z, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}
