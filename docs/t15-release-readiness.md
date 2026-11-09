# T15 Release Readiness - System Hardening and Regression Testing

## 1. Developer Information

- **Name:** Adrian Paolo S. Follante
- **GitHub Username:** Gojatora
- **Technology Stack:** C++17/20 with Drogon 1.9.13 (HTTP, CSP views, Drogon test framework), SQLite 3 (prepared statements), CMake + MinGW (MSYS2 UCRT64)
- **T15 Branch:** feature/t15-system-hardening

---

## 2. Release Candidate Baseline

Recorded on a clean working tree, on a branch created from `main` after the T14 merge (`3a00cb2`), before any T15 change.

| Item | Before T15 hardening |
| --- | --- |
| Application startup | Working |
| `/health` | Working (200, JSON `status: ok`) |
| Dashboard (`/dashboard`) | Working (200) |
| Resident Management (`/residents`) | Working (200) |
| Service Request Management (`/service-requests`) | Working (200) |
| Starter through T14 automated tests | All passing: **196 test cases, 1120 assertions** |

No failing test or broken page existed at the baseline. The defects and risks below were found by reviewing the code and probing the running application, not by a failing test.

---

## 3. Hardening Changes

| Area | Issue or Risk Found | Action Taken | Files Changed | Test/Evidence |
| --- | --- | --- | --- | --- |
| 1. Full regression | None found. | Reviewed - no defect identified. Complete suite re-run after every change. | - | Runs 1 and 2 below |
| 2. Invalid / missing input | Malformed ids (`abc`, `-1`, `0`, `1.5`, `1abc`, `1 OR 1=1`, 20-digit numbers) already return the controlled 404 page through `parseId`. Nothing was verified across *all* routes together. | Reviewed - no defect identified. Added a test that sends every malformed id to all seven id-based routes. | `test/test_main.cc` | `T15MalformedRouteIdentifiersAreRejectedOnEveryRouteTest`; manual Scenario 11 |
| 2. Invalid input (search) | Resident search (T05) bound its term safely, but `%` and `_` in the term were treated as SQL `LIKE` wildcards, so searching `%` returned every Resident. Service Request search (T11) already treated them literally, so the two modules disagreed. | Escaped `%`, `_` and `\` in the term (`ESCAPE '\'`) before wrapping it in `%...%`. The term is still bound, never concatenated. Ordinary partial, case-insensitive search is unchanged. | `repositories/ResidentRepository.cc` | `T15ResidentSearchTreatsSpecialCharactersAsLiteralTextTest`; all T05 tests still pass |
| 2. Unexpected errors | When an unexpected exception occurred (reproduced by pointing `CSMS_DB_PATH` at a directory), Drogon's default handler returned an empty-bodied 500 with no feedback for the user. It did not leak the message, and the app stayed up. | Registered an exception handler that logs details server-side and returns a generic "Something Went Wrong" page with links back to Dashboard, Residents and Service Requests. No exception text, SQL or path is sent to the browser. | `controllers/ControllerSupport.h`, `main.cc` | `T15UnexpectedFailureResponseRevealsNoInternalsTest`; live check with an unopenable database (500 page, `/health` still 200) |
| 3. Persistence integrity | Resident update validates the whole candidate before a single `UPDATE`, so partial updates cannot occur. Submission and status services return before any write on rejection. | Reviewed - no defect identified. Added a test mixing every rejected mutation and comparing a full persisted snapshot. | `test/test_main.cc` | `T15RejectedMutationsLeaveEveryPersistedRowUnchangedTest`; manual Scenarios 2, 5, 7 |
| 4. HTTP / mutation safety | All state changes are POST routes with a 303 redirect (Post/Redirect/Get). GET on those paths returns 404. GET pages are read-only. | Reviewed - no defect identified. Added a cross-page read-only test. | `test/test_main.cc` | `T15ReadOnlyPagesNeverMutatePersistenceTest`; manual Scenario 10 |
| 5. Query / database safety | Every repository statement uses `sqlite3_prepare_v2` with bound parameters. The only dynamic SQL is the Service Request search, assembled from fixed fragments with all values bound. Dashboard uses `COUNT(*)` and `LIMIT` queries. | Reviewed - no defect identified (apart from the wildcard row above). Dashboard was not changed. | - | Code review; search tests with `' OR 1=1 --` and `DROP TABLE` text |
| 6. Workflow / lifecycle | Transition rules live in `ServiceRequestStatusService`; the UI only hides buttons. Deactivation is soft and idempotent; no reactivation or deletion exists. | Reviewed - no defect identified. Added cross-feature and repeatability tests. | `test/test_main.cc` | `T15ResidentDeactivationStaysConsistentAcrossAllFeaturesTest`, `T15RepeatedActionsAndTerminalStatesAreSafeTest` |
| 7. Interface / navigation | Every page carries the shared Dashboard / Residents / Service Requests nav bar; all links resolve. The landing page (`/`) still said "Starter application initialized" and "Sprint 0 - Developer Onboarding". | Updated those two stale sentences on the landing page. No layout change. | `controllers/HomeController.cc` | Manual Scenario 12 (nav links checked on 7 pages) |
| 8. Test reliability | Test databases were created as one file per test directly in the system temp folder and never removed: **698 leftover `csms_test_*.sqlite3` files** had accumulated. Each test already used its own fresh file, so there was no order dependency, but leftovers piled up between runs. | Test databases now live in one folder (`<temp>/csms_cpp_tests`) that is wiped the first time a run needs it. Removed the 698 stale files from the temp folder. No test was changed, skipped or weakened. | `test/test_main.cc` | Two consecutive full runs pass; folder holds only the current run's files |
| 9. Repository hygiene | See section 7. | Reviewed - no accidental artifacts found; none removed from the repository. | - | `git ls-files`, `.gitignore`, grep |

