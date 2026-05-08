#include "tsp_ga.h"
#include <algorithm>
#include <numeric>

const int NUM_CITIES = 30;
const int POP_SIZE = 100;
const double MUTATION_RATE = 0.05;
const int WIDTH = 800;
const int HEIGHT = 600;

mt19937 rng(static_cast<long unsigned int>(time(0)));

double calculateDistance(const vector<int>& route, const vector<City>& cities) {
    double dist = 0;
    for (size_t i = 0; i < route.size(); ++i) {
        Point p1 = cities[route[i]].pos;
        Point p2 = cities[route[(i + 1) % route.size()]].pos;
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

void drawRoute(Mat& img, const Individual& best, const vector<City>& cities, int gen, bool paused) {
    img = Scalar(30, 30, 30);
    for (size_t i = 0; i < best.route.size(); ++i) {
        line(img, cities[best.route[i]].pos, cities[best.route[(i + 1) % NUM_CITIES]].pos, Scalar(0, 255, 0), 2, LINE_AA);
    }
    for (const auto& city : cities) {
        circle(img, city.pos, 5, Scalar(0, 0, 255), -1);
        putText(img, to_string(city.id), city.pos + Point(5, -5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
    }
    string info = "Gen: " + to_string(gen) + " | Dist: " + to_string((int)best.distance);
    string status = paused ? "PAUSADO - 'S' (Step), 'A' (Auto)" : "CORRIENDO... 'P' (Pausar)";
    putText(img, info, Point(20, 40), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 2);
    putText(img, status, Point(20, HEIGHT - 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 255), 1);
}