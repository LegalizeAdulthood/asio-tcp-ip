#include <nntp/client.h>

namespace nntp
{

bool client::connected() const
{
    return m_connected;
}

void client::receive(char const *text)
{
    m_connected = true;
}

} // namespace nntp
