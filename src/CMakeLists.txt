add_executable(draft1 draft1.cpp)
add_executable(draft3 draft3.cpp)
add_executable(composition composition.cpp)
add_executable(soa_aos_abstraction soa_aos_abstraction.cpp)
target_include_directories(soa_aos_abstraction PRIVATE "../range-v3/include")

add_subdirectory ( prototype )
add_subdirectory ( prototype2 )
add_subdirectory ( type-erased-prototype )
