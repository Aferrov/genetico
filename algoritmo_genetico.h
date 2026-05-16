#ifndef TSP_GA_H
#define TSP_GA_H

#include <vector>
#include <random>

struct ImDrawList;

using namespace std;

// Constantes globales de dibujo
extern const int WIDTH;
extern const int HEIGHT;
extern mt19937 rng;

enum class InitMode { RANDOM, GREEDY };
enum class SelectionMode { TOURNAMENT, ROULETTE };
enum class CrossoverMode { OX, PMX };
enum class MutationMode { SWAP, INVERSION };
enum class ReplacementMode { GENERATIONAL, STEADY_STATE };

struct GAConfig {
    int num_cities = 30;
    int pop_size = 100;
    float mutation_rate = 0.05f;
    int tournament_size = 5;
    int elite_count = 1;
    
    InitMode init_mode = InitMode::RANDOM;
    SelectionMode selection_mode = SelectionMode::TOURNAMENT;
    CrossoverMode crossover_mode = CrossoverMode::OX;
    MutationMode mutation_mode = MutationMode::SWAP;
    ReplacementMode replacement_mode = ReplacementMode::GENERATIONAL;
};

struct Point2D {
    float x, y;
};

struct City {
    int id;
    Point2D pos;
};

struct Individual {
    vector<int> route;
    double distance;
};

double calculateDistance(const vector<int>& route, const vector<City>& cities);
void mutate(Individual& ind, const GAConfig& config);
Individual crossover(const Individual& p1, const Individual& p2, const GAConfig& config);
Individual selectParent(const vector<Individual>& population, const GAConfig& config);
void initializePopulation(vector<Individual>& population, const vector<City>& cities, const GAConfig& config);

void drawRoute(ImDrawList* draw_list, const Individual& best, const vector<City>& cities, int gen, bool paused);

#endif