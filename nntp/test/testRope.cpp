#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <string_view>
#include <vector>

class Rope
{
public:
    Rope() = default;
    Rope(std::string_view span)
    {
        if (!span.empty())
            m_spans.push_back(span);
    }
    [[nodiscard]] bool empty() const
    {
        return m_spans.empty();
    }
    void append(std::string_view span)
    {
        m_spans.push_back(span);
    }

    using const_iterator = std::vector<std::string_view>::const_iterator;
    [[nodiscard]] const_iterator begin() const { return m_spans.cbegin(); }
    [[nodiscard]] const_iterator end() const { return m_spans.cend(); }

    using size_type = std::string_view::size_type;
    size_type length() const
    {
        size_type result{};
        for (const std::string_view &span : m_spans)
        {
            result += span.length();
        }
        return result;
    }

private:
    std::vector<std::string_view> m_spans;

    friend bool operator==(std::string_view lhs, const Rope &rope);
};

bool operator==(std::string_view lhs, const Rope &rope)
{
    if (lhs.empty())
    {
        return lhs.empty() == rope.empty();
    }
    bool equal = false;
    for (const std::string_view &rhs : rope.m_spans)
    {
        if (lhs.length() < rhs.length())
            return false;
        if (lhs.length() == rhs.length())
            equal = lhs == rhs;
        else
        {
            equal = lhs.substr(0, rhs.length()) == rhs;
            lhs = lhs.substr(rhs.length());
        }
        if (!equal)
            break;
    }
    return equal;
}
bool operator==(const Rope &rope, std::string_view span)
{
    return span == rope;
}

const char *const ARBITRARY_TEXT{"arbitrary text"};

TEST(RopeTest, defaultConstructedRopeIsEmpty)
{
    Rope rope;

    ASSERT_TRUE(rope.empty());
    ASSERT_EQ(0, rope.length());
}

TEST(RopeTest, constructedFromLiteralIsNotEmpty)
{
    Rope rope(ARBITRARY_TEXT);

    ASSERT_FALSE(rope.empty());
}

TEST(RopeTest, constructedFromEmptyLiteralIsEmpty)
{
    Rope rope("");

    ASSERT_TRUE(rope.empty());
}

TEST(RopeTest, comparesToStringViewLeft)
{
    Rope rope(ARBITRARY_TEXT);

    ASSERT_EQ(std::string_view{ARBITRARY_TEXT}, rope);
}

TEST(RopeTest, comparesToStringViewRight)
{
    Rope rope(ARBITRARY_TEXT);

    ASSERT_EQ(rope, std::string_view{ARBITRARY_TEXT});
}

TEST(RopeTest, comparesToCStringLeft)
{
    Rope rope(ARBITRARY_TEXT);

    ASSERT_EQ(ARBITRARY_TEXT, rope);
}

TEST(RopeTest, comparesToCStringRight)
{
    Rope rope(ARBITRARY_TEXT);

    ASSERT_EQ(rope, ARBITRARY_TEXT);
}

TEST(RopeTest, comparesToStringLeft)
{
    Rope rope(ARBITRARY_TEXT);

    ASSERT_EQ(std::string{ARBITRARY_TEXT}, rope);
}

TEST(RopeTest, comparesToStringRight)
{
    Rope rope(ARBITRARY_TEXT);

    ASSERT_EQ(rope, std::string{ARBITRARY_TEXT});
}

TEST(RopeTest, append)
{
    Rope rope{"Hello"};

    rope.append(", ");
    rope.append("world");
    rope.append("!");

    ASSERT_EQ("Hello, world!", rope);
}

TEST(RopeTest, rangeFor)
{
    Rope rope{"Hello"};
    rope.append(", ");
    rope.append("world");
    rope.append("!");
    std::string str;

    for (std::string_view span : rope)
    {
        str += span;
    }

    ASSERT_EQ("Hello, world!", str);
}

TEST(RopeTest, length)
{
    Rope rope{"Hello"};
    rope.append(", ");
    rope.append("world");
    rope.append("!");
    
    ASSERT_EQ(std::strlen("Hello, world!"), rope.length());
}
