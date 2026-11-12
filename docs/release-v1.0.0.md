# CSMS v1.0.0 Release Notes

## 1. Release

- **Application:** Community Services Management System
- **Release:** v1.0.0
- **Release Type:** Final Course Release

---

## 2. Release Summary

CSMS v1.0.0 is a browser-based system for a barangay-style community office. It keeps a register of Residents and the Service Requests they submit, and lets staff move each request through a controlled workflow. A Dashboard summarises the current state of both. The application is written in C++ with the Drogon web framework, stores its data in SQLite through prepared statements, and is built with CMake.

The code is layered. Domain models and validators know the rules, repositories own the SQL, application services enforce the business rules, and thin controllers and server-rendered views present the result. Everything is covered by an automated test suite that runs against throwaway SQLite files.

This release adds a release identity. The application name and version (1.0.0) are defined once in `AppInfo.h`. `/health` reports them, and every browser page shows `CSMS v1.0.0` in the shared navigation bar.

---

## 3. Resident Management Capabilities

- **Register** a Resident with first name, last name, address, contact number and email. The data is validated before it is saved: required fields cannot be blank, the contact number must match `09XXXXXXXXX`, and the email must have a valid shape. Invalid input shows field messages and saves nothing.
- **List** every Resident, sorted by last name then first name, with an Active or Inactive badge.
- **Search** Residents by name. The search is case-insensitive and partial. Characters such as `%` and `_` are matched literally.
- **Update** a Resident's details. An update is all-or-nothing: if any field is invalid, none of it is saved.
- **Deactivate** a Resident. Deactivation is a soft state change. The record stays stored, listed and searchable. Repeating it is safe.

---

## 4. Service Request Management Capabilities

- **Submit** a request for a Resident with a service type, description and date requested (`YYYY-MM-DD`, a real calendar date). The Resident must exist and be Active. Requests for unknown or Inactive Residents are refused and nothing is saved.
- **List, search and filter.** The list shows every request. A text search matches the service type or description, and a status filter narrows the list. The two can be combined.
- **View details** of any request, including requests in a final state.
- **Controlled workflow.** Every new request starts as Pending. Only these moves are allowed:

  | From | Allowed next status |
  | --- | --- |
  | Pending | In Progress, Cancelled |
  | In Progress | Completed, Cancelled |
  | Completed | none (final) |
  | Cancelled | none (final) |

  Any other move, such as Pending directly to Completed or any change after a final state, is rejected with a conflict message and the stored status is unchanged. The interface offers only the buttons that are valid for the current status.
- Requests of a Resident who is later deactivated can still be managed. Only new submissions are blocked.

---

## 5. Dashboard Capabilities

The Dashboard (`/dashboard`) is read-only and computes every figure from the database each time it is opened:

- **Resident summary:** total, Active and Inactive Residents.
- **Service Request summary:** total, Pending, In Progress, Completed and Cancelled, plus **Open Service Requests** (Pending + In Progress).
- **Recent Service Requests:** the five most recent requests, newest first, in any status, each linking to its details page.
- **Module links** to Resident Management and Service Request Management.

The shared navigation bar on every page links Dashboard, Residents and Service Requests, and shows the release version.

---

## 6. Quality and Hardening

**T15 hardening.** Malformed route ids are answered with a controlled not-found page on every route. Rejected changes were proven to leave persisted rows unchanged. Read-only pages were proven not to modify data. Cross-feature behaviour around deactivation was verified. Repeated actions and final states were verified to be safe. Resident search no longer treats `%` and `_` as wildcards, so it now matches the Service Request search. Unexpected failures return a generic 500 page with no internal details. Test databases now live in one cleaned folder.

**T16 final verification.**
- One authoritative version in `AppInfo.h`, used by `/health`, the navigation bar, the home page and the tests.
- `/health` returns only the application short name, full name, version and status.
- Two release-version tests: one for `/health`, and one proving that ten different pages show the same `v1.0.0` and none shows the old `0.1.0`.
- One final integration test that runs a complete lifecycle through the real controllers and services, then reopens the database file on a new connection to check the stored state. It covers Resident registration and validation, eligibility, the status workflow, deactivation, search and filtering, and Dashboard aggregation.
- Two full regression runs and a manual acceptance pass against the running application.
- No earlier test was removed, skipped or changed.

---

## 7. Final Verification

Recorded on `feature/t16-final-release` before merge, on Windows with MSYS2 UCRT64 (GCC, Drogon 1.9.13, SQLite 3). The suite is run again on the merged `main` before the `v1.0.0` tag is created.

| Item | Result |
| --- | --- |
| Final Regression Run 1 | **PASS** (206 test cases, 1610 assertions) |
| Final Regression Run 2 | **PASS** (206 test cases, 1610 assertions) |
| Application Startup | **PASS** |
| /health | **PASS** (`{"application":"CSMS","name":"Community Services Management System","status":"OK","version":"1.0.0"}`) |
| Dashboard | **PASS** |
| Resident Management | **PASS** |
| Service Request Management | **PASS** |
| Service Request Workflow | **PASS** (Pending, In Progress, Completed; invalid transitions rejected with 409) |
| Read-only safety | **PASS** (database contents identical before and after repeated page loads) |
| Navigation | **PASS** |
| Final Release Version | **PASS** (1.0.0 everywhere) |

---

## 8. Known Limitations

These were identified during T15, lie outside the defined scope, and were deliberately left unchanged. None blocks the release.

- A stored status text that is not recognised is read back as Active (Resident) or Pending (Service Request). The application only writes valid values, so only a hand-edited database could reach this.
- `service_requests.resident_id` has no database foreign key. The submission service enforces that the Resident exists and is Active.
- A status change reads the current status and then updates it, as two steps. This is safe with the single-threaded configuration in `main.cc`, but it is not a compare-and-swap.
- Dashboard numbers come from several separate `COUNT` queries, not one transaction, so a multi-user change between them could show a momentary mismatch.
- Route ids with leading zeros, such as `/residents/01/edit`, resolve to the same record as `/residents/1/edit`.
- Search is case-insensitive for ASCII letters only (SQLite `LIKE`).
- There is no authentication or CSRF protection. Both are explicitly out of scope.
- The Resident form uses browser `required` attributes, while the Service Request form relies on server messages only. The server validates both.
- The test helper uses a fixed temp folder name, so two test processes must not run at the same time.
