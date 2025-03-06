#pragma once

#include "http_client.hpp"

#include <memory>
#include <string>

namespace redfish_client_daemon
{

// Source interface
class IRedfishSource
{
  public:
    IRedfishSource() = default;
    virtual ~IRedfishSource() = default;
    IRedfishSource(const IRedfishSource&) = default;
    IRedfishSource(IRedfishSource&&) = default;
    IRedfishSource& operator=(const IRedfishSource&) = default;
    IRedfishSource& operator=(IRedfishSource&&) = default;

    virtual std::string getBody(const std::string& url) = 0;
};

std::shared_ptr<IRedfishSource>
    createHttpClientRedfishSource(std::unique_ptr<HttpClient> httpClient);

} // namespace redfish_client_daemon
