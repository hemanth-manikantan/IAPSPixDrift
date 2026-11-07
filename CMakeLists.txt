# Use the code in this section for standalone projects
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(uRwell)
if(NOT TARGET Garfield::Garfield)
  find_package(Garfield REQUIRED)
endif()

# ---Build executable------------------------------------------------------------
add_executable(iapspix_driftfield iapspix_driftfield.cpp)
target_link_libraries(iapspix_driftfield Garfield::Garfield)

# ---Copy all files locally to the build directory-------------------------------
foreach(_file IonMobility_Ar+_Ar.txt ar_80_dme_20_bis.gas)
  configure_file(${_file} ${CMAKE_CURRENT_BINARY_DIR}/${_file} COPYONLY)
endforeach()
