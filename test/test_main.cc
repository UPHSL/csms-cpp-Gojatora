#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include "../models/Resident.h"

// Test 1: Resident Creation
// Verifies that a Resident can be created using valid Resident information.
DROGON_TEST(ResidentCreationTest)
{
    Resident resident(1,
                       "Adrian Paolo",
                       "Follante",
                       "Kinnari Phase 1, Lantic, Carmona, Cavite",
                       "09763214551",
                       "adrian.paolo@example.com",
                       ResidentStatus::Active);

    CHECK(resident.getId() == 1);
    CHECK(resident.getFirstName() == "Adrian Paolo");
    CHECK(resident.getLastName() == "Follante");
}

// Test 2: Resident Information Access
// Verifies that Resident information can be assigned and retrieved correctly.
DROGON_TEST(ResidentInformationAccessTest)
{
    Resident resident;

    resident.setId(2);
    resident.setFirstName("Wilmar");
    resident.setLastName("Lipata");
    resident.setAddress("Kinnari Phase 2, Lantic, Carmona, Cavite");
    resident.setContactNumber("09179876543");
    resident.setEmail("wilmar.lipata@example.com");
    resident.setStatus(ResidentStatus::Active);

    CHECK(resident.getId() == 2);
    CHECK(resident.getFirstName() == "Wilmar");
    CHECK(resident.getLastName() == "Lipata");
    CHECK(resident.getAddress() == "Kinnari Phase 2, Lantic, Carmona, Cavite");
    CHECK(resident.getContactNumber() == "09179876543");
    CHECK(resident.getEmail() == "wilmar.lipata@example.com");
}

// Test 3: Resident Status
// Verifies that the Resident model can represent the "Active" status.
DROGON_TEST(ResidentStatusTest)
{
    Resident resident;
    resident.setStatus(ResidentStatus::Active);

    CHECK(resident.getStatus() == ResidentStatus::Active);
    CHECK(residentStatusToString(resident.getStatus()) == "Active");
}

// Keeping the original starter test so existing behavior is preserved.
DROGON_TEST(BasicTest)
{
    // Add your tests here
}

int main(int argc, char** argv) 
{
    using namespace drogon;

    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    // Start the main loop on another thread
    std::thread thr([&]() {
        // Queues the promise to be fulfilled after starting the loop
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    // The future is only satisfied after the event loop started
    f1.get();
    int status = test::run(argc, argv);

    // Ask the event loop to shutdown and wait
    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}