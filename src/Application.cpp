#include "Application.h"
#include <chrono>
#include "imgui/imgui-SFML.h"

Application::Application() {
	//srand(time(nullptr));

	window = std::make_shared<sf::RenderWindow>(sf::VideoMode(WINDOW_X, WINDOW_Y), "SFML");
	window->setMouseCursorVisible(false);
	window->setKeyRepeatEnabled(false);
	window->setVerticalSyncEnabled(false);

	ImGui::SFML::Init(*window);
	window->resetGLStates();

}

Application::~Application() {

	light_handle->~LightHandle();
	light_controller->~LightController();
}

bool Application::init_clcaster() {

	//Map _map(32);
	//return 0;

	// Start up the raycaster
	raycaster = std::make_shared<CLCaster>();
	if (!raycaster->init())
		abort();

	// Create and generate the old 3d array style map
	map = std::make_shared<Old_Map>(sf::Vector3i(MAP_X, MAP_Y, MAP_Z));
	map->generate_terrain();

	// Send the data to the GPU
	raycaster->assign_map(map);

	octree = std::make_shared<Map>(32);
	raycaster->assign_octree(octree);


	// Create a new camera with (starting position, direction)
	camera = std::make_shared<Camera>(
		sf::Vector3f(50, 50, 50),
		sf::Vector2f(1.5f, 0.0f),
		window.get()
	);

	// *link* the camera to the GPU
	raycaster->assign_camera(camera);

	// Generate and send the viewport to the GPU. Also creates the viewport texture
	raycaster->create_viewport(WINDOW_X, WINDOW_Y, 0.625f * 90.0f, 90.0f);

	// Initialize the light controller and link it to the GPU
	light_controller = std::make_shared<LightController>(raycaster);

	// Create a light prototype, send it to the controller, and get the handle back
	LightPrototype prototype(
		sf::Vector3f(100.0f, 100.0f, 75.0f),
		sf::Vector3f(-1.0f, -1.0f, -1.5f),
		sf::Vector4f(0.4f, 0.4f, 0.4f, 1.0f)
	);
	light_handle = light_controller->create_light(prototype);

	// Load in the spritesheet texture

	if (!spritesheet.loadFromFile("../assets/textures/minecraft_tiles.png"))
		Logger::log("Failed to load spritesheet from file", Logger::LogLevel::WARN);
	raycaster->create_texture_atlas(&spritesheet, sf::Vector2i(16, 16));

	// Checks to see if proper data was uploaded, then sets the kernel args
	// ALL DATA LOADING MUST BE FINISHED
	if (!raycaster->validate()) {
		abort();
	};

	return true;
}

bool Application::init_events() {

	// Link the camera to the input handler
	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::KeyHeld);
	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::KeyPressed);
	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::MouseMoved);
	camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::MouseButtonPressed);

	// Start up a window handler which subscribes to input and listens for window closed events
	window_handler = std::make_shared<WindowHandler>(WindowHandler(window.get()));
	window_handler->subscribe_to_publisher(&input_handler, vr::Event::EventType::Closed);
	window_handler->subscribe_to_publisher(&input_handler, vr::Event::EventType::KeyPressed);

	//camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::JoystickMoved);
	
	return true;
}

