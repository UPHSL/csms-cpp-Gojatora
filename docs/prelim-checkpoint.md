# Preliminary Examination Checkpoint — T03

## Developer Information

- **Name:** Adrian Paolo S. Follante
- **GitHub Username:** [your GitHub username]
- **Primary Technology Stack:** C++ with Drogon
- **T03 Branch:** feature/t03-resident-persistence

---

## My T03 Implementation

Resident data is stored in a file-backed SQLite database. The `Database` class (`database/Database.h` and `.cc`) is responsible for opening the SQLite connection and creating the `residents` table if it does not already exist. The `ResidentRepository` class (`repositories/ResidentRepository.h` and `.cc`) is responsible for the actual persistence operations: `save()` and `findById()`.

When `save()` is called, it builds a prepared `INSERT` statement, binds each Resident field as a parameter to avoid SQL injection, and executes it. The `id` is not provided manually — SQLite auto-generates it because the `id` column is declared as `INTEGER PRIMARY KEY AUTOINCREMENT`. After the insert completes, `sqlite3_last_insert_rowid()` is used to retrieve that generated id, and a new `Resident` object is returned containing it.

`findById()` prepares a `SELECT` statement, binds the requested id, and executes it. If a matching row is found, its columns are read out and used to reconstruct a `Resident` object, and the result is wrapped in `std::optional<Resident>`. If no row matches, `std::nullopt` is returned instead, which safely represents "not found" without throwing an exception or returning a fake object.

## My Persistence Design Decision

One decision I made was to split the SQLite connection/setup logic (Database)
from the Resident-specific persistence logic (ResidentRepository), instead of
putting everything in one class. Database only knows how to open a SQLite
file and make sure the residents table exists — it has no knowledge of what
a Resident even is. ResidentRepository, on the other hand, knows about
Resident objects and how to save/retrieve them, but it doesn't manage the
raw database connection itself; it just borrows a Database reference.

I chose this separation because it keeps each class responsible for one
thing. If I ever needed to store a different kind of record in the future
(not just Residents), I could reuse the same Database class without
duplicating the connection-opening logic. It also matches how T01 and T02
were structured — Resident represents data, ResidentValidator checks data,
and now Database/ResidentRepository store data — each class has exactly one
job.

The alternative I considered was putting the sqlite3_open() and table
creation code directly inside ResidentRepository's constructor. I decided
against this because it would mean every ResidentRepository would need to
duplicate connection-management logic, and it would make it harder to have
multiple repositories share one connection cleanly (which Test 7 in T03
actually requires me to demonstrate).


## My Database Resource Management

I open the SQLite connection using sqlite3_open() inside the Database
constructor, and I close it using sqlite3_close() inside the Database
destructor. This follows the RAII (Resource Acquisition Is Initialization)
pattern — the connection is automatically opened when a Database object is
created, and automatically closed when that object goes out of scope, even
if an exception is thrown somewhere in between. This means I never have to
remember to manually close the connection.

I also deleted Database's copy constructor and copy assignment operator.
This is important because if two Database objects were allowed to hold a
copy of the same sqlite3* pointer, one of them being destroyed would close
the connection while the other object still thought it was usable, leading
to a use-after-close bug. Deleting the copy operations turns that mistake
into a compile-time error instead of a runtime crash.

For prepared statements, both save() and findById() call sqlite3_finalize()
on the sqlite3_stmt* after they are done using it — including in the error
path, where I finalize the statement before throwing an exception, so a
failed operation doesn't leave the statement resource open. If a database
operation fails (for example, if sqlite3_prepare_v2 or sqlite3_step does
not return the expected result code), I read the error text using
sqlite3_errmsg() and throw a std::runtime_error with that message, rather
than silently continuing with a broken statement or connection.


## My Resident Table Design

- Primary key design: id INTEGER PRIMARY KEY AUTOINCREMENT
- Contact number column type: TEXT
- Status column type: TEXT

I used INTEGER PRIMARY KEY AUTOINCREMENT for id because this is SQLite's
built-in mechanism for automatically generating a unique, increasing
identifier for every new row, without me having to generate or track IDs
manually in my C++ code.

I used TEXT for contact_number instead of an integer type because the
contact number can start with a leading zero (e.g. 09171234567), and if it
were stored as a number, SQLite would drop that leading zero, turning it
into 9171234567. Since T02 already established that contactNumber must
remain textual information, storing it as TEXT in the database keeps this
behavior consistent from the Resident model all the way through to
persistence.

I used TEXT for status as well, even though my C++ Resident model
represents status as an enum class (ResidentStatus). SQLite does not have a
native enum type, so I store the human-readable string form ("Active" or
"Inactive") using my existing residentStatusToString() function, and
convert it back into the enum using a small statusFromString() helper when
reading a row back out of the database.


## Files I Changed

**File:** `database/Database.h`, `database/Database.cc`
**Purpose:** Manages the raw SQLite connection — opening the database file and creating the `residents` table if it doesn't already exist. Uses RAII so the connection is automatically closed when the `Database` object is destroyed.

