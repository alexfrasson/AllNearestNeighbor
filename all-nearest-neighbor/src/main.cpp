// ImGui - standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "imgui\imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>

#include "PointValidation.h"
#include "Parser.h"
#include "Timer.h"
#include "Point.h"
#include "KdTree.h"

#include <random>
#include <iostream>
#include <vector>
#include <thread>
#include <cmath>

KdTree g_kdtree;
std::vector<Point> g_points;
ImVec4 g_canvas_color = ImVec4(0.45f * 0.5f, 0.55f* 0.5f, 0.60f* 0.5f, 1.00f);
ImVec4 g_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

ImVec2 g_translation;
ImVec2 g_canvas_pos;
ImVec2 g_canvas_offset;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}


void drawKdTree(ImDrawList* draw_list, KdTreeNode* node, AABB aabb)
{
	if (node->isLeaf())
		return;

	if (node->axis == 0)
		draw_list->AddLine(ImVec2(node->value + g_translation.x, aabb.min.m_y + g_translation.y), ImVec2(node->value + g_translation.x, aabb.max.m_y + g_translation.y), ImColor(0.1, 0.7, 0.4, 2.0f));
	else
		draw_list->AddLine(ImVec2(aabb.min.m_x + g_translation.x, node->value + g_translation.y), ImVec2(aabb.max.m_x + g_translation.x, node->value + g_translation.y), ImColor(0.1, 0.7, 0.4, 2.0f));
	
	AABB aabbLeft = aabb;
	AABB aabbRight = aabb;

	if (node->axis == 0)
		aabbLeft.max.m_x = aabbRight.min.m_x = node->value;
	else
		aabbLeft.max.m_y = aabbRight.min.m_y = node->value;

	drawKdTree(draw_list, node->left, aabbLeft);
	drawKdTree(draw_list, node->right, aabbRight);
}


