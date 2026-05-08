#ifndef TSP_GA_H
#define TSP_GA_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <random>

using namespace std;
using namespace cv;

// Constantes globales
extern const int NUM_CITIES;
extern const int POP_SIZE;
extern const double MUTATION_RATE;
extern const int WIDTH;
extern const int HEIGHT;
extern mt19937 rng;

struct City {
    int id;
    Point pos;
};

struct Individual {
    vector<int> route;
    double distance;
};


double calculateDistance(const vector<int>& route, const vector<City>& cities);
void mutate(Individual& ind);
Individual crossover(const Individual& p1, const Individual& p2);


void drawRoute(Mat& img, const Individual& best, const vector<City>& cities, int gen, bool paused);

#endif