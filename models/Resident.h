#pragma once

#include <string>
#include <optional>

// Represents the possible states a Resident can be in.
// Using an enum class (instead of a plain string) prevents typos like
// "Active " or "active" from silently being treated as valid values.

enum class ResidentStatus
{
    Active,
    Inactive 
};



// Converts a ResidentStatus into a human-readable string.
// Useful for printing, logging, or sending status as JSON text later.
std::string residentStatusToString(ResidentStatus status);

class Resident
{
public:
    // Default constructor (creates an "empty" Resident).
    Resident() = default;

    // Constructor used when the id is already known
    // (e.g. reconstructing a Resident retrieved from the database).
    Resident(int id,
            const std::string &firstName,
            const std::string &lastName,
            const std::string &address,
            const std::string &contactNumber,
            const std::string &email,
            ResidentStatus status
            );

    // Constructor for a brand-new Resident that hasn't been saved yet.
    // No id is provided — id_ defaults to std::nullopt.
    Resident(const std::string &firstName,
            const std::string &lastName,
            const std::string &address,
            const std::string &contactNumber,
            const std::string &email,
            ResidentStatus status
            );

    // --- Getters: read the value of a field ---
    std::optional<int> getId() const;
    std::string getFirstName() const;
    std::string getLastName() const;
    std::string getAddress() const;
    std::string getContactNumber() const;
    std::string getEmail() const;
    ResidentStatus getStatus() const;

    // --- Setters: change the value of a field ---
    void setId(int id);
    void setFirstName(const std::string &firstName);
    void setLastName(const std::string &lastName);
    void setAddress(const std::string &address);
    void setContactNumber(const std::string &contactNumber);
    void setEmail(const std::string &email);
    void setStatus(ResidentStatus status);

private:
    std::optional<int> id_ ; // optional, can be null
    std::string firstName_;
    std::string lastName_;
    std::string address_;
    std::string contactNumber_;
    std::string email_;
    ResidentStatus status_ = ResidentStatus::Active;
};
