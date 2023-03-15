#pragma once

namespace nntp
{

class client
{
  public:
    bool connected() const;
    void receive(char const *text);

  private:
    bool m_connected{};
};

} // namespace nntp
