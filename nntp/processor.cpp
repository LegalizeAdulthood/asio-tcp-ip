#include <nntp/processor.h>

#include <iostream>
#include <string>

namespace nntp
{

namespace asio = boost::asio;
namespace ip = asio::ip;

void Processor::connect(const char *server)
{
    m_resolver.async_resolve(
        server, "nntp", [this](const error_code &ec, const resolver_results &results) { handle_resolve(ec, results); });
}

void Processor::handle_resolve(const error_code &ec, const resolver_results &results)
{
    if (ec)
        return;

    asio::async_connect(m_socket, results,
                        [this](const error_code &ec, const ip::tcp::endpoint &endpoint)
                        { handle_connect(ec, endpoint); });
}

void Processor::handle_connect(const error_code &ec, const ip::tcp::endpoint &endpoint)
{
    if (ec)
        return;

    async_read_until(m_socket, m_input, "\r\n", [this](const error_code &ec, size_t len){ handle_greeting(ec, len); });
}

void Processor::handle_greeting(const error_code &ec, size_t len)
{
    std::istream str(&m_input);
    int status;
    std::string greeting;
    str >> status;
    std::getline(str, greeting);
    std::cout << status << greeting << '\n';
}

} // namespace nntp
