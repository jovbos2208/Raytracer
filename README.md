# 🚀 VLEO Satellite Drag Simulation

## Overview

This project simulates the drag on satellites flying in the **Very Low Earth Orbit (VLEO)** using a **ray-tracing-based particle method**. It models gas-surface interactions under rarefied conditions (free molecular flow) using Maxwell-Boltzmann sampled particles and computes the total drag force and resulting drag coefficient (**C<sub>D</sub>**) for different satellite geometries and flow conditions.

Key features include:
- DRIA and Sentman scattering models
- Parallelism via **MPI** and **OpenMP**
- Configurable species composition from atmospheric data
- Export to **VTK** for ray visualization

## Use Case

This simulation is tailored for satellite flight dynamics in the **150–300 km** altitude range, where:
- Flow is rarefied (Knudsen number ≫ 1),
- Molecular collisions are negligible,
- Classical CFD fails.

Typical applications:
- **Drag coefficient estimation** for attitude control
- **Design validation** for low-drag geometries
- **Thermal modeling** of surface-molecule interactions
- **Parametric sweeps** across AoA and altitude

## Installation

To build the simulation environment and prepare all required files:

```bash
chmod +x install.sh
./install.sh
```

This script performs the following:
- Creates the `build/` directory
- Runs CMake and builds the source with Make
- Runs all tests to validate core components
- Copies required assets:
  - `config.ini`
  - All `database_*.csv` files
  - All mesh models (`*.obj`, `*.h5`) into `build/`

## Configuration

Simulation input is controlled through the `config.ini` file. This contains:
- `geometry`: Path to the 3D satellite mesh (e.g., `models/SOAR.obj`)
- `ray_count`: Number of simulated rays
- `energy_accommodation`, `reflection_ratio`, `absorption_ratio`: Surface interaction model parameters
- `flow_velocity`, `direction`: Freestream conditions
- Per-species density and mass

> ✅ `config.ini` is automatically **updated at runtime** using atmospheric CSVs (e.g., `database_300km.csv`) based on altitude and selected row index.

## Running the Simulation

To execute the simulation using MPI:

```bash
mpirun -n <num_procs> ./simulation <altitude_km> <angle_of_attack_deg> <csv_row_index>
```

Example:

```bash
mpirun -n 4 ./simulation 300 10 0
```

This:
1. Reads atmospheric data from `assets/atmos_data/database_300km.csv`
2. Updates `config.ini` with correct conditions
3. Runs the ray-tracing simulation in parallel
4. Outputs:
   - `ray_trace.vtk` — 3D ray paths (for ParaView)
   - `totalDragCoefficient_300km_idx0.txt` — computed total drag coefficient

## SLURM Job Execution

For HPC environments using **SLURM**, the file `test_ray.sh` provides a basic job submission script:

```bash
#!/bin/bash
#SBATCH --job-name=ray_sim
#SBATCH --output=ray_output_%j.txt
#SBATCH --ntasks=4
#SBATCH --time=01:00:00
#SBATCH --partition=standard

module load mpi
srun ./simulation 300 10 0
```

Submit the job using:

```bash
sbatch test_ray.sh
```

## Output Files

- **`ray_trace.vtk`**: Visualization of rays and geometry (use ParaView)
- **`totalDragCoefficient_<alt>km_idx<index>.txt`**: Resulting drag coefficient
- Console output:
  - Reference area, mass flux, forces
  - Hit statistics and bounce distributions
  - Per-species density contribution

## Dependencies

- **MPI** (e.g. OpenMPI or MPICH)
- **OpenMP** (multi-threading)
- **CMake** (≥ 3.12)
- **C++17** compiler (e.g. `g++`, `clang++`)
- Optional: [ParaView](https://www.paraview.org/) for visualizing `.vtk` files

## License

MIT License © 2025

## Contact

For issues, suggestions, or academic collaboration, feel free to reach out.
