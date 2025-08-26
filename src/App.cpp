#include "App.h"
#include <assert.h>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdexcept>

#include <GLFW/glfw3.h> // Will drag system OpenGL headers

static void SetDarkThemeColors()
{
	ImGuiStyle &style = ImGui::GetStyle();
	ImVec4 *colors = style.Colors;

	// Keep the same spacing and rounding
	style.WindowRounding = 6.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowPadding = ImVec2(12, 12);
	style.FramePadding = ImVec2(6, 4);
	style.FrameRounding = 4.0f;
	style.ItemSpacing = ImVec2(8, 6);
	style.ItemInnerSpacing = ImVec2(6, 4);
	style.IndentSpacing = 22.0f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 8.0f;
	style.GrabMinSize = 12.0f;
	style.GrabRounding = 3.0f;
	style.PopupRounding = 4.0f;

	// Base colors
	colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);			// Light grey text (not pure white)
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // Medium grey for disabled
	colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.95f);		// Dark m_window background
	colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.95f);		// Match m_window background
	colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.95f);		// Slightly darker than m_window
	colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 0.50f);		// Dark grey border
	colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);		// No shadow

	// Frame colors
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 0.95f);		  // Dark element backgrounds
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 0.95f); // Slightly lighter on hover
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);  // Even lighter when active

	// Title bar colors
	colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);			// Dark grey inactive title
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);	// Slightly lighter active title
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 0.75f); // Transparent when collapsed
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);		// Slightly darker than title

	// Scrollbar colors
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.95f);			// Scrollbar background
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);		// Scrollbar grab
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f); // Scrollbar grab when hovered
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);	// Scrollbar grab when active

	// Widget colors
	colors[ImGuiCol_CheckMark] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);		// Light grey checkmark
	colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);		// Slider grab
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f); // Slider grab when active
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);			// Dark grey buttons
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);	// Slightly lighter on hover
	colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);		// Even lighter when active

	// Header colors (TreeNode, Selectable, etc)
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 0.76f);		 // Pure dark grey
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 0.80f); // Slightly lighter on hover
	colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);	 // Even lighter when active

	// Separator
	colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);		// Separator color
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f); // Separator when hovered
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);	// Separator when active

	// Resize grip
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.25f, 0.25f, 0.50f);		 // Resize grip
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.30f, 0.30f, 0.30f, 0.75f); // Resize grip when hovered
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);	 // Resize grip when active

	// Text input cursor
	colors[ImGuiCol_InputTextCursor] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f); // Text input cursor

	// ALL TAB COLORS (both old and new names)
	// Using the newer tab color naming from your enum
	colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 0.86f);						 // Unselected tab
	colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.80f);				 // Tab when hovered
	colors[ImGuiCol_TabSelected] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);				 // Selected tab
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);		 // Selected tab overline
	colors[ImGuiCol_TabDimmed] = ImVec4(0.13f, 0.13f, 0.13f, 0.86f);				 // Dimmed/unfocused tab
	colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);		 // Selected but unfocused tab
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f); // Overline of unfocused selected tab

	// For backward compatibility with older ImGui versions
	// These might be what your version is using
	if (ImGuiCol_TabActive != ImGuiCol_TabSelected)
	{																			  // Only set if they're different (to avoid warnings)
		colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);		  // Active tab (old name)
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.13f, 0.86f);		  // Unfocused tab (old name)
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f); // Unfocused active tab (old name)
	}

	// Docking colors
	colors[ImGuiCol_DockingPreview] = ImVec4(0.30f, 0.30f, 0.30f, 0.40f); // Preview when docking
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f); // Empty docking space

	// Plot colors
	colors[ImGuiCol_PlotLines] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);			// Plot lines
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);		// Plot lines when hovered
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);		// Plot histogram
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f); // Plot histogram when hovered

	// Table colors
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);	 // Table header background
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // Table outer borders
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);	 // Table inner borders
	colors[ImGuiCol_TableRowBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.90f);		 // Table row background (even)
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.16f, 0.16f, 0.16f, 0.90f);	 // Table row background (odd)

	// Miscellaneous
	colors[ImGuiCol_TextLink] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);				 // Light grey for links (not blue)
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.30f, 0.30f, 0.30f, 0.35f);		 // Light grey selection background
	colors[ImGuiCol_TreeLines] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);			 // Tree node lines
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);		 // Drag and drop target
	colors[ImGuiCol_NavCursor] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);			 // Navigation cursor
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.40f, 0.40f, 0.40f, 0.70f); // Nav windowing highlight
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);	 // Nav windowing dim
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.75f);		 // Modal m_window dim
}

