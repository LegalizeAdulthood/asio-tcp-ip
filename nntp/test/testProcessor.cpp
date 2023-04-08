#include <nntp/processor.h>

#include <gtest/gtest.h>

TEST(ProcessorTest, connect)
{
    boost::asio::io_context ctx;
    nntp::Processor processor(ctx);

    processor.connect("news.gmane.io");

    ctx.run();
}
