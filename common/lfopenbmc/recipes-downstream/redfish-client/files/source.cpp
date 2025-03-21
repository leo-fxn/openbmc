#include "source.hpp"

namespace redfish_client_daemon
{

namespace
{
class HttpClientRedfishSource : public IRedfishSource
{
  public:
    explicit HttpClientRedfishSource(std::unique_ptr<HttpClient> httpClient) :
        client(std::move(httpClient))
    {}

    std::string getBody(const std::string& url) override
    {
        auto response = client->get(url.c_str());
        if (response.responseCode != 200)
        {
            throw std::runtime_error("Failed to read redfish json");
        }
        // Ensure that the body is null terminated.
        response.body.push_back(0);
        const char* bodyCstr = response.body.data();
        return bodyCstr;
    }

  private:
    std::unique_ptr<HttpClient> client;
};

} // namespace

std::shared_ptr<IRedfishSource> createHttpClientRedfishSource(
    std::unique_ptr<HttpClient> httpClient)
{
    return std::make_shared<HttpClientRedfishSource>(std::move(httpClient));
}

} // namespace redfish_client_daemon
