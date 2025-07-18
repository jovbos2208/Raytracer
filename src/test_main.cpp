#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <numeric>
#include <mpi.h>
#include <omp.h>
#include <algorithm>
#include <atomic>

#include "MeshLoader.h"
#include "SimulationController.h"
#include "IntersectionEngine.h"
#include "SurfaceInteractionModel.h"
#include "ConfigLoader.h"
#include "HeatmapExporter.h"
#include "DragForceCalculator.h"
#include "Vector3.h"
#include "Ray.h"
#include "Triangle.h"

std::vector<std::pair<Vector3, Vector3>> raySegmentsDebug;

/// Helper structure for communicating rays across MPI ranks
struct MPI_RayData {
    double origin[3];
    double velocity[3];
    double weight;
    int panelId;
};

/// Compute the area of a triangle given its vertices
inline double computeTriangleArea(const Vector3& a, const Vector3& b, const Vector3& c) {
    return 0.5 * (b - a).cross(c - a).norm();
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    // --- MPI Struct for RayMPI
    MPI_Datatype MPI_RayMPI_Type;
    int blockLengths[] = { 3, 3, 3, 3, 1, 1, 1, 32, 1, 1, 1 };
    MPI_Aint displacements[12];
    MPI_Datatype types[] = {
        MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
        MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
        MPI_CHAR,
        MPI_C_BOOL,
        MPI_DOUBLE,
        MPI_INT
    };
    displacements[0] = offsetof(RayMPI, origin);
    displacements[1] = offsetof(RayMPI, direction);
    displacements[2] = offsetof(RayMPI, momentum);
    displacements[3] = offsetof(RayMPI, velocity);
    displacements[4] = offsetof(RayMPI, energy);
    displacements[5] = offsetof(RayMPI, speciesMass);
    displacements[6] = offsetof(RayMPI, speciesDensity);
    displacements[7] = offsetof(RayMPI, speciesName);
    displacements[8] = offsetof(RayMPI, active);
    displacements[9] = offsetof(RayMPI, weight);
    displacements[10] = offsetof(RayMPI, panelId);
    MPI_Type_create_struct(11, blockLengths, displacements, types, &MPI_RayMPI_Type);
    MPI_Type_commit(&MPI_RayMPI_Type);

    // --- MPI Setup
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // --- Parse command line
    if (argc != 4) {
        if (rank == 0)
            std::cerr << "Usage: ./simulation <altitude> <angle_deg> <index>\n";
        MPI_Finalize();
        return 1;
    }

    std::string alt = argv[1];
    double AoA_deg = std::stod(argv[2]);
    int index = std::stoi(argv[3]);
    std::vector<std::string> values;
    double paddingFraction = 0.1;

    // --- Master rank loads atmospheric data and generates config
    if (rank == 0) {
        std::ifstream file("../assets/atmos_data/database_" + alt + "km.csv");
        if (!file) {
            std::cerr << "Failed to open atmospheric CSV.\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        std::string line;
        std::getline(file, line); // skip header
        for (int i = 0; i <= index; ++i) {
            if (!std::getline(file, line)) {
                std::cerr << "Failed to read line " << index << "\n";
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }

        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) values.push_back(cell);

        // Calculate direction vector from AoA
        double rad = AoA_deg * M_PI / 180.0;
        double dirX = sin(rad);
        double dirY = cos(rad);
        double dirZ = 0.0;

        // Update config.ini dynamically
        std::vector<std::string> species = {"N2", "O2", "O", "HE", "H", "AR", "N", "AO", "NO"};
        std::ifstream in("config.ini");
        std::ostringstream out;
        std::string cl, currentSpecies;
        bool inSpeciesBlock = false;

        while (std::getline(in, cl)) {
            bool replaced = false;

            if (cl.rfind("direction", 0) == 0)
                out << "direction = " << dirX << "," << dirY << "," << dirZ << "\n", replaced = true;
            else if (cl.rfind("temperature", 0) == 0)
                out << "temperature = " << values[10] << "\n", replaced = true;
            else if (cl.rfind("mass_density",0) == 0)
                out << "mass_density = " << values[0] << "\n", replaced = true;

            for (auto& sp : species) {
                if (cl == "[species:" + sp + "]") {
                    currentSpecies = sp;
                    inSpeciesBlock = true;
                    out << cl << "\n", replaced = true;
                    break;
                }
            }

            if (inSpeciesBlock && cl.rfind("density", 0) == 0) {
                auto it = std::find(species.begin(), species.end(), currentSpecies);
                if (it != species.end()) {
                    size_t idx = std::distance(species.begin(), it);
                    out << "density = " << values[idx + 1] << "\n", replaced = true;
                }
            } else if (inSpeciesBlock && cl.rfind("mass", 0) == 0) {
                out << cl << "\n", inSpeciesBlock = false, replaced = true;
            }

            if (!replaced) out << cl << "\n";
        }

        std::ofstream configOut("config.ini");
        configOut << out.str();
    }

    // --- All ranks wait for config.ini
    MPI_Barrier(MPI_COMM_WORLD);

    // --- Load configuration
    ConfigLoader loader;
    loader.loadFromFile("config.ini");
    auto cfg = loader.getConfig();

    // --- Load geometry
    MeshLoader mesh;
    mesh.load(cfg.geometryFile);
    const auto& vertices = mesh.getVertices();
    auto tris = mesh.getTriangles();
    for (size_t i = 0; i < tris.size(); ++i)
        tris[i].panelId = static_cast<int>(i);

    // --- Initialize simulation components
    SimulationController sim;
    IntersectionEngine engine;
    SurfaceInteractionModel model(cfg.reflectionRatio, cfg.absorptionRatio);
    sim.setIntersectionEngine(&engine);
    sim.setSurfaceModel(&model);
    sim.loadMesh(cfg.geometryFile);
    engine.setMesh(vertices, tris);

    // --- Distribute ray workload
    int totalRays = cfg.rayCount;
    int raysPerProc = totalRays / size;
    int remainder = totalRays % size;
    int myCount = raysPerProc + (rank < remainder ? 1 : 0);

    std::vector<RayMPI> flatRays;
    if (rank == 0) {
        sim.generateMixedRays(cfg, tris, vertices, paddingFraction, totalRays, totalRays);
        auto allRays = sim.getRays();
        flatRays.resize(totalRays);
        for (int i = 0; i < totalRays; ++i)
            flatRays[i] = toRayMPI(allRays[i]);
    }

    // --- Scatter rays across MPI ranks
    std::vector<int> sendCounts(size), displs(size);
    if (rank == 0) {
        int offset = 0;
        for (int i = 0; i < size; ++i) {
            sendCounts[i] = raysPerProc + (i < remainder ? 1 : 0);
            displs[i] = offset;
            offset += sendCounts[i];
        }
    }

    std::vector<RayMPI> myRayData(myCount);
    MPI_Scatterv(flatRays.data(), sendCounts.data(), displs.data(), MPI_RayMPI_Type,
                 myRayData.data(), myCount, MPI_RayMPI_Type, 0, MPI_COMM_WORLD);

    std::vector<Ray> myRays(myCount);
    for (int i = 0; i < myCount; ++i)
        myRays[i] = fromRayMPI(myRayData[i]);

    // --- Local simulation loop
    std::vector<DragForceCalculator> dragCalcs(omp_get_max_threads());
    std::vector<double> panelAreas(tris.size());
    for (size_t i = 0; i < tris.size(); ++i)
        panelAreas[i] = computeTriangleArea(vertices[tris[i].v1], vertices[tris[i].v2], vertices[tris[i].v3]);

    std::vector<int> rayHitCounts(myCount, 0);
    std::atomic<int> totalHits = 0, raysWithHits = 0, maxBounces = 0;

    #pragma omp parallel
    {
        std::vector<std::pair<Vector3, Vector3>> localSegments;

        #pragma omp for
        for (int i = 0; i < myCount; ++i) {
            int tid = omp_get_thread_num();
            Ray r = myRays[i];
            double remaining = r.energy;
            double minE = 0.1 * r.energy;
            int bounces = 0, hits = 0;

            Vector3 lastOrigin = r.origin;

            while (bounces < 10 && remaining > minE) {
                auto hit = engine.intersect(r);
                if (!hit) break;

                ++hits;
                Ray refl = model.generateReflection(cfg, r, *hit);
                double area = panelAreas[hit->panelId];
                dragCalcs[tid].accumulateForce(r, refl, area);

                localSegments.emplace_back(lastOrigin, hit->point);
                lastOrigin = refl.origin;

                r = refl;
                remaining = refl.energy * (1.0 - cfg.energyLoss);
                ++bounces;
            }

            rayHitCounts[i] = hits;
            if (hits > 0) ++raysWithHits;
            totalHits += hits;
            maxBounces = std::max(maxBounces.load(), hits);
        }

        #pragma omp critical
        raySegmentsDebug.insert(raySegmentsDebug.end(), localSegments.begin(), localSegments.end());
    }

    // --- Reduce results across ranks
    DragForceCalculator local;
    for (auto& dc : dragCalcs) local.merge(dc);

    Vector3 localF = local.getTotalDragForce(), totalF;
    MPI_Reduce(&localF, &totalF, 3, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    int localHitSum = std::accumulate(rayHitCounts.begin(), rayHitCounts.end(), 0);
    int globalHitSum = 0, globalHitRays = raysWithHits, globalMax = maxBounces;
    MPI_Reduce(&localHitSum, &globalHitSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&raysWithHits, &globalHitRays, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&maxBounces, &globalMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    double localRayMass = std::accumulate(myRays.begin(), myRays.end(), 0.0,
                                          [](double sum, const Ray& r) { return sum + r.weight; });
    double totalRayMass = 0.0;
    MPI_Reduce(&localRayMass, &totalRayMass, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        Vector3 flowDir = cfg.flowVelocity.normalize();
        double v = cfg.flowVelocity.norm();
        double dynP = std::stod(values.back());
        double A_ref = std::accumulate(panelAreas.begin(), panelAreas.end(), 0.0) / 2.0;

        double dragParallel = totalF.dot(flowDir);
        double cd_total = -dragParallel / (A_ref * dynP);

        std::cout << "\n[RESULT] Drag force: " << dragParallel << " N\n";
        std::cout << "[RESULT] Total C_d: " << cd_total << "\n";

        std::ostringstream fname;
        fname << "totalDragCoefficient_" << alt << "km_idx" << index << ".txt";
        std::ofstream outFile(fname.str());
        if (outFile) {
            outFile << cd_total << "\n";
            std::cout << "Saved to " << fname.str() << "\n";
        }

        std::cout << "\n[STATS] Total rays: " << totalRays
                  << "\nRays with hits: " << globalHitRays
                  << "\nTotal hits: " << globalHitSum
                  << "\nAvg. hits per ray: " << static_cast<double>(globalHitSum) / totalRays
                  << "\nMax bounces: " << globalMax << "\n";

        HeatmapExporter heat;
        heat.exportRaysAsVTK("ray_trace.vtk", raySegmentsDebug, vertices, tris, 1.0);
    }

    MPI_Type_free(&MPI_RayMPI_Type);
    MPI_Finalize();
    return 0;
}