static void glfw_error_callback(int error, const char *description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

App *App::s_Instance = nullptr;

App::App()
{
	assert(s_Instance == nullptr && "App already exists!");
	s_Instance = this;
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	const char *glsl_version = "#version 400";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create window with graphics context
	float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
	m_window = glfwCreateWindow((int)(1920 * main_scale), (int)(1080 * main_scale), "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
	if (m_window == nullptr)
		throw std::runtime_error("Failed to create GLFW window");
	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	  // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	  // Enable Multi-Viewport / Platform Windows
	// io.ConfigViewportsNoAutoMerge = true;
	// io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	SetDarkThemeColors();
	// ImGui::StyleColorsLight();

	// Setup scaling
	ImGuiStyle &style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 3
	io.ConfigDpiScaleFonts = true;	   // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
	io.ConfigDpiScaleViewports = true; // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.
#endif

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);

	ImGui_ImplOpenGL3_Init(glsl_version);

	// get pid of this process and create randomized name
#ifdef _WIN32
	auto pid = _getpid();
#else
	auto pid = getpid();
#endif
	m_randomName = "user_" + std::to_string(pid);
	m_client = std::make_unique<WebRTCClient>(m_randomName);
	
	// FLOW STEP 1: Set up callback for when someone wants to connect to us
	// This gets called when we receive a "connection-request" message
	m_client->onConnectionRequest = [this](const std::string& fromClientId, const std::string& fromClientName) {
		// Store who's asking and show the popup to user
		m_requestingClientId = fromClientId;
		m_requestingClientName = fromClientName;
		m_showConnectionPopup = true; // This triggers the "Accept/Reject" popup
	};

	std::cout << "Connecting to signaling server..." << std::endl;
	if (!m_client->connectToSignalingServer("ws://localhost:8080/ws"))
	{
		std::cout << "Failed to connect. Make sure Go server is running!" << std::endl;
		throw std::runtime_error("Failed to connect to signaling server");
	}
}

App::~App()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void App::run()
{
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	// Main loop
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		// 1. Show the big demo m_window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (m_show_demo_window)
			ImGui::ShowDemoWindow(&m_show_demo_window);

		// 2. Show a simple m_window that we create ourselves. We use a Begin/End pair to create a named m_window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!"); // Create a m_window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");			 // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &m_show_demo_window); // Edit bools storing our m_window open/close state
			ImGui::Checkbox("Another Window", &m_show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);			   // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float *)&m_clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		// 3. Show another simple m_window.
		if (m_show_another_window)
		{
			ImGui::Begin("Another Window", &m_show_another_window); // Pass a pointer to our bool variable (the m_window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another m_window!");
			if (ImGui::Button("Close Me"))
				m_show_another_window = false;
			ImGui::End();
		}

		{
			ImGui::Begin("WebRTC Client");
			ImGui::Text("Your ID: %s", m_randomName.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Copy##CopyID"))
			{
				ImGui::SetClipboardText(m_randomName.c_str());
			}
			ImGui::Separator();
			
			// Active users section
			auto connected_peers = m_client->getConnectedPeerIds();
			ImGui::Text("Online Users (%zu) | Connected (%zu):", m_client->getConnectedClients().size(), connected_peers.size());
			
			if (!connected_peers.empty()) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "• %zu active connections", connected_peers.size());
			}
			
			ImGui::BeginChild("ActiveUsers", ImVec2(0, 120), true);
			for (const auto& clientId : m_client->getConnectedClients())
			{
				bool isConnected = m_client->isConnectedToPeer(clientId);
				
				// Show connection status with color coding
				if (isConnected)
				{
					ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "★ %s (Connected)", clientId.c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "• %s (Online)", clientId.c_str());
				}
				
				ImGui::SameLine();
				if (ImGui::SmallButton(("Copy##" + clientId).c_str()))
				{
					ImGui::SetClipboardText(clientId.c_str());
				}
				
				if (!isConnected)
				{
					ImGui::SameLine();
					if (ImGui::SmallButton(("Connect##" + clientId).c_str()))
					{
						// FLOW STEP 2: User clicks "Connect" - we send a connection request
						// This sends JSON: {"type":"connection-request","from":"us","to":"them"}
						m_client->sendConnectionRequest(clientId);
					}
				}
				else
				{
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Connected");
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
					if (ImGui::SmallButton(("Disconnect##" + clientId).c_str()))
					{
						m_client->disconnectFromPeer(clientId);
					}
					ImGui::PopStyleColor(3);
				}
			}
			if (m_client->getConnectedClients().empty())
			{
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No other users online");
			}
			ImGui::EndChild();
			
			ImGui::Separator();
			
			// Manual connection section
			ImGui::Text("Manual Connect:");
			static char targetClientId[256] = {};
			ImGui::InputText("Client ID", targetClientId, 256);
			ImGui::SameLine();
			if (ImGui::Button("Request Connection") && strlen(targetClientId) > 0)
			{
				m_client->sendConnectionRequest(targetClientId);
			}
			
			ImGui::Separator();
			
			// Message section
			ImGui::Text("Send Message:");
			static char buffer[1024] = {};
			ImGui::InputText("Message", buffer, 1024);
			
			// Send options
			if (ImGui::Button("Broadcast to All") && strlen(buffer) > 0)
			{
				m_client->sendMessage(buffer); // Empty peer_id = broadcast
				memset(buffer, 0, sizeof(buffer));
			}
			
			// Send to specific peers
			auto connected_peers_list = m_client->getConnectedPeerIds();
			if (!connected_peers_list.empty()) {
				ImGui::SameLine();
				ImGui::Text("or send to:");
				for (const auto& peer_id : connected_peers_list) {
					ImGui::SameLine();
					if (ImGui::SmallButton((peer_id + "##send").c_str()) && strlen(buffer) > 0) {
						m_client->sendMessage(buffer, peer_id);
						memset(buffer, 0, sizeof(buffer));
					}
				}
			}

			ImGui::Separator();
			ImGui::Text("Message History:");
			ImGui::BeginChild("MessageHistory", ImVec2(0, 180), true);
			for (const auto &msg : m_client->getMessageHistory())
			{
				ImGui::TextWrapped("%s", msg.c_str());
			}
			ImGui::EndChild();
			ImGui::End();
		}
		
		// Connection request popup
		if (m_showConnectionPopup)
		{
			ImGui::OpenPopup("Connection Request");
		}
		
		if (ImGui::BeginPopupModal("Connection Request", &m_showConnectionPopup, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("User '%s' wants to connect with you.", m_requestingClientName.c_str());
			ImGui::Text("Do you want to accept this connection?");
			ImGui::Separator();
			
			if (ImGui::Button("Accept"))
			{
				// FLOW STEP 3: User accepts connection request
				// Sends JSON: {"type":"connection-response","data":{"accepted":true}}
				m_client->sendConnectionResponse(m_requestingClientId, true);
				m_showConnectionPopup = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Reject"))
			{
				// FLOW STEP 3 (Alternative): User rejects connection request  
				// Sends JSON: {"type":"connection-response","data":{"accepted":false}}
				m_client->sendConnectionResponse(m_requestingClientId, false);
				m_showConnectionPopup = false;
				ImGui::CloseCurrentPopup();
			}
			
			ImGui::EndPopup();
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(m_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(m_clear_color.x * m_clear_color.w, m_clear_color.y * m_clear_color.w, m_clear_color.z * m_clear_color.w, m_clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(m_window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow *backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(m_window);
	}
}