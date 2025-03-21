#include "source.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace redfish_client_daemon;

class MockHttpClient : public HttpClient
{
  public:
    MockHttpClient() : HttpClient(1) {}
    MOCK_METHOD(HttpResponse, get, (const char* url), (override));
};

namespace
{

static const char* kTestUrl = "http://example.com";

} // namespace

TEST(SourceTest, Success)
{
    auto testClient = std::make_unique<MockHttpClient>();

    // Create the mocked response.
    HttpResponse expectedResponse;
    expectedResponse.responseCode = 200;
    // Note that the payload is not null terminated.
    for (int i = 0; i < 10; i++)
    {
        expectedResponse.body.push_back('0' + i);
    }
    EXPECT_CALL(*testClient, get(testing::StrEq(kTestUrl)))
        .WillOnce(testing::Return(expectedResponse));

    // Try to use the "source".
    auto redfishSource = createHttpClientRedfishSource(std::move(testClient));
    auto body = redfishSource->getBody(kTestUrl);

    // Returned value is properly null terminated.
    EXPECT_STREQ("0123456789", body.c_str());
}

TEST(SourceTest, BadResponseCode)
{
    auto testClient = std::make_unique<MockHttpClient>();

    // Create the mocked response.
    HttpResponse expectedResponse;
    expectedResponse.responseCode = 400;
    EXPECT_CALL(*testClient, get(testing::StrEq(kTestUrl)))
        .WillOnce(testing::Return(expectedResponse));

    // Try to use the "source" and expect an exception.
    auto redfishSource = createHttpClientRedfishSource(std::move(testClient));
    EXPECT_THROW(redfishSource->getBody(kTestUrl), std::runtime_error);
}