---

## 4. Five New T15 Hardening Tests

All five are in `test/test_main.cc`. Two supporting tests (listed after the table) cover the production changes above.

| # | Test Name | Category |
| --- | --- | --- |
| 1 | `T15MalformedRouteIdentifiersAreRejectedOnEveryRouteTest` | 1 - Invalid or malformed input |
| 2 | `T15RejectedMutationsLeaveEveryPersistedRowUnchangedTest` | 2 - Rejected mutation protects persistence |
| 3 | `T15ReadOnlyPagesNeverMutatePersistenceTest` | 3 - Read-only request safety |
| 4 | `T15ResidentDeactivationStaysConsistentAcrossAllFeaturesTest` | 4 - Cross-feature regression |
| 5 | `T15RepeatedActionsAndTerminalStatesAreSafeTest` | 5 - Repeatability / boundary behavior |

**Test 1 - Malformed route identifiers.**
*Verifies:* eleven malformed ids (letters, negative, zero, decimal, `+1`, `1abc`, SQL text, empty, 20 digits, `0x1`) sent to all seven id-based routes (Resident edit form, update, deactivate; Service Request details, start, complete, cancel). Each returns 404 with no internal text, and the full persisted snapshot is identical afterwards, while the real record at its real id still works.
*Added value:* T12 and T13 each test one malformed id on one route. This covers every route and every shape of bad id, including ones that start like a real id and must not reach it.

**Test 2 - Rejected mutations protect persistence.**
*Verifies:* in a populated system, a Resident update with a valid first name, an invalid contact number and forged `status`/`id` fields; an invalid registration; update and deactivation of a missing Resident; submissions for an Inactive, a missing and an invalid-date case; and Pending to Completed plus every action on a Cancelled request. All are refused, and the records are re-read to confirm every row is unchanged and nothing was created.
*Added value:* earlier tests check one rejection each at one layer. This shows the *whole mix* leaves the database byte-for-byte the same, through the web layer, including the partial-update and forged-field cases.

