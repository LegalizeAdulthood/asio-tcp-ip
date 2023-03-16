#include <nntp/Client.h>

#include <gtest/gtest.h>

class ClientTest : public ::testing::Test
{
protected:
    nntp::Client m_client;
};

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
        m_client.receive("200 OK");
        ASSERT_EQ(nntp::State::connected, m_client.state());
    }
};

class CapabilitiesTest : public CommandTest
{
public:
    void SetUp() override
    {
        m_client.send("CAPABILITIES");
    }
};

TEST_F(CapabilitiesTest, start_capabilities)
{
    ASSERT_EQ(nntp::State::capabilitiesList, m_client.state());
}

TEST_F(CapabilitiesTest, capabilities_begin_list)
{
    m_client.receive("101 Capability list:");

    ASSERT_EQ(nntp::State::capabilitiesList, m_client.state());
}

TEST_F(CapabilitiesTest, capabilities_partial_list)
{
    m_client.receive("101 Capability list:");

    m_client.receive("VERSION 2");

    ASSERT_EQ(nntp::State::capabilitiesList, m_client.state());
}

TEST_F(CapabilitiesTest, capabilities_complete_list)
{
    m_client.receive("101 Capability list:");
    m_client.receive("VERSION 2");
    m_client.receive("IMPLEMENTATION INN 2.6.3");

    m_client.receive(".");

    ASSERT_EQ(nntp::State::connected, m_client.state());
}
