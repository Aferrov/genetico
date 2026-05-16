#include "algoritmo_genetico.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

using namespace std;

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void generateCities(vector<City>& cities, int num_cities) {
    cities.clear();
    uniform_int_distribution<int> dist_x(50, WIDTH - 50), dist_y(50, HEIGHT - 50);
    for (int i = 0; i < num_cities; ++i) {
        cities.push_back({i, Point2D{(float)dist_x(rng), (float)dist_y(rng)}});
    }
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Bloquea redimensionar o maximizar la ventana

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Algoritmo Genetico - TSP Avanzado", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.FontGlobalScale = 1.4f; // Aumentar la escala general de ImGui
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    GAConfig config;
    vector<City> cities;
    vector<Individual> population;
    
    generateCities(cities, config.num_cities);
    initializePopulation(population, cities, config);

    int generation = 0;
    bool paused = true;
    bool step_requested = false;
    bool needs_restart = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (needs_restart) {
            if (cities.size() != config.num_cities) {
                generateCities(cities, config.num_cities);
            }
            initializePopulation(population, cities, config);
            // Asegurar que la generacion inicial esté ordenada
            sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
                return a.distance < b.distance;
            });
            generation = 0;
            needs_restart = false;
        }

        if (!paused || step_requested) {
            if (config.replacement_mode == ReplacementMode::GENERATIONAL) {
                vector<Individual> nextGen;
                for(int i = 0; i < config.elite_count && i < population.size(); i++) {
                    nextGen.push_back(population[i]); // Elitismo
                }
                while (nextGen.size() < config.pop_size) {
                    Individual p1 = selectParent(population, config);
                    Individual p2 = selectParent(population, config);
                    Individual child = crossover(p1, p2, config);
                    mutate(child, config);
                    child.distance = calculateDistance(child.route, cities);
                    nextGen.push_back(child);
                }
                population = nextGen;
                sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
                    return a.distance < b.distance;
                });
            } else {
                // STEADY STATE (Estacionario): Reemplazamos un porcentaje de la poblacion
                int children_to_generate = max(1, config.pop_size / 10); 
                vector<Individual> children;
                for(int i = 0; i < children_to_generate; i++) {
                    Individual p1 = selectParent(population, config);
                    Individual p2 = selectParent(population, config);
                    Individual child = crossover(p1, p2, config);
                    mutate(child, config);
                    child.distance = calculateDistance(child.route, cities);
                    children.push_back(child);
                }
                // Reemplazamos a los peores individuos (los del final de la lista)
                for(int i = 0; i < children_to_generate; i++) {
                    population[population.size() - 1 - i] = children[i];
                }
                sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
                    return a.distance < b.distance;
                });
            }
            generation++;
            step_requested = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(WIDTH - 550, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSizeConstraints(ImVec2(750, -1), ImVec2(750, -1));
        
        ImGui::Begin("PanelConfiguracion", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        
        // Convertimos el título en un enorme CollapsingHeader para que se pueda clickear en cualquier parte
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.15f, 0.35f, 0.65f, 1.0f)); 
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.45f, 0.85f, 1.0f)); 
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.25f, 0.55f, 1.0f)); 
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.2f, 1.0f)); 
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 12.0f)); 
        
        bool main_open = ImGui::CollapsingHeader(" --- CONFIGURACION AVANZADA TSP --- ", ImGuiTreeNodeFlags_DefaultOpen);
        
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        if (main_open) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Generacion: %d", generation);
            if (!population.empty()) ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Distancia Minima: %d", (int)population[0].distance);
            
            ImGui::SetWindowFontScale(1.3f); // Escalar fuente para los botones (1.5x sobre la base)
            
            // Reanudar/Pausar (Color Verde/Naranja dependiendo del estado)
            if (paused) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.4f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.5f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.3f, 0.1f, 1.0f));
            }
            if (ImGui::Button(paused ? "Reanudar" : "Pausar", ImVec2(220, 50))) paused = !paused;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            
            // Paso a Paso (Color Azul)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
            if (ImGui::Button("Paso a Paso", ImVec2(220, 50))) { paused = true; step_requested = true; }
            ImGui::PopStyleColor(3);

            // Reiniciar Simulacion (Color Rojo)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            if (ImGui::Button("Reiniciar Simulacion", ImVec2(448, 50))) needs_restart = true;
            ImGui::PopStyleColor(3);

            ImGui::SetWindowFontScale(1.0f); // Restaurar escala de fuente original

            ImGui::Separator();
            if (ImGui::CollapsingHeader("1-3. Estructura e Inicializacion", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::TextDisabled("Representacion: Permutacion");
                ImGui::TextDisabled("Fitness: Distancia Total (Min)");
                
                if (ImGui::SliderInt("Num Ciudades", &config.num_cities, 10, 100)) needs_restart = true;
                if (ImGui::SliderInt("Tam. Poblacion", &config.pop_size, 10, 500)) needs_restart = true;
                
                int init_mode = (int)config.init_mode;
                const char* init_items[] = { "Aleatoria", "Greedy (Vecino Mas Cercano)" };
                if (ImGui::Combo("Inicializacion", &init_mode, init_items, 2)) {
                    config.init_mode = (InitMode)init_mode;
                    needs_restart = true;
                }
            }

            if (ImGui::CollapsingHeader("4-6. Operadores Geneticos", ImGuiTreeNodeFlags_DefaultOpen)) {
                int sel_mode = (int)config.selection_mode;
                const char* sel_items[] = { "Torneo", "Ruleta" };
                if (ImGui::Combo("Seleccion", &sel_mode, sel_items, 2)) config.selection_mode = (SelectionMode)sel_mode;
                if (config.selection_mode == SelectionMode::TOURNAMENT) {
                    ImGui::SliderInt("Tamaño Torneo", &config.tournament_size, 2, 20);
                }

                int cross_mode = (int)config.crossover_mode;
                const char* cross_items[] = { "OX (Order)", "PMX (Partially Mapped)" };
                if (ImGui::Combo("Cruce", &cross_mode, cross_items, 2)) config.crossover_mode = (CrossoverMode)cross_mode;

                int mut_mode = (int)config.mutation_mode;
                const char* mut_items[] = { "Swap (Intercambio)", "Inversion (2-opt seg)" };
                if (ImGui::Combo("Mutacion", &mut_mode, mut_items, 2)) config.mutation_mode = (MutationMode)mut_mode;
                ImGui::SliderFloat("Tasa Mutacion", &config.mutation_rate, 0.0f, 0.20f, "%.3f");
            }

            if (ImGui::CollapsingHeader("7. Reemplazo", ImGuiTreeNodeFlags_DefaultOpen)) {
                int rep_mode = (int)config.replacement_mode;
                const char* rep_items[] = { "Generacional", "Estacionario (10%)" };
                if (ImGui::Combo("Reemplazo", &rep_mode, rep_items, 2)) config.replacement_mode = (ReplacementMode)rep_mode;
                ImGui::SliderInt("Elitismo", &config.elite_count, 0, config.pop_size / 2);
            }
        }

        ImGui::End();

        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
        if (!population.empty()) {
            drawRoute(draw_list, population[0], cities, generation, paused);
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}