#include "tsp_ga.h"
#include <iostream>

int main() {
    uniform_int_distribution<int> dist_x(50, WIDTH - 50), dist_y(50, HEIGHT - 50);
    vector<City> cities;
    for (int i = 0; i < NUM_CITIES; ++i) cities.push_back({i, Point(dist_x(rng), dist_y(rng))});

    vector<Individual> population(POP_SIZE);
    for (auto& ind : population) {
        ind.route.resize(NUM_CITIES);
        iota(ind.route.begin(), ind.route.end(), 0);
        shuffle(ind.route.begin(), ind.route.end(), rng);
    }

    namedWindow("Algoritmo Genético", WINDOW_AUTOSIZE);
    Mat canvas(HEIGHT, WIDTH, CV_8UC3);
    int generation = 0;
    bool paused = true;

    while (true) {
        for (auto& ind : population) ind.distance = calculateDistance(ind.route, cities);
        sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
            return a.distance < b.distance;
        });

        drawRoute(canvas, population[0], cities, generation, paused);
        imshow("TSP Genetic Algorithm", canvas);

        int key = waitKey(paused ? 0 : 1);
        if (key == 27) break;
        if (key == 's' || key == 'S') paused = true;
        if (key == 'a' || key == 'A') paused = false;
        if (key == 'p' || key == 'P') paused = true;

        if (getWindowProperty("Algoritmo Genético", WND_PROP_VISIBLE) < 1) break;

        if (!paused || (key == 's' || key == 'S')) {
            vector<Individual> nextGen;
            nextGen.push_back(population[0]); 
            uniform_int_distribution<int> dist_elite(0, POP_SIZE / 2);
            while (nextGen.size() < POP_SIZE) {
                Individual child = crossover(population[dist_elite(rng)], population[dist_elite(rng)]);
                mutate(child);
                nextGen.push_back(child);
            }
            population = nextGen;
            generation++;
        }
    }
    return 0;
}