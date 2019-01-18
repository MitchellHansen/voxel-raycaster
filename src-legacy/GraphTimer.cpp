#include "../include-legacy/GraphTimer.h"

GraphTimer::GraphTimer()  {
    if (!started) {
        start_time = std::chrono::system_clock::now();
        started = true;
    }

    // TODO: This is a cardinal sin
    if (instance == nullptr){
        instance = this;
    }
}

GraphTimer::~GraphTimer() {

}

unsigned int GraphTimer::create_line(std::string label) {
    fps_vectors.push_back(std::vector<float>(FPS_ARRAY_LENGTH, 0));
    counters.push_back(0);
    checkpoints.push_back(0);
    labels.push_back(label);
    return fps_vectors.size()-1;
}

unsigned int GraphTimer::delete_line(unsigned int idx){
    fps_vectors.erase(fps_vectors.begin() + idx);
	return 1;
}

void GraphTimer::start(unsigned int idx){

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = now - start_time;

    checkpoints.at(idx) = elapsed_time.count();
}

void GraphTimer::stop(unsigned int idx){

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = now - start_time;

    if (++counters.at(idx) >= FPS_ARRAY_LENGTH)
        counters.at(idx) = 0;

    fps_vectors.at(idx).at(counters.at(idx)) = elapsed_time.count() - checkpoints.at(idx);
    fps_vectors.at(idx).at(counters.at(idx)) = 1.0f / fps_vectors.at(idx).at(counters.at(idx));

}

void GraphTimer::frame(unsigned int idx, double delta_time) {

    // Do this before we access to make the title generation in draw a bit easier
    if (++counters.at(idx) >= FPS_ARRAY_LENGTH)
        counters.at(idx) = 0;

    fps_vectors.at(idx).at(counters.at(idx)) =  1.0 / delta_time;
}

void GraphTimer::draw() {

    ImGui::Begin("Performance");

    std::string title = std::to_string(fps_vectors.at(0).at(counters.at(0)));

    std::vector<ImColor> colors = {
            ImColor(255, 255, 255),
            ImColor(255, 255,   0),
            ImColor(  0, 255, 255),
            ImColor(255,   0, 255),
            ImColor(255,   0,   0),
            ImColor(  0, 255,   0),
            ImColor(  0,   0, 255),
    };

    ImVec2 wh = ImGui::GetContentRegionAvail();
    wh.x -= wh.x * 0.15;
    sf::Vector2f graph_size(wh.x, wh.y);

    ImGui::PlotMultiLines(fps_vectors, title, labels, colors, 200, 0,
                          graph_size);

    ImGui::End();

}

void GraphTimer::count(unsigned int idx, int counter) {
    if (++counters.at(idx) >= FPS_ARRAY_LENGTH)
        counters.at(idx) = 0;
    fps_vectors.at(idx).at(counters.at(idx)) = counter;

}

GraphTimer *GraphTimer::get_instance() {
    return instance;
}

std::chrono::time_point<std::chrono::system_clock> GraphTimer::start_time;
bool GraphTimer::started;
GraphTimer* GraphTimer::instance = nullptr;