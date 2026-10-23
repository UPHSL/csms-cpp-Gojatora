#include "ResidentServices.h"

#include <cstdlib>

namespace
{
std::unique_ptr<ResidentServices> &sharedInstance()
{
    static std::unique_ptr<ResidentServices> instance;
    return instance;
}
} // namespace

ResidentServices::ResidentServices(const std::string &dbPath)
    : database_(dbPath),
      repository_(database_),
      registration_(validator_, repository_),
      search_(repository_),
      update_(validator_, repository_),
      deactivation_(repository_)
{
}

ResidentServices &ResidentServices::instance()
{
    auto &instance = sharedInstance();

    if (!instance)
    {
        const char *configuredPath = std::getenv("CSMS_DB_PATH");
        instance.reset(new ResidentServices(
            configuredPath != nullptr && *configuredPath != '\0' ? configuredPath
                                                                  : "csms.sqlite3"));
    }

    return *instance;
}

void ResidentServices::useDatabase(const std::string &dbPath)
{
    auto &instance = sharedInstance();
    instance.reset(); // close the old connection before opening another
    instance.reset(new ResidentServices(dbPath));
}
