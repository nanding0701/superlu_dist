# Install script for directory: /ccs/home/nanding/myproject/superLU/superlu_dist/EXAMPLE

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive1")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive2")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive3")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive4")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive3d")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d_block_diag" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d_block_diag")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d_block_diag"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive3d_block_diag")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d_block_diag" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d_block_diag")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d_block_diag"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d_block_diag")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d1" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d1")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d1"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive3d1")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d1" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d1")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d1"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d1")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d2")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d2"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive3d2")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d2")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d2"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d2")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d3" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d3")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d3"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive3d3")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d3" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d3")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d3"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3d3")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_ABglobal")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_ABglobal"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive_ABglobal")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_ABglobal")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_ABglobal"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_ABglobal")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1_ABglobal")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1_ABglobal"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive1_ABglobal")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1_ABglobal")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1_ABglobal"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive1_ABglobal")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2_ABglobal")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2_ABglobal"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive2_ABglobal")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2_ABglobal")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2_ABglobal"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive2_ABglobal")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3_ABglobal")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3_ABglobal"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive3_ABglobal")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3_ABglobal")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3_ABglobal"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive3_ABglobal")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4_ABglobal")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4_ABglobal"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive4_ABglobal")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4_ABglobal" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4_ABglobal")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4_ABglobal"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive4_ABglobal")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_spawn" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_spawn")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_spawn"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/pddrive_spawn")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_spawn" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_spawn")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_spawn"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/pddrive_spawn")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive1" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive1")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive1"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive1")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive1" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive1")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive1"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive1")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive2")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive2"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive2")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive2")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive2"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive2")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive3")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive4" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive4")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive4"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive4")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive4" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive4")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive4"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive4")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive3d")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d1" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d1")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d1"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive3d1")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d1" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d1")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d1"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d1")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d2")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d2"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive3d2")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d2" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d2")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d2"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d2")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d3" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d3")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d3"
         RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE" TYPE EXECUTABLE FILES "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/EXAMPLE/psdrive3d3")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d3" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d3")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d3"
         OLD_RPATH "/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/ccs/home/nanding/myproject/superLU/superlu_dist/build+0518/lib:/ccs/home/nanding/mysoftware/parmetis-4.0.3/build/Linux-x86_64/libparmetis:/ccs/home/nanding/mysoftware/metis-5.1.0/build/Linux-x86_64/libmetis:/opt/rocm-5.2.0/lib:/opt/rocm-5.2.0/hip/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/EXAMPLE/psdrive3d3")
    endif()
  endif()
endif()

