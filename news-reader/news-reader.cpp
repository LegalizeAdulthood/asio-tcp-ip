#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <curses.h>

#include <cstring>
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
        endwin();
    }
};

inline int waddstr(WINDOW *window, const std::string &str)
{
    return waddstr(window, str.c_str());
}

enum class Status
{
    None = 0,
    CapabilityListFollows = 101,
    ServiceAvailablePostingAllowed = 200,
    ServiceAvailablePostingProhibited = 201,
    ServiceTemporarilyUnavailable = 400,
    ServicePermanentlyUnavailable = 502
};

enum class Commands
{
    None = 0,
    Capabilities,
};

class Connection
{
public:
    Connection(const char *name, WINDOW *window, asio::io_context &context, const resolver_results &results) :
        m_name(name),
        m_window(window),
        m_socket(context)
    {
        status("Connecting...");
        asio::async_connect(m_socket, results,
                            [this](const error_code &ec, const ip::tcp::endpoint &endpoint)
                            { handleConnect(ec, endpoint); });
    }

private:
    void status(const std::string &text)
    {
        waddstr(m_window, m_name + ": " + text + '\n');
        wrefresh(m_window);
        napms(50);
    }
    std::string getLine();
    std::string getStatus();
    void        handleConnect(const error_code &ec, const ip::tcp::endpoint &endpoint);
    void        handleGreeting(const error_code &ec, size_t len);
    void        sendCommand(const std::string &command, bool returnsDataBlock = false);
    void        handleCommandWritten(const error_code &ec, size_t len, bool returnsDataBlock);
    void        handleCommandResponse(const error_code &ec, size_t len);
    void        handleDataBlockLine(const error_code &ec, size_t len);
    std::string command()
    {
        return m_command.substr(0, m_command.length() - 2);
    }

    int             m_number;
    std::string     m_name;
    WINDOW         *m_window;
    ip::tcp::socket m_socket;
    asio::streambuf m_input;
    std::string     m_command;
    bool            m_returnsDataBlock{};
    Status          m_status{};
    Status          m_expectedStatus{};
};

std::string Connection::getLine()
{
    std::string  line;
    std::istream str(&m_input);
    std::getline(str, line);
    line.pop_back();
    return line;
}

std::string Connection::getStatus()
{
    std::string line = getLine();
    const int   status = (line[0] - '0') * 100 + (line[1] - '0') * 10 + line[2] - '0';
    m_status = static_cast<Status>(status);
    return line;
}

void Connection::handleConnect(const error_code &ec, const ip::tcp::endpoint &endpoint)
{
    if (ec)
    {
        status("Error " + ec.what() + " connecting to server " + m_name);
        return;
    }

    status("Connected");
    async_read_until(m_socket, m_input, "\r\n", [this](const error_code &ec, size_t len) { handleGreeting(ec, len); });
}

void Connection::handleGreeting(const error_code &ec, size_t len)
{
    if (ec)
    {
        status("Error " + ec.what() + " greeting server " + m_name);
        return;
    }

    status(getStatus());
    if (m_status == Status::ServiceAvailablePostingAllowed || m_status == Status::ServiceAvailablePostingProhibited)
    {
        m_expectedStatus = Status::CapabilityListFollows;
        sendCommand("CAPABILITIES", true);
    }
}

void Connection::sendCommand(const std::string &command, bool returnsDataBlock)
{
    m_command = command + "\r\n";
    m_returnsDataBlock = returnsDataBlock;
    async_write(m_socket, asio::buffer(m_command),
                [this](const error_code &ec, size_t len) { handleCommandWritten(ec, len, false); });
}

void Connection::handleCommandWritten(const error_code &ec, size_t len, bool returnsDataBlock)
{
    if (ec)
    {
        status(m_name + ": Error " + ec.what() + " sending " + command() + " command.");
        return;
    }

    status(command());
    async_read_until(m_socket, m_input, "\r\n",
                     [this](const error_code &ec, size_t len) { handleCommandResponse(ec, len); });
}

void Connection::handleCommandResponse(const error_code &ec, size_t len)
{
    if (ec)
    {
        status(m_name + ": Error " + ec.what() + " reading status for " + command() + " command.");
        return;
    }

    status(getStatus());
    if (m_status != m_expectedStatus)
        return;

    if (m_returnsDataBlock)
    {
        async_read_until(m_socket, m_input, "\r\n",
                         [this](const error_code &ec, size_t len) { handleDataBlockLine(ec, len); });
    }
}

void Connection::handleDataBlockLine(const error_code &ec, size_t len)
{
    if (ec)
    {
        status(m_name + ": Error " + ec.what() + " reading data block.");
        return;
    }

    const std::string line = getLine();
    if (line != ".")
    {
        status(line);
        async_read_until(m_socket, m_input, "\r\n",
                         [this](const error_code &ec, size_t len) { handleDataBlockLine(ec, len); });
    }
    else
    {
        status("Finished.");
    }
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
    void handleResolve(const error_code &ec, const resolver_results &results, const char *server, WINDOW *window);

    asio::io_context                         m_context;
    ip::tcp::resolver                        m_resolver;
    int                                      m_connecting{};
    std::vector<std::shared_ptr<Connection>> m_connections;
};

void Reader::addServer(const char *server)
{
    int height;
    int width;
    getmaxyx(stdscr, height, width);
    const int LINES_PER_WINDOW = (height - 1) / 4;
    const int y = m_connecting * LINES_PER_WINDOW;
    ++m_connecting;
    WINDOW *window = newwin(LINES_PER_WINDOW, 00, y, 0);
    scrollok(window, TRUE);
    waddstr(window, "Resolving server " + std::string{server} + "...\n");
    wrefresh(window);
    m_resolver.async_resolve(server, "nntp",
                             [this, server, window](const error_code &ec, const resolver_results &results)
                             { handleResolve(ec, results, server, window); });
}

void Reader::handleResolve(const error_code &ec, const resolver_results &results, const char *server, WINDOW *window)
{
    if (ec)
    {
        --m_connecting;
        waddstr(window, "Error " + ec.what() + " resolving server " + std::string{server} + '\n');
        refresh();
        return;
    }

    m_connections.push_back(std::make_shared<Connection>(server, window, m_context, results));
}

void Reader::run()
{
    m_context.run();
}

WINDOW *createCommandWindow()
{
    int width;
    int height;
    getmaxyx(stdscr, height, width);
    return newwin(1, 0, height - 1, 0);
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
    WINDOW *commandWin = createCommandWindow();
    waddstr(commandWin, "Press a key to exit.");
    wrefresh(commandWin);

    reader.run();

    wgetch(commandWin);

    return 0;
}
