# Programa Interactivo del Algoritmo Genético (TSP)

Este proyecto es una aplicacion interactiva escrita en **C++** y **OpenGL** que resuelve el **Problema del Viajante de Comercio (TSP - Traveling Salesperson Problem)** utilizando un Algoritmo Genético. Su propósito principal es educativo, permitiendo visualizar en tiempo real cómo diferentes estrategias y operadores genéticos afectan la evolución y convergencia hacia la ruta óptima.

La interfaz gráfica fue construida usando **Dear ImGui**, lo que permite modificar los parámetros del algoritmo genético al vuelo y observar el impacto inmediato en la simulación.

## 🚀 Características y Opciones del Simulador

La aplicacion cuenta con un panel de control que te permite experimentar con diferentes fases del algoritmo genético:

* **Control de Simulación y Monitoreo:**
  * **Reanudar / Pausar:** Detén la evolución en cualquier momento para analizar la ruta actual.
  * **Paso a Paso:** Avanza generación por generación manualmente.
  * **Reiniciar Simulación:** Genera un nuevo conjunto de ciudades y reinicia la población.
  * **Generaciones sin mejora:** Indicador en tiempo real que cuenta las generaciones consecutivas sin hallar una ruta más corta. Es fundamental para saber cuándo el algoritmo ha convergido en un óptimo local y sirve como un excelente criterio de parada.

* **1. Estructura e Inicialización:**
  * **Número de Ciudades:** Ajustable de 10 a 100 ciudades distribuidas aleatoriamente.
  * **Tamaño de la Población:** Ajustable de 10 a 500 individuos.
  * **Modo de Inicialización:** 
    * *Aleatoria:* Rutas generadas al azar.
    * *Greedy (Vecino Más Cercano):* Genera individuos iniciales buscando siempre la ciudad no visitada más cercana, acelerando la convergencia.

* **2. Operadores Genéticos (Selección, Cruce y Mutación):**
  * **Selección:** Torneo (con tamaño de torneo ajustable) o Ruleta.
  * **Cruce:** OX (Order Crossover) o PMX (Partially Mapped Crossover).
  * **Mutación:** Intercambio Simple (Swap) o Inversión (2-opt). La tasa de mutación es completamente configurable.

* **3. Reemplazo y Elitismo:**
  * **Modo de Reemplazo:** Generacional (toda la población se renueva) o Estacionario (solo un 10% de la población se renueva por generación).
  * **Elitismo:** Preserva a los mejores individuos intactos de una generación a la siguiente.

## 🛠️ Tecnologías Utilizadas

- **Lenguaje:** C++ (Requiere C++17)
- **Gráficos:** OpenGL 3 / GLFW
- **Interfaz de Usuario:** Dear ImGui
- **Sistema de Construcción:** CMake

## ⚙️ Cómo Compilar y Ejecutar

El proyecto utiliza `CMake` junto con `FetchContent`, lo que significa que descargará y configurará automáticamente las dependencias (GLFW e ImGui) durante el proceso de configuración. No necesitas instalarlas manualmente.

### Prerrequisitos
- Compilador de C++ (MinGW-w64, GCC, Clang, o MSVC) compatible con C++17.
- CMake (versión 3.15 o superior).

### Pasos para compilar en Windows

1. Clona el repositorio y abre una terminal en la carpeta del proyecto.
2. Configura el proyecto con CMake:
   ```bash
   cmake -B build -S .
   ```
3. Construye el ejecutable:
   ```bash
   cmake --build build
   ```
4. Ejecuta el simulador:
   ```bash
   .\build\genetico_opengl.exe
   ```

*(Nota: En sistemas Linux/macOS, los comandos de CMake son los mismos, y el ejecutable generado suele llamarse `./build/genetico_opengl`)*.

## 🧠 Arquitectura del Código

El código fue refactorizado para mantener una estructura limpia y fácil de entender:

- `main.cpp`: Punto de entrada del programa. Maneja la inicialización de GLFW, OpenGL, la creación del contexto de ImGui, y contiene el bucle principal de renderizado y simulación.
- `algoritmo_genetico.h` / `algoritmo_genetico.cpp`: Contienen toda la lógica, estructuras y matemáticas del algoritmo genético. Aquí encontrarás la implementación de los operadores de cruce (OX, PMX), inicialización Greedy, fitness, etc.
- `CMakeLists.txt`: Configuración de compilación que incluye la descarga automática de librerías de terceros (ImGui y GLFW).

## 📄 Licencia

Este proyecto está abierto a cualquier uso educativo o personal.
