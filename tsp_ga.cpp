#include "tsp_ga.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <string>
#include <ctime>
#include "imgui.h"

const int NUM_CITIES = 30;
const int POP_SIZE = 100;
const double MUTATION_RATE = 0.05;
const int WIDTH = 800;
const int HEIGHT = 600;

mt19937 rng(static_cast<long unsigned int>(time(0)));

double calculateDistance(const vector<int>& route, const vector<City>& cities) {
    double dist = 0;
    for (size_t i = 0; i < route.size(); ++i) {
        Point2D p1 = cities[route[i]].pos;
        Point2D p2 = cities[route[(i + 1) % route.size()]].pos;
        dist += sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
    }
    return dist;
}

void mutate(Individual& ind) {
    uniform_real_distribution<double> dist_prob(0.0, 1.0);
    if (dist_prob(rng) < MUTATION_RATE) {
        uniform_int_distribution<int> dist_city(0, NUM_CITIES - 1);
        swap(ind.route[dist_city(rng)], ind.route[dist_city(rng)]);
    }
}

Individual crossover(const Individual& p1, const Individual& p2) {
    Individual child;
    child.route.assign(NUM_CITIES, -1);
    
    uniform_int_distribution<int> dist_city(0, NUM_CITIES - 1);
    int start = dist_city(rng), end = dist_city(rng);
    if (start > end) swap(start, end);

    for (int i = start; i <= end; ++i) child.route[i] = p1.route[i];

    int childIdx = (end + 1) % NUM_CITIES;
    for (int i = 0; i < NUM_CITIES; ++i) {
        int cityP2 = p2.route[(end + 1 + i) % NUM_CITIES];
        if (find(child.route.begin(), child.route.end(), cityP2) == child.route.end()) {
            child.route[childIdx] = cityP2;
            childIdx = (childIdx + 1) % NUM_CITIES;
        }
    }
    return child;
}

void drawRoute(ImDrawList* draw_list, const Individual& best, const vector<City>& cities, int gen, bool paused) {
    // Dibujar fondo oscuro
    draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(WIDTH, HEIGHT), IM_COL32(30, 30, 30, 255));
    
    // Dibujar líneas
    for (size_t i = 0; i < best.route.size(); ++i) {
        Point2D p1 = cities[best.route[i]].pos;
        Point2D p2 = cities[best.route[(i + 1) % NUM_CITIES]].pos;
        draw_list->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), IM_COL32(0, 255, 0, 255), 2.0f);
    }
    
    // Dibujar ciudades
    for (const auto& city : cities) {
        draw_list->AddCircleFilled(ImVec2(city.pos.x, city.pos.y), 5.0f, IM_COL32(255, 0, 0, 255));
        string id_str = to_string(city.id);
        draw_list->AddText(ImVec2(city.pos.x + 5, city.pos.y - 15), IM_COL32(255, 255, 255, 255), id_str.c_str());
    }
    
    // Información de texto
    string info = "Gen: " + to_string(gen) + " | Dist: " + to_string((int)best.distance);
    string status = paused ? "PAUSADO - Click en 'Paso a Paso' o 'Auto'" : "CORRIENDO... Click en 'Pausa'";
    
    draw_list->AddText(ImVec2(20, 40), IM_COL32(255, 255, 255, 255), info.c_str());
    draw_list->AddText(ImVec2(20, HEIGHT - 30), IM_COL32(0, 255, 255, 255), status.c_str());
}