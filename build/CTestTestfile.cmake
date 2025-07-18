# CMake generated Testfile for 
# Source directory: /home/jovan/Software/Raytracer
# Build directory: /home/jovan/Software/Raytracer/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(Vector3Test "/home/jovan/Software/Raytracer/build/Vector3Tests")
set_tests_properties(Vector3Test PROPERTIES  _BACKTRACE_TRIPLES "/home/jovan/Software/Raytracer/CMakeLists.txt;43;add_test;/home/jovan/Software/Raytracer/CMakeLists.txt;0;")
add_test(IntersectionTest "/home/jovan/Software/Raytracer/build/IntersectionTests")
set_tests_properties(IntersectionTest PROPERTIES  _BACKTRACE_TRIPLES "/home/jovan/Software/Raytracer/CMakeLists.txt;52;add_test;/home/jovan/Software/Raytracer/CMakeLists.txt;0;")
add_test(DragForceTest "/home/jovan/Software/Raytracer/build/DragForceTests")
set_tests_properties(DragForceTest PROPERTIES  _BACKTRACE_TRIPLES "/home/jovan/Software/Raytracer/CMakeLists.txt;59;add_test;/home/jovan/Software/Raytracer/CMakeLists.txt;0;")
add_test(SurfaceInteractionTest "/home/jovan/Software/Raytracer/build/SurfaceInteractionTests")
set_tests_properties(SurfaceInteractionTest PROPERTIES  _BACKTRACE_TRIPLES "/home/jovan/Software/Raytracer/CMakeLists.txt;67;add_test;/home/jovan/Software/Raytracer/CMakeLists.txt;0;")
add_test(MaxwellSamplerTest "/home/jovan/Software/Raytracer/build/MaxwellSamplerTests")
set_tests_properties(MaxwellSamplerTest PROPERTIES  _BACKTRACE_TRIPLES "/home/jovan/Software/Raytracer/CMakeLists.txt;74;add_test;/home/jovan/Software/Raytracer/CMakeLists.txt;0;")
add_test(ConfigLoaderTest "/home/jovan/Software/Raytracer/build/ConfigLoaderTests")
set_tests_properties(ConfigLoaderTest PROPERTIES  _BACKTRACE_TRIPLES "/home/jovan/Software/Raytracer/CMakeLists.txt;81;add_test;/home/jovan/Software/Raytracer/CMakeLists.txt;0;")
add_test(MeshLoaderTest "/home/jovan/Software/Raytracer/build/MeshLoaderTests")
set_tests_properties(MeshLoaderTest PROPERTIES  _BACKTRACE_TRIPLES "/home/jovan/Software/Raytracer/CMakeLists.txt;89;add_test;/home/jovan/Software/Raytracer/CMakeLists.txt;0;")
add_test(SimulationControllerTest "/home/jovan/Software/Raytracer/build/SimulationControllerTests")
set_tests_properties(SimulationControllerTest PROPERTIES  _BACKTRACE_TRIPLES "/home/jovan/Software/Raytracer/CMakeLists.txt;108;add_test;/home/jovan/Software/Raytracer/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
