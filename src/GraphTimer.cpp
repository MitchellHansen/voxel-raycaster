#include "GraphTimer.h"

GraphTimer::GraphTimer()  {
    if (!started) {
        start_time = std::chrono::system_clock::now();
        started = true;
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
}

void GraphTimer::start(unsigned int idx){

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = now - start_time;

    checkpoints.at(idx) = elapsed_time.count();
}

void GraphTimer::stop(unsigned int idx){

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = now - start_time;

    fps_vectors.at(idx).at(counters.at(idx)) = elapsed_time.count() - checkpoints.at(idx);
    fps_vectors.at(idx).at(counters.at(idx)) = 1.0f / fps_vectors.at(idx).at(counters.at(idx));
    if (++counters.at(idx) >= FPS_ARRAY_LENGTH - 1)
        counters.at(idx) = 0;
}

void GraphTimer::frame(unsigned int idx, double delta_time) {
    fps_vectors.at(idx).at(counters.at(idx)) =  1.0 / delta_time;
    if (++counters.at(idx) >= FPS_ARRAY_LENGTH - 1)
        counters.at(idx) = 0;
}

void GraphTimer::draw() {

    ImGui::Begin("Performance");

    std::vector<std::vector<int>> data = {
            {1, 2, 3, 4},
            {9, 3, 7, 1},
            {8, 3, 6, 2}
    };

    std::string title = std::to_string(fps_vectors.at(0).at(counters.at(0)));


    std::vector<ImColor> colors = {
            ImColor(255, 255, 255),
            ImColor(0, 255, 0),
            ImColor(255, 0, 0),
    };

    ImVec2 wh = ImGui::GetContentRegionAvail();
    wh.x -= wh.x * 0.15;
    sf::Vector2f graph_size(wh.x, wh.y);

    ImGui::PlotMultiLines(fps_vectors, title, labels, colors, 200, 0,
                          graph_size);


    //ImVec2 wh(100, 200);
//		ImGui::PlotLines("FPS", fps_array, 1000, 0,
//						 std::to_string(1.0 / fps_average).c_str(),
//						 0.0f, 150.0f, wh);

    ImGui::End();

}

std::chrono::time_point<std::chrono::system_clock> GraphTimer::start_time;
bool GraphTimer::started;