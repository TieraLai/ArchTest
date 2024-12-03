if(ENABLE_RT)
add_library(ECM STATIC ecm_rt.cpp)
else()
add_library(ECM STATIC ecm.cpp)
endif()

target_link_libraries(ECM ECNic pthread)
target_include_directories(ECM PUBLIC ${PROJECT_SOURCE_DIR}/PF_NexRTOS)