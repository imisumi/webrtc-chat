#pragma once

#include <imgui.h>


#include <memory>
#include <string>

#include "Client.h"

struct GLFWwindow;
class App
{
public:
	App();
	~App();

	void run();

	const std::string& getRandomName() const { return m_randomName; }

private:
	static App *s_Instance;

	GLFWwindow *m_window = nullptr;

	bool m_show_demo_window = true;
	bool m_show_another_window = false;
	ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	std::string m_randomName;
	std::unique_ptr<WebRTCClient> m_client;
	
	// Connection request popup state
	bool m_showConnectionPopup = false;
	std::string m_requestingClientName;
	std::string m_requestingClientId;
};