cmake_minimum_required(VERSION 3.14)
project(TestMain LANGUAGES C CXX)

# ✅ C++20 aktivieren
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 📦 Abhängigkeiten finden
find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)

# 📂 Globale Include-Verzeichnisse
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/tinyobjloader
    ${CMAKE_SOURCE_DIR}/external/inih
)

# ✅ Beispiel-Asset (ändern je nach Bedarf)
configure_file(${CMAKE_SOURCE_DIR}/assets/atmos_data/database_100km.csv database_100km.csv COPYONLY)

# 📦 INIH als statische Library bauen
add_library(inih STATIC
    external/inih/INIReader.cpp
    external/inih/ini.c
)

# 📦 GoogleTest über FetchContent laden
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

# ========== Tests ==========

add_executable(Vector3Tests test/test/test_Vector3.cpp)
target_link_libraries(Vector3Tests gtest_main)
add_test(NAME Vector3Test COMMAND Vector3Tests)

add_executable(IntersectionTests
    test/test/test_IntersectionEngine.cpp
    src/IntersectionEngine.cpp
    src/MeshLoader.cpp
    external/tinyobjloader/tiny_obj_loader.cc
)
target_link_libraries(IntersectionTests gtest_main)
add_test(NAME IntersectionTest COMMAND IntersectionTests)

add_executable(DragForceTests
    test/test/test_DragForceCalculator.cpp
    src/DragForceCalculator.cpp
)
target_link_libraries(DragForceTests gtest gtest_main)
add_test(NAME DragForceTest COMMAND DragForceTests)

add_executable(SurfaceInteractionTests
    test/test/test_SurfaceInteractionModel.cpp
    src/SurfaceInteractionModel.cpp
    src/ConfigLoader.cpp
)
target_link_libraries(SurfaceInteractionTests gtest gtest_main inih)
add_test(NAME SurfaceInteractionTest COMMAND SurfaceInteractionTests)

add_executable(MaxwellSamplerTests
    test/test/test_MaxwellSampler.cpp
    src/MaxwellSampler.cpp
)
target_link_libraries(MaxwellSamplerTests gtest gtest_main)
add_test(NAME MaxwellSamplerTest COMMAND MaxwellSamplerTests)

add_executable(ConfigLoaderTests
    test/test/test_ConfigLoader.cpp
    src/ConfigLoader.cpp
)
target_link_libraries(ConfigLoaderTests gtest gtest_main inih)
add_test(NAME ConfigLoaderTest COMMAND ConfigLoaderTests)

add_executable(MeshLoaderTests
    test/test/test_MeshLoader.cpp
    src/MeshLoader.cpp
    external/tinyobjloader/tiny_obj_loader.cc
)
target_link_libraries(MeshLoaderTests gtest_main)
add_test(NAME MeshLoaderTest COMMAND MeshLoaderTests)

add_executable(SimulationControllerTests
    src/SimulationController.cpp
    src/MeshLoader.cpp
    src/ConfigLoader.cpp
    src/DragForceCalculator.cpp
    src/IntersectionEngine.cpp
    src/SurfaceInteractionModel.cpp
    src/HeatmapExporter.cpp
    src/MaxwellSampler.cpp
    external/tinyobjloader/tiny_obj_loader.cc
    external/inih/INIReader.cpp
    external/inih/ini.c
    test/test/test_SimulationController.cpp
)
target_link_libraries(SimulationControllerTests
    PRIVATE gtest_main
)
add_test(NAME SimulationControllerTest COMMAND SimulationControllerTests)

# ========== Hauptprogramm ==========
add_executable(TestMain
    src/test_main.cpp
    src/ConfigLoader.cpp
    src/SimulationController.cpp
    src/SurfaceInteractionModel.cpp
    src/MaxwellSampler.cpp
    src/IntersectionEngine.cpp
    src/MeshLoader.cpp
    src/DragForceCalculator.cpp
    src/HeatmapExporter.cpp
    external/tinyobjloader/tiny_obj_loader.cc
    external/inih/INIReader.cpp
    external/inih/ini.c
)
target_link_libraries(TestMain
    PRIVATE inih OpenMP::OpenMP_CXX MPI::MPI_CXX
)

