#ifndef FAIRPORT_UTIL_ERRORS_H
#define FAIRPORT_UTIL_ERRORS_H

#include <stdexcept>

namespace fairport
{

class can_not_resize : public std::runtime_error
{
public:
    explicit can_not_resize(const std::string& error)
        : runtime_error(error) { }
};

class not_implemented : public std::logic_error
{
public:
    explicit not_implemented(const std::string& error)
        : logic_error(error) { }
};

class write_error : public std::runtime_error
{
public:
    explicit write_error(const std::string& error)
        : runtime_error(error) { }
};

class database_corrupt : public std::runtime_error 
{
public:
    explicit database_corrupt(const std::string& error)
        : runtime_error(error) { }
};

class invalid_format : public database_corrupt
{
public:
    invalid_format()
        : database_corrupt("Unexpected Database Format") { }
};

class unexpected_page : public database_corrupt
{
public:
    explicit unexpected_page(const std::string& error)
        : database_corrupt(error) { }
};

class unexpected_block : public database_corrupt
{
public:
    explicit unexpected_block(const std::string& error)
        : database_corrupt(error) { }
};

class crc_fail : public database_corrupt
{
public:
    crc_fail(const std::string& error, ulonglong location, block_id id, ulong actual, ulong expected)
        : database_corrupt(error), m_location(location), m_id(id), m_actual(actual), m_expected(expected) { }
private:
    ulonglong m_location;
    block_id m_id;
    ulong m_actual;
    ulong m_expected;
};

class sig_mismatch : public database_corrupt
{
public:
    sig_mismatch(const std::string& error, ulonglong location, block_id id, ulong actual, ulong expected)
        : database_corrupt(error), m_location(location), m_id(id), m_actual(actual), m_expected(expected) { }
private:
    ulonglong m_location;
    block_id m_id;
    ulong m_actual;
    ulong m_expected;
};

template<typename K>
class key_not_found : public std::exception
{
public:
    explicit key_not_found(const K& k) 
        : m_k(k) { } 

    const char* what() const throw()
        { return "key not found"; }

    const K& which() const
        { return m_k; }
private:
    key_not_found<K>& operator=(const key_not_found<K>&);
    const K m_k;
};


} // end namespace

#endif
