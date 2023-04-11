#include <boost/asio.hpp>
#include <curses.h>

#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace asio = boost::asio;
namespace ip = asio::ip;

using error_code = boost::system::error_code;
using resolver_results = ip::tcp::resolver::results_type;

class Curses
{
public:
    Curses()
    {
        initscr();
    }
    ~Curses()
    {
        getch();
        endwin();
    }
};

inline int addstr(const std::string &str)
{
    return addstr(str.c_str());
}

class Connection
{
public:
    Connection(const char *name, asio::io_context &context, const resolver_results &results) :
        m_name(name),
        m_socket(context)
    {
        addstr("Connecting to server " + m_name + "...\n");
        refresh();
        asio::async_connect(m_socket, results,
                            [this](const error_code &ec, const ip::tcp::endpoint &endpoint)
                            { handleConnect(ec, endpoint); });
    }

private:
    void handleConnect(const error_code &ec, const ip::tcp::endpoint &endpoint);
    void handleGreeting(const error_code &ec, size_t len);

    std::string     m_name;
    ip::tcp::socket m_socket;
    asio::streambuf m_input;
};

void Connection::handleConnect(const error_code &ec, const ip::tcp::endpoint &endpoint)
{
    if (ec)
    {
        addstr("Error " + ec.what() + " connecting to server " + m_name + '\n');
        return;
    }

    addstr("Awaiting greeting from " + m_name + '\n');
    refresh();
    async_read_until(m_socket, m_input, "\r\n", [this](const error_code &ec, size_t len) { handleGreeting(ec, len); });
}

void Connection::handleGreeting(const error_code &ec, size_t len)
{
    if (ec)
    {
        addstr("Error " + ec.what() + " greeting server " + m_name + '\n');
        return;
    }

    std::string  greeting;
    std::istream str(&m_input);
    std::getline(str, greeting, '\r');
    addstr("Server " + m_name + " sends greeting: " + greeting + '\n');
    refresh();
}

class Reader
{
public:
    Reader() :
        m_resolver(m_context)
    {
    }

    void run();
    void addServer(const char *server);

private:
    void handleResolve(const error_code &ec, const resolver_results &results, const char *server);

    asio::io_context                         m_context;
    ip::tcp::resolver                        m_resolver;
    std::vector<std::shared_ptr<Connection>> m_connections;
};

void Reader::addServer(const char *server)
{
    addstr("Resolving server " + std::string{server} + "...\n");
    refresh();
    m_resolver.async_resolve(server, "nntp",
                             [this, server](const error_code &ec, const resolver_results &results)
                             { handleResolve(ec, results, server); });
}

void Reader::handleResolve(const error_code &ec, const resolver_results &results, const char *server)
{
    if (ec)
    {
        addstr("Error " + ec.what() + " resolving server " + std::string{server} + '\n');
        return;
    }

    m_connections.push_back(std::make_shared<Connection>(server, m_context, results));
}

void Reader::run()
{
    m_context.run();
}

int main()
{
    Curses curses;
    Reader reader;

    const char *servers[]{"news.xmission.com", "news.gmane.io", "news.gwene.org", "news.eternal-september.org"};
    for (const char *server : servers)
    {
        reader.addServer(server);
    }

    reader.run();

    return 0;
}