void ShowExampleAppCustomRendering(bool* p_open)
{
	ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Example: Custom rendering", p_open))
	{
		ImGui::End();
		return;
	}

	// Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of overloaded operators, etc.
	// Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your types and ImVec2/ImVec4.
	// ImGui defines overloaded operators but they are internal to imgui.cpp and not exposed outside (to avoid messing with your types)
	// In this example we are not using the maths operators!
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// Primitives
	/*ImGui::Text("Primitives");
	static float sz = 36.0f;
	static ImVec4 col = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
	ImGui::DragFloat("Size", &sz, 0.2f, 2.0f, 72.0f, "%.0f");
	ImGui::ColorEdit3("Color", &col.x);
	{
		const ImVec2 p = ImGui::GetCursorScreenPos();
		const ImU32 col32 = ImColor(col);
		float x = p.x + 4.0f, y = p.y + 4.0f, spacing = 8.0f;
		for (int n = 0; n < 2; n++)
		{
			float thickness = (n == 0) ? 1.0f : 4.0f;
			draw_list->AddCircle(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col32, 20, thickness); x += sz + spacing;
			draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 0.0f, ImDrawCornerFlags_All, thickness); x += sz + spacing;
			draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 10.0f, ImDrawCornerFlags_All, thickness); x += sz + spacing;
			draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 10.0f, ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight, thickness); x += sz + spacing;
			draw_list->AddTriangle(ImVec2(x + sz*0.5f, y), ImVec2(x + sz, y + sz - 0.5f), ImVec2(x, y + sz - 0.5f), col32, thickness); x += sz + spacing;
			draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y), col32, thickness); x += sz + spacing;
			draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, thickness); x += sz + spacing;
			draw_list->AddLine(ImVec2(x, y), ImVec2(x, y + sz), col32, thickness); x += spacing;
			draw_list->AddBezierCurve(ImVec2(x, y), ImVec2(x + sz*1.3f, y + sz*0.3f), ImVec2(x + sz - sz*1.3f, y + sz - sz*0.3f), ImVec2(x + sz, y + sz), col32, thickness);
			x = p.x + 4;
			y += sz + spacing;
		}
		draw_list->AddCircleFilled(ImVec2(x + sz*0.5f, y + sz*0.5f), sz*0.5f, col32, 32); x += sz + spacing;
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col32); x += sz + spacing;
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 10.0f); x += sz + spacing;
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col32, 10.0f, ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight); x += sz + spacing;
		draw_list->AddTriangleFilled(ImVec2(x + sz*0.5f, y), ImVec2(x + sz, y + sz - 0.5f), ImVec2(x, y + sz - 0.5f), col32); x += sz + spacing;
		draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + sz, y + sz), IM_COL32(0, 0, 0, 255), IM_COL32(255, 0, 0, 255), IM_COL32(255, 255, 0, 255), IM_COL32(0, 255, 0, 255));
		ImGui::Dummy(ImVec2((sz + spacing) * 8, (sz + spacing) * 3));
	}*/
	ImGui::Separator();
	{
		
		/*static ImVector<ImVec2> points;
		static bool adding_line = false;
		ImGui::Text("Canvas example");
		if (ImGui::Button("Clear")) points.clear();
		if (points.Size >= 2) { ImGui::SameLine(); if (ImGui::Button("Undo")) { points.pop_back(); points.pop_back(); } }
		ImGui::Text("Left-click and drag to add lines,\nRight-click to undo");*/

		//// Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
		//// However you can draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
		//// If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
		g_canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
		ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
		if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
		if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
		draw_list->AddRectFilled(g_canvas_pos, ImVec2(g_canvas_pos.x + canvas_size.x, g_canvas_pos.y + canvas_size.y), ImColor(g_canvas_color.x, g_canvas_color.y, g_canvas_color.z, g_canvas_color.w));
		draw_list->AddRect(g_canvas_pos, ImVec2(g_canvas_pos.x + canvas_size.x, g_canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255));

		//bool adding_preview = false;
		ImGui::InvisibleButton("canvas", canvas_size);
		//ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
		//if (adding_line)
		//{
		//	adding_preview = true;
		//	points.push_back(mouse_pos_in_canvas);
		//	if (!ImGui::IsMouseDown(0))
		//		adding_line = adding_preview = false;
		//}
		//if (ImGui::IsItemHovered())
		//{
		//	if (!adding_line && ImGui::IsMouseClicked(0))
		//	{
		//		points.push_back(mouse_pos_in_canvas);
		//		adding_line = true;
		//	}
		//	if (ImGui::IsMouseClicked(1) && !points.empty())
		//	{
		//		adding_line = adding_preview = false;
		//		points.pop_back();
		//		points.pop_back();
		//	}
		//}
		//draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);      // clip lines within the canvas (if we resize it, etc.)
		//for (int i = 0; i < points.Size - 1; i += 2)
		//	draw_list->AddLine(ImVec2(canvas_pos.x + points[i].x, canvas_pos.y + points[i].y), ImVec2(canvas_pos.x + points[i + 1].x, canvas_pos.y + points[i + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
		//draw_list->PopClipRect();
		//if (adding_preview)
		//	points.pop_back();

		//g_canvas_offset = ImVec2(0, 0);
		if (ImGui::IsItemHovered() && ImGui::GetIO().MouseDown[0])
			g_canvas_offset = ImVec2(g_canvas_offset.x + ImGui::GetIO().MouseDelta.x, g_canvas_offset.y + ImGui::GetIO().MouseDelta.y);

		g_translation = ImVec2(g_canvas_pos.x + g_canvas_offset.x, g_canvas_pos.y + g_canvas_offset.y);


		draw_list->AddLine(ImVec2(-99999 + g_translation.x, g_translation.y), ImVec2(99999 + g_translation.x, g_translation.y), ImColor(0.3f, 0.3f, 0.3f, 1.0f), 2);
		draw_list->AddLine(ImVec2(g_translation.x, -99999 + g_translation.y), ImVec2(g_translation.x, 99999 + g_translation.y), ImColor(0.3f, 0.3f, 0.3f, 1.0f), 2);

		if (g_kdtree.m_Root != nullptr)
		{			
			draw_list->AddLine(ImVec2(g_kdtree.m_AABB.min.m_x + g_translation.x, g_kdtree.m_AABB.min.m_y + g_translation.y), ImVec2(g_kdtree.m_AABB.min.m_x + g_translation.x, g_kdtree.m_AABB.max.m_y + g_translation.y), ImColor(0.1, 0.7, 0.4, 2.0f));
			draw_list->AddLine(ImVec2(g_kdtree.m_AABB.min.m_x + g_translation.x, g_kdtree.m_AABB.min.m_y + g_translation.y), ImVec2(g_kdtree.m_AABB.max.m_x + g_translation.x, g_kdtree.m_AABB.min.m_y + g_translation.y), ImColor(0.1, 0.7, 0.4, 2.0f));
			draw_list->AddLine(ImVec2(g_kdtree.m_AABB.max.m_x + g_translation.x, g_kdtree.m_AABB.max.m_y + g_translation.y), ImVec2(g_kdtree.m_AABB.min.m_x + g_translation.x, g_kdtree.m_AABB.max.m_y + g_translation.y), ImColor(0.1, 0.7, 0.4, 2.0f));
			draw_list->AddLine(ImVec2(g_kdtree.m_AABB.max.m_x + g_translation.x, g_kdtree.m_AABB.max.m_y + g_translation.y), ImVec2(g_kdtree.m_AABB.max.m_x + g_translation.x, g_kdtree.m_AABB.min.m_y + g_translation.y), ImColor(0.1, 0.7, 0.4, 2.0f));

			drawKdTree(draw_list, g_kdtree.m_Root, g_kdtree.m_AABB);
		}

		for (size_t i = 0; i < g_points.size(); i++)
		{
			ImVec2 pos = ImVec2(g_points[i].m_x + g_translation.x, g_points[i].m_y + g_translation.y);
			draw_list->AddCircleFilled(pos, 2.0f, IM_COL32(255, 255, 255, 255), 5);
		}
	}
	ImGui::End();
}

