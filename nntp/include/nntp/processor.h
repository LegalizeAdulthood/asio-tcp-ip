#pragma once

#include <boost/asio.hpp>

namespace nntp
{

class Processor
{
public:
    Processor(boost::asio::io_context &context) :
        m_context(context),
        m_resolver(context),
        m_socket(context)
    {
    }

    void connect(const char *server);

private:
    using error_code = boost::system::error_code;
    using resolver_results = boost::asio::ip::tcp::resolver::results_type;

    void handle_resolve(const error_code &ec, const resolver_results &results);
    void handle_connect(const error_code &ec, const boost::asio::ip::tcp::endpoint &endpoint);
    void handle_greeting(const error_code &ec, size_t len);

    boost::asio::io_context &m_context;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::streambuf m_input;
};

} // namespace nntp
