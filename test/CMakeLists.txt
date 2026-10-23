cmake_minimum_required(VERSION 3.5)
project(csms_cpp_test CXX)

add_executable(${PROJECT_NAME}
    test_main.cc
    ../models/Resident.cc
    ../models/ResidentValidator.cc
    ../models/ServiceRequest.cc
    ../models/ServiceRequestValidator.cc
    ../database/Database.cc
    ../repositories/ResidentRepository.cc
    ../repositories/ServiceRequestRepository.cc
    ../services/ResidentRegistrationService.cc
    ../services/ResidentSearchService.cc
    ../services/ResidentUpdateService.cc
    ../services/ResidentDeactivationService.cc
    ../services/ResidentServices.cc
    ../controllers/ResidentController.cc
    ../services/ServiceRequestSubmissionService.cc
    ../services/ServiceRequestStatusService.cc
    ../services/ServiceRequestQueryService.cc
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
# The Resident controller renders the CSP views, so the test executable needs
# the same generated view classes as the application.
drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/../views
                    ${CMAKE_CURRENT_BINARY_DIR})
