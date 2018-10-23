#pragma once

#include <limits.h>
#include <cctype>
#include <regex>
#include <algorithm>
#include <string>
#include <ostream>
#include <iterator>

template <typename T>
class elem_traits
{
  public:
    typedef T value_type;
};

template <typename T, typename U>
bool is_numeric(const T &t)
{
    std::string tmp = t;
    const std::regex re{"^[1-9][0-9]*$"};
    if (std::regex_match(tmp.begin(), tmp.end(), re))
        return true;
    std::cout << "Параметр должен быть числом в интервале от 1 до " << std::numeric_limits<U>::max() << std::endl;
    return false;
}

template <typename T>
bool is_port(const T &t)
{
    std::string tmp = t;
    const std::regex re{"^([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$"};
    if (std::regex_match(tmp.begin(), tmp.end(), re))
        return true;
    std::cout << "TCP port must be in interval  1 - " << std::numeric_limits<unsigned short>::max() << std::endl;
    return false;
}

template <typename T>
class TypeID
{
    static std::size_t counter;

  public:
    static T value()
    {
        counter++;
        if (counter > std::numeric_limits<T>::max())
        {
            counter = 1;
        }
        return counter;
    }
};

template <class T>
std::size_t TypeID<T>::counter = 0;

template <typename T>
struct Property
{
  private:
    T value;

  public:
    Property(const T initial_value)
    {
        *this = initial_value;
    }
    operator T()
    {
        // perform some getter action
        return value;
    }
    T operator=(T new_value)
    {
        // perform some setter action
        return value = new_value;
    }
};
