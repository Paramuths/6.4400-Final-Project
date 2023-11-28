#include <iostream>
#include <chrono>
#include <string>
#include <cstdio>
#include <stdexcept>

#include "SimulationApp.hpp"
#include "IntegratorType.hpp"

using namespace GLOO;

int main(int argc, char** argv) {
  if ((argc != 3) && (argc != 4)) {
    printf("Usage: %s <e|t|r> <timestep> [index]\n", argv[0]);
    printf("       e: Integrator: Forward Euler\n");
    printf("       t: Integrator: Trapezoid\n");
    printf("       r: Integrator: RK 4\n");
    printf("       index: Render the springs connected to a particle with this index\n");
    printf("\n");
    printf("Try  : %s t 0.001\n", argv[0]);
    printf("       for Trapezoid (1ms steps)\n");
    printf("Or   : %s r 0.005\n", argv[0]);
    printf("       for RK4 (5ms steps)\n");
    printf("Or   : %s e 0.005 2\n", argv[0]);
    printf("       for Forward Euler (5ms steps) with springs connected to a particle with index 2\n");
    return -1;
  }

  IntegratorType integrator_type;
  switch (argv[1][0]) {
    case 'e':
      integrator_type = IntegratorType::Euler;
      break;
    case 't':
      integrator_type = IntegratorType::Trapezoidal;
      break;
    case 'r':
      integrator_type = IntegratorType::RK4;
      break;
    default:
      throw std::runtime_error(
          "Unrecognized integrator type: " + std::string(1, argv[1][0]) + ".");
  }

  float integration_step = std::stof(argv[2]);
  int spring_particle_index = (argc == 4)? std::stoi(argv[3]): -1;

  std::unique_ptr<SimulationApp> app = make_unique<SimulationApp>(
      "Assignment3", glm::ivec2(1440, 900), integrator_type, integration_step, spring_particle_index);

  app->SetupScene();

  using Clock = std::chrono::high_resolution_clock;
  using TimePoint =
      std::chrono::time_point<Clock, std::chrono::duration<double>>;
  TimePoint last_tick_time = Clock::now();
  TimePoint start_tick_time = last_tick_time;
  while (!app->IsFinished()) {
    TimePoint current_tick_time = Clock::now();
    double delta_time = (current_tick_time - last_tick_time).count();
    double total_elapsed_time = (current_tick_time - start_tick_time).count();
    app->Tick(delta_time, total_elapsed_time);
    last_tick_time = current_tick_time;
  }
  return 0;
}
