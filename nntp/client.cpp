#include <nntp/client.h>

namespace nntp
{

enum Status
{
    postingOk = 200,
    postingProhibited = 201,
    temporarilyUnavailable = 400,
    permanentlyUnavailable = 502
};

void Client::send(std::string_view line)
{
    m_state = State::capabilitiesList;
}

void Client::receive(std::string_view line)
{
    if (m_state == State::disconnected)
    {
        const int status = (line[0] - '0') * 100 + (line[1] - '0') * 10 + line[2] - '0';
        switch (status)
        {
        case postingOk:
        case postingProhibited:
            m_state = State::connected;
            m_postingAllowed = line[2] == '0';
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
    else if (m_state == State::capabilitiesList)
    {
        if (line[0] == '.' && line[1] == '\0')
        {
            m_state = State::connected;
        }
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
