#pragma once
#include <gtest/gtest-assertion-result.h>

namespace nntp
{

enum class State
{
    disconnected = 1,
    connected,
    error,
};

class Client
{
  public:
    void receive(char const *text);
    State state() const;
    bool postingAllowed() const;

private:
    State m_state{State::disconnected};
    bool m_postingAllowed{};
};

} // namespace nntp