**Test 3 - Read-only pages never mutate.**
*Verifies:* three rounds over Dashboard, Resident list, search (including SQL text), register and edit forms, Service Request list, search, status filter, combined filter, an unsupported filter, submit form, details, flash-code URLs and missing records. The persisted snapshot and the dashboard aggregates are unchanged.
*Added value:* T12, T13 and T14 each checked their own module's GET pages. This checks all modules together, repeatedly, and includes the aggregates.

**Test 4 - Cross-feature regression.**
*Verifies:* deactivating a Resident (T07 via T12) blocks that Resident's new requests (T09) but not another Resident's; the deactivated Resident's existing requests can still be completed and cancelled (T10 via T13); the Resident stays searchable and shown Inactive (T05); Completed requests stay filterable (T11); and dashboard counts (T14) match persistence (Active/Inactive, Pending, Completed, Cancelled, Open).
*Added value:* it follows one business event across seven tickets instead of testing each ticket separately.

**Test 5 - Repeatability and terminal states.**
*Verifies:* deactivating the same Resident four times changes state once, then returns "already inactive" each time with an unchanged snapshot; start/complete/cancel on a Completed and on a Cancelled request are refused three times over, their details pages offer no action forms, and statuses and counts stay stable.
*Added value:* it covers refresh and double-submit behavior and terminal-state protection under repetition, which earlier tests only tried once.

**Supporting tests (additional, not among the required five):**
- `T15ResidentSearchTreatsSpecialCharactersAsLiteralTextTest` - proves the Resident search wildcard fix and that T05 search still works.
- `T15UnexpectedFailureResponseRevealsNoInternalsTest` - proves the generic 500 page hides exception text, SQL and paths.

---

## 5. Full Regression Test Results

- **Test command:** from `build/`, `mingw32-make -j6` then `./test/csms_cpp_test.exe` (the Drogon test runner; it executes Starter, T01-T15 and the T10 student midterm test in one binary).
- **Run 1:** PASS - exit code 0, `All tests passed (1510 assertions in 203 test cases)`.
- **Run 2 (immediately after):** PASS - exit code 0, `All tests passed (1510 assertions in 203 test cases)`.
- **Total passing tests reported by the framework:** 203 test cases, 1510 assertions (baseline was 196 / 1120; the difference is the 7 new T15 tests).
- **Run from merged `main` (after the Pull Request is merged):** to be recorded at merge time.

No existing test was deleted, skipped or weakened. The only test-file change to existing code is the temp-folder handling in `makeTempDbPath`.

---

## 6. Manual Release Verification

Performed against the rebuilt application (`csms_cpp.exe`) on `127.0.0.1:8080` with a fresh, throwaway database, using HTTP requests that mirror what the browser forms send.

| Verification | Result | Notes |
| --- | --- | --- |
| Application starts | PASS | Starts and listens on 8080. |
| /health works | PASS | 200 with `status: ok`; still 200 even when the database cannot be opened. |
| Dashboard loads | PASS | 200. |
| Resident listing/search works | PASS | List shows both Residents; name search finds them case-insensitively; `%` no longer matches everything. |
| Valid Resident registration works | PASS | 303 to `/residents?msg=registered`; persisted with generated ids 1 and 2, status Active. |
| Invalid Resident registration is rejected | PASS | 422 with field messages; the list still shows only the two valid Residents. |
| Resident update works | PASS | Name changed to "Mariaa"; id and status unchanged even though a forged `status=Inactive` was sent. An invalid update (bad contact number) returned 422 and saved nothing. |
| Resident deactivation works and record remains visible | PASS | Status became Inactive; still listed and found by search `cruz`; a repeat returned "already inactive". |
| Valid Service Request submission for Active Resident works | PASS | 303 to submitted; two requests created, Pending, retrievable. |
| Missing/Inactive Resident submission is rejected | PASS | Resident 999999 and Inactive Resident 2 both returned 422; request count stayed at 2. |
| Service Request search/filter/details work | PASS | List 2 rows; `q=clearance` 1; `status=Pending` 2; combined 1; combined with Completed 0; details page 200. |
| Valid Service Request workflow works | PASS | Pending to In Progress to Completed; second request Pending to Cancelled. |
| Invalid/terminal transition is rejected | PASS | Pending to Completed returned 409 with no change; start/cancel on Completed and Cancelled requests returned 409. Both terminal requests remain viewable and filterable. |
| Dashboard reflects Resident/Service Request changes | PASS | Residents 2 total / 1 active / 1 inactive; requests 2 total, 0 open, 1 Completed, 1 Cancelled. |
| Read-only GET operations do not modify persistence | PASS | Repeated page reads gave identical dashboard numbers; automated Test 3 compares the full snapshot. |

