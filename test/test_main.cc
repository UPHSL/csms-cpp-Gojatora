#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>

#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../models/ServiceRequest.h"
#include "../models/ServiceRequestValidator.h"

#include "../database/Database.h"

#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"

#include "../services/ResidentRegistrationService.h"
#include "../services/ResidentSearchService.h"
#include "../services/ResidentUpdateService.h"
#include "../services/ResidentDeactivationService.h"
#include "../services/ServiceRequestSubmissionService.h"
#include "../services/ServiceRequestStatusService.h"
#include "../services/ServiceRequestQueryService.h"
#include "../services/ResidentServices.h"

#include "../controllers/ResidentController.h"

#include <algorithm>
#include <map>
#include <sqlite3.h>
#include <filesystem>
#include <cstdio>
#include <set>
#include <vector>

// Creates a fresh, uniquely-named temporary SQLite file path for a test.
// Using a unique name per test (via a counter) avoids tests interfering
// with each other, and avoids leftover data from a previous test run.
std::string makeTempDbPath(const std::string &testName)
{
    static int counter = 0;
    counter++;

    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    std::filesystem::path dbPath = tempDir / ("csms_test_" + testName + "_" + std::to_string(counter) + ".sqlite3");

    // Remove any leftover file from a previous run, if it exists,
    // so each test starts from a clean, empty database.
    if (std::filesystem::exists(dbPath))
    {
        std::filesystem::remove(dbPath);
    }

    return dbPath.string();
}

// T01 Required Test 1: Resident Creation
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

// T01 Required Test 2: Resident Information Access
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

// T01 Required Test 3: Resident Status
// Verifies that the Resident model can represent the "Active" status.
DROGON_TEST(ResidentStatusTest)
{
    Resident resident;
    resident.setStatus(ResidentStatus::Active);

    CHECK(resident.getStatus() == ResidentStatus::Active);
    CHECK(residentStatusToString(resident.getStatus()) == "Active");
}

// T02 Required Test 1: Valid Resident information passes validation
DROGON_TEST(ValidResidentInformationPassesValidationTest)
{
    Resident resident(1,
                       "Adrian Paolo",
                       "Follante",
                       "Barangay Lantic, Carmona, Cavite",
                       "09763214551",
                       "adrian.paolo@example.com",
                       ResidentStatus::Active);

    ResidentValidator validator;
    CHECK(validator.isValid(resident));
}

// T02 Required Test 2: Missing first name fails validation
DROGON_TEST(MissingFirstNameFailsValidationTest)
{
    Resident resident(1,
                       "",
                       "Follante",
                       "Barangay Lantic, Carmona, Cavite",
                       "09763214551",
                       "adrian.paolo@example.com",
                       ResidentStatus::Active);

    ResidentValidator validator;
    auto errors = validator.validate(resident);

    CHECK(!validator.isValid(resident));
    CHECK(std::find(errors.begin(), errors.end(), "firstName") != errors.end());
}

// T02 Required Test 3: Missing last name fails validation
DROGON_TEST(MissingLastNameFailsValidationTest)
{
    Resident resident(1,
                       "Adrian Paolo",
                       "",
                       "Barangay Lantic, Carmona, Cavite",
                       "09763214551",
                       "adrian.paolo@example.com",
                       ResidentStatus::Active);

    ResidentValidator validator;
    auto errors = validator.validate(resident);

    CHECK(!validator.isValid(resident));
    CHECK(std::find(errors.begin(), errors.end(), "lastName") != errors.end());
}

// T02 Required Test 4: Missing address fails validation
DROGON_TEST(MissingAddressFailsValidationTest)
{
    Resident resident(1,
                       "Adrian Paolo",
                       "Follante",
                       "",
                       "09763214551",
                       "adrian.paolo@example.com",
                       ResidentStatus::Active);

    ResidentValidator validator;
    auto errors = validator.validate(resident);

    CHECK(!validator.isValid(resident));
    CHECK(std::find(errors.begin(), errors.end(), "address") != errors.end());
}

// T02 Required Test 5: Whitespace-only required information fails validation
DROGON_TEST(WhitespaceOnlyRequiredInformationFailsValidationTest)
{
    Resident resident(1,
                       "   ",
                       "Follante",
                       "Barangay Lantic, Carmona, Cavite",
                       "09763214551",
                       "adrian.paolo@example.com",
                       ResidentStatus::Active);

    ResidentValidator validator;
    auto errors = validator.validate(resident);

    CHECK(!validator.isValid(resident));
    CHECK(std::find(errors.begin(), errors.end(), "firstName") != errors.end());
}

// T02 Required Test 6: Invalid contact number fails validation
DROGON_TEST(InvalidContactNumberFailsValidationTest)
{
    Resident resident(1,
                       "Adrian Paolo",
                       "Follante",
                       "Barangay Lantic, Carmona, Cavite",
                       "0976ACE4551",
                       "adrian.paolo@example.com",
                       ResidentStatus::Active);

    ResidentValidator validator;
    auto errors = validator.validate(resident);

    CHECK(!validator.isValid(resident));
    CHECK(std::find(errors.begin(), errors.end(), "contactNumber") != errors.end());
}

// T02 Required Test 7: Invalid email fails validation
DROGON_TEST(InvalidEmailFailsValidationTest)
{
    Resident resident(1,
                       "Adrian Paolo",
                       "Follante",
                       "Barangay Lantic, Carmona, Cavite",
                       "09763214551",
                       "adrian.paolo.example.com",
                       ResidentStatus::Active);

    ResidentValidator validator;
    auto errors = validator.validate(resident);

    CHECK(!validator.isValid(resident));
    CHECK(std::find(errors.begin(), errors.end(), "email") != errors.end());
}

// T02 Required Test 8: Supported statuses (Active and Inactive) pass validation
DROGON_TEST(SupportedResidentStatusesPassValidationTest)
{
    Resident activeResident(1,
                             "Adrian Paolo",
                             "Follante",
                             "Barangay Lantic, Carmona, Cavite",
                             "09763214551",
                             "adrian.paolo@example.com",
                             ResidentStatus::Active);

    Resident inactiveResident(2,
                               "Wilmar",
                               "Lipata",
                               "Barangay 1, Carmona, Cavite",
                               "09181234567",
                               "wilmar.lipata@example.com",
                               ResidentStatus::Inactive);

    ResidentValidator validator;

    CHECK(validator.isValid(activeResident));
    CHECK(validator.isValid(inactiveResident));
}

// T02 Required Test 9: Unsupported status fails validation
// Since ResidentStatus is an enum class, we simulate an invalid/unexpected
// value using static_cast — similar to how corrupted or unexpected data
// (e.g. from a database) might arrive in a real system.
DROGON_TEST(UnsupportedResidentStatusFailsValidationTest)
{
    ResidentStatus invalidStatus = static_cast<ResidentStatus>(99);

    Resident resident(1,
                       "Adrian Paolo",
                       "Follante",
                       "Barangay Lantic, Carmona, Cavite",
                       "09763214551",
                       "adrian.paolo@example.com",
                       invalidStatus);

    ResidentValidator validator;
    auto errors = validator.validate(resident);

    CHECK(!validator.isValid(resident));
    CHECK(std::find(errors.begin(), errors.end(), "status") != errors.end());
}

// Regression test (not part of T03's required scenarios): confirms the
// Resident model still creates residents with no id (std::nullopt) before
// persistence, after the id_ type was changed from int to std::optional<int>.
DROGON_TEST(NewResidentHasNoIdBeforePersistenceTest)
{
    Resident resident("Adrian Paolo",
                       "Follante",
                       "Barangay Lantic, Carmona, Cavite",
                       "09763214551",
                       "adrian.paolo@example.com",
                       ResidentStatus::Active);

    CHECK(!resident.getId().has_value());
}

// T03 Required Test 1: Persist a Resident
DROGON_TEST(PersistAResidentTest)
{
    Database db(makeTempDbPath("persist"));
    ResidentRepository repository(db);

    Resident newResident("Adrian Paolo",
                          "Follante",
                          "Barangay Lantic, Carmona, Cavite",
                          "09763214551",
                          "adrian.paolo@example.com",
                          ResidentStatus::Active);

    Resident saved = repository.save(newResident);

    // If save() completed without throwing, and returned a Resident, we succeeded.
    CHECK(saved.getFirstName() == "Adrian Paolo");
}

// T03 Required Test 2: Resident receives an identifier
DROGON_TEST(ResidentReceivesAnIdentifierTest)
{
    Database db(makeTempDbPath("receives_id"));
    ResidentRepository repository(db);

    Resident newResident("Wilmar",
                          "Lipata",
                          "Barangay 1 Carmona, Cavite",
                          "09181234567",
                          "wilmar.lipata@example.com",
                          ResidentStatus::Active);

    // Before persistence, the T01 behavior applies: no id yet.
    CHECK(!newResident.getId().has_value());

    Resident saved = repository.save(newResident);

    // After persistence, SQLite must have assigned a real, usable id.
    CHECK(saved.getId().has_value());
}

// T03 Required Test 3: Retrieve Resident by identifier
DROGON_TEST(RetrieveResidentByIdentifierTest)
{
    Database db(makeTempDbPath("retrieve_by_id"));
    ResidentRepository repository(db);

    Resident newResident("Aaron",
                          "Cuartero",
                          "Barangay 5 Binan, Laguna",
                          "09191234567",
                          "aaron@example.com",
                          ResidentStatus::Active);

    Resident saved = repository.save(newResident);
    int savedId = saved.getId().value();

    std::optional<Resident> found = repository.findById(savedId);

    CHECK(found.has_value());
    CHECK(found->getId().value() == savedId);
    CHECK(found->getFirstName() == "Aaron");
}

// T03 Required Test 4: Resident information is preserved
DROGON_TEST(ResidentInformationIsPreservedTest)
{
    Database db(makeTempDbPath("info_preserved"));
    ResidentRepository repository(db);

    Resident newResident("Dominie",
                          "Cruz",
                          "Barangay Bancal, Carmona, Cavite",
                          "09201234567",
                          "dominie@example.com",
                          ResidentStatus::Active);

    Resident saved = repository.save(newResident);
    std::optional<Resident> found = repository.findById(saved.getId().value());

    CHECK(found.has_value());
    CHECK(found->getFirstName() == "Dominie");
    CHECK(found->getLastName() == "Cruz");
    CHECK(found->getAddress() == "Barangay Bancal, Carmona, Cavite");
    CHECK(found->getContactNumber() == "09201234567"); // leading zero preserved
    CHECK(found->getEmail() == "dominie@example.com");
}

// T03 Required Test 5: Active status is preserved
DROGON_TEST(ActiveStatusIsPreservedTest)
{
    Database db(makeTempDbPath("status_preserved"));
    ResidentRepository repository(db);

    Resident newResident("Carlo",
                          "Villanueva",
                          "Barangay San Isidro, Carmona, Cavite",
                          "09211234567",
                          "carlo@example.com",
                          ResidentStatus::Active);

    Resident saved = repository.save(newResident);
    std::optional<Resident> found = repository.findById(saved.getId().value());

    CHECK(found.has_value());
    CHECK(found->getStatus() == ResidentStatus::Active);
}

// T03 Required Test 6: Missing Resident is handled safely
DROGON_TEST(MissingResidentIsHandledTest)
{
    Database db(makeTempDbPath("missing_resident"));
    ResidentRepository repository(db);

    std::optional<Resident> found = repository.findById(999999);

    CHECK(!found.has_value());
}

// T03 Required Test 7: Persistence is not limited to one repository object
DROGON_TEST(PersistenceSurvivesNewRepositoryInstanceTest)
{
    std::string dbPath = makeTempDbPath("shared_file");

    int savedId;
    {
        // First Database/repository instance: save a Resident, then let
        // this scope end (simulating "finished using the first repository").
        Database firstDb(dbPath);
        ResidentRepository firstRepository(firstDb);

        Resident newResident("Liza",
                              "Fernandez",
                              "Barangay Balibago, Carmona, Cavite",
                              "09221234567",
                              "liza@example.com",
                              ResidentStatus::Active);

        Resident saved = firstRepository.save(newResident);
        savedId = saved.getId().value();
    }

    // Second, completely separate Database/repository instance,
    // connected to the SAME file on disk.
    Database secondDb(dbPath);
    ResidentRepository secondRepository(secondDb);

    std::optional<Resident> found = secondRepository.findById(savedId);

    // If this passes, the data truly lives in the SQLite file,
    // not just inside one in-memory C++ object.
    CHECK(found.has_value());
    CHECK(found->getFirstName() == "Liza");
}

// Student-designed test: Contact number's leading zero survives a full
// save + retrieve round trip through SQLite.
//
// Why this scenario: contact_number is stored as TEXT specifically to
// preserve the leading "0" in numbers like "09171234567". If a future
// change accidentally stored it as an INTEGER column, or someone
// mistakenly used sqlite3_bind_int instead of sqlite3_bind_text, SQLite
// would silently strip the leading zero (storing it as 9171234567).
// This test exists specifically to catch that class of regression,
// which would otherwise be easy to miss since the value would still
// "look like a number" and other fields would be unaffected.
DROGON_TEST(ContactNumberLeadingZeroSurvivesRoundTripTest)
{
    Database db(makeTempDbPath("leading_zero"));
    ResidentRepository repository(db);

    Resident newResident("Klaire",
                          "Torres",
                          "Barangay Langkaan I, Dasmarinas, Cavite",
                          "09001234567",
                          "klaire@example.com",
                          ResidentStatus::Active);

    Resident saved = repository.save(newResident);
    std::optional<Resident> found = repository.findById(saved.getId().value());

    CHECK(found.has_value());
    CHECK(found->getContactNumber() == "09001234567");
    CHECK(found->getContactNumber().length() == 11);
    CHECK(found->getContactNumber()[0] == '0');
}

