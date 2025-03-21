#include "daemon.hpp"
#include "http_client.hpp"

#include <stdio.h>

#include <phosphor-logging/lg2.hpp>

#include <exception>
#include <fstream>
#include <streambuf>
#include <string>
#include <thread>

PHOSPHOR_LOG2_USING;

using namespace redfish_client_daemon;

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <config-file-path>", argv[0]);
        return 1;
    }
    installSignalHandlers();
    std::ifstream fileStream(argv[1]);
    std::string jsonContents((std::istreambuf_iterator<char>(fileStream)),
                             std::istreambuf_iterator<char>());

    auto daemonConfig = DaemonConfig::fromJson(jsonContents.c_str());

    HttpClient::globalInit();

    // TODO: Increase the pool size after parallelization. Currently it is
    // serial.
    auto httpClient = std::make_unique<HttpClient>(1);
    auto redfishSource = createHttpClientRedfishSource(std::move(httpClient));

    // Create a new scope to ensure the context is destroyed cleanly before
    // http client is destroyed.
    {
        sdbusplus::async::context ctx;
        runDbusServerTillInterrupted(daemonConfig, ctx, redfishSource);
    }

    HttpClient::globalDeinit();

    info("daemon-main clean exit\n");
    return 0;
}
