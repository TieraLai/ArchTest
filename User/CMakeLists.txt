add_executable(User user.cpp)
target_link_libraries(User ECM)
target_include_directories(User PUBLIC ${PROJECT_SOURCE_DIR}/Ecm)

