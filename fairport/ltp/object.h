#ifndef FAIRPORT_LTP_OBJECT_H
#define FAIRPORT_LTP_OBJECT_H

#include <type_traits>
#include <functional>
#include <algorithm>
#include <boost/iostreams/concepts.hpp>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244)
#endif
#include <boost/iostreams/stream.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "fairport/util/primatives.h"
#include "fairport/util/errors.h"

#include "fairport/ltp/heap.h"

#include "fairport/ndb/node.h"

namespace fairport
{

class hnid_stream_device : public boost::iostreams::device<boost::iostreams::input_seekable, byte>
{
public:
    std::streamsize read(byte* pbuffer, std::streamsize n)
        { if(m_is_hid) return m_hid_device.read(pbuffer, n); else return m_node_device.read(pbuffer, n); }
    std::streampos seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
        { if(m_is_hid) return m_hid_device.seek(off, way); else return m_node_device.seek(off, way); }

    hnid_stream_device(const hid_stream_device& hid_device) : m_hid_device(hid_device), m_is_hid(true) { }
    hnid_stream_device(const node_stream_device& node_device) : m_node_device(node_device), m_is_hid(false) { }

private:
    hid_stream_device m_hid_device;
    node_stream_device m_node_device;
    bool m_is_hid;

    std::streamsize m_pos;
};

typedef boost::iostreams::stream<hnid_stream_device> prop_stream;

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
    virtual prop_stream open_prop_stream(prop_id id) = 0;

// GCC has time_t defined as a typedef of a long, so calling
// read_prop<long> activates the time_t specialization. I'm
// turning them into first class member functions in GCC for now until
// I figure out a portable way to deal with time.
#ifdef __GNUC__
    time_t read_time_t_prop(prop_id id) const;
    std::vector<time_t> read_time_t_array(prop_id id) const;
#endif

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
#ifdef _MSC_VER
#pragma warning(suppress:4127)
#endif
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
#ifdef _MSC_VER
#pragma warning(suppress:4127)
#endif
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

// See the note in the class definition - convert the time_t read_prop
// specialization into a member function in GCC
#ifdef __GNUC__
inline time_t const_property_object::read_time_t_prop(prop_id id) const
#else
template<>
inline time_t const_property_object::read_prop<time_t>(prop_id id) const
#endif
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

#ifdef __GNUC__
inline std::vector<time_t> const_property_object::read_time_t_array(prop_id id) const
#else
template<>
inline std::vector<time_t> const_property_object::read_prop_array<time_t>(prop_id id) const
#endif
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
        if(buffer.size())
            return std::wstring(reinterpret_cast<wchar_t*>(&buffer[0]), buffer.size()/sizeof(wchar_t));
        else
            return std::wstring();
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
        if(buffer.size())
        {
            std::wstring s((wchar_t*)&buffer[0], buffer.size()/sizeof(wchar_t));
            return std::string(s.begin(), s.end());
        }
        return std::string();
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

#endif
