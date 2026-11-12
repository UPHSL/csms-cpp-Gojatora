#pragma once

// The release identity of CSMS (T16). This is the ONE place the application
// name and version are defined. /health, the browser pages and the tests all
// read these constants, so a future release changes a single line here.
namespace app_info
{
inline constexpr const char *kShortName = "CSMS";
inline constexpr const char *kFullName = "Community Services Management System";
inline constexpr const char *kVersion = "1.0.0";
} // namespace app_info