**File:** `repositories/ResidentRepository.h`, `repositories/ResidentRepository.cc`
**Purpose:** Implements `save()` and `findById()` for Resident records, using prepared statements to safely insert and query data, and mapping between SQLite rows and `Resident` objects.

**File:** `models/Resident.h`, `models/Resident.cc`
**Purpose:** The `id_` field was changed from `int` to `std::optional<int>` so a Resident could represent "not yet persisted" (no id) versus "persisted with a real database-assigned id" — matching what T03 required. A second constructor overload (without an `id` parameter) was added for creating new, unsaved Residents.


## SQL I Can Explain

SELECT id, first_name, last_name, address, contact_number, email, status
FROM residents
WHERE id = ?;

This SQL statement retrieves a single Resident row from the residents table,
matching the id supplied by the caller. There is one placeholder (?),
which represents the id being searched for. It is bound using
sqlite3_bind_int() before the statement is executed. If a row with that id
exists, all seven columns are returned so they can be mapped back into a
Resident object. If no row matches, the statement simply produces no rows,
which findById() interprets as "not found" and returns std::nullopt for.

This SQL statement is used by ResidentRepository::findById().


## My Resident Mapping

A SQLite database row and a Resident object are two different
representations of the same information, so ResidentRepository is
responsible for converting between them in both directions.

When saving a Resident, ResidentRepository::save() takes the values out of
the Resident object (using its getters, like getFirstName() and
getContactNumber()) and binds them as parameters into an INSERT statement.
The status field requires an extra conversion step here: the Resident
model stores status as a ResidentStatus enum, but SQLite has no enum type,
so residentStatusToString() converts it into plain text ("Active" or
"Inactive") before it is stored.

When retrieving a Resident, ResidentRepository::findById() reads each
column value out of the SQLite row using functions like
sqlite3_column_int() and sqlite3_column_text(), and uses those values to
construct a new Resident object through its constructor. The status
column goes through the reverse conversion here: statusFromString() takes
the text value read from the database and converts it back into the
correct ResidentStatus enum value, so the reconstructed Resident behaves
the same as any other Resident object in the application.

One column whose name differs from its C++ representation is
contact_number (the SQLite column, written in snake_case) versus
contactNumber (the Resident property, written in camelCase). The mapping
code in ResidentRepository is what bridges this naming difference — column
index 4 (contact_number) is read out and passed as the contactNumber
argument when constructing the Resident object.


## Problem I Encountered

**Problem or error:**
When I added `find_package(SQLite3 REQUIRED)` to link SQLite into my project, CMake configuration failed with:

**CMake Error at C:/msys64/ucrt64/lib/cmake/Drogon/FindSQLite3.cmake:47 (add_library):
add_library cannot create imported target "SQLite3_lib" because another
target with the same name already exists.**


**Cause:**
Drogon's own installed CMake configuration already calls `find_package(SQLite3)` internally, because Drogon itself supports SQLite as one of its optional database backends. This already created a CMake imported target named `SQLite3_lib`. When I called `find_package(SQLite3)` a second time myself, CMake tried to create a target with that same name again, which is not allowed — a CMake target can only be defined once per configuration.

**How I resolved it:**
Instead of calling `find_package(SQLite3)` myself, I linked directly against the `SQLite3_lib` target that Drogon's own `find_package(Drogon)` call had already created. I removed my `find_package(SQLite3 REQUIRED)` line and replaced `target_link_libraries(... SQLite::SQLite3)` with `target_link_libraries(${PROJECT_NAME} PRIVATE SQLite3_lib)` in both the main `CMakeLists.txt` and `test/CMakeLists.txt`. This let my project reuse the already-located SQLite3 library instead of searching for it again.



## My Student-Designed Test

### ContactNumberLeadingZeroSurvivesRoundTripTest

This test verifies that a Resident's contact number is not altered when it
is saved to SQLite and then retrieved again. Since a valid contact number
always starts with "0" (e.g. 09171234567), it must be stored using a TEXT
column instead of an INTEGER column — otherwise SQLite would treat it as a
number and silently drop the leading zero, turning it into 9171234567.
This test saves a Resident, retrieves it back using findById(), and checks
that the contact number is still exactly 11 characters long, still starts
with "0", and matches the original value exactly. This test exists to catch
a regression where someone might accidentally change the contact_number
column type, or use sqlite3_bind_int() instead of sqlite3_bind_text() when
saving — a mistake that would be easy to miss since other fields would
still work correctly. 

## Tools and References Used

**Documentation**
- https://devdocs.io/cpp/
- https://sqlite.org/docs.html
- https://www.w3schools.com/cpp/

**IDE Extensions**
- CMake Tools
- C/C++
- C/C++ Extension Pack
- C/C++ DevTools

**AI tools**
- Claude (Anthropic): Used throughout Sprint 0 through T03 to explain C++
  syntax and concepts (as a beginner to the language), help set up the
  MSYS2/Drogon/CMake toolchain, and to help design and write the SQLite
  persistence code (Database and ResidentRepository classes) along with
  explanations of each part (prepared statements, RAII, parameter binding,
  etc.). All code was reviewed, tested, and I am responsible for
  understanding and being able to explain and modify it.

