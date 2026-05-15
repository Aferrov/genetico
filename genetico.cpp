#include "tsp_ga.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h> // Incluir GLFW

using namespace std;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main() {
    // Configurar GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Algoritmo Genetico - TSP (OpenGL + ImGui)", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // Inicializar Backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- Lógica del Algoritmo Genético (Inicialización) ---
    uniform_int_distribution<int> dist_x(50, WIDTH - 50), dist_y(50, HEIGHT - 50);
    vector<City> cities;
    for (int i = 0; i < NUM_CITIES; ++i) {
        cities.push_back({i, Point2D{(float)dist_x(rng), (float)dist_y(rng)}});
    }

    vector<Individual> population(POP_SIZE);
    for (auto& ind : population) {
        ind.route.resize(NUM_CITIES);
        iota(ind.route.begin(), ind.route.end(), 0);
        shuffle(ind.route.begin(), ind.route.end(), rng);
    }

    int generation = 0;
    bool paused = true;
    bool step_requested = false;

    // --- Bucle Principal ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // 1. Lógica Matemática
        if (!paused || step_requested) {
            for (auto& ind : population) ind.distance = calculateDistance(ind.route, cities);
            sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
                return a.distance < b.distance;
            });

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
            step_requested = false;
        }

        // Asegurarse de ordenar una vez si estamos en pausa en gen 0
        if (generation == 0 && paused && population[0].distance == 0) {
             for (auto& ind : population) ind.distance = calculateDistance(ind.route, cities);
             sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
                return a.distance < b.distance;
            });
        }

        // 2. Iniciar Frame de ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 3. Ventana de Control
        ImGui::Begin("Control de TSP", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button(paused ? "Reanudar" : "Pausar")) paused = !paused;
        ImGui::SameLine();
        if (ImGui::Button("Paso a Paso")) { paused = true; step_requested = true; }
        
        ImGui::Separator();
        ImGui::Text("Generacion: %d", generation);
        if (!population.empty()) {
            ImGui::Text("Distancia Minima: %d", (int)population[0].distance);
        }
        ImGui::End();

        // 4. Dibujar Ruta en el fondo
        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
        if (!population.empty()) {
            drawRoute(draw_list, population[0], cities, generation, paused);
        }

        // 5. Renderizado final
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Limpieza
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}