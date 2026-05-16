#include "algoritmo_genetico.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <string>
#include <ctime>
#include <set>
#include "imgui.h"

const int WIDTH = 1600;
const int HEIGHT = 900;

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

void initializePopulation(vector<Individual>& population, const vector<City>& cities, const GAConfig& config) {
    population.resize(config.pop_size);
    for (int i = 0; i < config.pop_size; ++i) {
        population[i].route.resize(config.num_cities);
        iota(population[i].route.begin(), population[i].route.end(), 0);
        
        if (config.init_mode == InitMode::GREEDY && i < min(config.pop_size, config.num_cities)) {
            // Greedy Initialization: empezar en ciudad 'i', buscar el más cercano
            vector<bool> visited(config.num_cities, false);
            int current_city = i; // ciudad inicial
            population[i].route[0] = current_city;
            visited[current_city] = true;
            
            for (int step = 1; step < config.num_cities; ++step) {
                int best_next = -1;
                double min_dist = 1e9;
                for (int next_city = 0; next_city < config.num_cities; ++next_city) {
                    if (!visited[next_city]) {
                        Point2D p1 = cities[current_city].pos;
                        Point2D p2 = cities[next_city].pos;
                        double d = pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2); // Distancia al cuadrado es más rápida
                        if (d < min_dist) {
                            min_dist = d;
                            best_next = next_city;
                        }
                    }
                }
                population[i].route[step] = best_next;
                visited[best_next] = true;
                current_city = best_next;
            }
        } else {
            // Random Initialization
            shuffle(population[i].route.begin(), population[i].route.end(), rng);
        }
        population[i].distance = calculateDistance(population[i].route, cities);
    }
}

Individual selectParent(const vector<Individual>& population, const GAConfig& config) {
    if (config.selection_mode == SelectionMode::TOURNAMENT) {
        uniform_int_distribution<int> dist_pop(0, population.size() - 1);
        int best_idx = dist_pop(rng);
        for (int i = 1; i < config.tournament_size; ++i) {
            int contender = dist_pop(rng);
            if (population[contender].distance < population[best_idx].distance) {
                best_idx = contender;
            }
        }
        return population[best_idx];
    } 
    else { // ROULETTE
        double total_fitness = 0;
        vector<double> fitness(population.size());
        for (size_t i = 0; i < population.size(); ++i) {
            fitness[i] = 1.0 / population[i].distance;
            total_fitness += fitness[i];
        }
        uniform_real_distribution<double> dist_prob(0.0, total_fitness);
        double r = dist_prob(rng);
        double running_sum = 0;
        for (size_t i = 0; i < population.size(); ++i) {
            running_sum += fitness[i];
            if (running_sum >= r) return population[i];
        }
        return population.back();
    }
}

void mutate(Individual& ind, const GAConfig& config) {
    uniform_real_distribution<double> dist_prob(0.0, 1.0);
    if (dist_prob(rng) < config.mutation_rate) {
        uniform_int_distribution<int> dist_city(0, config.num_cities - 1);
        int i = dist_city(rng);
        int j = dist_city(rng);
        
        if (config.mutation_mode == MutationMode::SWAP) {
            swap(ind.route[i], ind.route[j]);
        } 
        else if (config.mutation_mode == MutationMode::INVERSION) {
            if (i > j) swap(i, j);
            reverse(ind.route.begin() + i, ind.route.begin() + j + 1);
        }
    }
}

Individual crossover(const Individual& p1, const Individual& p2, const GAConfig& config) {
    Individual child;
    child.route.assign(config.num_cities, -1);
    
    uniform_int_distribution<int> dist_city(0, config.num_cities - 1);
    int start = dist_city(rng), end = dist_city(rng);
    if (start > end) swap(start, end);

    if (config.crossover_mode == CrossoverMode::OX) {
        for (int i = start; i <= end; ++i) child.route[i] = p1.route[i];

        int childIdx = (end + 1) % config.num_cities;
        for (int i = 0; i < config.num_cities; ++i) {
            int cityP2 = p2.route[(end + 1 + i) % config.num_cities];
            if (find(child.route.begin(), child.route.end(), cityP2) == child.route.end()) {
                child.route[childIdx] = cityP2;
                childIdx = (childIdx + 1) % config.num_cities;
            }
        }
    } 
    else if (config.crossover_mode == CrossoverMode::PMX) {
        for (int i = start; i <= end; ++i) child.route[i] = p1.route[i];
        
        for (int i = start; i <= end; ++i) {
            int cityP2 = p2.route[i];
            if (find(child.route.begin(), child.route.end(), cityP2) == child.route.end()) {
                int current = p1.route[i];
                int idxP2 = find(p2.route.begin(), p2.route.end(), current) - p2.route.begin();
                while (start <= idxP2 && idxP2 <= end) {
                    current = p1.route[idxP2];
                    idxP2 = find(p2.route.begin(), p2.route.end(), current) - p2.route.begin();
                }
                child.route[idxP2] = cityP2;
            }
        }
        for (int i = 0; i < config.num_cities; ++i) {
            if (child.route[i] == -1) child.route[i] = p2.route[i];
        }
    }
    return child;
}

void drawRoute(ImDrawList* draw_list, const Individual& best, const vector<City>& cities, int gen, int gens_without_improvement, bool paused) {
    draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(WIDTH, HEIGHT), IM_COL32(30, 30, 30, 255));
    
    if (best.route.size() != cities.size()) return;

    for (size_t i = 0; i < best.route.size(); ++i) {
        Point2D p1 = cities[best.route[i]].pos;
        Point2D p2 = cities[best.route[(i + 1) % cities.size()]].pos;
        draw_list->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), IM_COL32(0, 255, 0, 255), 2.0f);
    }
    
    for (const auto& city : cities) {
        draw_list->AddCircleFilled(ImVec2(city.pos.x, city.pos.y), 5.0f, IM_COL32(255, 0, 0, 255));
        string id_str = to_string(city.id);
        draw_list->AddText(ImVec2(city.pos.x + 5, city.pos.y - 15), IM_COL32(255, 255, 255, 255), id_str.c_str());
    }
    
    string info = "Generacion: " + to_string(gen) + " | Distancia minima: " + to_string((int)best.distance) + " | Sin mejora: " + to_string(gens_without_improvement) + " gens";
    string status = paused ? "PAUSADO - Modifica config o dale a Play" : "CORRIENDO... Buscando optima";
    
    draw_list->AddText(ImVec2(20, 40), IM_COL32(255, 255, 255, 255), info.c_str());
    draw_list->AddText(ImVec2(20, 70), IM_COL32(0, 255, 255, 255), status.c_str());
}