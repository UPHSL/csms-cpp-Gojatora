cmake_minimum_required(VERSION 3.5)
project(csms_cpp_test CXX)

add_executable(${PROJECT_NAME}
    test_main.cc
    ../models/Resident.cc
    ../models/ResidentValidator.cc
    ../database/Database.cc
    ../repositories/ResidentRepository.cc
    ../services/ResidentRegistrationService.cc
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../models)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon 
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

target_link_libraries(${PROJECT_NAME} PRIVATE SQLite3_lib)

ParseAndAddDrogonTests(${PROJECT_NAME})