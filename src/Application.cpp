#include "Application.h"

const int Application::MAP_X = 16;
const int Application::MAP_Y = 16;
const int Application::MAP_Z = 16;

Application::Application() {

	window = std::make_shared<sf::RenderWindow>(sf::VideoMode(WINDOW_X, WINDOW_Y), "SFML");
	window->setMouseCursorVisible(false);
	window->setKeyRepeatEnabled(false);
	window->setVerticalSyncEnabled(false);

	ImGui::SFML::Init(*window);
	window->resetGLStates();
}

Application::~Application() {

    if (window.unique())
        window.reset();
    else {
        Logger::log("Can't release window, shared_ptr count : " + window.use_count(), Logger::LogLevel::WARN);
    }
}

bool Application::init_clcaster() {

	// Start up the raycaster
	raycaster = std::make_shared<CLCaster>();

	if (!raycaster->init())
		abort();

    raycaster->add_to_settings_buffer("octree_dimensions", "OCTDIM", MAP_X);

    // TODO: ALLOW RVALUES FOR SETTINGS BUFFER
    uint64_t oct_enabled = 0;
    raycaster->add_to_settings_buffer("using_octree", "OCTENABLED", oct_enabled);

	map = std::make_shared<Map>(MAP_X);

    // TODO: Implement this
    sf::Image bitmap = map->GenerateHeightBitmap(sf::Vector3i(MAP_X, MAP_Y, MAP_Z));
	map->ApplyHeightmap(bitmap);
	
	//map->octree.CastRayOctree(sf::Vector2f(1.57f, 0.0001f), sf::Vector3f(0.5f, 0.5f, 0.5f));

    // TODO: Consolidate this to one call
	raycaster->assign_octree(map);
	raycaster->assign_map(map);


	camera = std::make_shared<Camera>(
		sf::Vector3f(3.8f, 3.4f, 2.6f), // Starting position
		sf::Vector2f(1.56f, .81f),      // Direction
		window.get()
	);
	raycaster->assign_camera(camera);

	// Generate and send the viewport to the GPU. Also creates the viewport texture
	raycaster->create_viewport(WINDOW_X, WINDOW_Y, 0.625f * 90.0f, 90.0f);

	// Initialize the light controller and link it to the GPU
    // TODO: Bad behaviour when raycaster has errors!!!!
	light_controller = std::make_shared<LightController>(raycaster);

	// Create a light prototype, send it to the controller, and get the handle back
	LightPrototype prototype(
		sf::Vector3f(10, 10.0f, 10.0f),
		sf::Vector3f(-1.0f, -1.0f, -1.5f),
		sf::Vector4f(0.01f, 0.01f, 0.01f, 0.2f)
	);
	light_handle = light_controller->create_light(prototype);

	// Load in the spritesheet texture
	if (!spritesheet.loadFromFile("../assets/textures/minecraft_tiles.png"))
		Logger::log("Failed to load spritesheet from file", Logger::LogLevel::WARN);
	raycaster->create_texture_atlas(&spritesheet, sf::Vector2i(16, 16));

	// Compiles the kernel, Checks to see if proper data was uploaded, then sets the kernel args
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

	//raycaster->subscribe_to_publisher(&input_handler, vr::Event::EventType::KeyPressed);
	//camera->subscribe_to_publisher(&input_handler, vr::Event::EventType::JoystickMoved);
	
	return true;
}

bool Application::game_loop() {

    int fps_idx = fps.create_line("FPS");
    int compute_fps_idx = fps.create_line("Compute");
    int event_fps_idx = fps.create_line("Event Time");

	while (true) {

		// Have the input handler empty the event stack, generate events for held keys, and then dispatch the events to listeners
        fps.start(event_fps_idx);
        input_handler.consume_sf_events(window.get());
        input_handler.handle_held_keys();
		input_handler.dispatch_events();
        fps.stop(event_fps_idx);

		if (!window->isOpen())
			break;
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
            fps.start(compute_fps_idx);
			if (!raycaster->compute()) {
				abort();
			};
            fps.stop(compute_fps_idx);
		}

		// Let the raycaster draw it screen buffer
		raycaster->draw(window.get());

		// Give the frame counter the frame time and draw the average frame time
		fps.frame(fps_idx, delta_time);
		fps.draw();

		Gui::do_render();

		ImGui::Begin("Window");
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

		if (ImGui::Button("Pause")) {

			paused = !paused;

			if (paused)
				Logger::log("Pausing", Logger::LogLevel::INFO);
			else
				Logger::log("Unpausing", Logger::LogLevel::INFO);

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

