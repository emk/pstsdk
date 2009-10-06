#ifndef FAIRPORT_UTIL_UTIL_H
#define FAIRPORT_UTIL_UTIL_H

#include <cstdio>
#include <time.h>
#include <memory>
#include <vector>

#include "fairport/util/errors.h"
#include "fairport/util/primatives.h"

namespace fairport
{

class file
{
public:
    file(const std::wstring& filename);
    ~file();
    
    file(file&& other);
    file& operator=(file&& other);

    size_t read(std::vector<byte>& buffer, ulonglong offset) const;
    size_t write(const std::vector<byte>& buffer, ulonglong offset);

private:
    file(const file& other); // = delete
    file& operator=(const file& other); // = delete
    std::wstring m_filename;
    FILE * m_pfile;
};


// FILETIME is a Win32 date/time type representing the number of 100 ns
// intervals since Jan 1st, 1601
time_t filetime_to_time_t(ulonglong filetime);
ulonglong time_t_to_filetime(time_t time);

// VT_DATE is an OLE date/time type where the integer part is the date and
// the fraction part is the time
time_t vt_date_to_time_t(double vt_time);
double time_t_to_vt_date(time_t time);

bool test_bit(const byte* pbytes, ulong bit);

} // end fairport namespace

inline fairport::file::file(const std::wstring& filename)
: m_filename(filename)
{
#ifdef HIDDEN_API
    const char* mode = "r+b";
#else
    const char* mode = "rb";
#endif

#ifdef _MSC_VER 
    errno_t err = fopen_s(&m_pfile, std::string(filename.begin(), filename.end()).c_str(), mode);
    if(err != 0)
        m_pfile = NULL;
#else
    m_pfile = fopen(std::string(filename.begin(), filename.end()).c_str(), mode);
#endif
    if(m_pfile == NULL)
        throw std::runtime_error("fopen failed");
}

inline fairport::file::file(file&& other)
: m_filename(std::move(other.m_filename)), m_pfile(other.m_pfile)
{
    other.m_pfile = NULL;
}

inline fairport::file& fairport::file::operator=(file&& other)
{
    std::swap(m_pfile, other.m_pfile);
    std::swap(m_filename, other.m_filename);
}

inline fairport::file::~file()
{
    fflush(m_pfile);
    fclose(m_pfile);
}

inline size_t fairport::file::read(std::vector<byte>& buffer, ulonglong offset) const
{
#ifdef _MSC_VER
    if(_fseeki64(m_pfile, offset, SEEK_SET) != 0)
#else
    if(fseek(m_pfile, offset, SEEK_SET) != 0)
#endif
    {
        throw std::out_of_range("fseek failed");
    }

    size_t read = fread(&buffer[0], 1, buffer.size(), m_pfile);

    if(read != buffer.size())
        throw std::out_of_range("fread failed");

    return read;
}

inline size_t fairport::file::write(const std::vector<byte>& buffer, ulonglong offset)
{
#ifdef _MSC_VER
    if(_fseeki64(m_pfile, offset, SEEK_SET) != 0)
#else
    if(fseek(m_pfile, offset, SEEK_SET) != 0)
#endif
    {
        throw std::out_of_range("fseek failed");
    }

    size_t write = fwrite(&buffer[0], 1, buffer.size(), m_pfile);

    if(write != buffer.size())
        throw std::out_of_range("fwrite failed");

    return write;
}

inline time_t fairport::filetime_to_time_t(ulonglong filetime)
{
    ulonglong jan1970 = 116444736000000000ULL;

    return (filetime - jan1970) / 10000000;
}

inline fairport::ulonglong fairport::time_t_to_filetime(time_t time)
{
    ulonglong jan1970 = 116444736000000000ULL;

    return (time * 10000000) + jan1970;
}

inline time_t fairport::vt_date_to_time_t(double)
{
    throw not_implemented("vt_date_to_time_t");
}

inline double fairport::time_t_to_vt_date(time_t)
{
    throw not_implemented("vt_date_to_time_t");
}

inline bool fairport::test_bit(const byte* pbytes, ulong bit)
{
    return (*(pbytes + (bit >> 3)) & (0x80 >> (bit & 7))) != 0;
}

#endif
