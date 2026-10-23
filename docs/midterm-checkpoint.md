# Midterm Examination Checkpoint — T10

## 1. Developer Information

- **Name:** Adrian Paolo S. Follante
- **GitHub Username:** Gojatora
- **Primary Technology Stack:** C++ with Drogon (SQLite persistence, Drogon test framework)
- **T10 Branch:** feature/t10-service-request-status

---

## 2. My T10 Implementation

The status workflow is managed by a new class, `ServiceRequestStatusService`, which sits in the services layer next to `ServiceRequestSubmissionService`. Its `changeStatus(id, requestedStatus)` method first asks the existing `ServiceRequestRepository::findById` for the persisted request; if nothing comes back it returns a result with `notFound = true` and stops, so an unknown ID can never create or modify a row. The current status is simply the status of the `ServiceRequest` that was just loaded from SQLite, not something the caller supplies. The requested status arrives as text and is converted by `serviceRequestStatusFromString`, which only accepts the exact values `Pending`, `In Progress`, `Completed` and `Cancelled`; anything else (such as `Approved`) produces `unsupportedStatus = true`. A private `isTransitionAllowed(current, target)` function then uses a `switch` on the current status to decide whether the move is one of the four permitted ones, and any other move (including a same-status request) produces `invalidTransition = true`. All three failure paths return before the repository's write method is ever called, so persistence is never touched and nothing has to be rolled back. Only for a valid transition does the service call the new `ServiceRequestRepository::updateStatus`, which runs a parameterized `UPDATE service_requests SET status = ? WHERE id = ?` and therefore changes only the status column of only that row. Finally the service re-reads the row with `findById` and returns that object inside the result, so the caller receives what is really stored (same ID, resident, type, description and date, with the new status).

---

## 3. My Transition Rules

| Current Status | Allowed Next Status |
| --- | --- |
| Pending | In Progress, Cancelled |
| In Progress | Completed, Cancelled |
| Completed | none |
| Cancelled | none |

Allowed transitions enforced by my code:

- **Pending → In Progress:** allowed.
- **Pending → Cancelled:** allowed.
- **In Progress → Completed:** allowed.
- **In Progress → Cancelled:** allowed.

Rejected cases:

- **Why Pending → Completed is rejected:** a request must be worked on before it can be finished. `isTransitionAllowed` for `Pending` only returns true for `In Progress` and `Cancelled`, so `Completed` falls through to false and the result is `invalidTransition`. The same function also rejects In Progress → Pending, because a request that has started never goes back.
- **Why Completed is terminal:** a completed request is a finished workflow. `isTransitionAllowed` returns false for every target when the current status is `Completed`, so Completed → Pending / In Progress / Cancelled are all invalid transitions.
- **Why Cancelled is terminal:** T10 does not support reopening a cancelled request, so `isTransitionAllowed` returns false for every target when the current status is `Cancelled`.
- **How same-status requests are handled:** a same-status request (e.g. Pending → Pending) is not in the allowed list for any status, so it is treated as an invalid transition (`invalidTransition = true`). The persisted request is not updated, which is checked in the test `SameStatusRequestIsRejectedTest`.

Two further decisions: an unsupported target status is reported as `unsupportedStatus`, which is separate from `invalidTransition`, so callers can tell the two apart. The existing request is looked up before the target status is checked, so an unknown ID reports `notFound` even if the status text is also bad.

---

## 4. Files I Changed

File: `services/ServiceRequestStatusService.h` / `services/ServiceRequestStatusService.cc` (new)
Purpose: The application-layer component that implements T10. It loads the request, handles not-found, checks the target status, applies the transition rules, and asks the repository to persist only valid transitions.

File: `services/ServiceRequestStatusResult.h` (new)
Purpose: The result type returned by the service. It has `success`, `notFound`, `unsupportedStatus`, `invalidTransition` and the updated `ServiceRequest`, so callers (and tests) never need to read console output.

File: `repositories/ServiceRequestRepository.h` / `repositories/ServiceRequestRepository.cc`
Purpose: Added `updateStatus(id, status)`, a parameterized `UPDATE ... SET status = ? WHERE id = ?` that changes only the status of one row and returns whether a row matched. It contains no transition rules.

