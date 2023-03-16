#include <nntp/Client.h>

#include <gtest/gtest.h>

class ClientTest : public ::testing::Test
{
protected:
    nntp::Client client;
};

TEST_F(ClientTest, initially_disconnected)
{
    ASSERT_EQ(nntp::State::disconnected, client.state());
}

TEST_F(ClientTest, greeting_posting_ok_connects)
{
    client.receive("200 news.gmane.io InterNetNews NNRP server INN 2.6.3 ready (posting ok)");

    ASSERT_EQ(nntp::State::connected, client.state());
    ASSERT_TRUE(client.postingAllowed());
}

TEST_F(ClientTest, greeting_posting_prohibited_connects)
{
    client.receive("201 news.gmane.io InterNetNews NNRP server INN 2.6.3 ready (posting prohibited)");

    ASSERT_EQ(nntp::State::connected, client.state());
    ASSERT_FALSE(client.postingAllowed());
}

TEST_F(ClientTest, temporarily_unavailable)
{
    client.receive("400 Service temporarily unavailable");

    ASSERT_EQ(nntp::State::disconnected, client.state());
    ASSERT_FALSE(client.postingAllowed());
}

TEST_F(ClientTest, permanently_unavailable)
{
    client.receive("502 Service permanently unavailable");

    ASSERT_EQ(nntp::State::disconnected, client.state());
    ASSERT_FALSE(client.postingAllowed());
}

TEST_F(ClientTest, error)
{
    client.receive("300 some random message");

    ASSERT_EQ(nntp::State::error, client.state());
    ASSERT_FALSE(client.postingAllowed());
}
