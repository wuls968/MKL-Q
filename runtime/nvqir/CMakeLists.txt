# ============================================================================ #
# Copyright (c) 2022 - 2026 NVIDIA Corporation & Affiliates.                   #
# All rights reserved.                                                         #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

set(LIBRARY_NAME nvqir)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ctad-maybe-unsupported")

set(NVQIR_RUNTIME_SRC
  AnalysisScope.cpp
  QIRTypes.cpp
  NVQIR.cpp
  dem/DemScope.cpp
  resourcecounter/ResourceCounter.cpp
  ../cudaq/qis/state.cpp
)

add_library(${LIBRARY_NAME} SHARED ${NVQIR_RUNTIME_SRC})
target_include_directories(${LIBRARY_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/runtime> 
         $<INSTALL_INTERFACE:include> 
  PRIVATE .)

set_property(GLOBAL APPEND PROPERTY CUDAQ_RUNTIME_LIBS ${LIBRARY_NAME})

# The general build keeps the qpp backend as its private default dependency.
# The release wheel intentionally contains only the MKL-Q simulators and
# links the MKL-Q CPU plugin after it has been declared below.
target_link_libraries(${LIBRARY_NAME}
  PUBLIC cudaq-common cudaq-operator dl fmt::fmt-header-only
  PRIVATE cudaq-logger)
if(NOT CUDAQ_MKLQ_PACKAGE_ONLY)
  target_link_libraries(${LIBRARY_NAME} PRIVATE nvqir-qpp)
endif()

install(TARGETS ${LIBRARY_NAME} 
        EXPORT nvqir-targets 
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include/nvqir)

if (NOT CUDAQ_MKLQ_PACKAGE_ONLY)
  add_subdirectory(qpp)
endif()
if(CUDAQ_ENABLE_MKLQ_BACKEND AND CUDAQ_MKLQ_APPLE_SILICON_TARGET)
  add_subdirectory(mklq)
  if(CUDAQ_MKLQ_PACKAGE_ONLY)
    # Target plugins are loaded from the installed target configuration. Build
    # the default CPU plugin with nvqir, but do not make it part of nvqir's
    # exported link interface.
    add_dependencies(nvqir nvqir-mklq_cpu)
  endif()
endif()
if (NOT CUDAQ_MKLQ_PACKAGE_ONLY)
  add_subdirectory(stim)
endif()

if (NOT CUDAQ_MKLQ_PACKAGE_ONLY AND cuStateVec_FOUND)
  add_subdirectory(custatevec)
endif()
if (NOT CUDAQ_MKLQ_PACKAGE_ONLY AND cuTensorNet_FOUND)
  add_subdirectory(cutensornet)
endif()
if (NOT CUDAQ_MKLQ_PACKAGE_ONLY AND cuDensityMat_FOUND)
  add_subdirectory(cudensitymat)
endif()

install(EXPORT nvqir-targets
        FILE NVQIRTargets.cmake
        NAMESPACE nvqir::
        DESTINATION lib/cmake/nvqir)
