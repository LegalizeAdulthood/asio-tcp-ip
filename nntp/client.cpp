#include <nntp/Client.h>

#include <cctype>

namespace nntp
{

enum Status
{
    postingOk = 200,
    postingProhibited = 201,
    temporarilyUnavailable = 400,
    permanentlyUnavailable = 502
};

void Client::receive(char const *text)
{
    const int status = (text[0] - '0')*100 + (text[1] - '0')*10 + text[2] - '0';
    switch (status)
    {
    case postingOk:
    case postingProhibited:
        m_state = State::connected;
        m_postingAllowed = text[2] == '0';
        break;

    case temporarilyUnavailable:
    case permanentlyUnavailable:
        m_state = State::disconnected;
        m_postingAllowed = false;
        break;

    default:
        m_state = State::error;
        m_postingAllowed = false;
        break;
    }
}

State Client::state() const
{
    return m_state;
}

bool Client::postingAllowed() const
{
    return m_postingAllowed;
}

} // namespace nntp