void genRandomPoints(std::vector<Point>& points, int n)
{
	points.reserve(points.size() + n);
	
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(5, 995);

	for (size_t i = 0; i < n; i++)
		points.emplace_back(dis(gen), dis(gen));
}

void findClosestKdTree(KdTree& kdtree, std::vector<Point>& points, std::vector<std::pair<Point, Point>>& pointPairs, size_t begin, size_t end)
{
	for (int i = begin; i < end; i++)
	{
		Point p = kdtree.nearestNeighbor(points[i]);
		pointPairs[i] = std::pair<Point, Point>(points[i], p);
	}
}

int main(int, char**)
{
	//genRandomPoints(g_points, 100);

	//g_kdtree.build(3, g_points);



	/*std::string pointSetFilename = "./point_set_small.txt";
	std::string solutionFilename = "./solution_small.txt";/**/

	/**/std::string pointSetFilename = "./point_set_big.txt";
	std::string solutionFilename = "./solution_big.txt";/**/

	std::vector<Point> points;
	{
		//measures the time in milliseconds
		Timer timer("Point Loading");

		std::cout << "Reading points..." << std::endl;

		// reads the point data
		points = Parser::readPoints(pointSetFilename);
	}

	//empty vector to put your solution in
	std::vector<std::pair<Point, Point>> pointPairs(points.size(), std::pair<Point, Point>(Point(), Point()));

	//TODO: Find the closest neighbor for each point in "points" and save it in "pointPairs"
	//e.g. pointPairs.push_back(std::pair<Point, Point>(point0, closest neighbor of point0));
	{
		KdTree kdtree;

		{
			std::cout << "Building KdTree..." << std::endl;
			Timer timer("Building KdTree");

			kdtree.build(500, points);
		}

		std::cout << "Searching..." << std::endl;
		Timer timer("Searching");

		unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
		unsigned step = std::floor(points.size() / (float)concurentThreadsSupported);

		std::vector<std::thread> threads;

		//for (size_t i = 0; i < concurentThreadsSupported; i++)
		//	threads.emplace_back(findClosestNaive, std::ref(points), std::ref(pointPairs), i * step, i == concurentThreadsSupported - 1 ? points.size() : (i + 1) * step);

		for (size_t i = 0; i < concurentThreadsSupported; i++)
			threads.emplace_back(findClosestKdTree, std::ref(kdtree), std::ref(points), std::ref(pointPairs), i * step, i == concurentThreadsSupported - 1 ? points.size() : (i + 1) * step);


		for (size_t i = 0; i < concurentThreadsSupported; i++)
			threads[i].join();
	}


	//validates your solution
	std::string result = PointValidation::validatePoints(solutionFilename, pointPairs) ? "Correct" : "Wrong";
	std::cout << std::endl;
	std::cout << result << std::endl;




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
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    gl3wInit();

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL3_Init(window, true);
    //io.NavFlags |= ImGuiNavFlags_EnableKeyboard;  // Enable Keyboard Controls
    //io.NavFlags |= ImGuiNavFlags_EnableGamepad;   // Enable Gamepad Controls

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    bool show_demo_window = true;
	bool show_another_window = false;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

		static bool showCanvas = true;
		if (showCanvas)    ShowExampleAppCustomRendering(&showCanvas);

        // 1. Show a simple window.
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
        //{
        //    static float f = 0.0f;
        //    static int counter = 0;
        //    ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
        //    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
        //    ImGui::ColorEdit3("clear color", (float*)&g_clear_color); // Edit 3 floats representing a color

        //    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
        //    ImGui::Checkbox("Another Window", &show_another_window);

        //    if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
        //        counter++;
        //    ImGui::SameLine();
        //    ImGui::Text("counter = %d", counter);

        //    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //}

        // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
        /*if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }*/

        // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
        //if (show_demo_window)
        //{
        //    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
        //    ImGui::ShowDemoWindow(&show_demo_window);
        //}

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(g_clear_color.x, g_clear_color.y, g_clear_color.z, g_clear_color.w);
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
