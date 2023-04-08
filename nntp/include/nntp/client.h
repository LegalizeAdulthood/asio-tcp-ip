#ifndef NNTP_CLIENT_H
#define NNTP_CLIENT_H

#include <memory>
#include <string_view>

namespace nntp
{

class IClient
{
public:
    virtual ~IClient() = default;

    virtual void receive(std::string_view line) = 0;
};

class IServer
{
public:
    virtual ~IServer() = default;

    virtual void setClient(IClient *client) = 0;
    virtual void send(std::string_view line) = 0;
};

enum class State
{
    disconnected = 1,
    connected,
    error,
    capabilitiesList,
};

class Client : public IClient
{
public:
    Client() = default;
    Client(IServer *server) :
        m_server(server)
    {
        m_server->setClient(this);
    }

    void send(std::string_view line);
    void receive(std::string_view line) override;
    State state() const;
    bool postingAllowed() const;

private:
    IServer *m_server{};
    State m_state{State::disconnected};
    bool m_postingAllowed{};
};

} // namespace nntp

#endif
