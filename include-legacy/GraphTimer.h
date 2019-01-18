#pragma once
#include <chrono>
#include <imgui/imgui.h>
#include <imgui/imgui-multilines.hpp>
#include <vector>
#include <string>

struct GraphTimer {
public:
    GraphTimer();

    ~GraphTimer();

    static GraphTimer* get_instance();

    unsigned int create_line(std::string label);
    unsigned int delete_line(unsigned int idx);

    void start(unsigned int idx);
    void stop(unsigned int idx);
    void frame(unsigned int idx, double delta_time);
    void count(unsigned int idx, int counter);

    void draw();

private:

    static std::chrono::time_point<std::chrono::system_clock> start_time;
    static bool started;

    const unsigned int FPS_ARRAY_LENGTH = 1000;
    std::vector<std::vector<float>> fps_vectors;
    std::vector<double> checkpoints;
    std::vector<int> counters;
    std::vector<std::string> labels;


    // I actually think I might make this a singleton, along
    static GraphTimer* instance;
};