bool Application::game_loop() {

	while (window->isOpen()) {

		// Have the input handler empty the event stack, generate events for held keys, and then dispatch the events to listeners
		input_handler.consume_sf_events(window.get());
		input_handler.handle_held_keys();
		input_handler.dispatch_events();

		// Time keeping
		elapsed_time = elap_time();
		delta_time = elapsed_time - current_time;
		current_time = elapsed_time;
		if (delta_time > 0.2f)
			delta_time = 0.2f;
		accumulator_time += delta_time;
		while ((accumulator_time - step_size) >= step_size) {
			accumulator_time -= step_size;

			// ==== DELTA TIME LOCKED ====
		}

		// ==== FPS LOCKED ====

		window->clear(sf::Color::Black);

		ImGui::SFML::Update(*window, sf_delta_clock.restart());

		// Pausing stops camera and light updates, as well as raycaster computes
		if (!paused) {
			camera->update(delta_time);
			light_handle->update(delta_time);

			// Run the raycast
			if (!raycaster->compute()) {
				abort();
			};
		}

		// Let the raycaster draw it screen buffer
		raycaster->draw(window.get());

		// Give the frame counter the frame time and draw the average frame time
		fps.frame(delta_time);
		fps.draw();

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
		bool window_show = true;
		ImGui::Begin("Camera", &window_show, window_flags);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Columns(2);

		ImGui::Text("Camera Inclination");
		ImGui::Text("Camera Azimuth");
		ImGui::Text("Camera Pos_X");
		ImGui::Text("Camera Poz_Y");
		ImGui::Text("Camera Poz_Z");

		ImGui::NextColumn();

		sf::Vector2f dir = camera->get_direction();
		sf::Vector3f pos = camera->get_position();

		ImGui::Text("%f", dir.x);
		ImGui::Text("%f", dir.y);
		ImGui::Text("%f", pos.x);
		ImGui::Text("%f", pos.y);
		ImGui::Text("%f", pos.z);

		ImGui::NextColumn();

		ImGui::InputText("filename", screenshot_buf, 128);
		if (ImGui::Button("Take Screen shot")) {

			std::string path = "../assets/";
			std::string filename(screenshot_buf);
			filename += ".png";

			sf::Texture window_texture;
			window_texture.create(window->getSize().x, window->getSize().y);
			window_texture.update(*window);

			sf::Image image = window_texture.copyToImage();
			image.saveToFile(path + filename);

		}

		ImGui::NextColumn();

		if (ImGui::Button("Recompile kernel")) {
			while (!raycaster->debug_quick_recompile());
		}
		if (ImGui::Button("Pause")) {

			paused = !paused;

			if (paused)
				Logger::log("Pausing", Logger::LogLevel::INFO);
			else
				Logger::log("Unpausing", Logger::LogLevel::INFO);

		}

		ImGui::End();

		ImGui::Begin("Lights");

		if (ImGui::SliderFloat4("Color", light_color, 0, 1)) {
			sf::Vector4f light(light_color[0], light_color[1], light_color[2], light_color[3]);
			light_handle->set_rgbi(light);
		}

		if (ImGui::SliderFloat("Camera Speed", &camera_speed, 0, 4)) {
			camera->setSpeed(camera_speed);
		}

		if (ImGui::SliderFloat3("Position", light_pos, 0, MAP_X)) {
			sf::Vector3f light(light_pos[0], light_pos[1], light_pos[2]);
			light_handle->set_position(light);
		}

		if (ImGui::CollapsingHeader("Window options"))
		{
			if (ImGui::TreeNode("Style"))
			{
				ImGui::ShowStyleEditor();
				ImGui::TreePop();
			}
		}

		ImGui::End();

		ImGui::Begin("Controller debugger");

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		static ImVec4 col = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
		const ImVec2 p = ImGui::GetCursorScreenPos();
		const ImU32 col32 = ImColor(col);

		std::vector<float> axis_values = {
			 sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) / 2,
			 sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) / 2,
			 sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::U) / 2,
			 sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::R) / 2,
			 sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Z) / 2,
			 sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::V) / 2
		};
		
		ImGui::Columns(3, "Axis's"); // 4-ways, with border
		ImGui::Separator();
		ImGui::Text("X Y"); ImGui::NextColumn();
		ImGui::Text("U R"); ImGui::NextColumn();
		ImGui::Text("Z V"); ImGui::NextColumn();
		ImGui::Separator();

		for (int i = 0; i < 3; i++) {
			

			float offset = ImGui::GetColumnWidth(i);
			
			draw_list->AddLine(ImVec2(p.x + 0 + offset * i, p.y + 50), ImVec2(p.x + 100 + offset * i, p.y + 50), col32, 1.0);
			draw_list->AddLine(ImVec2(p.x + 50 + offset * i, p.y + 0), ImVec2(p.x + 50 + offset * i, p.y + 100), col32, 1.0);
			draw_list->AddCircleFilled(ImVec2(p.x + axis_values[2 * i] + 50 + offset * i, p.y + axis_values[2 * i + 1] + 50), 6, col32, 32);
			
			ImGui::Dummy(ImVec2(100, 100));
			ImGui::NextColumn();
		}

		
		ImGui::End();

		//ImGui::ShowTestWindow();

		ImGui::Render();

		// ImGUI messes up somthing in the SFML GL state, so we need a single draw call to right things
		// then we can move on to flip the screen buffer via display
		window->draw(sf::CircleShape(0));
		window->display();
	}
}

float Application::elap_time() {
	static std::chrono::time_point<std::chrono::system_clock> start;
	static bool started = false;

	if (!started) {
		start = std::chrono::system_clock::now();
		started = true;
	}

	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_time = now - start;
	return static_cast<float>(elapsed_time.count());
}

