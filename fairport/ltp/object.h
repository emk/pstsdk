#ifndef FAIRPORT_LTP_OBJECT_H
#define FAIRPORT_LTP_OBJECT_H

#include <type_traits>
#include <functional>
#include <algorithm>

#include "fairport/util/primatives.h"
#include "fairport/util/errors.h"

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif

namespace fairport
{

class const_property_object
{
public:
    virtual ~const_property_object() { }
    virtual std::vector<prop_id> get_prop_list() const = 0;
    virtual prop_type get_prop_type(prop_id id) const = 0;
    virtual bool prop_exists(prop_id id) const = 0;

    template<typename T>
    T read_prop(prop_id id) const;
    template<typename T>
    std::vector<T> read_prop_array(prop_id id) const;

protected:
    virtual byte get_value_1(prop_id id) const = 0;
    virtual ushort get_value_2(prop_id id) const = 0;
    virtual ulong get_value_4(prop_id id) const = 0;
    virtual ulonglong get_value_8(prop_id id) const = 0;
    virtual std::vector<byte> get_value_variable(prop_id id) const = 0;
};

} // end fairport namespace

template<typename T>
inline T fairport::const_property_object::read_prop(prop_id id) const
{
    if(!std::is_pod<T>::value)
        throw std::invalid_argument("T must be a POD or one of the specialized classes");

    if(sizeof(T) == sizeof(ulonglong))
    {
        return (T)get_value_8(id);
    }
    else if(sizeof(T) == sizeof(ulong))
    {
        return (T)get_value_4(id);
    }
    else if(sizeof(T) == sizeof(ushort))
    {
        return (T)get_value_2(id);
    }
    else if(sizeof(T) == sizeof(byte))
    {
        return (T)get_value_1(id);
    }
    else 
    {
        std::vector<byte> buffer = get_value_variable(id); 
        return *(T*)&buffer[0];
    }
}

template<typename T>
inline std::vector<T> fairport::const_property_object::read_prop_array(prop_id id) const
{
    if(!std::is_pod<T>::value)
        throw std::invalid_argument("T must be a POD or one of the specialized classes");

    std::vector<byte> buffer = get_value_variable(id); 
    return std::vector<T>((T*)&buffer[0], (T*)&buffer[buffer.size()]);
}

namespace fairport 
{

template<>
inline bool const_property_object::read_prop<bool>(prop_id id) const
{
    return get_value_4(id) != 0;
}

template<>
inline std::vector<bool> fairport::const_property_object::read_prop_array<bool>(prop_id id) const
{
    using namespace std::placeholders;

    std::vector<ulong> values = read_prop_array<ulong>(id);
    std::vector<bool> results(values.size());
    std::transform(values.begin(), values.end(), results.begin(), std::bind(std::not_equal_to<ulong>(), 0, _1));
    return results;
}

template<>
inline time_t const_property_object::read_prop<time_t>(prop_id id) const
{
    if(get_prop_type(id) == prop_type_apptime)
    {
        double time_value = read_prop<double>(id);
        return vt_date_to_time_t(time_value);
    }
    else
    {
        ulonglong time_value = read_prop<ulonglong>(id);
        return filetime_to_time_t(time_value);
    }
}

template<>
inline std::vector<time_t> const_property_object::read_prop_array<time_t>(prop_id id) const
{
    if(get_prop_type(id) == prop_type_mv_apptime)
    {
        std::vector<double> time_values = read_prop_array<double>(id);
        std::vector<time_t> result(time_values.size());
        std::transform(time_values.begin(), time_values.end(), result.begin(), vt_date_to_time_t);
        return result;
    }
    else
    {   
        std::vector<ulonglong> time_values = read_prop_array<ulonglong>(id);
        std::vector<time_t> result(time_values.size());
        std::transform(time_values.begin(), time_values.end(), result.begin(), filetime_to_time_t);
        return result;
    }

}

template<>
inline std::wstring const_property_object::read_prop<std::wstring>(prop_id id) const
{
    std::vector<byte> buffer = get_value_variable(id); 

    if(get_prop_type(id) == prop_type_string)
    {
        std::string s(buffer.begin(), buffer.end());
        return std::wstring(s.begin(), s.end());
    }
    else
    {
        return std::wstring((wchar_t*)&buffer[0], buffer.size()/sizeof(wchar_t));
    }
}

template<>
inline std::vector<std::wstring> const_property_object::read_prop_array<std::wstring>(prop_id) const
{
    throw not_implemented("const_property_object::read_prop_array<wstring>");
}

template<>
inline std::string const_property_object::read_prop<std::string>(prop_id id) const
{
    std::vector<byte> buffer = get_value_variable(id); 

    if(get_prop_type(id) == prop_type_string)
    {
        return std::string(buffer.begin(), buffer.end());
    }
    else
    {
        std::wstring s((wchar_t*)&buffer[0], buffer.size()/sizeof(wchar_t));
        return std::string(s.begin(), s.end());
    }
}

template<>
inline std::vector<std::string> const_property_object::read_prop_array<std::string>(prop_id) const
{
    throw not_implemented("const_property_object::read_prop_array<string>");
}

template<>
inline std::vector<byte> const_property_object::read_prop<std::vector<byte>>(prop_id id) const
{
    return get_value_variable(id); 
}

template<>
inline std::vector<std::vector<byte>> const_property_object::read_prop_array<std::vector<byte>>(prop_id) const
{
    throw not_implemented("const_property_object::read_prop_array<vector<byte>>");
}

} // end fairport namespace

#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

#endif