Also verified manually: missing and malformed ids (`/residents/999999/edit`, `/residents/abc/edit`, `/service-requests/999999`, `/service-requests/not-a-number`, and POST forms for them) all return a controlled 404 page with no SQL, exception or stack text; GET on mutation URLs returns 404; every page has working Dashboard / Residents / Service Requests navigation.

---

## 7. Repository Hygiene Review

| Check | Result |
| --- | --- |
| Debug output | Checked: no `cout`, `cerr`, `printf`, `LOG_DEBUG`, `TODO`, `FIXME` in production code. |
| Temporary files | Checked: none tracked. 698 stale test databases in the system temp folder were removed and the test helper no longer leaks them. |
| Generated build/runtime artifacts | Checked: `build/` and `uploads/` are untracked and ignored (`.gitignore`). |
| Accidental local database files | Checked: `csms.sqlite3` in the project root is ignored by `*.sqlite3` and is not tracked. |
| Commented-out abandoned code | Checked: none found; remaining comments explain behavior. |
| Secrets/credentials | Checked: none. `config.json` and `config.yaml` are the starter files with empty `user` / `passwd` fields and the default sample TLS path; no tokens or keys. |
| Broken navigation/links | Checked: all seven page types link to Dashboard, Residents and Service Requests; stale landing-page text corrected. |
| Direct SQL parameter binding | Checked: all values are bound with `sqlite3_bind_*`; dynamic SQL uses only fixed fragments. |

Cleanup performed: stale landing-page text, temp-folder test cleanup, removal of 698 leaked temp test databases. No tracked file was deleted.

---

## 8. Known Limitations

These are outside T01-T15 requirements and were deliberately not changed during the feature freeze:

- A stored status text that is not recognized is read back as Active (Resident) or Pending (Service Request). This cannot happen through the application, which only writes valid values, but a hand-edited database could hit it.
- `service_requests.resident_id` has no database foreign key. The submission service enforces that the Resident exists and is Active.
- A status change reads the current status and then updates it as two steps. This is safe with the single-threaded configuration in `main.cc`, but is not a compare-and-swap.
- Dashboard numbers come from several separate `COUNT` queries, not one transaction, so a change made between them in a multi-user setting could show a momentary mismatch.
- Route ids with leading zeros (for example `/residents/01/edit`) resolve to the same record as `/residents/1/edit`.
- Search is case-insensitive for ASCII letters only (SQLite `LIKE`).
- There is no authentication or CSRF protection (explicitly out of scope).
- The Resident form uses browser `required` attributes while the Service Request form relies on server messages only; the server validates both.
- The test helper uses a fixed temp folder name, so two test processes must not run at the same time.

---

## 9. Release Candidate Statement

**READY FOR T16 FINAL EXAM**

No release-blocking defect is known: the complete suite passed twice (203 test cases, 1510 assertions each) and all manual verification items passed. The final step that remains is to record the post-merge run from `main` in section 5.
