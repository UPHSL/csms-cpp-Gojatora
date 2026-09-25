#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>

#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../models/ServiceRequest.h"

#include "../database/Database.h"

#include "../repositories/ResidentRepository.h"

#include "../services/ResidentRegistrationService.h"
#include "../services/ResidentSearchService.h"
#include "../services/ResidentUpdateService.h"
#include "../services/ResidentDeactivationService.h"

#include <sqlite3.h>
#include <filesystem>
#include <cstdio>

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