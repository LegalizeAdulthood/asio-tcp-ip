#include <nntp/client.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

namespace {

class MockServer : public nntp::IServer
{
public:
    MOCK_METHOD(void, setClient, (nntp::IClient *), (override));
    MOCK_METHOD(void, send, (std::string_view), (override));
};

class ServerWithClientSet : public StrictMock<MockServer>
{
public:
    ServerWithClientSet()
    {
        EXPECT_CALL(*this, setClient(NotNull())).WillOnce(SaveArg<0>(&m_client));
    }

    nntp::IClient *m_client{};
};

} // namespace

TEST(BasicClientTest, client_set_on_server)
{
    ServerWithClientSet server;

    nntp::Client client(&server);
}

namespace {

class ClientTest : public Test
{
protected:
    ServerWithClientSet m_server;
    nntp::Client m_client{&m_server};
};

} // namespace

TEST_F(ClientTest, initially_disconnected)
{
    ASSERT_EQ(nntp::State::disconnected, m_client.state());
}

TEST_F(ClientTest, greeting_posting_ok_connects)
{
    m_client.receive("200 news.gmane.io InterNetNews NNRP server INN 2.6.3 ready (posting ok)");

    ASSERT_EQ(nntp::State::connected, m_client.state());
    ASSERT_TRUE(m_client.postingAllowed());
}

TEST_F(ClientTest, greeting_posting_prohibited_connects)
{
    m_client.receive("201 news.gmane.io InterNetNews NNRP server INN 2.6.3 ready (posting prohibited)");

    ASSERT_EQ(nntp::State::connected, m_client.state());
    ASSERT_FALSE(m_client.postingAllowed());
}

TEST_F(ClientTest, temporarily_unavailable)
{
    m_client.receive("400 Service temporarily unavailable");

    ASSERT_EQ(nntp::State::disconnected, m_client.state());
    ASSERT_FALSE(m_client.postingAllowed());
}

TEST_F(ClientTest, permanently_unavailable)
{
    m_client.receive("502 Service permanently unavailable");

    ASSERT_EQ(nntp::State::disconnected, m_client.state());
    ASSERT_FALSE(m_client.postingAllowed());
}

TEST_F(ClientTest, error)
{
    m_client.receive("300 some random message");

    ASSERT_EQ(nntp::State::error, m_client.state());
    ASSERT_FALSE(m_client.postingAllowed());
}

class CommandTest : public ClientTest
{
public:
    void SetUp() override
    {
        ClientTest::SetUp();
        m_client.receive("200 OK");
        ASSERT_EQ(nntp::State::connected, m_client.state());
    }
};

class ClientCapabilitiesTest : public CommandTest
{
public:
    void SetUp() override
    {
        CommandTest::SetUp();
    }
};

TEST_F(ClientCapabilitiesTest, start_capabilities)
{
    EXPECT_CALL(m_server, send(Eq("CAPABILITIES")));

    m_client.capabilities();

    ASSERT_EQ(nntp::State::capabilitiesList, m_client.state());
}

TEST_F(ClientCapabilitiesTest, capabilities_begin_list)
{
    EXPECT_CALL(m_server, send(Eq("CAPABILITIES")))
        .WillOnce(Invoke([this] { m_client.receive("101 Capability list:"); }));

    m_client.capabilities();

    ASSERT_EQ(nntp::State::capabilitiesList, m_client.state());
}

TEST_F(ClientCapabilitiesTest, capabilities_partial_list)
{
    EXPECT_CALL(m_server, send(Eq("CAPABILITIES")))
        .WillOnce(Invoke(
            [this]
            {
                m_client.receive("101 Capability list:");
                m_client.receive("VERSION 2");
            }));
    
    m_client.capabilities();

    ASSERT_EQ(nntp::State::capabilitiesList, m_client.state());
}

TEST_F(ClientCapabilitiesTest, capabilities_complete_list)
{
    EXPECT_CALL(m_server, send(Eq("CAPABILITIES")))
        .WillOnce(Invoke(
            [this]
            {
                m_client.receive("101 Capability list:");
                m_client.receive("VERSION 2");
                m_client.receive("IMPLEMENTATION INN 2.6.3");
                m_client.receive(".");
            }));

    m_client.capabilities();

    ASSERT_EQ(nntp::State::connected, m_client.state());
}
