# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.23

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /autofs/nccs-svm1_sw/frontier/spack-envs/base/opt/linux-sles15-x86_64/gcc-7.5.0/cmake-3.23.2-4r4mpiba7cwdw2hlakh5i7tchi64s3qd/bin/cmake

# The command to remove a file.
RM = /autofs/nccs-svm1_sw/frontier/spack-envs/base/opt/linux-sles15-x86_64/gcc-7.5.0/cmake-3.23.2-4r4mpiba7cwdw2hlakh5i7tchi64s3qd/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /ccs/home/nanding/myproject/superLU/superlu_dist

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518

# Include any dependencies generated for this target.
include FORTRAN/CMakeFiles/f_5x5.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include FORTRAN/CMakeFiles/f_5x5.dir/compiler_depend.make

# Include the progress variables for this target.
include FORTRAN/CMakeFiles/f_5x5.dir/progress.make

# Include the compile flags for this target's objects.
include FORTRAN/CMakeFiles/f_5x5.dir/flags.make

FORTRAN/CMakeFiles/f_5x5.dir/f_5x5.F90.o: FORTRAN/CMakeFiles/f_5x5.dir/flags.make
FORTRAN/CMakeFiles/f_5x5.dir/f_5x5.F90.o: ../FORTRAN/f_5x5.F90
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building Fortran object FORTRAN/CMakeFiles/f_5x5.dir/f_5x5.F90.o"
	cd /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN && /opt/cray/pe/craype/2.7.19/bin/ftn $(Fortran_DEFINES) $(Fortran_INCLUDES) $(Fortran_FLAGS) -ffree-form -cpp -c /ccs/home/nanding/myproject/superLU/superlu_dist/FORTRAN/f_5x5.F90 -o CMakeFiles/f_5x5.dir/f_5x5.F90.o

FORTRAN/CMakeFiles/f_5x5.dir/f_5x5.F90.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing Fortran source to CMakeFiles/f_5x5.dir/f_5x5.F90.i"
	cd /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN && /opt/cray/pe/craype/2.7.19/bin/ftn $(Fortran_DEFINES) $(Fortran_INCLUDES) $(Fortran_FLAGS) -ffree-form -cpp -E /ccs/home/nanding/myproject/superLU/superlu_dist/FORTRAN/f_5x5.F90 > CMakeFiles/f_5x5.dir/f_5x5.F90.i

FORTRAN/CMakeFiles/f_5x5.dir/f_5x5.F90.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling Fortran source to assembly CMakeFiles/f_5x5.dir/f_5x5.F90.s"
	cd /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN && /opt/cray/pe/craype/2.7.19/bin/ftn $(Fortran_DEFINES) $(Fortran_INCLUDES) $(Fortran_FLAGS) -ffree-form -cpp -S /ccs/home/nanding/myproject/superLU/superlu_dist/FORTRAN/f_5x5.F90 -o CMakeFiles/f_5x5.dir/f_5x5.F90.s

FORTRAN/CMakeFiles/f_5x5.dir/sp_ienv.c.o: FORTRAN/CMakeFiles/f_5x5.dir/flags.make
FORTRAN/CMakeFiles/f_5x5.dir/sp_ienv.c.o: ../FORTRAN/sp_ienv.c
FORTRAN/CMakeFiles/f_5x5.dir/sp_ienv.c.o: FORTRAN/CMakeFiles/f_5x5.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object FORTRAN/CMakeFiles/f_5x5.dir/sp_ienv.c.o"
	cd /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN && /opt/cray/pe/craype/2.7.19/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT FORTRAN/CMakeFiles/f_5x5.dir/sp_ienv.c.o -MF CMakeFiles/f_5x5.dir/sp_ienv.c.o.d -o CMakeFiles/f_5x5.dir/sp_ienv.c.o -c /ccs/home/nanding/myproject/superLU/superlu_dist/FORTRAN/sp_ienv.c

FORTRAN/CMakeFiles/f_5x5.dir/sp_ienv.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/f_5x5.dir/sp_ienv.c.i"
	cd /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN && /opt/cray/pe/craype/2.7.19/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /ccs/home/nanding/myproject/superLU/superlu_dist/FORTRAN/sp_ienv.c > CMakeFiles/f_5x5.dir/sp_ienv.c.i