File: `models/ServiceRequest.h` / `models/ServiceRequest.cc`
Purpose: Added `serviceRequestStatusFromString`, which converts text to a `ServiceRequestStatus` and returns `std::nullopt` for unsupported values. The existing `ServiceRequestStatus` enum was reused and no second model was created.

File: `test/test_main.cc`
Purpose: Added the thirteen required T10 tests, my student-designed test, and four extra tests. No earlier test was removed or changed.

File: `CMakeLists.txt` and `test/CMakeLists.txt`
Purpose: Registered `ServiceRequestStatusService.cc` for the application and test executables.

File: `docs/midterm-checkpoint.md` (new)
Purpose: This checkpoint document.

---

## 5. Problem I Encountered

When I set up the baseline build (Starter through T09, before starting T10) on my Windows machine, CMake configuration failed before compiling anything:

```
CMake Error at C:/msys64/ucrt64/lib/cmake/Drogon/FindFilesystem.cmake:258 (message):
  Cannot run simple program using std::filesystem
```

The error message pointed at `std::filesystem`, which was misleading, because my project code was not involved at all. I opened `build/CMakeFiles/CMakeConfigureLog.yaml` and looked at the `try_run` entries. The test program compiled and linked, but its run result was `Exit code 0xc0000139` (`FAILED_TO_RUN`), which Windows uses for "entry point not found" in a DLL. That told me the program was starting but loading a wrong or incompatible DLL. The cause was my `PATH`: Git's own `C:\Program Files\Git\mingw64\bin` appeared before `C:\msys64\ucrt64\bin`, so the freshly compiled program picked up Git's runtime DLLs instead of the matching MSYS2 (ucrt64) ones. I fixed it by putting the MSYS2 directory first in `PATH` for the build shell (`export PATH="/c/msys64/ucrt64/bin:$PATH"`), deleting the half-configured `build` directory contents, and reconfiguring. After that, configure, build, and the full test suite worked (82 test cases passing before T10), and I could start the T10 work from a verified baseline.

---

## 6. My Student-Designed Test

- **Test Name:** `StatusChangeAffectsOnlyTheTargetedServiceRequestTest`

- **What the Test Verifies:** It persists three Service Requests (for two different Residents) through the real T09 submission service. It then moves the first request through the whole legitimate lifecycle (Pending → In Progress → Completed) and cancels the third one. Reading directly from SQLite, it verifies that the first request ended as `Completed`, the third as `Cancelled`, and that the second request remained `Pending` with the same ID, resident, service type, description and date. It also verifies that the table still contains exactly three rows, so nothing was created or deleted.

- **Why I Added This Test:** The required tests only ever check one request at a time, so a `WHERE id = ?` mistake in the `UPDATE` (for example, updating every row) would not be noticed. This test checks that the update affects only the intended request, that several valid transitions in a row work, and that requests belonging to different Residents stay independent.

I also added four smaller extra tests: an existing request can still progress after its Resident is deactivated (while T09 still rejects a new request for that Resident); a status change survives re-opening the database with new repository instances; unsupported spellings such as `in progress`, `InProgress` and the empty string are rejected; and `updateStatus` on an unknown ID reports failure without inserting a row.

---

## 7. Tools and References Used

- **AI / coding assistant:** Claude Code (Anthropic, model Claude Sonnet 5.5) was used as a coding assistant during T10. It helped draft the service, repository method, result type, the tests, and this document, and it helped me diagnose the build problem in Section 5. I reviewed the generated code and I am responsible for being able to explain it.
- **SQLite documentation:** `sqlite3_changes`, `sqlite3_prepare_v2` and `sqlite3_bind_*`, for the parameterized `UPDATE` and checking that one row was changed.
- **Drogon documentation:** the `DROGON_TEST`, `CHECK` and `REQUIRE` test macros.
- **Visual Studio Code**, **MSYS2** (GCC 16.1.0, CMake, Ninja) and **Git / GitHub**.
- My own earlier tickets (T03, T08, T09) as the pattern for repositories, result types and tests.
