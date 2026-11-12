# Final Examination Checkpoint - T16

## 1. Developer Information

- **Name:** Adrian Paolo S. Follante
- **GitHub Username:** Gojatora
- **Technology Stack:** C++17/20 with Drogon 1.9.13 (HTTP, CSP views, Drogon test framework), SQLite 3, CMake + MinGW (MSYS2 UCRT64)
- **T16 Branch:** feature/t16-final-release
- **Final Release Version:** 1.0.0

---

## 2. My Final Release Implementation

I established the release identity in one small header, `AppInfo.h`, at the project root. It holds the short name `CSMS`, the full application name and the version `1.0.0` as inline constants in a `app_info` namespace. A header fits this C++ project because the controllers, the view helpers and the tests are all compiled from the same source tree and already include from the project root. It needed no CMake plumbing and no new runtime configuration. `HealthController` reads those constants when it builds its JSON response, so `/health` reports the application name, the version and a status of `OK` and nothing else. The browser interface gets the version through `navigationBar()` in `views/ResidentLayout.h`, which every Dashboard, Resident and Service Request page already calls, so one edit puts `CSMS v1.0.0` on every page. The home page reads the same constant, and I replaced its old hardcoded `0.1.0`. No other application source file contains a version literal, so a future release changes only `AppInfo.h`. The tests also read the constant and compare it with the literal `1.0.0`, which means any accidental change in either place is caught.

---

## 3. Files I Changed

| File | Purpose |
| --- | --- |
| `AppInfo.h` (new) | The single source of truth for the application name and version. |
| `controllers/HealthController.cc` | `/health` now reports `application: CSMS`, the full name, `version: 1.0.0` and `status: OK`, taken from `AppInfo.h`. It replaced the hardcoded `0.1.0`. |
| `controllers/HomeController.cc` | The home page shows the version from `AppInfo.h` instead of `0.1.0`. I also changed the stale "Sprint 5" and "Starter application" text to the final release wording. |
| `views/ResidentLayout.h` | The shared navigation bar now shows `CSMS v1.0.0` on every page, with one small CSS rule to right-align it. |
| `test/test_main.cc` | Added three T16 tests: the `/health` verification, the version-on-every-page verification and the Final Examination integration test. No existing test was changed. |
| `test/CMakeLists.txt` | Added `HealthController.cc` and `HomeController.cc` to the test executable so `/health` and the home page can be tested. |
| `docs/release-v1.0.0.md` (new) | The official CSMS v1.0.0 release notes. |
| `docs/final-checkpoint.md` (new) | This document. |
| `docs/project-overview.md` | Updated the stated current version and sprint, which still said `0.1.0` and Sprint 0. |
| `docs/git-cheatsheet.md` | Updated the sample `/health` response so it matches the real output. |

---

## 4. My Final Examination Test

**Test Name:** `T16ResidentLifecycleRequestWorkflowAndDashboardAreConsistentOnDiskTest`

**Modules/Responsibilities Covered:** Resident validation and registration (T02/T04/T12), Service Request eligibility and submission (T09/T13), the controlled status workflow (T10), Resident deactivation (T07), Resident search and Service Request status filtering (T05/T11), Dashboard aggregation (T14), and SQLite persistence (T03).

**Starting State:** A brand-new empty SQLite database file. Zero Residents, zero Service Requests, and a Dashboard showing zeros.

**Operations Performed:**
1. Register two valid Residents (Maria Santos and Juan Cruz) through the Resident controller, and attempt one invalid registration, which is refused.
2. Submit Service Requests for both Residents. A request for an unknown Resident is refused.
3. Run Maria's first request through Pending, In Progress, Completed. Start her second request. Attempt two invalid transitions (Pending to Completed, and a move out of Completed), and both are rejected.
4. Deactivate Maria. A new request for her is refused, but her In Progress request can still be cancelled, and Juan can still submit a new request.
5. Filter Service Requests by Pending status and search Residents by name.
6. Load the Dashboard, then load several read-only pages and confirm nothing was modified.
7. Open the same database file on a new, independent connection and read it back.

**Expected Final State:** Two Residents (Maria Inactive, Juan Active) and four Service Requests: one Completed, one Cancelled and two Pending. The Dashboard shows 2 total, 1 Active and 1 Inactive Resident, and 4 total, 2 Pending, 0 In Progress, 1 Completed, 1 Cancelled and 2 Open requests. The independently reopened database file reports the same counts and statuses, and every refused operation (the invalid registration, the refused transitions and the refused submission) is shown to have left a full before/after snapshot of all stored rows unchanged.

