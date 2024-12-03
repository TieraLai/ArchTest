

if(ENABLE_RT)
add_library(ECNic STATIC ecnic_rt.cpp)
else()
add_library(ECNic STATIC ecnic.cpp)
endif()

## libpci
# 查找 pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPCI REQUIRED libpci)
message("LIBPCI_INCLUDE_DIRS :" ${LIBPCI_INCLUDE_DIRS})
message("LIBPCI_LIBRARIES :" ${LIBPCI_LIBRARIES})

target_include_directories(ECNic PUBLIC 
    ${PROJECT_SOURCE_DIR}/CodeCommon
    ${LIBPCI_INCLUDE_DIRS}
    )
target_link_libraries(ECNic PUBLIC 
    ${LIBPCI_LIBRARIES}
    )