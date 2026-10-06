#include "Resident.h"

// Converts a ResidentStatus enum value into a readable string.
// We use this to compare against "Active" if needed, or to print/log status.
std::string residentStatusToString(ResidentStatus status)
{
    switch (status)
    {
        case ResidentStatus::Active:
            return "Active";
        case ResidentStatus::Inactive:
            return "Inactive";
        // add other enum values here
        default:
            return "Unknown";
    }
}

// Constructor: runs when you create a Resident with all fields at once,
// e.g. Resident r(1, "Juan", "Dela Cruz", "123 Main St", "0917...", "juan@email.com", ResidentStatus::Active);
Resident::Resident(int id,
                    const std::string &firstName,
                    const std::string &lastName,
                    const std::string &address,
                    const std::string &contactNumber,
                    const std::string &email,
                    ResidentStatus status)
    : id_(id),
      firstName_(firstName),
      lastName_(lastName),
      address_(address),
      contactNumber_(contactNumber),
      email_(email),
      status_(status)
{
}

// new/unsaved constructor
Resident::Resident(const std::string &firstName,
                    const std::string &lastName,
                    const std::string &address,
                    const std::string &contactNumber,
                    const std::string &email,
                    ResidentStatus status)
    : id_(std::nullopt),
      firstName_(firstName),
      lastName_(lastName),
      address_(address),
      contactNumber_(contactNumber),
      email_(email),
      status_(status)
{
}

// --- Getters ---
std::optional<int> Resident::getId() const { return id_; }
std::string Resident::getFirstName() const { return firstName_; }
std::string Resident::getLastName() const { return lastName_; }
std::string Resident::getAddress() const { return address_; }
std::string Resident::getContactNumber() const { return contactNumber_; }
std::string Resident::getEmail() const { return email_; }
ResidentStatus Resident::getStatus() const { return status_; }

// --- Setters ---
void Resident::setId(int id) { id_ = id; }
void Resident::setFirstName(const std::string &firstName) { firstName_ = firstName; }
void Resident::setLastName(const std::string &lastName) { lastName_ = lastName; }
void Resident::setAddress(const std::string &address) { address_ = address; }
void Resident::setContactNumber(const std::string &contactNumber) { contactNumber_ = contactNumber; }
void Resident::setEmail(const std::string &email) { email_ = email; }
void Resident::setStatus(ResidentStatus status) { status_ = status; }