**Why This Test Is Meaningful:** A unit test of one validator or one repository cannot show that the parts of CSMS still agree with each other. This test drives the real controllers, which call the real services, validators and repositories, and a real SQLite file. Refused operations are checked as well as accepted ones, so it proves that nothing is saved when a rule says no. The final check reads the file through a different connection instead of trusting the same objects that wrote the data. It would fail if, for example, deactivation stopped blocking submissions, a rejected transition still changed a row, or the Dashboard counted from something other than persistence.

---

## 5. My Final Technical Decision

**Decision:** I defined the release identity as `inline constexpr` constants in a standalone header, `AppInfo.h`, and used it from the `/health` controller, the shared navigation bar and the home page. The header is also included from the tests.

**Alternative Considered:** Declaring the version once in `CMakeLists.txt` with `project(... VERSION 1.0.0)` and passing it to the compiler as a preprocessor definition. I also considered adding it to `config.json`.

**Reason for My Decision:** The CMake route is idiomatic, but the app and the test executable are separate CMake targets that would both need the definition, and a missing definition would only show up as a compile or runtime surprise. A `config.json` value would turn the version into a runtime setting that could differ between deployments, and a release version should be fixed in the code that was built. The header is a plain compile-time constant that both targets already see, and it needs no new build logic. It also keeps the version testable, since the tests can compare it with the literal `1.0.0`.

---

## 6. Problem or Verification Issue Encountered

**Problem:** My first baseline run on the freshly merged `main` reported "100% tests passed out of 196". But the merged T15 work should have brought the suite to 203 tests. When I first built the checkout on the merged `main`, every test also failed at once with exit code `0xc0000139`.

**Cause or Investigation:** The `0xc0000139` failure was an environment problem. In my shell, `/mingw64/bin` came before the UCRT64 toolchain on `PATH`, so the test executable loaded mismatched DLLs. Putting `/c/msys64/ucrt64/bin` first fixed it. The 196 count was a different issue. CTest builds its list of tests when CMake configures, not when the code is compiled. My local build folder had been configured before the T15 merge, so ctest was still using the pre-T15 list (196). The executable itself contained all 203 tests. I confirmed this by counting `DROGON_TEST` entries in `main`'s `test_main.cc` (203), which also matches the 196-test pre-T15 baseline recorded in `t15-release-readiness.md`.

**Resolution:** I did not rely on the 196 result. I stashed my T16 work, rebuilt pure `main`, and ran the test executable directly: all 203 test cases (1510 assertions) passed. I restored my work, re-ran `cmake .` to refresh the test list, and ran both final regression runs on 206 tests (203 plus my 3 new ones). I also noted that the ERROR line in the test output comes from a T15 test that deliberately triggers an internal failure, so it is expected.

---

## 7. Final Regression Results

**Test Command:** From the `build` folder, with `/c/msys64/ucrt64/bin` first on `PATH`: `cmake . && cmake --build . && ctest`, and also `./test/csms_cpp_test.exe` run directly.

**Final Regression Run 1:** PASS (ctest: 100% passed out of 206. Direct run: 206 test cases, 1610 assertions.)

**Final Regression Run 2:** PASS (ctest: 100% passed out of 206. Direct run: 206 test cases, 1610 assertions.)

**Total Passing Tests Reported:** 206 test cases, 1610 assertions. This is the 203 tests from T15 plus the three T16 tests. The suite is run again on the merged `main` before the tag is created.

---

## 8. Tools and References Used

- Existing CSMS repository documentation, mainly `docs/t15-release-readiness.md`, `docs/architecture.md` and `docs/release-workflow.md`.
- Drogon framework documentation for controllers, views and the test framework, and the CMake and CTest documentation (for how `ParseAndAddDrogonTests` builds the test list).
- Git and GitHub documentation for the branch, pull request, annotated tag and release workflow.
- Claude Code (an AI coding assistant, model Claude Sonnet 5.5), used in my editor to read the repository, propose and make the T16 changes, run the builds and tests, and draft the release documents. I reviewed what it produced, and I remain responsible for understanding and defending the implementation. The commit, push, pull request, merge, tag and GitHub Release steps are done by me.

---

## 9. Final Release Declaration

CSMS v1.0.0 - READY FOR RELEASE