FORTRAN/CMakeFiles/f_5x5.dir/sp_ienv.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/f_5x5.dir/sp_ienv.c.s"
	cd /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN && /opt/cray/pe/craype/2.7.19/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /ccs/home/nanding/myproject/superLU/superlu_dist/FORTRAN/sp_ienv.c -o CMakeFiles/f_5x5.dir/sp_ienv.c.s

# Object files for target f_5x5
f_5x5_OBJECTS = \
"CMakeFiles/f_5x5.dir/f_5x5.F90.o" \
"CMakeFiles/f_5x5.dir/sp_ienv.c.o"

# External object files for target f_5x5
f_5x5_EXTERNAL_OBJECTS =

FORTRAN/f_5x5: FORTRAN/CMakeFiles/f_5x5.dir/f_5x5.F90.o
FORTRAN/f_5x5: FORTRAN/CMakeFiles/f_5x5.dir/sp_ienv.c.o
FORTRAN/f_5x5: FORTRAN/CMakeFiles/f_5x5.dir/build.make
FORTRAN/f_5x5: FORTRAN/libsuperlu_dist_fortran.a
FORTRAN/f_5x5: /opt/cray/pe/libsci/22.12.1.1/GNU/9.1/x86_64/lib/libsci_gnu_82_mp.so
FORTRAN/f_5x5: SRC/libsuperlu_dist.a
FORTRAN/f_5x5: /ccs/home/nanding/mysoftware/parmetis-4.0.3//build/Linux-x86_64/libparmetis/libparmetis.so
FORTRAN/f_5x5: /ccs/home/nanding/mysoftware/metis-5.1.0//build/Linux-x86_64/libmetis/libmetis.so
FORTRAN/f_5x5: /opt/rocm-5.2.0/lib/libroctx64.so
FORTRAN/f_5x5: /opt/rocm-5.2.0/lib/libroctracer64.so
FORTRAN/f_5x5: /opt/rocm-5.2.0/lib/libhipblas.so.0.1.50200
FORTRAN/f_5x5: /opt/rocm-5.2.0/lib/librocsolver.so.0.1.50200
FORTRAN/f_5x5: /opt/rocm-5.2.0/lib/librocblas.so.0.1.50200
FORTRAN/f_5x5: /opt/rocm-5.2.0/hip/lib/libamdhip64.so.5.2.50200
FORTRAN/f_5x5: /opt/rocm-5.2.0/llvm/lib/clang/14.0.0/lib/linux/libclang_rt.builtins-x86_64.a
FORTRAN/f_5x5: /opt/cray/pe/libsci/22.12.1.1/GNU/9.1/x86_64/lib/libsci_gnu_82_mpi_mp.so
FORTRAN/f_5x5: /opt/cray/pe/libsci/22.12.1.1/GNU/9.1/x86_64/lib/libsci_gnu_82_mp.so
FORTRAN/f_5x5: /opt/cray/pe/gcc/11.2.0/snos/lib64/libgomp.so
FORTRAN/f_5x5: FORTRAN/CMakeFiles/f_5x5.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking Fortran executable f_5x5"
	cd /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/f_5x5.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
FORTRAN/CMakeFiles/f_5x5.dir/build: FORTRAN/f_5x5
.PHONY : FORTRAN/CMakeFiles/f_5x5.dir/build

FORTRAN/CMakeFiles/f_5x5.dir/clean:
	cd /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN && $(CMAKE_COMMAND) -P CMakeFiles/f_5x5.dir/cmake_clean.cmake
.PHONY : FORTRAN/CMakeFiles/f_5x5.dir/clean

FORTRAN/CMakeFiles/f_5x5.dir/depend:
	cd /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /ccs/home/nanding/myproject/superLU/superlu_dist /ccs/home/nanding/myproject/superLU/superlu_dist/FORTRAN /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518 /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN /ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/FORTRAN/CMakeFiles/f_5x5.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : FORTRAN/CMakeFiles/f_5x5.dir/depend

