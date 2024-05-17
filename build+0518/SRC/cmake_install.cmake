# Install script for directory: /ccs/home/nanding/myproject/superLU/superlu_dist/SRC

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE STATIC_LIBRARY FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/SRC/libsuperlu_dist.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/superlu_FCnames.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/dcomplex.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/machines.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/psymbfact.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/superlu_defs.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/superlu_enum_consts.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/supermatrix.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/util_dist.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/gpu_api_utils.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/gpu_wrapper.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/superlu_upacked.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/superlu_dist_config.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/superlu_FortranCInterface.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/oneside.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/lupanels.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/commWrapper.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/lupanels_GPU.cuh"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/batch_block_copy.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/anc25d-GPU_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/anc25d.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/anc25d_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/lupanelsComm3dGPU_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/lupanels_GPU_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/lupanels_comm3d_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/cublas_cusolver_wrappers.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/lupanels_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/dAncestorFactor_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/dsparseTreeFactorGPU_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/sparseTreeFactor_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/superlu_blas.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/l_panels_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/u_panels_impl.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/luAuxStructTemplated.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/xlupanels.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/lu_common.hpp"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/xlupanels_GPU.cuh"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/schurCompUpdate_impl.cuh"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/batch_factorize.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/CplusplusFactor/batch_factorize_marshall.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/superlu_ddefs.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/dlustruct_gpu.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/superlu_sdefs.h"
    "/ccs/home/nanding/myproject/superLU/superlu_dist/SRC/include/slustruct_gpu.h"
    )
endif()

