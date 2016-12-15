if("${CMAKE_C_COMPILER_ID}" MATCHES "Intel")
  list(APPEND SIMINT_C_FLAGS "-xCORE-AVX512")
  list(APPEND SIMINT_TESTS_CXX_FLAGS "-xCORE-AVX512")
elseif("${CMAKE_C_COMPILER_ID}" MATCHES "GNU" OR
       "${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
  list(APPEND SIMINT_C_FLAGS "-mavx512f;-mavx512pf;-mavx512er;-mavx512cd")
  list(APPEND SIMINT_TESTS_CXX_FLAGS "-mavx512f;-mavx512pf;-mavx512er;-mavx512cd")
else()
  message(FATAL_ERROR "Unsupported compiler")
endif()
