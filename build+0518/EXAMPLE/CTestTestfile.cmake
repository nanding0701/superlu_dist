# CMake generated Testfile for 
# Source directory: /ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE
# Build directory: /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(pddrive1 "/usr/bin/srun" "-n" "4" "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive1" "-r" "2" "-c" "2" "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/big.rua")
set_tests_properties(pddrive1 PROPERTIES  _BACKTRACE_TRIPLES "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;21;add_test;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;46;add_superlu_dist_example;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;0;")
add_test(pddrive2 "/usr/bin/srun" "-n" "4" "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive2" "-r" "2" "-c" "2" "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/big.rua")
set_tests_properties(pddrive2 PROPERTIES  _BACKTRACE_TRIPLES "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;21;add_test;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;52;add_superlu_dist_example;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;0;")
add_test(pddrive3 "/usr/bin/srun" "-n" "4" "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive3" "-r" "2" "-c" "2" "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/big.rua")
set_tests_properties(pddrive3 PROPERTIES  _BACKTRACE_TRIPLES "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;21;add_test;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;58;add_superlu_dist_example;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;0;")
add_test(psdrive1 "/usr/bin/srun" "-n" "4" "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive1" "-r" "2" "-c" "2" "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/big.rua")
set_tests_properties(psdrive1 PROPERTIES  _BACKTRACE_TRIPLES "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;21;add_test;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;132;add_superlu_dist_example;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;0;")
add_test(psdrive2 "/usr/bin/srun" "-n" "4" "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive2" "-r" "2" "-c" "2" "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/big.rua")
set_tests_properties(psdrive2 PROPERTIES  _BACKTRACE_TRIPLES "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;21;add_test;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;138;add_superlu_dist_example;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;0;")
add_test(psdrive3 "/usr/bin/srun" "-n" "4" "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive3" "-r" "2" "-c" "2" "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/big.rua")
set_tests_properties(psdrive3 PROPERTIES  _BACKTRACE_TRIPLES "/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;21;add_test;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;144;add_superlu_dist_example;/ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE/CMakeLists.txt;0;")
