#include <nntp/client.h>

#include <gtest/gtest.h>

TEST(client, initially_disconnected)
{
    nntp::client client;

    ASSERT_FALSE(client.connected());
}

TEST(client, greeting_connects)
{
    nntp::client client;

    client.receive("200 news.gmane.io InterNetNews NNRP server INN 2.6.3 ready (posting ok)\r\n");

    ASSERT_TRUE(client.connected());
}
