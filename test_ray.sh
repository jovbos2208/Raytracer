#!/bin/bash
#SBATCH --job-name=raytrace_batch_{altitude}
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=48
#SBATCH --time=00:30:00
#SBATCH --partition=test
#SBATCH --output=raytrace_batch-%j.out
#SBATCH --error=raytrace_batch-%j.err
#SBATCH --account=pn76du

module load gcc/11.2.0
module load openmpi/4.1.2-gcc11

mpirun -np 48 ./TestMain 200 -10.0 0