// valid-Resident helper
Resident makeValidResidentForRegistration()
{
    return Resident("Juan",
                     "Dela Cruz",
                     "Barangay Santo Tomas",
                     "09171234567",
                     "juan@example.com",
                     ResidentStatus::Active);
}

// Directly queries SQLite to count Resident rows, bypassing our own
// repository — used to prove invalid Residents never reach the database,
// rather than just trusting the registration result's return value.
int countResidentsInDatabase(const std::string &databasePath)
{
    sqlite3 *database = nullptr;
    sqlite3_open(databasePath.c_str(), &database);

    const char *sql = "SELECT COUNT(*) FROM residents";
    sqlite3_stmt *statement = nullptr;
    sqlite3_prepare_v2(database, sql, -1, &statement, nullptr);
    sqlite3_step(statement);

    int count = sqlite3_column_int(statement, 0);

    sqlite3_finalize(statement);
    sqlite3_close(database);

    return count;
}

// T04 Required Test 1: Register a valid Resident
DROGON_TEST(RegisterValidResidentTest)
{
    Database db(makeTempDbPath("register_valid"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentRegistrationService service(validator, repository);

    Resident resident = makeValidResidentForRegistration();

    ResidentRegistrationResult result = service.registerResident(resident);

    CHECK(result.success);
    CHECK(result.resident.has_value());
    CHECK(result.errors.empty());
}

// T04 Required Test 2: Registered Resident receives an identifier
DROGON_TEST(RegisteredResidentReceivesIdentifierTest)
{
    Database db(makeTempDbPath("register_id"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentRegistrationService service(validator, repository);

    Resident resident = makeValidResidentForRegistration();
    CHECK(!resident.getId().has_value());

    ResidentRegistrationResult result = service.registerResident(resident);

    CHECK(result.success);
    CHECK(result.resident->getId().has_value());
}

// T04 Required Test 3: Registered Resident is actually persisted
DROGON_TEST(RegisteredResidentIsPersistedTest)
{
    Database db(makeTempDbPath("register_persisted"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentRegistrationService service(validator, repository);

    Resident resident = makeValidResidentForRegistration();
    ResidentRegistrationResult result = service.registerResident(resident);

    CHECK(result.success);
    int residentId = result.resident->getId().value();

    std::optional<Resident> stored = repository.findById(residentId);
    CHECK(stored.has_value());
}

// T04 Required Test 4: Registered Resident information is preserved
DROGON_TEST(RegisteredResidentInformationIsPreservedTest)
{
    Database db(makeTempDbPath("register_info"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentRegistrationService service(validator, repository);

    Resident resident = makeValidResidentForRegistration();
    ResidentRegistrationResult result = service.registerResident(resident);

    CHECK(result.success);
    int residentId = result.resident->getId().value();

    std::optional<Resident> stored = repository.findById(residentId);
    CHECK(stored.has_value());
    CHECK(stored->getFirstName() == "Juan");
    CHECK(stored->getLastName() == "Dela Cruz");
    CHECK(stored->getAddress() == "Barangay Santo Tomas");
    CHECK(stored->getContactNumber() == "09171234567");
    CHECK(stored->getEmail() == "juan@example.com");
    CHECK(stored->getStatus() == ResidentStatus::Active);
}

// T04 Required Test 5: Default Active status is preserved through registration
DROGON_TEST(RegisteredResidentPreservesDefaultActiveStatusTest)
{
    Database db(makeTempDbPath("register_status"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentRegistrationService service(validator, repository);

    Resident resident = makeValidResidentForRegistration();
    CHECK(resident.getStatus() == ResidentStatus::Active);

    ResidentRegistrationResult result = service.registerResident(resident);

    CHECK(result.success);
    CHECK(result.resident->getStatus() == ResidentStatus::Active);
}

// T04 Required Test 6: Invalid Resident registration fails
DROGON_TEST(InvalidResidentRegistrationFailsTest)
{
    Database db(makeTempDbPath("register_invalid"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentRegistrationService service(validator, repository);

    Resident invalidResident("",
                              "Dela Cruz",
                              "Barangay Santo Tomas",
                              "09171234567",
                              "juan@example.com",
                              ResidentStatus::Active);

    ResidentRegistrationResult result = service.registerResident(invalidResident);

    CHECK(!result.success);
    CHECK(!result.resident.has_value());
    CHECK(!result.errors.empty());
}

// T04 Required Test 7: Invalid Resident is not persisted
DROGON_TEST(InvalidResidentIsNotPersistedTest)
{
    std::string dbPath = makeTempDbPath("register_not_persisted");
    Database db(dbPath);
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentRegistrationService service(validator, repository);

    Resident invalidResident("",
                              "Dela Cruz",
                              "Barangay Santo Tomas",
                              "09171234567",
                              "juan@example.com",
                              ResidentStatus::Active);

    int countBefore = countResidentsInDatabase(dbPath);
    ResidentRegistrationResult result = service.registerResident(invalidResident);
    int countAfter = countResidentsInDatabase(dbPath);

    CHECK(!result.success);
    CHECK(countAfter == countBefore);
}

// T04 Required Test 8: Validation failure can be identified
DROGON_TEST(RegistrationReturnsValidationErrorsTest)
{
    Database db(makeTempDbPath("register_errors"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentRegistrationService service(validator, repository);

    Resident invalidResident("",
                              "Dela Cruz",
                              "Barangay Santo Tomas",
                              "09171234567",
                              "juan@example.com",
                              ResidentStatus::Active);

    ResidentRegistrationResult result = service.registerResident(invalidResident);

    CHECK(!result.success);
    CHECK(std::find(result.errors.begin(), result.errors.end(), "firstName") != result.errors.end());
}

// Persists a Resident directly through the repository, for setting up
// search/listing test scenarios (bypasses registration/validation on
// purpose, since T05 is testing query behavior, not registration).
Resident persistResident(ResidentRepository &repository,
                          const std::string &firstName,
                          const std::string &lastName,
                          ResidentStatus status = ResidentStatus::Active)
{
    Resident resident(firstName, lastName, "Some Address", "09171234567", "test@example.com", status);
    return repository.save(resident);
}

// T05 Required Test 1: List all persisted Residents
DROGON_TEST(ListAllPersistedResidentsTest)
{
    Database db(makeTempDbPath("list_all"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    persistResident(repository, "Juan", "Dela Cruz");
    persistResident(repository, "Maria", "Santos");

    std::vector<Resident> results = service.listResidents();

    CHECK(results.size() == 2);
}

// T05 Required Test 2: Empty Resident listing
DROGON_TEST(EmptyResidentListingTest)
{
    Database db(makeTempDbPath("list_empty"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    std::vector<Resident> results = service.listResidents();

    CHECK(results.empty());
}

// T05 Required Test 3: Listing uses required ordering (lastName, firstName, id)
DROGON_TEST(ListingUsesRequiredOrderingTest)
{
    Database db(makeTempDbPath("list_order"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    // Insert deliberately out of the expected display order.
    persistResident(repository, "Ana", "Santos");
    persistResident(repository, "Pedro", "Cruz");
    persistResident(repository, "Maria", "Andres");
    persistResident(repository, "Juan", "Cruz");

    std::vector<Resident> results = service.listResidents();

    CHECK(results.size() == 4);
    // Expected order: Andres, Cruz(Juan), Cruz(Pedro), Santos
    CHECK(results[0].getLastName() == "Andres");
    CHECK(results[1].getLastName() == "Cruz");
    CHECK(results[1].getFirstName() == "Juan");
    CHECK(results[2].getLastName() == "Cruz");
    CHECK(results[2].getFirstName() == "Pedro");
    CHECK(results[3].getLastName() == "Santos");
}

// T05 Required Test 4: Partial first name search is case-insensitive
DROGON_TEST(PartialFirstNameSearchIsCaseInsensitiveTest)
{
    Database db(makeTempDbPath("search_first_name"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    persistResident(repository, "Juan", "Dela Cruz");

    std::vector<Resident> results = service.searchResidents("jUa");

    CHECK(results.size() == 1);
    CHECK(results[0].getFirstName() == "Juan");
}

// T05 Required Test 5: Partial last name search is case-insensitive
DROGON_TEST(PartialLastNameSearchIsCaseInsensitiveTest)
{
    Database db(makeTempDbPath("search_last_name"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    persistResident(repository, "Juan", "Dela Cruz");

    std::vector<Resident> results = service.searchResidents("cRuZ");

    CHECK(results.size() == 1);
    CHECK(results[0].getLastName() == "Dela Cruz");
}

// T05 Required Test 6: Blank search returns all Residents
DROGON_TEST(BlankSearchReturnsAllResidentsTest)
{
    Database db(makeTempDbPath("search_blank"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    persistResident(repository, "Juan", "Dela Cruz");
    persistResident(repository, "Maria", "Santos");

    std::vector<Resident> blankResults = service.searchResidents("   ");
    std::vector<Resident> listResults = service.listResidents();

    CHECK(blankResults.size() == listResults.size());
    CHECK(blankResults.size() == 2);
}

// T05 Required Test 7: Search with no match returns empty collection
DROGON_TEST(SearchWithNoMatchReturnsEmptyCollectionTest)
{
    Database db(makeTempDbPath("search_no_match"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    persistResident(repository, "Juan", "Dela Cruz");

    std::vector<Resident> results = service.searchResidents("ZzzUnknownResident");

    CHECK(results.empty());
}

// T05 Required Test 8: Search results preserve Resident information
DROGON_TEST(SearchResultsPreserveResidentInformationTest)
{
    Database db(makeTempDbPath("search_preserve_info"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    Resident resident("Juan", "Dela Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", ResidentStatus::Active);
    repository.save(resident);

    std::vector<Resident> results = service.searchResidents("Juan");

    CHECK(results.size() == 1);
    CHECK(results[0].getFirstName() == "Juan");
    CHECK(results[0].getLastName() == "Dela Cruz");
    CHECK(results[0].getAddress() == "Barangay Santo Tomas");
    CHECK(results[0].getContactNumber() == "09171234567");
    CHECK(results[0].getEmail() == "juan@example.com");
    CHECK(results[0].getStatus() == ResidentStatus::Active);
}

// T05 Required Test 9: Active and Inactive Residents are both included
DROGON_TEST(ActiveAndInactiveResidentsAreIncludedTest)
{
    Database db(makeTempDbPath("list_status_mix"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    persistResident(repository, "Juan", "Dela Cruz", ResidentStatus::Active);
    persistResident(repository, "Maria", "Santos", ResidentStatus::Inactive);

    std::vector<Resident> results = service.listResidents();

    CHECK(results.size() == 2);
}

// T05 Required Test 10: Matching Resident is not duplicated
DROGON_TEST(MatchingResidentIsNotDuplicatedTest)
{
    Database db(makeTempDbPath("search_no_duplicate"));
    ResidentRepository repository(db);
    ResidentSearchService service(repository);

    // "Cruz" appears in both the first name and last name.
    persistResident(repository, "Cruzita", "Cruz");

    std::vector<Resident> results = service.searchResidents("Cruz");

    CHECK(results.size() == 1);
}

// T06 Required Test 1: Valid Resident update succeeds
DROGON_TEST(ValidResidentUpdateSucceedsTest)
{
    Database db(makeTempDbPath("update_valid"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService service(validator, repository);

    Resident resident = persistResident(repository, "Juan", "Cruz");
    int id = resident.getId().value();

    ResidentUpdateResult result = service.updateResident(
        id, "Juan Miguel", "Dela Cruz", "New Address", "09181234567", "juanmiguel@example.com");

    CHECK(result.success);
    CHECK(!result.residentNotFound);
    CHECK(result.errors.empty());
}

// T06 Required Test 2: Resident ID is preserved
DROGON_TEST(UpdatePreservesResidentIdTest)
{
    Database db(makeTempDbPath("update_preserve_id"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService service(validator, repository);

    Resident resident = persistResident(repository, "Juan", "Cruz");
    int originalId = resident.getId().value();

    ResidentUpdateResult result = service.updateResident(
        originalId, "Juan Miguel", "Dela Cruz", "New Address", "09181234567", "juanmiguel@example.com");

    CHECK(result.success);
    CHECK(result.resident->getId().value() == originalId);
}

// T06 Required Test 3: Permitted Resident information is persisted
DROGON_TEST(PermittedResidentInformationIsPersistedTest)
{
    Database db(makeTempDbPath("update_info_persisted"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService service(validator, repository);

    Resident resident = persistResident(repository, "Juan", "Cruz");
    int id = resident.getId().value();

    service.updateResident(id, "Juan Miguel", "Dela Cruz", "New Address", "09181234567", "juanmiguel@example.com");

    std::optional<Resident> stored = repository.findById(id);
    CHECK(stored.has_value());
    CHECK(stored->getFirstName() == "Juan Miguel");
    CHECK(stored->getLastName() == "Dela Cruz");
    CHECK(stored->getAddress() == "New Address");
    CHECK(stored->getContactNumber() == "09181234567");
    CHECK(stored->getEmail() == "juanmiguel@example.com");
}

// T06 Required Test 4: Resident status is preserved
DROGON_TEST(UpdatePreservesResidentStatusTest)
{
    Database db(makeTempDbPath("update_preserve_status"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService service(validator, repository);

    Resident resident = persistResident(repository, "Juan", "Cruz", ResidentStatus::Inactive);
    int id = resident.getId().value();

    ResidentUpdateResult result = service.updateResident(
        id, "Juan Miguel", "Dela Cruz", "New Address", "09181234567", "juanmiguel@example.com");

    CHECK(result.success);
    CHECK(result.resident->getStatus() == ResidentStatus::Inactive);
}

// T06 Required Test 5: Invalid update fails
DROGON_TEST(InvalidUpdateFailsTest)
{
    Database db(makeTempDbPath("update_invalid"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService service(validator, repository);

    Resident resident = persistResident(repository, "Juan", "Cruz");
    int id = resident.getId().value();

    ResidentUpdateResult result = service.updateResident(
        id, "", "Dela Cruz", "New Address", "09181234567", "juanmiguel@example.com");

    CHECK(!result.success);
    CHECK(!result.residentNotFound);
    CHECK(!result.errors.empty());
}

// T06 Required Test 6: Invalid update does not modify persisted information
DROGON_TEST(InvalidUpdateDoesNotModifyPersistedInformationTest)
{
    Database db(makeTempDbPath("update_invalid_no_change"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService service(validator, repository);

    Resident resident = persistResident(repository, "Juan", "Cruz");
    int id = resident.getId().value();

    service.updateResident(id, "", "Dela Cruz", "New Address", "09181234567", "juanmiguel@example.com");

    std::optional<Resident> stored = repository.findById(id);
    CHECK(stored.has_value());
    CHECK(stored->getFirstName() == "Juan"); // unchanged
    CHECK(stored->getLastName() == "Cruz");  // unchanged
}

// T06 Required Test 7: Updating a nonexistent Resident is handled safely
DROGON_TEST(UpdatingNonexistentResidentIsHandledSafelyTest)
{
    Database db(makeTempDbPath("update_not_found"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService service(validator, repository);

    ResidentUpdateResult result = service.updateResident(
        999999, "Juan", "Cruz", "Some Address", "09171234567", "juan@example.com");

    CHECK(!result.success);
    CHECK(result.residentNotFound);
}

// T06 Required Test 8: Nonexistent update does not create a Resident
DROGON_TEST(NonexistentUpdateDoesNotCreateResidentTest)
{
    std::string dbPath = makeTempDbPath("update_not_found_no_create");
    Database db(dbPath);
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService service(validator, repository);

    int countBefore = countResidentsInDatabase(dbPath);
    service.updateResident(999999, "Juan", "Cruz", "Some Address", "09171234567", "juan@example.com");
    int countAfter = countResidentsInDatabase(dbPath);

    CHECK(countAfter == countBefore);
}

// T06 Required Test 9: Updated Resident is visible through T05 querying
DROGON_TEST(UpdatedResidentIsVisibleThroughSearchTest)
{
    Database db(makeTempDbPath("update_visible_search"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService updateService(validator, repository);
    ResidentSearchService searchService(repository);

    Resident resident = persistResident(repository, "Juan", "Cruz");
    int id = resident.getId().value();

    updateService.updateResident(id, "Miguel", "Santos", "New Address", "09181234567", "miguel@example.com");

    std::vector<Resident> results = searchService.searchResidents("Miguel");

    CHECK(results.size() == 1);
    CHECK(results[0].getId().value() == id);
}

// T06 Required Test 10: Updated information and contact number are preserved
DROGON_TEST(UpdatedContactNumberPreservesLeadingZeroTest)
{
    Database db(makeTempDbPath("update_contact_number"));
    ResidentRepository repository(db);
    ResidentValidator validator;
    ResidentUpdateService service(validator, repository);

    Resident resident = persistResident(repository, "Juan", "Cruz");
    int id = resident.getId().value();

    ResidentUpdateResult result = service.updateResident(
        id, "Juan", "Cruz", "Some Address", "09181234567", "juan@example.com");

    CHECK(result.success);
    std::optional<Resident> stored = repository.findById(id);
    CHECK(stored->getContactNumber() == "09181234567");
    CHECK(stored->getContactNumber()[0] == '0');
    CHECK(stored->getId().value() == id);
    CHECK(stored->getStatus() == ResidentStatus::Active);
}

// Persists a Resident with fully known details, for T07 preservation checks.
Resident persistDetailedResident(ResidentRepository &repository,
                                  ResidentStatus status = ResidentStatus::Active)
{
    Resident resident("Juan",
                       "Dela Cruz",
                       "Barangay Santo Tomas",
                       "09171234567",
                       "juan@example.com",
                       status);
    return repository.save(resident);
}

// T07 Required Test 1: Active Resident can be deactivated
DROGON_TEST(ActiveResidentCanBeDeactivatedTest)
{
    Database db(makeTempDbPath("deactivate_active"));
    ResidentRepository repository(db);
    ResidentDeactivationService service(repository);

    Resident resident = persistDetailedResident(repository);

    ResidentDeactivationResult result = service.deactivateResident(resident.getId().value());

    CHECK(result.success);
    CHECK(result.stateChanged);
    CHECK(!result.residentNotFound);
    CHECK(result.resident.has_value());
}

// T07 Required Test 2: Status becomes Inactive in actual persistence
DROGON_TEST(DeactivatedStatusIsPersistedTest)
{
    Database db(makeTempDbPath("deactivate_persisted"));
    ResidentRepository repository(db);
    ResidentDeactivationService service(repository);

    Resident resident = persistDetailedResident(repository);
    int id = resident.getId().value();

    service.deactivateResident(id);

    // A brand-new repository on the same connection proves the change is
    // in the database, not in a temporary application object.
    ResidentRepository freshRepository(db);
    std::optional<Resident> stored = freshRepository.findById(id);
    CHECK(stored.has_value());
    CHECK(stored->getStatus() == ResidentStatus::Inactive);
}

// T07 Required Test 3: Resident ID is preserved
DROGON_TEST(DeactivationPreservesResidentIdTest)
{
    Database db(makeTempDbPath("deactivate_preserve_id"));
    ResidentRepository repository(db);
    ResidentDeactivationService service(repository);

    Resident resident = persistDetailedResident(repository);
    int originalId = resident.getId().value();

    ResidentDeactivationResult result = service.deactivateResident(originalId);

    CHECK(result.resident->getId().value() == originalId);
    CHECK(repository.findById(originalId)->getId().value() == originalId);
}

// T07 Required Test 4: Resident information is preserved
DROGON_TEST(DeactivationPreservesResidentInformationTest)
{
    Database db(makeTempDbPath("deactivate_preserve_info"));
    ResidentRepository repository(db);
    ResidentDeactivationService service(repository);

    Resident resident = persistDetailedResident(repository);
    int id = resident.getId().value();

    service.deactivateResident(id);

    std::optional<Resident> stored = repository.findById(id);
    CHECK(stored.has_value());
    CHECK(stored->getFirstName() == "Juan");
    CHECK(stored->getLastName() == "Dela Cruz");
    CHECK(stored->getAddress() == "Barangay Santo Tomas");
    CHECK(stored->getContactNumber() == "09171234567");
    CHECK(stored->getContactNumber()[0] == '0');
    CHECK(stored->getEmail() == "juan@example.com");
}

// T07 Required Test 5: Deactivated Resident remains persisted and retrievable
DROGON_TEST(DeactivatedResidentRemainsRetrievableTest)
{
    std::string dbPath = makeTempDbPath("deactivate_retrievable");
    Database db(dbPath);
    ResidentRepository repository(db);
    ResidentDeactivationService service(repository);

    Resident resident = persistDetailedResident(repository);
    int id = resident.getId().value();

    int countBefore = countResidentsInDatabase(dbPath);
    service.deactivateResident(id);
    int countAfter = countResidentsInDatabase(dbPath);

    std::optional<Resident> stored = repository.findById(id);
    CHECK(countAfter == countBefore);
    CHECK(stored.has_value());
    CHECK(stored->getStatus() == ResidentStatus::Inactive);
}

// T07 Required Test 6: Deactivated Resident remains available through T05
DROGON_TEST(DeactivatedResidentRemainsInSearchAndListingTest)
{
    Database db(makeTempDbPath("deactivate_t05"));
    ResidentRepository repository(db);
    ResidentDeactivationService deactivationService(repository);
    ResidentSearchService searchService(repository);

    Resident resident = persistResident(repository, "Juan", "Cruz");
    int id = resident.getId().value();

    deactivationService.deactivateResident(id);

    std::vector<Resident> searchResults = searchService.searchResidents("Juan");
    CHECK(searchResults.size() == 1);
    CHECK(searchResults[0].getId().value() == id);
    CHECK(searchResults[0].getStatus() == ResidentStatus::Inactive);

    std::vector<Resident> listResults = searchService.listResidents();
    CHECK(listResults.size() == 1);
    CHECK(listResults[0].getStatus() == ResidentStatus::Inactive);
}

// T07 Required Test 7: Already-Inactive Resident is handled safely
DROGON_TEST(AlreadyInactiveResidentIsHandledSafelyTest)
{
    std::string dbPath = makeTempDbPath("deactivate_already_inactive");
    Database db(dbPath);
    ResidentRepository repository(db);
    ResidentDeactivationService service(repository);

    Resident resident = persistDetailedResident(repository, ResidentStatus::Inactive);
    int id = resident.getId().value();

    int countBefore = countResidentsInDatabase(dbPath);
    ResidentDeactivationResult first = service.deactivateResident(id);
    ResidentDeactivationResult second = service.deactivateResident(id);
    int countAfter = countResidentsInDatabase(dbPath);

    CHECK(first.success);
    CHECK(!first.stateChanged);
    CHECK(second.success);
    CHECK(!second.stateChanged);
    CHECK(countAfter == countBefore);

    std::optional<Resident> stored = repository.findById(id);
    CHECK(stored->getId().value() == id);
    CHECK(stored->getStatus() == ResidentStatus::Inactive);
    CHECK(stored->getFirstName() == "Juan");
    CHECK(stored->getLastName() == "Dela Cruz");
    CHECK(stored->getAddress() == "Barangay Santo Tomas");
    CHECK(stored->getContactNumber() == "09171234567");
    CHECK(stored->getEmail() == "juan@example.com");
}

// T07 Required Test 8: Nonexistent Resident is handled safely
DROGON_TEST(DeactivatingNonexistentResidentIsHandledSafelyTest)
{
    Database db(makeTempDbPath("deactivate_not_found"));
    ResidentRepository repository(db);
    ResidentDeactivationService service(repository);

    ResidentDeactivationResult result = service.deactivateResident(999999);

    CHECK(!result.success);
    CHECK(result.residentNotFound);
    CHECK(!result.stateChanged);
    CHECK(!result.resident.has_value());
}

// T07 Required Test 9: Nonexistent deactivation does not create or delete records
DROGON_TEST(NonexistentDeactivationDoesNotCreateOrDeleteRecordsTest)
{
    std::string dbPath = makeTempDbPath("deactivate_not_found_no_change");
    Database db(dbPath);
    ResidentRepository repository(db);
    ResidentDeactivationService service(repository);

    Resident resident = persistDetailedResident(repository);
    int id = resident.getId().value();

    int countBefore = countResidentsInDatabase(dbPath);
    service.deactivateResident(999999);
    int countAfter = countResidentsInDatabase(dbPath);

    CHECK(countAfter == countBefore);
    CHECK(!repository.findById(999999).has_value());

    std::optional<Resident> stored = repository.findById(id);
    CHECK(stored.has_value());
    CHECK(stored->getStatus() == ResidentStatus::Active);
    CHECK(stored->getFirstName() == "Juan");
}

// T07 Required Test 10: Deactivating one Resident does not affect another
DROGON_TEST(DeactivatingOneResidentDoesNotAffectAnotherTest)
{
    Database db(makeTempDbPath("deactivate_isolated"));
    ResidentRepository repository(db);
    ResidentDeactivationService service(repository);

    Resident first = persistResident(repository, "Juan", "Cruz");
    Resident second = persistResident(repository, "Maria", "Santos");
    Resident third = persistResident(repository, "Pedro", "Reyes");

    service.deactivateResident(second.getId().value());

    std::optional<Resident> storedFirst = repository.findById(first.getId().value());
    std::optional<Resident> storedSecond = repository.findById(second.getId().value());
    std::optional<Resident> storedThird = repository.findById(third.getId().value());

    CHECK(storedFirst->getStatus() == ResidentStatus::Active);
    CHECK(storedSecond->getStatus() == ResidentStatus::Inactive);
    CHECK(storedThird->getStatus() == ResidentStatus::Active);

    CHECK(storedFirst->getFirstName() == "Juan");
    CHECK(storedFirst->getLastName() == "Cruz");
    CHECK(storedFirst->getContactNumber() == "09171234567");
    CHECK(storedThird->getFirstName() == "Pedro");
    CHECK(storedThird->getLastName() == "Reyes");
}

// T08 Required Test 1: Service Request can be created
DROGON_TEST(ServiceRequestCanBeCreatedTest)
{
    ServiceRequest request(25,
                           "Barangay Clearance",
                           "Request for employment requirement",
                           "2026-09-25");

    CHECK(request.getResidentId() == 25);
}

// T08 Required Test 2: Service Request information is accessible
DROGON_TEST(ServiceRequestInformationIsAccessibleTest)
{
    ServiceRequest request(25,
                           "Barangay Clearance",
                           "Request for employment requirement",
                           "2026-09-25");

    CHECK(request.getResidentId() == 25);
    CHECK(request.getServiceType() == "Barangay Clearance");
    CHECK(request.getDescription() == "Request for employment requirement");
    CHECK(request.getDateRequested() == "2026-09-25");
}

// T08 Required Test 3: Resident ID is preserved
DROGON_TEST(ServiceRequestPreservesResidentIdTest)
{
    ServiceRequest request(25, "Permit Request", "Business permit", "2026-09-25");

    CHECK(request.getResidentId() == 25);
}

// T08 Required Test 4: New Service Request has an unassigned ID
DROGON_TEST(NewServiceRequestHasNoIdTest)
{
    ServiceRequest request(25, "Permit Request", "Business permit", "2026-09-25");

    CHECK(!request.getId().has_value());
}

// T08 Required Test 5: New Service Request defaults to Pending
DROGON_TEST(NewServiceRequestDefaultsToPendingTest)
{
    ServiceRequest request(25, "Permit Request", "Business permit", "2026-09-25");

    CHECK(request.getStatus() == ServiceRequestStatus::Pending);
}

// T08 Required Test 6: Service Request information is independent between objects
DROGON_TEST(ServiceRequestsAreIndependentTest)
{
    ServiceRequest first(25, "Barangay Clearance", "Employment requirement", "2026-09-25");
    ServiceRequest second(31, "Community Assistance", "Medical assistance", "2026-10-01");

    CHECK(first.getResidentId() == 25);
    CHECK(first.getServiceType() == "Barangay Clearance");
    CHECK(first.getDescription() == "Employment requirement");
    CHECK(first.getDateRequested() == "2026-09-25");

    CHECK(second.getResidentId() == 31);
    CHECK(second.getServiceType() == "Community Assistance");
    CHECK(second.getDescription() == "Medical assistance");
    CHECK(second.getDateRequested() == "2026-10-01");
}

// Directly queries SQLite to count Service Request rows, bypassing our own
// repository, to prove rejected submissions never reach the database.
int countServiceRequestsInDatabase(const std::string &databasePath)
{
    sqlite3 *database = nullptr;
    sqlite3_open(databasePath.c_str(), &database);

    const char *sql = "SELECT COUNT(*) FROM service_requests";
    sqlite3_stmt *statement = nullptr;
    sqlite3_prepare_v2(database, sql, -1, &statement, nullptr);
    sqlite3_step(statement);

    int count = sqlite3_column_int(statement, 0);

    sqlite3_finalize(statement);
    sqlite3_close(database);

    return count;
}

// A valid new Service Request for the given Resident (id unassigned, Pending).
ServiceRequest makeValidServiceRequest(int residentId)
{
    return ServiceRequest(residentId,
                          "Barangay Clearance",
                          "Request for employment requirement",
                          "2026-09-15");
}

// T09 Required Test 1: Valid Service Request submission succeeds
DROGON_TEST(ValidServiceRequestSubmissionSucceedsTest)
{
    Database db(makeTempDbPath("sr_submit_valid"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");

    ServiceRequestSubmissionResult result =
        service.submitServiceRequest(makeValidServiceRequest(resident.getId().value()));

    CHECK(result.success);
    CHECK(!result.residentNotFound);
    CHECK(!result.residentInactive);
    CHECK(result.errors.empty());
    CHECK(result.serviceRequest.has_value());
}

// T09 Required Test 2: Submitted Service Request receives a generated ID
DROGON_TEST(SubmittedServiceRequestReceivesGeneratedIdTest)
{
    Database db(makeTempDbPath("sr_generated_id"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    ServiceRequest request = makeValidServiceRequest(resident.getId().value());

    CHECK(!request.getId().has_value()); // unassigned before submission

    ServiceRequestSubmissionResult result = service.submitServiceRequest(request);

    CHECK(result.success);
    CHECK(result.serviceRequest->getId().has_value());
    CHECK(result.serviceRequest->getId().value() > 0);
}

// T09 Required Test 3: Submitted Service Request is persisted and retrievable
DROGON_TEST(SubmittedServiceRequestIsRetrievableTest)
{
    Database db(makeTempDbPath("sr_retrievable"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    ServiceRequestSubmissionResult result =
        service.submitServiceRequest(makeValidServiceRequest(resident.getId().value()));

    int id = result.serviceRequest->getId().value();
    std::optional<ServiceRequest> stored = requestRepository.findById(id);

    CHECK(stored.has_value());
    CHECK(stored->getId().value() == id);
}

// T09 Required Test 4: Submitted Service Request information is preserved
DROGON_TEST(SubmittedServiceRequestInformationIsPreservedTest)
{
    Database db(makeTempDbPath("sr_preserved"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int residentId = resident.getId().value();

    ServiceRequestSubmissionResult result =
        service.submitServiceRequest(makeValidServiceRequest(residentId));

    std::optional<ServiceRequest> stored =
        requestRepository.findById(result.serviceRequest->getId().value());

    CHECK(stored.has_value());
    CHECK(stored->getResidentId() == residentId);
    CHECK(stored->getServiceType() == "Barangay Clearance");
    CHECK(stored->getDescription() == "Request for employment requirement");
    CHECK(stored->getDateRequested() == "2026-09-15");
    CHECK(stored->getStatus() == ServiceRequestStatus::Pending);
}

// T09 Required Test 5: Submitted Service Request status is Pending
DROGON_TEST(SubmittedServiceRequestStatusIsPendingTest)
{
    Database db(makeTempDbPath("sr_pending"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    ServiceRequestSubmissionResult result =
        service.submitServiceRequest(makeValidServiceRequest(resident.getId().value()));

    CHECK(result.serviceRequest->getStatus() == ServiceRequestStatus::Pending);
    CHECK(requestRepository.findById(result.serviceRequest->getId().value())->getStatus() ==
          ServiceRequestStatus::Pending);
}

// T09 Required Test 6: Blank service type fails validation
DROGON_TEST(BlankServiceTypeFailsValidationTest)
{
    Database db(makeTempDbPath("sr_blank_type"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int residentId = resident.getId().value();

    ServiceRequestSubmissionResult emptyResult = service.submitServiceRequest(
        ServiceRequest(residentId, "", "Some details", "2026-09-15"));
    ServiceRequestSubmissionResult whitespaceResult = service.submitServiceRequest(
        ServiceRequest(residentId, "   ", "Some details", "2026-09-15"));

    CHECK(!emptyResult.success);
    CHECK(std::find(emptyResult.errors.begin(), emptyResult.errors.end(), "serviceType") != emptyResult.errors.end());
    CHECK(!whitespaceResult.success);
    CHECK(std::find(whitespaceResult.errors.begin(), whitespaceResult.errors.end(), "serviceType") != whitespaceResult.errors.end());
}

// T09 Required Test 7: Blank description fails validation
DROGON_TEST(BlankDescriptionFailsValidationTest)
{
    Database db(makeTempDbPath("sr_blank_description"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int residentId = resident.getId().value();

    ServiceRequestSubmissionResult emptyResult = service.submitServiceRequest(
        ServiceRequest(residentId, "Permit Request", "", "2026-09-15"));
    ServiceRequestSubmissionResult whitespaceResult = service.submitServiceRequest(
        ServiceRequest(residentId, "Permit Request", "  \t ", "2026-09-15"));

    CHECK(!emptyResult.success);
    CHECK(std::find(emptyResult.errors.begin(), emptyResult.errors.end(), "description") != emptyResult.errors.end());
    CHECK(!whitespaceResult.success);
    CHECK(std::find(whitespaceResult.errors.begin(), whitespaceResult.errors.end(), "description") != whitespaceResult.errors.end());
}

// T09 Required Test 8: Invalid request does not reach persistence
DROGON_TEST(InvalidServiceRequestDoesNotReachPersistenceTest)
{
    std::string dbPath = makeTempDbPath("sr_invalid_not_persisted");
    Database db(dbPath);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");

    int countBefore = countServiceRequestsInDatabase(dbPath);
    service.submitServiceRequest(ServiceRequest(resident.getId().value(), "", "", "not-a-date"));
    int countAfter = countServiceRequestsInDatabase(dbPath);

    CHECK(countAfter == countBefore);
}

// T09 Required Test 9: Nonexistent Resident prevents submission
DROGON_TEST(NonexistentResidentPreventsSubmissionTest)
{
    std::string dbPath = makeTempDbPath("sr_resident_not_found");
    Database db(dbPath);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    int countBefore = countServiceRequestsInDatabase(dbPath);
    ServiceRequestSubmissionResult result = service.submitServiceRequest(makeValidServiceRequest(999999));
    int countAfter = countServiceRequestsInDatabase(dbPath);

    CHECK(!result.success);
    CHECK(result.residentNotFound);
    CHECK(!result.residentInactive);
    CHECK(result.errors.empty());
    CHECK(!result.serviceRequest.has_value());
    CHECK(countAfter == countBefore);
}

// T09 Required Test 10: Inactive Resident cannot submit a new Service Request
DROGON_TEST(InactiveResidentCannotSubmitServiceRequestTest)
{
    std::string dbPath = makeTempDbPath("sr_resident_inactive");
    Database db(dbPath);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz", ResidentStatus::Inactive);
    int residentId = resident.getId().value();

    int countBefore = countServiceRequestsInDatabase(dbPath);
    ServiceRequestSubmissionResult result = service.submitServiceRequest(makeValidServiceRequest(residentId));
    int countAfter = countServiceRequestsInDatabase(dbPath);

    CHECK(!result.success);
    CHECK(result.residentInactive);
    CHECK(!result.residentNotFound);
    CHECK(!result.serviceRequest.has_value());
    CHECK(countAfter == countBefore);
    CHECK(residentRepository.findById(residentId)->getStatus() == ResidentStatus::Inactive);
}

// T09 Required Test 11: Non-Pending initial status is rejected
DROGON_TEST(NonPendingInitialStatusIsRejectedTest)
{
    std::string dbPath = makeTempDbPath("sr_non_pending");
    Database db(dbPath);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int residentId = resident.getId().value();

    ServiceRequestStatus badStatuses[] = {ServiceRequestStatus::InProgress,
                                          ServiceRequestStatus::Completed,
                                          ServiceRequestStatus::Cancelled};

    for (ServiceRequestStatus badStatus : badStatuses)
    {
        ServiceRequest request(residentId, "Permit Request", "Business permit", "2026-09-15", badStatus);
        ServiceRequestSubmissionResult result = service.submitServiceRequest(request);

        CHECK(!result.success);
        CHECK(std::find(result.errors.begin(), result.errors.end(), "status") != result.errors.end());
    }

    CHECK(countServiceRequestsInDatabase(dbPath) == 0);
}

// T09 Required Test 12: Service Request persists across repository access
DROGON_TEST(ServiceRequestPersistsAcrossRepositoryAccessTest)
{
    std::string dbPath = makeTempDbPath("sr_persists");
    int residentId = 0;
    int requestId = 0;

    {
        Database db(dbPath);
        ResidentRepository residentRepository(db);
        ServiceRequestRepository requestRepository(db);
        ServiceRequestValidator validator;
        ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

        Resident resident = persistResident(residentRepository, "Juan", "Cruz");
        residentId = resident.getId().value();

        ServiceRequestSubmissionResult result = service.submitServiceRequest(makeValidServiceRequest(residentId));
        requestId = result.serviceRequest->getId().value();
    } // first connection closed here

    Database secondDb(dbPath);
    ServiceRequestRepository secondRepository(secondDb);
    std::optional<ServiceRequest> stored = secondRepository.findById(requestId);

    CHECK(stored.has_value());
    CHECK(stored->getResidentId() == residentId);
    CHECK(stored->getServiceType() == "Barangay Clearance");
    CHECK(stored->getStatus() == ServiceRequestStatus::Pending);
}

// T09 Required Test 13: Submission does not modify the Resident
DROGON_TEST(SubmissionDoesNotModifyResidentTest)
{
    Database db(makeTempDbPath("sr_resident_unchanged"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);

    Resident resident = persistDetailedResident(residentRepository);
    int id = resident.getId().value();

    service.submitServiceRequest(makeValidServiceRequest(id));

    std::optional<Resident> stored = residentRepository.findById(id);
    CHECK(stored.has_value());
    CHECK(stored->getId().value() == id);
    CHECK(stored->getFirstName() == "Juan");
    CHECK(stored->getLastName() == "Dela Cruz");
    CHECK(stored->getAddress() == "Barangay Santo Tomas");
    CHECK(stored->getContactNumber() == "09171234567");
    CHECK(stored->getEmail() == "juan@example.com");
    CHECK(stored->getStatus() == ResidentStatus::Active);
}

// T09 Date Validation Test: missing or invalid dates cannot be submitted
DROGON_TEST(InvalidOrMissingDateFailsValidationTest)
{
    ServiceRequestValidator validator;

    const std::string badDates[] = {"", "   ", "2026/09/15", "15-09-2026", "2026-9-15",
                                    "2026-13-01", "2026-02-30", "2026-00-10", "2026-04-31",
                                    "2025-02-29", "abcd-ef-gh"};

    for (const std::string &badDate : badDates)
    {
        std::vector<std::string> errors =
            validator.validate(ServiceRequest(25, "Permit Request", "Business permit", badDate));

        CHECK(std::find(errors.begin(), errors.end(), "dateRequested") != errors.end());
    }

    // Real dates, including a leap day, are accepted.
    CHECK(validator.isValid(ServiceRequest(25, "Permit Request", "Business permit", "2026-09-15")));
    CHECK(validator.isValid(ServiceRequest(25, "Permit Request", "Business permit", "2024-02-29")));
}

// Extra validator test: an already-assigned id and a non-positive Resident id are rejected
DROGON_TEST(AssignedIdAndInvalidResidentIdFailValidationTest)
{
    ServiceRequestValidator validator;

    ServiceRequest alreadyPersisted(17, 25, "Permit Request", "Business permit", "2026-09-15",
                                    ServiceRequestStatus::Pending);
    std::vector<std::string> idErrors = validator.validate(alreadyPersisted);
    CHECK(std::find(idErrors.begin(), idErrors.end(), "id") != idErrors.end());

    std::vector<std::string> zeroErrors =
        validator.validate(ServiceRequest(0, "Permit Request", "Business permit", "2026-09-15"));
    CHECK(std::find(zeroErrors.begin(), zeroErrors.end(), "residentId") != zeroErrors.end());

    std::vector<std::string> negativeErrors =
        validator.validate(ServiceRequest(-5, "Permit Request", "Business permit", "2026-09-15"));
    CHECK(std::find(negativeErrors.begin(), negativeErrors.end(), "residentId") != negativeErrors.end());
}

// Extra test: an Inactive Resident stays searchable even though it cannot submit
DROGON_TEST(InactiveResidentRemainsSearchableAfterRejectedSubmissionTest)
{
    Database db(makeTempDbPath("sr_inactive_searchable"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, residentRepository, requestRepository);
    ResidentSearchService searchService(residentRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz", ResidentStatus::Inactive);
    service.submitServiceRequest(makeValidServiceRequest(resident.getId().value()));

    std::vector<Resident> results = searchService.searchResidents("Juan");
    CHECK(results.size() == 1);
    CHECK(results[0].getStatus() == ResidentStatus::Inactive);
}

// ===========================================================================
// T10: Manage Service Request Status
// ===========================================================================

// Reads the status column straight from SQLite, bypassing our repository, so
// tests prove what is REALLY persisted.
std::string readStatusFromDatabase(const std::string &databasePath, int serviceRequestId)
{
    sqlite3 *database = nullptr;
    sqlite3_open(databasePath.c_str(), &database);

    sqlite3_stmt *statement = nullptr;
    sqlite3_prepare_v2(database, "SELECT status FROM service_requests WHERE id = ?", -1, &statement, nullptr);
    sqlite3_bind_int(statement, 1, serviceRequestId);

    std::string status = "(no row)";
    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        status = reinterpret_cast<const char *>(sqlite3_column_text(statement, 0));
    }

    sqlite3_finalize(statement);
    sqlite3_close(database);

    return status;
}

// Submits a valid request through the real T09 service so every T10 test
// starts from a genuinely persisted Pending request. Returns its id.
int submitPendingRequest(Database &db, int residentId)
{
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService submission(validator, residentRepository, requestRepository);

    ServiceRequestSubmissionResult result = submission.submitServiceRequest(makeValidServiceRequest(residentId));
    return result.serviceRequest->getId().value();
}

// T10 Required Test 1: Pending can move to In Progress
DROGON_TEST(PendingCanMoveToInProgressTest)
{
    std::string path = makeTempDbPath("sr_status_pending_to_inprogress");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    ServiceRequestStatusResult result = service.changeStatus(id, "In Progress");

    CHECK(result.success);
    CHECK(!result.notFound);
    CHECK(!result.unsupportedStatus);
    CHECK(!result.invalidTransition);
    CHECK(result.serviceRequest.has_value());
    CHECK(result.serviceRequest->getStatus() == ServiceRequestStatus::InProgress);
    CHECK(requestRepository.findById(id)->getStatus() == ServiceRequestStatus::InProgress);
    CHECK(readStatusFromDatabase(path, id) == "In Progress");
}

// T10 Required Test 2: Pending can move to Cancelled
DROGON_TEST(PendingCanMoveToCancelledTest)
{
    std::string path = makeTempDbPath("sr_status_pending_to_cancelled");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    ServiceRequestStatusResult result = service.changeStatus(id, "Cancelled");

    CHECK(result.success);
    CHECK(result.serviceRequest->getStatus() == ServiceRequestStatus::Cancelled);
    CHECK(readStatusFromDatabase(path, id) == "Cancelled");
}

// T10 Required Test 3: In Progress can move to Completed
// The In Progress state is reached through the real workflow, not by
// hand-writing a database row.
DROGON_TEST(InProgressCanMoveToCompletedTest)
{
    std::string path = makeTempDbPath("sr_status_inprogress_to_completed");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    REQUIRE(service.changeStatus(id, "In Progress").success);

    ServiceRequestStatusResult result = service.changeStatus(id, "Completed");

    CHECK(result.success);
    CHECK(result.serviceRequest->getStatus() == ServiceRequestStatus::Completed);
    CHECK(readStatusFromDatabase(path, id) == "Completed");
}

// T10 Required Test 4: In Progress can move to Cancelled
DROGON_TEST(InProgressCanMoveToCancelledTest)
{
    std::string path = makeTempDbPath("sr_status_inprogress_to_cancelled");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    REQUIRE(service.changeStatus(id, "In Progress").success);

    ServiceRequestStatusResult result = service.changeStatus(id, "Cancelled");

    CHECK(result.success);
    CHECK(result.serviceRequest->getStatus() == ServiceRequestStatus::Cancelled);
    CHECK(readStatusFromDatabase(path, id) == "Cancelled");
}

// T10 Required Test 5: Pending cannot move directly to Completed
DROGON_TEST(PendingCannotMoveDirectlyToCompletedTest)
{
    std::string path = makeTempDbPath("sr_status_pending_to_completed");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    ServiceRequestStatusResult result = service.changeStatus(id, "Completed");

    CHECK(!result.success);
    CHECK(result.invalidTransition);
    CHECK(!result.serviceRequest.has_value());
    CHECK(requestRepository.findById(id)->getStatus() == ServiceRequestStatus::Pending);
    CHECK(readStatusFromDatabase(path, id) == "Pending");
}

// T10 Required Test 6: In Progress cannot return to Pending
DROGON_TEST(InProgressCannotReturnToPendingTest)
{
    std::string path = makeTempDbPath("sr_status_inprogress_to_pending");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    REQUIRE(service.changeStatus(id, "In Progress").success);

    ServiceRequestStatusResult result = service.changeStatus(id, "Pending");

    CHECK(!result.success);
    CHECK(result.invalidTransition);
    CHECK(requestRepository.findById(id)->getStatus() == ServiceRequestStatus::InProgress);
    CHECK(readStatusFromDatabase(path, id) == "In Progress");
}

// T10 Required Test 7: Completed is terminal
DROGON_TEST(CompletedIsTerminalTest)
{
    std::string path = makeTempDbPath("sr_status_completed_terminal");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    REQUIRE(service.changeStatus(id, "In Progress").success);
    REQUIRE(service.changeStatus(id, "Completed").success);

    ServiceRequestStatusResult toPending = service.changeStatus(id, "Pending");
    ServiceRequestStatusResult toInProgress = service.changeStatus(id, "In Progress");
    ServiceRequestStatusResult toCancelled = service.changeStatus(id, "Cancelled");

    CHECK(!toPending.success);
    CHECK(toPending.invalidTransition);
    CHECK(!toInProgress.success);
    CHECK(toInProgress.invalidTransition);
    CHECK(!toCancelled.success);
    CHECK(toCancelled.invalidTransition);
    CHECK(requestRepository.findById(id)->getStatus() == ServiceRequestStatus::Completed);
    CHECK(readStatusFromDatabase(path, id) == "Completed");
}

// T10 Required Test 8: Cancelled is terminal
DROGON_TEST(CancelledIsTerminalTest)
{
    std::string path = makeTempDbPath("sr_status_cancelled_terminal");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    REQUIRE(service.changeStatus(id, "Cancelled").success);

    ServiceRequestStatusResult toPending = service.changeStatus(id, "Pending");
    ServiceRequestStatusResult toInProgress = service.changeStatus(id, "In Progress");
    ServiceRequestStatusResult toCompleted = service.changeStatus(id, "Completed");

    CHECK(!toPending.success);
    CHECK(toPending.invalidTransition);
    CHECK(!toInProgress.success);
    CHECK(toInProgress.invalidTransition);
    CHECK(!toCompleted.success);
    CHECK(toCompleted.invalidTransition);
    CHECK(requestRepository.findById(id)->getStatus() == ServiceRequestStatus::Cancelled);
    CHECK(readStatusFromDatabase(path, id) == "Cancelled");
}

// T10 Required Test 9: Unsupported status is rejected
DROGON_TEST(UnsupportedStatusIsRejectedTest)
{
    std::string path = makeTempDbPath("sr_status_unsupported");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    ServiceRequestStatusResult result = service.changeStatus(id, "Approved");

    CHECK(!result.success);
    CHECK(result.unsupportedStatus);
    CHECK(!result.invalidTransition);
    CHECK(!result.notFound);
    CHECK(!result.serviceRequest.has_value());
    CHECK(requestRepository.findById(id)->getStatus() == ServiceRequestStatus::Pending);
    CHECK(readStatusFromDatabase(path, id) == "Pending");
}

// T10 Required Test 10: Nonexistent Service Request is handled safely
DROGON_TEST(NonexistentServiceRequestIsHandledSafelyTest)
{
    std::string path = makeTempDbPath("sr_status_not_found");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int existingId = submitPendingRequest(db, resident.getId().value());

    ServiceRequestStatusResult result = service.changeStatus(999, "In Progress");

    CHECK(!result.success);
    CHECK(result.notFound);
    CHECK(!result.serviceRequest.has_value());
    CHECK(!requestRepository.findById(999).has_value());
    CHECK(countServiceRequestsInDatabase(path) == 1);
    CHECK(requestRepository.findById(existingId)->getStatus() == ServiceRequestStatus::Pending);
}

// T10 Required Test 11: Successful transition preserves Service Request information
DROGON_TEST(SuccessfulTransitionPreservesServiceRequestInformationTest)
{
    std::string path = makeTempDbPath("sr_status_preserves_info");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());
    ServiceRequest before = requestRepository.findById(id).value();

    ServiceRequestStatusResult result = service.changeStatus(id, "In Progress");
    REQUIRE(result.success);

    ServiceRequest returned = result.serviceRequest.value();
    ServiceRequest persisted = requestRepository.findById(id).value();

    for (const ServiceRequest &after : {returned, persisted})
    {
        CHECK(after.getId() == before.getId());
        CHECK(after.getResidentId() == before.getResidentId());
        CHECK(after.getServiceType() == before.getServiceType());
        CHECK(after.getDescription() == before.getDescription());
        CHECK(after.getDateRequested() == before.getDateRequested());
        CHECK(after.getStatus() == ServiceRequestStatus::InProgress);
    }

    // No second request was created by the update.
    CHECK(countServiceRequestsInDatabase(path) == 1);
}

// T10 Required Test 12: Invalid transition does not modify persistence
DROGON_TEST(InvalidTransitionDoesNotModifyPersistenceTest)
{
    std::string path = makeTempDbPath("sr_status_invalid_no_change");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());
    ServiceRequest before = requestRepository.findById(id).value();

    ServiceRequestStatusResult result = service.changeStatus(id, "Completed");
    REQUIRE(!result.success);

    ServiceRequest after = requestRepository.findById(id).value();

    CHECK(after.getId() == before.getId());
    CHECK(after.getResidentId() == before.getResidentId());
    CHECK(after.getServiceType() == before.getServiceType());
    CHECK(after.getDescription() == before.getDescription());
    CHECK(after.getDateRequested() == before.getDateRequested());
    CHECK(after.getStatus() == before.getStatus());
    CHECK(after.getStatus() == ServiceRequestStatus::Pending);
    CHECK(countServiceRequestsInDatabase(path) == 1);
}

// T10 Required Test 13: Same-status request is rejected
DROGON_TEST(SameStatusRequestIsRejectedTest)
{
    std::string path = makeTempDbPath("sr_status_same_status");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    // Pending -> Pending
    ServiceRequestStatusResult pendingAgain = service.changeStatus(id, "Pending");
    CHECK(!pendingAgain.success);
    CHECK(pendingAgain.invalidTransition);
    CHECK(readStatusFromDatabase(path, id) == "Pending");

    // In Progress -> In Progress
    REQUIRE(service.changeStatus(id, "In Progress").success);
    ServiceRequestStatusResult inProgressAgain = service.changeStatus(id, "In Progress");
    CHECK(!inProgressAgain.success);
    CHECK(inProgressAgain.invalidTransition);
    CHECK(readStatusFromDatabase(path, id) == "In Progress");

    // Completed -> Completed
    REQUIRE(service.changeStatus(id, "Completed").success);
    ServiceRequestStatusResult completedAgain = service.changeStatus(id, "Completed");
    CHECK(!completedAgain.success);
    CHECK(completedAgain.invalidTransition);
    CHECK(readStatusFromDatabase(path, id) == "Completed");
}

// T10 Student-Designed Test: changing one request never touches another.
// Three requests are persisted. One is moved through the whole
// Pending -> In Progress -> Completed lifecycle, another is cancelled, and
// the middle one must remain exactly as submitted. This proves the UPDATE
// targets only the intended id, that sequential valid transitions work, and
// that each request keeps its own status and information.
DROGON_TEST(StatusChangeAffectsOnlyTheTargetedServiceRequestTest)
{
    std::string path = makeTempDbPath("sr_status_isolation");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident juan = persistResident(residentRepository, "Juan", "Cruz");
    Resident maria = persistResident(residentRepository, "Maria", "Santos");
    int firstId = submitPendingRequest(db, juan.getId().value());
    int secondId = submitPendingRequest(db, maria.getId().value());
    int thirdId = submitPendingRequest(db, juan.getId().value());
    REQUIRE(firstId != secondId);

    ServiceRequest secondBefore = requestRepository.findById(secondId).value();

    // Whole lifecycle for the first request.
    CHECK(service.changeStatus(firstId, "In Progress").success);
    CHECK(readStatusFromDatabase(path, firstId) == "In Progress");
    CHECK(readStatusFromDatabase(path, secondId) == "Pending");
    CHECK(service.changeStatus(firstId, "Completed").success);

    // A different transition for the third request.
    CHECK(service.changeStatus(thirdId, "Cancelled").success);

    CHECK(readStatusFromDatabase(path, firstId) == "Completed");
    CHECK(readStatusFromDatabase(path, thirdId) == "Cancelled");

    // The second request is completely untouched.
    ServiceRequest secondAfter = requestRepository.findById(secondId).value();
    CHECK(secondAfter.getId() == secondBefore.getId());
    CHECK(secondAfter.getResidentId() == maria.getId().value());
    CHECK(secondAfter.getServiceType() == secondBefore.getServiceType());
    CHECK(secondAfter.getDescription() == secondBefore.getDescription());
    CHECK(secondAfter.getDateRequested() == secondBefore.getDateRequested());
    CHECK(secondAfter.getStatus() == ServiceRequestStatus::Pending);
    CHECK(countServiceRequestsInDatabase(path) == 3);
}

// Additional T10 coverage: an existing request keeps moving through its
// workflow after its Resident is deactivated (T09 only guards NEW requests),
// and the Resident record is not modified by status management.
DROGON_TEST(ExistingRequestCanProgressAfterResidentDeactivatedTest)
{
    Database db(makeTempDbPath("sr_status_inactive_resident"));
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);
    ResidentDeactivationService deactivation(residentRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    REQUIRE(deactivation.deactivateResident(resident.getId().value()).success);

    CHECK(service.changeStatus(id, "In Progress").success);
    CHECK(service.changeStatus(id, "Completed").success);
    CHECK(requestRepository.findById(id)->getStatus() == ServiceRequestStatus::Completed);
    CHECK(residentRepository.findById(resident.getId().value())->getStatus() == ResidentStatus::Inactive);

    // T09 still blocks NEW requests for the Inactive Resident.
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService submission(validator, residentRepository, requestRepository);
    CHECK(submission.submitServiceRequest(makeValidServiceRequest(resident.getId().value())).residentInactive);
}

// Additional T10 coverage: a status change is durable — a brand new
// Database connection and repository still see it.
DROGON_TEST(StatusChangePersistsAcrossRepositoryInstancesTest)
{
    std::string path = makeTempDbPath("sr_status_durable");
    int id = 0;

    {
        Database db(path);
        ResidentRepository residentRepository(db);
        ServiceRequestRepository requestRepository(db);
        ServiceRequestStatusService service(requestRepository);

        Resident resident = persistResident(residentRepository, "Juan", "Cruz");
        id = submitPendingRequest(db, resident.getId().value());
        REQUIRE(service.changeStatus(id, "In Progress").success);
    }

    Database reopened(path);
    ServiceRequestRepository reopenedRepository(reopened);
    ServiceRequestStatusService reopenedService(reopenedRepository);

    CHECK(reopenedRepository.findById(id)->getStatus() == ServiceRequestStatus::InProgress);
    CHECK(reopenedService.changeStatus(id, "Completed").success);
}

// Additional T10 coverage: only the exact supported spellings are accepted.
DROGON_TEST(StatusTextMustMatchSupportedValuesExactlyTest)
{
    std::string path = makeTempDbPath("sr_status_exact_text");
    Database db(path);
    ResidentRepository residentRepository(db);
    ServiceRequestRepository requestRepository(db);
    ServiceRequestStatusService service(requestRepository);

    Resident resident = persistResident(residentRepository, "Juan", "Cruz");
    int id = submitPendingRequest(db, resident.getId().value());

    for (const std::string &bad : {"", "in progress", "InProgress", "Processing", "Done", "Closed", "On Hold"})
    {
        ServiceRequestStatusResult result = service.changeStatus(id, bad);
        CHECK(!result.success);
        CHECK(result.unsupportedStatus);
    }

    CHECK(readStatusFromDatabase(path, id) == "Pending");
}

// Additional T10 coverage: the repository update only reports success for an
// id that exists and never inserts a row.
DROGON_TEST(RepositoryUpdateStatusIgnoresUnknownIdTest)
{
    std::string path = makeTempDbPath("sr_status_repo_unknown");
    Database db(path);
    ServiceRequestRepository requestRepository(db);

    CHECK(!requestRepository.updateStatus(42, ServiceRequestStatus::Completed));
    CHECK(countServiceRequestsInDatabase(path) == 0);
}

// ---------------------------------------------------------------------------
// T11 - Search and View Service Requests
// ---------------------------------------------------------------------------

// Persists a Service Request directly through the repository, for setting up
// query scenarios (bypasses submission/validation on purpose, since T11 is
// testing read behavior, and lets a test choose date and status freely).
ServiceRequest saveQueryRequest(ServiceRequestRepository &repository,
                                const std::string &serviceType,
                                const std::string &description,
                                const std::string &dateRequested,
                                ServiceRequestStatus status = ServiceRequestStatus::Pending,
                                int residentId = 1)
{
    return repository.save(ServiceRequest(residentId, serviceType, description, dateRequested, status));
}

std::vector<int> idsOf(const std::vector<ServiceRequest> &requests)
{
    std::vector<int> ids;
    for (const ServiceRequest &request : requests)
    {
        ids.push_back(request.getId().value());
    }
    return ids;
}

// T11 Required Test 1: List all persisted Service Requests
DROGON_TEST(ListAllPersistedServiceRequestsTest)
{
    Database db(makeTempDbPath("sr_query_list_all"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Barangay Clearance", "For employment", "2026-09-10");
    saveQueryRequest(repository, "Community Assistance", "Medical support", "2026-09-11");
    saveQueryRequest(repository, "Business Permit", "New stall", "2026-09-12");

    CHECK(service.listServiceRequests().size() == 3);
}

// T11 Required Test 2: Empty listing returns an empty collection
DROGON_TEST(EmptyServiceRequestListingTest)
{
    std::string path = makeTempDbPath("sr_query_list_empty");
    Database db(path);
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    CHECK(service.listServiceRequests().empty());
    CHECK(countServiceRequestsInDatabase(path) == 0);
}

// T11 Required Test 3: Listing uses dateRequested DESC, then id DESC
DROGON_TEST(ServiceRequestListingUsesRequiredOrderingTest)
{
    Database db(makeTempDbPath("sr_query_ordering"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    // Inserted in an order that differs from the required result.
    int id1 = saveQueryRequest(repository, "Type A", "Desc", "2026-09-15").getId().value();
    int id2 = saveQueryRequest(repository, "Type B", "Desc", "2026-09-18").getId().value();
    int id3 = saveQueryRequest(repository, "Type C", "Desc", "2026-09-10").getId().value();
    int id4 = saveQueryRequest(repository, "Type D", "Desc", "2026-09-18").getId().value();

    std::vector<int> expected = {id4, id2, id1, id3}; // 09-18 (higher id first), 09-18, 09-15, 09-10
    CHECK(idsOf(service.listServiceRequests()) == expected);
}

// T11 Required Test 4: View an existing Service Request by ID
DROGON_TEST(ViewExistingServiceRequestTest)
{
    Database db(makeTempDbPath("sr_query_view"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Barangay Clearance", "First", "2026-09-10");
    ServiceRequest target = saveQueryRequest(repository, "Community Assistance", "Second", "2026-09-11");

    std::optional<ServiceRequest> viewed = service.viewServiceRequest(target.getId().value());

    REQUIRE(viewed.has_value());
    CHECK(viewed->getId().value() == target.getId().value());
    CHECK(viewed->getServiceType() == "Community Assistance");
}

// T11 Required Test 5: Viewing an unknown ID is handled safely
DROGON_TEST(ViewUnknownServiceRequestIsHandledSafelyTest)
{
    std::string path = makeTempDbPath("sr_query_view_unknown");
    Database db(path);
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Barangay Clearance", "Existing", "2026-09-10");

    CHECK(!service.viewServiceRequest(999).has_value());
    CHECK(!service.viewServiceRequest(0).has_value());
    CHECK(!service.viewServiceRequest(-5).has_value());
    CHECK(countServiceRequestsInDatabase(path) == 1); // nothing was created
}

// T11 Required Test 6: Partial service-type search is case-insensitive
DROGON_TEST(PartialServiceTypeSearchIsCaseInsensitiveTest)
{
    Database db(makeTempDbPath("sr_query_type_search"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    int clearanceId = saveQueryRequest(repository, "Barangay Clearance", "Employment", "2026-09-10").getId().value();
    saveQueryRequest(repository, "Community Assistance", "Medical support", "2026-09-11");

    for (const std::string &term : {"cLeAr", "CLEAR", "clear"})
    {
        ServiceRequestQueryResult result = service.searchServiceRequests(term);
        REQUIRE(result.success);
        CHECK(idsOf(result.serviceRequests) == std::vector<int>{clearanceId});
    }
}

// T11 Required Test 7: Partial description search is case-insensitive
DROGON_TEST(PartialDescriptionSearchIsCaseInsensitiveTest)
{
    Database db(makeTempDbPath("sr_query_description_search"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    int targetId = saveQueryRequest(repository, "Barangay Clearance",
                                    "Requesting barangay clearance for employment requirements.",
                                    "2026-09-10").getId().value();
    saveQueryRequest(repository, "Community Assistance", "Medical support", "2026-09-11");

    ServiceRequestQueryResult result = service.searchServiceRequests("EmPlOyMeNt");

    REQUIRE(result.success);
    CHECK(idsOf(result.serviceRequests) == std::vector<int>{targetId});
}

// T11 Required Test 8: Blank search returns the same requests as listing
DROGON_TEST(BlankServiceRequestSearchReturnsAllInListingOrderTest)
{
    Database db(makeTempDbPath("sr_query_blank"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Barangay Clearance", "A", "2026-09-15");
    saveQueryRequest(repository, "Community Assistance", "B", "2026-09-18");
    saveQueryRequest(repository, "Business Permit", "C", "2026-09-18", ServiceRequestStatus::Completed);

    std::vector<int> listed = idsOf(service.listServiceRequests());
    REQUIRE(listed.size() == 3);

    for (const std::string &blank : {"", " ", "      ", "\t \n"})
    {
        ServiceRequestQueryResult result = service.searchServiceRequests(blank);
        REQUIRE(result.success);
        CHECK(idsOf(result.serviceRequests) == listed);
    }

    // Omitting both arguments is also a plain listing.
    CHECK(idsOf(service.searchServiceRequests().serviceRequests) == listed);
}

// T11 Required Test 9: No text match returns an empty collection
DROGON_TEST(ServiceRequestSearchWithNoMatchReturnsEmptyCollectionTest)
{
    Database db(makeTempDbPath("sr_query_no_match"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Barangay Clearance", "Employment", "2026-09-10");

    ServiceRequestQueryResult result = service.searchServiceRequests("zzz-not-present");

    CHECK(result.success);
    CHECK(!result.unsupportedStatus);
    CHECK(result.serviceRequests.empty());
}

// T11 Required Test 10: Status filter returns only the matching status
DROGON_TEST(StatusFilterReturnsOnlyMatchingStatusTest)
{
    Database db(makeTempDbPath("sr_query_status_filter"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Type A", "Desc", "2026-09-10", ServiceRequestStatus::Pending);
    saveQueryRequest(repository, "Type B", "Desc", "2026-09-11", ServiceRequestStatus::InProgress);
    saveQueryRequest(repository, "Type C", "Desc", "2026-09-12", ServiceRequestStatus::Pending);
    saveQueryRequest(repository, "Type D", "Desc", "2026-09-13", ServiceRequestStatus::Completed);
    saveQueryRequest(repository, "Type E", "Desc", "2026-09-14", ServiceRequestStatus::Cancelled);

    ServiceRequestQueryResult pending = service.searchServiceRequests("", "Pending");
    REQUIRE(pending.success);
    CHECK(pending.serviceRequests.size() == 2);
    for (const ServiceRequest &request : pending.serviceRequests)
    {
        CHECK(request.getStatus() == ServiceRequestStatus::Pending);
    }

    // Each of the other three statuses works as a filter too.
    CHECK(service.searchServiceRequests("", "In Progress").serviceRequests.size() == 1);
    CHECK(service.searchServiceRequests("", "Completed").serviceRequests.size() == 1);
    CHECK(service.searchServiceRequests("", "Cancelled").serviceRequests.size() == 1);
}

// T11 Required Test 11: Unfiltered query includes all lifecycle states
DROGON_TEST(UnfilteredQueryIncludesAllStatusesTest)
{
    Database db(makeTempDbPath("sr_query_all_statuses"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Type A", "Desc", "2026-09-10", ServiceRequestStatus::Pending);
    saveQueryRequest(repository, "Type B", "Desc", "2026-09-11", ServiceRequestStatus::InProgress);
    saveQueryRequest(repository, "Type C", "Desc", "2026-09-12", ServiceRequestStatus::Completed);
    saveQueryRequest(repository, "Type D", "Desc", "2026-09-13", ServiceRequestStatus::Cancelled);

    std::vector<ServiceRequest> listed = service.listServiceRequests();
    std::vector<ServiceRequest> searched = service.searchServiceRequests("desc").serviceRequests;

    for (const std::vector<ServiceRequest> *results : {&listed, &searched})
    {
        REQUIRE(results->size() == 4);
        std::set<ServiceRequestStatus> statuses;
        for (const ServiceRequest &request : *results)
        {
            statuses.insert(request.getStatus());
        }
        CHECK(statuses.size() == 4);
    }
}

// T11 Required Test 12: Text search and status filter combined
DROGON_TEST(CombinedTextSearchAndStatusFilterTest)
{
    Database db(makeTempDbPath("sr_query_combined"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    int both = saveQueryRequest(repository, "Barangay Clearance", "Employment requirement",
                                "2026-09-10", ServiceRequestStatus::Pending).getId().value();
    saveQueryRequest(repository, "Barangay Clearance", "Scholarship requirement",
                     "2026-09-11", ServiceRequestStatus::Completed);   // text only
    saveQueryRequest(repository, "Community Assistance", "Employment support",
                     "2026-09-12", ServiceRequestStatus::Pending);     // status only
    saveQueryRequest(repository, "Business Permit", "New stall",
                     "2026-09-13", ServiceRequestStatus::Cancelled);   // neither

    ServiceRequestQueryResult result = service.searchServiceRequests("clearance", "Pending");

    REQUIRE(result.success);
    CHECK(idsOf(result.serviceRequests) == std::vector<int>{both});
}

// T11 Required Test 13: A request matching in both fields appears once
DROGON_TEST(MatchingServiceRequestIsNotDuplicatedTest)
{
    Database db(makeTempDbPath("sr_query_no_duplicate"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    int id = saveQueryRequest(repository, "Community Assistance",
                              "Assistance requested for medical support", "2026-09-10").getId().value();
    saveQueryRequest(repository, "Business Permit", "New stall", "2026-09-11");

    ServiceRequestQueryResult result = service.searchServiceRequests("assistance");

    REQUIRE(result.success);
    CHECK(idsOf(result.serviceRequests) == std::vector<int>{id});
}

// T11 Required Test 14: Query results preserve complete Service Request information
DROGON_TEST(QueryResultsPreserveServiceRequestInformationTest)
{
    Database db(makeTempDbPath("sr_query_preserve"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    ServiceRequest saved = saveQueryRequest(repository, "Barangay Clearance",
                                            "Needed for employment requirements.",
                                            "2026-09-25", ServiceRequestStatus::InProgress, 7);
    int id = saved.getId().value();

    std::vector<ServiceRequest> fromList = service.listServiceRequests();
    std::vector<ServiceRequest> fromSearch = service.searchServiceRequests("clearance", "In Progress").serviceRequests;
    std::optional<ServiceRequest> fromView = service.viewServiceRequest(id);

    REQUIRE(fromList.size() == 1);
    REQUIRE(fromSearch.size() == 1);
    REQUIRE(fromView.has_value());

    for (const ServiceRequest *request : {&fromList[0], &fromSearch[0], &*fromView})
    {
        CHECK(request->getId().value() == id);
        CHECK(request->getResidentId() == 7);
        CHECK(request->getServiceType() == "Barangay Clearance");
        CHECK(request->getDescription() == "Needed for employment requirements.");
        CHECK(request->getDateRequested() == "2026-09-25");
        CHECK(request->getStatus() == ServiceRequestStatus::InProgress);
    }
}

// Additional T11 coverage: unsupported non-blank status filters are rejected,
// not silently treated as "no filter".
DROGON_TEST(UnsupportedStatusFilterIsRejectedTest)
{
    Database db(makeTempDbPath("sr_query_bad_status"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Barangay Clearance", "Employment", "2026-09-10");

    for (const std::string &bad : {"Approved", "Rejected", "Closed", "Done", "Processing", "pending", "InProgress"})
    {
        ServiceRequestQueryResult result = service.searchServiceRequests("clearance", bad);
        CHECK(!result.success);
        CHECK(result.unsupportedStatus);
        CHECK(result.serviceRequests.empty());
    }
}

// Additional T11 coverage: a blank status filter means every status.
DROGON_TEST(BlankStatusFilterMeansAllStatusesTest)
{
    Database db(makeTempDbPath("sr_query_blank_status"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Type A", "Desc", "2026-09-10", ServiceRequestStatus::Pending);
    saveQueryRequest(repository, "Type B", "Desc", "2026-09-11", ServiceRequestStatus::Cancelled);

    for (const std::string &blank : {"", "   "})
    {
        ServiceRequestQueryResult result = service.searchServiceRequests("", blank);
        CHECK(result.success);
        CHECK(result.serviceRequests.size() == 2);
    }
}

// Additional T11 coverage: leading/trailing whitespace on the search term is
// ignored, and the stored text is not changed.
DROGON_TEST(SearchTermWhitespaceIsIgnoredTest)
{
    Database db(makeTempDbPath("sr_query_trim"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    int id = saveQueryRequest(repository, "Barangay Clearance", "Employment", "2026-09-10").getId().value();

    ServiceRequestQueryResult result = service.searchServiceRequests("   clearance   ");

    REQUIRE(result.success);
    CHECK(idsOf(result.serviceRequests) == std::vector<int>{id});
    CHECK(service.viewServiceRequest(id)->getServiceType() == "Barangay Clearance");
}

// Additional T11 coverage: text search covers only serviceType and description.
DROGON_TEST(TextSearchDoesNotCoverOtherFieldsTest)
{
    Database db(makeTempDbPath("sr_query_other_fields"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    saveQueryRequest(repository, "Barangay Clearance", "Employment",
                     "2026-09-10", ServiceRequestStatus::Pending, 42);

    CHECK(service.searchServiceRequests("42").serviceRequests.empty());       // residentId
    CHECK(service.searchServiceRequests("2026-09").serviceRequests.empty());  // dateRequested
    CHECK(service.searchServiceRequests("Pending").serviceRequests.empty());  // status
}

// Additional T11 coverage: search input is bound as data, so SQL wildcard and
// injection characters match literally instead of changing the query.
DROGON_TEST(SearchTermWildcardsAndQuotesAreLiteralTest)
{
    std::string path = makeTempDbPath("sr_query_literal");
    Database db(path);
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    int percentId = saveQueryRequest(repository, "Discount", "Needs 50% subsidy", "2026-09-10").getId().value();
    saveQueryRequest(repository, "Barangay Clearance", "Employment", "2026-09-11");

    CHECK(idsOf(service.searchServiceRequests("%").serviceRequests) == std::vector<int>{percentId});
    CHECK(service.searchServiceRequests("_").serviceRequests.empty());
    CHECK(service.searchServiceRequests("' OR '1'='1").serviceRequests.empty());
    CHECK(service.searchServiceRequests("x'; DROP TABLE service_requests; --").serviceRequests.empty());
    CHECK(countServiceRequestsInDatabase(path) == 2);
}

// Additional T11 coverage: querying is read-only and works across statuses,
// including terminal ones, without changing any stored status.
DROGON_TEST(QueryingDoesNotModifyServiceRequestsTest)
{
    std::string path = makeTempDbPath("sr_query_read_only");
    Database db(path);
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    int completedId = saveQueryRequest(repository, "Barangay Clearance", "Done", "2026-09-10",
                                       ServiceRequestStatus::Completed).getId().value();
    int cancelledId = saveQueryRequest(repository, "Barangay Clearance", "Dropped", "2026-09-11",
                                       ServiceRequestStatus::Cancelled).getId().value();

    service.listServiceRequests();
    service.viewServiceRequest(completedId);
    service.searchServiceRequests("clearance", "Completed");
    service.searchServiceRequests("clearance", "Cancelled");
    service.searchServiceRequests("clearance", "Approved");

    CHECK(countServiceRequestsInDatabase(path) == 2);
    CHECK(readStatusFromDatabase(path, completedId) == "Completed");
    CHECK(readStatusFromDatabase(path, cancelledId) == "Cancelled");
    CHECK(idsOf(service.searchServiceRequests("clearance", "Completed").serviceRequests) == std::vector<int>{completedId});
    CHECK(idsOf(service.searchServiceRequests("clearance", "Cancelled").serviceRequests) == std::vector<int>{cancelledId});
}

// Additional T11 coverage: search and filter results use the same ordering as
// listing (dateRequested DESC, then id DESC).
DROGON_TEST(SearchResultsUseRequiredOrderingTest)
{
    Database db(makeTempDbPath("sr_query_search_ordering"));
    ServiceRequestRepository repository(db);
    ServiceRequestQueryService service(repository);

    int id1 = saveQueryRequest(repository, "Barangay Clearance", "A", "2026-09-15").getId().value();
    int id2 = saveQueryRequest(repository, "Barangay Clearance", "B", "2026-09-18").getId().value();
    int id3 = saveQueryRequest(repository, "Barangay Clearance", "C", "2026-09-10").getId().value();
    int id4 = saveQueryRequest(repository, "Barangay Clearance", "D", "2026-09-18").getId().value();

    std::vector<int> expected = {id4, id2, id1, id3};

    CHECK(idsOf(service.searchServiceRequests("clearance").serviceRequests) == expected);
    CHECK(idsOf(service.searchServiceRequests("clearance", "Pending").serviceRequests) == expected);
    CHECK(idsOf(service.searchServiceRequests("", "Pending").serviceRequests) == expected);
}

// ---------------------------------------------------------------------------
// T12 - Resident Management Interface (presentation/integration tests)
//
// These tests drive ResidentController with real HttpRequest objects, so the
// whole path is exercised: controller -> application services -> validator ->
// repository -> SQLite -> rendered view. ResidentServices is pointed at a
// throwaway database file, and Residents are persisted through the existing
// T04 registration service.
// ---------------------------------------------------------------------------

namespace t12
{
using Params = std::map<std::string, std::string>;

HttpRequestPtr makeRequest(HttpMethod method, const Params &params)
{
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(method);
    for (const auto &entry : params)
    {
        req->setParameter(entry.first, entry.second);
    }
    return req;
}

// The controller invokes its callback before returning, so a plain local
// variable is enough to capture the response.
HttpResponsePtr listPage(const Params &params = {})
{
    HttpResponsePtr response;
    ResidentController().list(makeRequest(Get, params),
                              [&](const HttpResponsePtr &r) { response = r; });
    return response;
}

HttpResponsePtr registerForm()
{
    HttpResponsePtr response;
    ResidentController().showRegisterForm(makeRequest(Get, {}),
                                          [&](const HttpResponsePtr &r) { response = r; });
    return response;
}

HttpResponsePtr submitRegistration(const Params &params)
{
    HttpResponsePtr response;
    ResidentController().registerResident(makeRequest(Post, params),
                                          [&](const HttpResponsePtr &r) { response = r; });
    return response;
}

HttpResponsePtr editForm(const std::string &id)
{
    HttpResponsePtr response;
    ResidentController().showEditForm(makeRequest(Get, {}),
                                      [&](const HttpResponsePtr &r) { response = r; },
                                      id);
    return response;
}

HttpResponsePtr submitUpdate(const std::string &id, const Params &params)
{
    HttpResponsePtr response;
    ResidentController().updateResident(makeRequest(Post, params),
                                        [&](const HttpResponsePtr &r) { response = r; },
                                        id);
    return response;
}

HttpResponsePtr submitDeactivation(const std::string &id)
{
    HttpResponsePtr response;
    ResidentController().deactivateResident(makeRequest(Post, {}),
                                            [&](const HttpResponsePtr &r) { response = r; },
                                            id);
    return response;
}

std::string bodyOf(const HttpResponsePtr &response)
{
    return std::string(response->body());
}

bool contains(const std::string &text, const std::string &fragment)
{
    return text.find(fragment) != std::string::npos;
}

Params validForm(const std::string &firstName = "Maria",
                 const std::string &lastName = "Santos")
{
    return {{"firstName", firstName},
            {"lastName", lastName},
            {"address", "12 Rizal Street, Carmona, Cavite"},
            {"contactNumber", "09171234567"},
            {"email", "resident@example.com"}};
}

// Starts a fresh database for the web layer and returns the persisted id.
void useFreshDatabase(const std::string &name)
{
    ResidentServices::useDatabase(makeTempDbPath(name));
}

int persistResident(const std::string &firstName,
                    const std::string &lastName,
                    ResidentStatus status = ResidentStatus::Active)
{
    // Emails cannot contain spaces, so strip them from the name-based address.
    std::string emailName = firstName + "." + lastName;
    emailName.erase(std::remove(emailName.begin(), emailName.end(), ' '), emailName.end());

    Resident resident(firstName,
                      lastName,
                      "12 Rizal Street, Carmona, Cavite",
                      "09171234567",
                      emailName + "@example.com",
                      status);
    auto result = ResidentServices::instance().registration().registerResident(resident);
    return result.resident->getId().value();
}

std::string statusOf(int id)
{
    return residentStatusToString(ResidentServices::instance().repository().findById(id)->getStatus());
}

size_t residentCount()
{
    return ResidentServices::instance().repository().findAll().size();
}
} // namespace t12

// T12 Required Test 1: Resident Management Page Loads Successfully
DROGON_TEST(T12ResidentManagementPageLoadsTest)
{
    t12::useFreshDatabase("t12_page_loads");

    auto response = t12::listPage();
    REQUIRE(response != nullptr);
    CHECK(response->statusCode() == k200OK);

    const std::string body = t12::bodyOf(response);
    CHECK(t12::contains(body, "<h1>Resident Management</h1>"));
    CHECK(t12::contains(body, "Register Resident"));
    CHECK(t12::contains(body, "name=\"q\""));
    CHECK(t12::contains(body, ">Search</button>"));
}

// An empty database must render a friendly message, not an error or "null".
DROGON_TEST(T12EmptyResidentListRendersMessageTest)
{
    t12::useFreshDatabase("t12_empty");

    auto response = t12::listPage();
    CHECK(response->statusCode() == k200OK);

    const std::string body = t12::bodyOf(response);
    CHECK(t12::contains(body, "No Residents found."));
    CHECK(!t12::contains(body, "null"));
    CHECK(!t12::contains(body, "<table"));
}

// T12 Required Test 2: Resident List Displays Persisted Residents
DROGON_TEST(T12ResidentListDisplaysPersistedResidentsTest)
{
    t12::useFreshDatabase("t12_list");
    int mariaId = t12::persistResident("Maria", "Santos");
    t12::persistResident("Juan", "Dela Cruz");

    const std::string body = t12::bodyOf(t12::listPage());

    CHECK(t12::contains(body, ">Maria<"));
    CHECK(t12::contains(body, ">Santos<"));
    CHECK(t12::contains(body, ">Juan<"));
    CHECK(t12::contains(body, ">Dela Cruz<"));
    CHECK(t12::contains(body, "12 Rizal Street, Carmona, Cavite"));
    CHECK(t12::contains(body, "09171234567"));
    CHECK(t12::contains(body, "Maria.Santos@example.com"));
    CHECK(t12::contains(body, ">Active<"));
    CHECK(t12::contains(body, "/residents/" + std::to_string(mariaId) + "/edit"));
}

// The page must preserve the T05 order: lastName, firstName, id.
DROGON_TEST(T12ResidentListKeepsT05OrderingTest)
{
    t12::useFreshDatabase("t12_ordering");
    t12::persistResident("Zed", "Santos");
    t12::persistResident("Ana", "Santos");
    t12::persistResident("Maria", "Abad");

    const std::string body = t12::bodyOf(t12::listPage());

    const size_t abad = body.find(">Abad<");
    const size_t anaSantos = body.find(">Ana<");
    const size_t zedSantos = body.find(">Zed<");
    REQUIRE(abad != std::string::npos);
    REQUIRE(anaSantos != std::string::npos);
    REQUIRE(zedSantos != std::string::npos);
    CHECK(abad < anaSantos);
    CHECK(anaSantos < zedSantos);
}

// T12 Required Test 3: Resident Search Uses Existing Search Behavior
DROGON_TEST(T12ResidentSearchUsesExistingSearchTest)
{
    t12::useFreshDatabase("t12_search");
    t12::persistResident("Maria", "Santos");
    t12::persistResident("Juan", "Dela Cruz");

    const std::string byFirst = t12::bodyOf(t12::listPage({{"q", "MAR"}}));
    CHECK(t12::contains(byFirst, ">Maria<"));
    CHECK(!t12::contains(byFirst, ">Juan<"));

    const std::string byLast = t12::bodyOf(t12::listPage({{"q", "   santos   "}}));
    CHECK(t12::contains(byLast, ">Santos<"));
    CHECK(!t12::contains(byLast, ">Dela Cruz<"));

    // The submitted term stays visible in the search box.
    CHECK(t12::contains(byFirst, "value=\"MAR\""));
}

// A search with no matches is a normal page, not an error.
DROGON_TEST(T12NoMatchSearchRendersMessageTest)
{
    t12::useFreshDatabase("t12_no_match");
    t12::persistResident("Maria", "Santos");

    auto response = t12::listPage({{"q", "zzzz"}});
    CHECK(response->statusCode() == k200OK);

    const std::string body = t12::bodyOf(response);
    CHECK(t12::contains(body, "No Residents found."));
    CHECK(!t12::contains(body, ">Maria<"));
}

// T12 Required Test 4: Blank Search Displays All Residents
DROGON_TEST(T12BlankSearchDisplaysAllResidentsTest)
{
    t12::useFreshDatabase("t12_blank_search");
    t12::persistResident("Maria", "Santos");
    t12::persistResident("Juan", "Dela Cruz");

    for (const std::string term : {"", " ", "       "})
    {
        auto response = t12::listPage({{"q", term}});
        CHECK(response->statusCode() == k200OK);

        const std::string body = t12::bodyOf(response);
        CHECK(t12::contains(body, ">Maria<"));
        CHECK(t12::contains(body, ">Juan<"));
    }
}

// T12 Required Test 5: Registration Form Is Accessible
DROGON_TEST(T12RegistrationFormIsAccessibleTest)
{
    t12::useFreshDatabase("t12_register_form");

    auto response = t12::registerForm();
    CHECK(response->statusCode() == k200OK);

    const std::string body = t12::bodyOf(response);
    for (const std::string field : {"firstName", "lastName", "address", "contactNumber", "email"})
    {
        CHECK(t12::contains(body, "name=\"" + field + "\""));
        CHECK(t12::contains(body, "<label for=\"" + field + "\">"));
    }
    CHECK(t12::contains(body, "method=\"post\""));
    CHECK(t12::contains(body, "action=\"/residents/register\""));

    // The user supplies neither an id nor a status.
    CHECK(!t12::contains(body, "name=\"id\""));
    CHECK(!t12::contains(body, "name=\"status\""));
    CHECK(t12::residentCount() == 0); // opening the form creates nothing
}

// T12 Required Test 6: Valid Registration Persists Resident
DROGON_TEST(T12ValidRegistrationPersistsResidentTest)
{
    t12::useFreshDatabase("t12_valid_register");

    // Extra id/status values in the request must be ignored.
    t12::Params form = t12::validForm();
    form["id"] = "999";
    form["status"] = "Inactive";

    auto response = t12::submitRegistration(form);

    // Post/Redirect/Get: success redirects instead of rendering directly.
    CHECK(response->statusCode() == k303SeeOther);
    CHECK(response->getHeader("location") == "/residents?msg=registered");

    auto all = ResidentServices::instance().repository().findAll();
    REQUIRE(all.size() == 1);
    CHECK(all[0].getId().has_value());
    CHECK(all[0].getId().value() != 999);
    CHECK(all[0].getFirstName() == "Maria");
    CHECK(all[0].getStatus() == ResidentStatus::Active);

    // Following the redirect shows the success message and the new Resident.
    const std::string body = t12::bodyOf(t12::listPage({{"msg", "registered"}}));
    CHECK(t12::contains(body, "Resident registered successfully."));
    CHECK(t12::contains(body, ">Maria<"));
}

// T12 Required Test 7: Invalid Registration Shows Validation Feedback and Does Not Persist
DROGON_TEST(T12InvalidRegistrationShowsFeedbackAndDoesNotPersistTest)
{
    t12::useFreshDatabase("t12_invalid_register");

    t12::Params form = t12::validForm("", "Cruz");
    form["contactNumber"] = "12345";
    form["email"] = "not-an-email";

    auto response = t12::submitRegistration(form);

    CHECK(response->statusCode() == k422UnprocessableEntity);

    const std::string body = t12::bodyOf(response);
    CHECK(t12::contains(body, "First name is required."));
    CHECK(t12::contains(body, "Contact number must follow 09XXXXXXXXX."));
    CHECK(t12::contains(body, "Email is invalid."));

    // Previously typed values are preserved so the user need not retype them.
    CHECK(t12::contains(body, "value=\"Cruz\""));
    CHECK(t12::contains(body, "value=\"12 Rizal Street, Carmona, Cavite\""));
    CHECK(t12::contains(body, "value=\"not-an-email\""));

    CHECK(t12::residentCount() == 0);
}

// User-entered text must be escaped when the form is redisplayed.
DROGON_TEST(T12FormEscapesEnteredValuesTest)
{
    t12::useFreshDatabase("t12_escape");

    t12::Params form = t12::validForm("<script>alert(1)</script>", "Cruz");
    form["email"] = "bad";

    const std::string body = t12::bodyOf(t12::submitRegistration(form));

    CHECK(!t12::contains(body, "<script>alert(1)</script>"));
    CHECK(t12::contains(body, "&lt;script&gt;alert(1)&lt;/script&gt;"));
}

// T12 Required Test 8: Edit Form Displays Existing Resident Information
DROGON_TEST(T12EditFormDisplaysExistingResidentTest)
{
    t12::useFreshDatabase("t12_edit_form");
    int id = t12::persistResident("Maria", "Santos");

    auto response = t12::editForm(std::to_string(id));
    CHECK(response->statusCode() == k200OK);

    const std::string body = t12::bodyOf(response);
    CHECK(t12::contains(body, "value=\"Maria\""));
    CHECK(t12::contains(body, "value=\"Santos\""));
    CHECK(t12::contains(body, "value=\"12 Rizal Street, Carmona, Cavite\""));
    CHECK(t12::contains(body, "value=\"09171234567\""));
    CHECK(t12::contains(body, "value=\"Maria.Santos@example.com\""));

    // ID and status are shown as plain text, never as editable fields.
    CHECK(t12::contains(body, "Resident ID: <span>" + std::to_string(id) + "</span>"));
    CHECK(t12::contains(body, "Status: <span>Active</span>"));
    CHECK(!t12::contains(body, "name=\"id\""));
    CHECK(!t12::contains(body, "name=\"status\""));
}

// T12 Required Test 9: Valid Update Persists Changes
DROGON_TEST(T12ValidUpdatePersistsChangesTest)
{
    t12::useFreshDatabase("t12_valid_update");
    int id = t12::persistResident("Maria", "Santos");
    ResidentServices::instance().deactivation().deactivateResident(id);

    t12::Params form = t12::validForm("Maria Luisa", "Santos-Reyes");
    form["address"] = "99 New Street";
    form["status"] = "Active"; // must be ignored by the update
    form["id"] = "12345";      // must be ignored by the update

    auto response = t12::submitUpdate(std::to_string(id), form);

    CHECK(response->statusCode() == k303SeeOther);
    CHECK(response->getHeader("location") == "/residents?msg=updated");

    auto stored = ResidentServices::instance().repository().findById(id);
    REQUIRE(stored.has_value());
    CHECK(stored->getId().value() == id);
    CHECK(stored->getFirstName() == "Maria Luisa");
    CHECK(stored->getLastName() == "Santos-Reyes");
    CHECK(stored->getAddress() == "99 New Street");
    CHECK(stored->getStatus() == ResidentStatus::Inactive); // status preserved
    CHECK(t12::residentCount() == 1);

    const std::string body = t12::bodyOf(t12::listPage({{"msg", "updated"}}));
    CHECK(t12::contains(body, "Resident information updated successfully."));
    CHECK(t12::contains(body, ">Maria Luisa<"));
}

// T12 Required Test 10: Invalid Update Does Not Modify Resident
DROGON_TEST(T12InvalidUpdateDoesNotModifyResidentTest)
{
    t12::useFreshDatabase("t12_invalid_update");
    int id = t12::persistResident("Maria", "Santos");

    t12::Params form = t12::validForm("Changed", "Name");
    form["contactNumber"] = "bad";

    auto response = t12::submitUpdate(std::to_string(id), form);

    CHECK(response->statusCode() == k422UnprocessableEntity);

    const std::string body = t12::bodyOf(response);
    CHECK(t12::contains(body, "Contact number must follow 09XXXXXXXXX."));
    // The form stays available for correction with the attempted values.
    CHECK(t12::contains(body, "value=\"Changed\""));
    CHECK(t12::contains(body, "Resident ID: <span>" + std::to_string(id) + "</span>"));

    auto stored = ResidentServices::instance().repository().findById(id);
    REQUIRE(stored.has_value());
    CHECK(stored->getFirstName() == "Maria");
    CHECK(stored->getLastName() == "Santos");
    CHECK(stored->getContactNumber() == "09171234567");
}

// T12 Required Test 11: Missing Resident Edit Is Handled Safely
DROGON_TEST(T12MissingResidentEditIsHandledSafelyTest)
{
    t12::useFreshDatabase("t12_missing");

    for (const std::string id : {"999", "0", "-5", "abc", "99999999999999999999", ""})
    {
        auto response = t12::editForm(id);
        REQUIRE(response != nullptr);
        CHECK(response->statusCode() == k404NotFound);
        CHECK(t12::contains(t12::bodyOf(response), "Resident Not Found"));
        CHECK(t12::contains(t12::bodyOf(response), "href=\"/residents\""));
    }

    // Submitting an update or deactivation for a missing Resident also 404s
    // and creates nothing.
    CHECK(t12::submitUpdate("999", t12::validForm())->statusCode() == k404NotFound);
    CHECK(t12::submitDeactivation("999")->statusCode() == k404NotFound);
    CHECK(t12::residentCount() == 0);
}

// T12 Required Test 12: Active Resident Can Be Deactivated Through the Interface
DROGON_TEST(T12ActiveResidentCanBeDeactivatedTest)
{
    t12::useFreshDatabase("t12_deactivate");
    int id = t12::persistResident("Maria", "Santos");

    // Active Residents are offered a Deactivate action (a POST form).
    const std::string before = t12::bodyOf(t12::listPage());
    CHECK(t12::contains(before, "action=\"/residents/" + std::to_string(id) + "/deactivate\""));
    CHECK(t12::contains(before, "Deactivate</button>"));

    auto response = t12::submitDeactivation(std::to_string(id));

    CHECK(response->statusCode() == k303SeeOther);
    CHECK(response->getHeader("location") == "/residents?msg=deactivated");

    auto stored = ResidentServices::instance().repository().findById(id);
    REQUIRE(stored.has_value()); // soft deactivation: the row still exists
    CHECK(stored->getStatus() == ResidentStatus::Inactive);
    CHECK(stored->getFirstName() == "Maria");
    CHECK(t12::residentCount() == 1);

    CHECK(t12::contains(t12::bodyOf(t12::listPage({{"msg", "deactivated"}})),
                        "Resident deactivated successfully."));
}

// A repeated deactivation is safe because T07, not the button, enforces it.
DROGON_TEST(T12RepeatedDeactivationIsSafeTest)
{
    t12::useFreshDatabase("t12_repeat_deactivate");
    int id = t12::persistResident("Maria", "Santos");

    t12::submitDeactivation(std::to_string(id));
    auto second = t12::submitDeactivation(std::to_string(id));

    CHECK(second->statusCode() == k303SeeOther);
    CHECK(second->getHeader("location") == "/residents?msg=alreadyinactive");
    CHECK(t12::statusOf(id) == "Inactive");
    CHECK(t12::residentCount() == 1);
}

// T12 Required Test 13: Inactive Resident Remains Visible
DROGON_TEST(T12InactiveResidentRemainsVisibleTest)
{
    t12::useFreshDatabase("t12_inactive_visible");
    int id = t12::persistResident("Maria", "Santos");
    t12::persistResident("Juan", "Dela Cruz");
    t12::submitDeactivation(std::to_string(id));

    for (const auto &params : {t12::Params{}, t12::Params{{"q", "maria"}}})
    {
        const std::string body = t12::bodyOf(t12::listPage(params));
        CHECK(t12::contains(body, ">Maria<"));
        CHECK(t12::contains(body, ">Inactive<"));
    }

    // No Deactivate or reactivation action is offered for the Inactive row.
    const std::string searched = t12::bodyOf(t12::listPage({{"q", "maria"}}));
    CHECK(!t12::contains(searched, "/deactivate\""));
    CHECK(!t12::contains(searched, "Activate"));
    CHECK(!t12::contains(searched, "Reactivate"));
    CHECK(t12::contains(searched, "/residents/" + std::to_string(id) + "/edit"));
}

// T12 Required Test 14: GET Requests Do Not Deactivate Residents
DROGON_TEST(T12GetRequestsDoNotChangeResidentsTest)
{
    t12::useFreshDatabase("t12_get_readonly");
    int id = t12::persistResident("Maria", "Santos");
    const std::string idText = std::to_string(id);

    t12::listPage();
    t12::listPage({{"q", "maria"}});
    t12::listPage({{"q", "maria"}, {"msg", "deactivated"}});
    t12::registerForm();
    t12::editForm(idText);

    CHECK(t12::statusOf(id) == "Active");
    CHECK(t12::residentCount() == 1);
    auto stored = ResidentServices::instance().repository().findById(id);
    CHECK(stored->getFirstName() == "Maria");
}

// Flash text comes from a fixed list; arbitrary msg values are ignored.
DROGON_TEST(T12UnknownFlashCodeIsIgnoredTest)
{
    t12::useFreshDatabase("t12_flash");

    const std::string body = t12::bodyOf(t12::listPage({{"msg", "<b>hacked</b>"}}));

    CHECK(!t12::contains(body, "hacked"));
    CHECK(!t12::contains(body, "class=\"message success\""));
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