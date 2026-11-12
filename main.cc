#include <drogon/drogon.h>

#include "controllers/ControllerSupport.h"

int main()
{
    drogon::app()
        .setLogLevel(trantor::Logger::kWarn)
        .addListener("127.0.0.1", 8080)
        .setThreadNum(1)
        // Unexpected failures get a generic 500 page; details stay in the log.
        .setExceptionHandler(controller_support::handleUnexpectedException)
        .run();

    return 0;
}