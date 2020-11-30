#ifndef _C4_FORMAT_HPP_
#define _C4_FORMAT_HPP_

/** @file format.hpp provides type-safe facilities for formatting arguments
 * to string buffers */

#include "c4/charconv.hpp"
#include "c4/blob.hpp"

#include <memory>
#include <functional>

#ifdef _MSC_VER
#   pragma warning(push)
#   if C4_MSVC_VERSION != C4_MSVC_VERSION_2017
#       pragma warning(disable: 4800) // forcing value to bool 'true' or 'false' (performance warning)
#   endif
#   pragma warning(disable: 4996) // snprintf/scanf: this function or variable may be unsafe
#elif defined(__clang__)
#   pragma clang diagnostic push
#elif defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

namespace c4 {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// formatting integral types as booleans

namespace fmt {

/** write a variable as an alphabetic boolean, ie as either true or false
 * @param strict_read */
template<class T>
struct boolalpha_
{
    boolalpha_(T val_, bool strict_read_=false) : val(val_ ? true : false), strict_read(strict_read_) {}
    bool val;
    bool strict_read;
};

template<class T>
boolalpha_<T> boolalpha(T const& val, bool strict_read=false)
{
    return boolalpha_<T>(val, strict_read);
}

/** write a variable as an alphabetic boolean, ie as either true or false */
template<class T>
inline size_t to_chars(substr buf, boolalpha_<T> fmt)
{
    return to_chars(buf, fmt.val ? "true" : "false");
}

} // namespace fmt



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// formatting integral types

namespace fmt {

/** format an integral type with a custom radix */
template<typename T>
struct integral_
{
    T val;
    T radix;
    C4_ALWAYS_INLINE integral_(T val_, T radix_) : val(val_), radix(radix_) {}
};

template<class T>
C4_ALWAYS_INLINE integral_<T> integral(T val, T radix=10)
{
    return integral_<T>(val, radix);
}
template<class T>
C4_ALWAYS_INLINE integral_<intptr_t> integral(T const* val, T radix=10)
{
    return integral_<intptr_t>(reinterpret_cast<intptr_t>(val), static_cast<intptr_t>(radix));
}
template<class T>
C4_ALWAYS_INLINE integral_<intptr_t> integral(std::nullptr_t, T radix=10)
{
    return integral_<intptr_t>(intptr_t(0), static_cast<intptr_t>(radix));
}

#if C4_CPP >= 17
/** format an integral_ type, C++17 version */
template<typename T>
C4_ALWAYS_INLINE void to_chars(substr buf, integral_<T> fmt)
{
    if constexpr (std::is_signed_v<T>)
    {
        return itoa(buf, fmt.val, fmt.radix);
    }
    else
    {
        static_assert(std::is_unsigned_v<T>);
        return utoa(buf, fmt.val, fmt.radix);
    }
}
#else
/** format an integral_ signed type */
template<typename T>
C4_ALWAYS_INLINE
typename std::enable_if<std::is_signed<T>::value, size_t>::type
to_chars(substr buf, integral_<T> fmt)
{
    return itoa(buf, fmt.val, fmt.radix);
}

/** format an integral_ unsigned type */
template<typename T>
C4_ALWAYS_INLINE
typename std::enable_if<std::is_unsigned<T>::value, size_t>::type
to_chars(substr buf, integral_<T> fmt)
{
    return utoa(buf, fmt.val, fmt.radix);
}
#endif


/** format the pointer as an hexadecimal value */
template<class T>
inline integral_<intptr_t> hex(T * v)
{
    return integral_<intptr_t>(reinterpret_cast<intptr_t>(v), intptr_t(16));
}
template<class T>
inline integral_<intptr_t> hex(T const* v)
{
    return integral_<intptr_t>(reinterpret_cast<intptr_t>(v), intptr_t(16));
}
/** format null as an hexadecimal value
 * @overload hex */
inline integral_<intptr_t> hex(std::nullptr_t)
{
    return integral_<intptr_t>(0, intptr_t(16));
}
/** format the integral_ argument as an hexadecimal value
 * @overload hex */
template<class T>
inline integral_<T> hex(T v)
{
    return integral_<T>(v, T(16));
}

/** format the pointer as an octal value */
template<class T>
inline integral_<intptr_t> oct(T const* v)
{
    return integral_<intptr_t>(reinterpret_cast<intptr_t>(v), intptr_t(8));
}
template<class T>
inline integral_<intptr_t> oct(T * v)
{
    return integral_<intptr_t>(reinterpret_cast<intptr_t>(v), intptr_t(8));
}
/** format null as an octal value */
inline integral_<intptr_t> oct(std::nullptr_t)
{
    return integral_<intptr_t>(intptr_t(0), intptr_t(8));
}
/** format the integral_ argument as an octal value */
template<class T>
inline integral_<T> oct(T v)
{
    return integral_<T>(v, T(8));
}

/** format the pointer as a binary 0-1 value
 * @see c4::raw() if you want to use a binary memcpy instead of 0-1 formatting */
template<class T>
inline integral_<intptr_t> bin(T const* v)
{
    return integral_<intptr_t>(reinterpret_cast<intptr_t>(v), intptr_t(2));
}
/** format the pointer as a binary 0-1 value
 * @see c4::raw() if you want to use a binary memcpy instead of 0-1 formatting */
template<class T>
inline integral_<intptr_t> bin(T * v)
{
    return integral_<intptr_t>(reinterpret_cast<intptr_t>(v), intptr_t(2));
}
/** format null as a binary 0-1 value
 * @see c4::raw() if you want to use a binary memcpy instead of 0-1 formatting */
inline integral_<intptr_t> bin(std::nullptr_t)
{
    return integral_<intptr_t>(intptr_t(0), intptr_t(2));
}
/** format the integral_ argument as a binary 0-1 value
 * @see c4::raw() if you want to use a binary memcpy instead of 0-1 formatting */
template<class T>
inline integral_<T> bin(T v)
{
    return integral_<T>(v, T(2));
}

} // namespace fmt


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// formatting real types

namespace fmt {

template<class T>
struct real_
{
    T val;
    int precision;
    RealFormat_e fmt;
    real_(T v, int prec=-1, RealFormat_e f=FTOA_FLOAT) : val(v), precision(prec), fmt(f)  {}
};

template<class T>
real_<T> real(T val, int precision, RealFormat_e fmt=FTOA_FLOAT)
{
    return real_<T>(val, precision, fmt);
}

inline size_t to_chars(substr buf, real_< float> fmt) { return ftoa(buf, fmt.val, fmt.precision, fmt.fmt); }
inline size_t to_chars(substr buf, real_<double> fmt) { return dtoa(buf, fmt.val, fmt.precision, fmt.fmt); }

} // namespace fmt


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// writing raw binary data

namespace fmt {

/** @see blob_ */
template<class T>
struct raw_wrapper_ : public blob_<T>
{
    size_t alignment;

    C4_ALWAYS_INLINE raw_wrapper_(blob_<T> data, size_t alignment_) noexcept
        :
        blob_<T>(data),
        alignment(alignment_)
    {
        C4_ASSERT_MSG(alignment > 0 && (alignment & (alignment - 1)) == 0, "alignment must be a power of two");
    }
};

using const_raw_wrapper = raw_wrapper_<cbyte>;
using raw_wrapper = raw_wrapper_<byte>;

/** mark a variable to be written in raw binary format, using memcpy
 * @see blob_ */
inline const_raw_wrapper craw(cblob data, size_t alignment=alignof(max_align_t))
{
    return const_raw_wrapper(data, alignment);
}
/** mark a variable to be written in raw binary format, using memcpy
 * @see blob_ */
template<class T>
inline const_raw_wrapper craw(T const& data, size_t alignment=alignof(T))
{
    return const_raw_wrapper(cblob(data), alignment);
}

/** mark a variable to be read in raw binary format, using memcpy */
inline raw_wrapper raw(blob data, size_t alignment=alignof(max_align_t))
{
    return raw_wrapper(data, alignment);
}
/** mark a variable to be read in raw binary format, using memcpy */
template<class T>
inline raw_wrapper raw(T & data, size_t alignment=alignof(T))
{
    return raw_wrapper(blob(data), alignment);
}

} // namespace fmt


/** write a variable in raw binary format, using memcpy */
size_t to_chars(substr buf, fmt::const_raw_wrapper r);

/** read a variable in raw binary format, using memcpy */
bool from_chars(csubstr buf, fmt::raw_wrapper *r);
/** read a variable in raw binary format, using memcpy */
inline bool from_chars(csubstr buf, fmt::raw_wrapper r)
{
    return from_chars(buf, &r);
}

/** read a variable in raw binary format, using memcpy */
inline size_t from_chars_first(csubstr buf, fmt::raw_wrapper *r)
{
    return from_chars(buf, r);
}
/** read a variable in raw binary format, using memcpy */
inline size_t from_chars_first(csubstr buf, fmt::raw_wrapper r)
{
    return from_chars(buf, &r);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// @cond dev
// terminates the variadic recursion
inline size_t cat(substr /*buf*/)
{
    return 0;
}
/// @endcond


/** serialize the arguments, concatenating them to the given fixed-size buffer.
 * The buffer size is strictly respected: no writes will occur beyond its end.
 * @return the number of characters needed to write all the arguments into the buffer.
 * @see catrs() if instead of a fixed-size buffer, a resizeable container is desired
 * @see uncat() for the inverse function
 * @see catsep() if a separator between each argument is to be used
 * @see format() if a format string is desired
 * @ingroup formatting_functions */
template<class Arg, class... Args>
size_t cat(substr buf, Arg const& C4_RESTRICT a, Args const& C4_RESTRICT ...more)
{
    size_t num = to_chars(buf, a);
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num += cat(buf, more...);
    return num;
}

/** like cat but return a substr instead of a size, checking
 * @ingroup formatting_functions */
template<class... Args>
substr cat_sub(substr buf, Args && ...args)
{
    size_t sz = cat(buf, std::forward<Args>(args)...);
    C4_CHECK(sz <= buf.len);
    return {buf.str, sz <= buf.len ? sz : buf.len};
}


//-----------------------------------------------------------------------------

/// @cond dev
// terminates the variadic recursion
inline size_t uncat(csubstr /*buf*/)
{
    return 0;
}
/// @endcond


/** deserialize the arguments from the given buffer.
 *
 * @return the number of characters read from the buffer, or csubstr::npos
 *   if a conversion was not successful.
 * @see cat(). uncat() is the inverse of cat().
 * @ingroup formatting_functions */
template<class Arg, class... Args>
size_t uncat(csubstr buf, Arg & C4_RESTRICT a, Args & C4_RESTRICT ...more)
{
    size_t out = from_chars_first(buf, &a);
    if(C4_UNLIKELY(out == csubstr::npos)) return csubstr::npos;
    buf  = buf.len >= out ? buf.sub(out) : substr{};
    size_t num = uncat(buf, more...);
    if(C4_UNLIKELY(num == csubstr::npos)) return csubstr::npos;
    return out + num;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace detail {

template<class Sep>
inline size_t catsep_more(substr /*buf*/, Sep const& C4_RESTRICT /*sep*/)
{
    return 0;
}

template<class Sep, class Arg, class... Args>
size_t catsep_more(substr buf, Sep const& C4_RESTRICT sep, Arg const& C4_RESTRICT a, Args const& C4_RESTRICT ...more)
{
    size_t ret = to_chars(buf, sep), num = ret;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = to_chars(buf, a);
    num += ret;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = catsep_more(buf, sep, more...);
    num += ret;
    return num;
}

template<class Sep>
inline size_t uncatsep_more(csubstr /*buf*/, Sep & /*sep*/)
{
    return 0;
}

template<class Sep, class Arg, class... Args>
size_t uncatsep_more(csubstr buf, Sep & C4_RESTRICT sep, Arg & C4_RESTRICT a, Args & C4_RESTRICT ...more)
{
    size_t ret = from_chars_first(buf, &sep), num = ret;
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = from_chars_first(buf, &a);
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    num += ret;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = uncatsep_more(buf, sep, more...);
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    num += ret;
    return num;
}

} // namespace detail


/** serialize the arguments, concatenating them to the given fixed-size
 * buffer, using a separator between each argument.
 * The buffer size is strictly respected: no writes will occur beyond its end.
 * @return the number of characters needed to write all the arguments into the buffer.
 * @see catseprs() if instead of a fixed-size buffer, a resizeable container is desired
 * @see uncatsep() for the inverse function
 * @see cat() if no separator is needed
 * @see format() if a format string is desired
 * @ingroup formatting_functions */
template<class Sep, class Arg, class... Args>
size_t catsep(substr buf, Sep const& C4_RESTRICT sep, Arg const& C4_RESTRICT a, Args const& C4_RESTRICT ...more)
{
    size_t num = to_chars(buf, a);
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num += detail::catsep_more(buf, sep, more...);
    return num;
}

/** like catsep but return a substr instead of a size
 * @see catsep(). uncatsep() is the inverse of catsep().
 * @ingroup formatting_functions */
template<class... Args>
substr catsep_sub(substr buf, Args && ...args)
{
    size_t sz = catsep(buf, std::forward<Args>(args)...);
    C4_CHECK(sz <= buf.len);
    return {buf.str, sz <= buf.len ? sz : buf.len};
}

/** deserialize the arguments from the given buffer, using a separator.
 *
 * @return the number of characters read from the buffer, or csubstr::npos
 *   if a conversion was not successful
 * @see catsep(). uncatsep() is the inverse of catsep().
 * @ingroup formatting_functions */
template<class Sep, class Arg, class... Args>
size_t uncatsep(csubstr buf, Sep & C4_RESTRICT sep, Arg & C4_RESTRICT a, Args & C4_RESTRICT ...more)
{
    size_t ret = from_chars_first(buf, &a), num = ret;
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = detail::uncatsep_more(buf, sep, more...);
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    num += ret;
    return num;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// @cond dev
// terminates the variadic recursion
inline size_t format(substr buf, csubstr fmt)
{
    return to_chars(buf, fmt);
}
// @endcond


/** using a format string, serialize the arguments into the given
 * fixed-size buffer.
 * The buffer size is strictly respected: no writes will occur beyond its end.
 * In the format string, each argument is marked with a compact
 * curly-bracket pair: {}. Arguments beyond the last curly bracket pair
 * are silently ignored. For example:
 * @code{.cpp}
 * c4::format(buf, "the {} drank {} {}", "partier", 5, "beers"); // the partier drank 5 beers
 * c4::format(buf, "the {} drank {} {}", "programmer", 6, "coffees"); // the programmer drank 6 coffees
 * @endcode
 * @return the number of characters needed to write into the buffer.
 * @see formatrs() if instead of a fixed-size buffer, a resizeable container is desired
 * @see unformat() for the inverse function
 * @see cat() if no format or separator is needed
 * @see catsep() if no format is needed, but a separator must be used
 * @ingroup formatting_functions */
template<class Arg, class... Args>
size_t format(substr buf, csubstr fmt, Arg const& C4_RESTRICT a, Args const& C4_RESTRICT ...more)
{
    auto pos = fmt.find("{}"); // @todo use _find_fmt()
    if(C4_UNLIKELY(pos == csubstr::npos))
    {
        return format(buf, fmt);
    }
    size_t num = to_chars(buf, fmt.sub(0, pos));
    size_t out = num;
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num  = to_chars(buf, a);
    out += num;
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num  = format(buf, fmt.sub(pos + 2), more...);
    out += num;
    return out;
}

/** like format() but return a substr instead of a size
 * @see format()
 * @see catsep(). uncatsep() is the inverse of catsep().
 * @ingroup formatting_functions */
template<class... Args>
substr format_sub(substr buf, csubstr fmt, Args const& C4_RESTRICT ...args)
{
    size_t sz = c4::format(buf, fmt, args...);
    C4_CHECK(sz <= buf.len);
    return {buf.str, sz <= buf.len ? sz : buf.len};
}


//-----------------------------------------------------------------------------

/// @cond dev
// terminates the variadic recursion
inline size_t unformat(csubstr /*buf*/, csubstr /*fmt*/)
{
    return 0;
}
// @endcond


/** using a format string, deserialize the arguments from the given
 * buffer.
 * @return the number of characters read from the buffer, or npos if a conversion failed.
 * @see format() this is the inverse function to format().
 * @ingroup formatting_functions */
template<class Arg, class... Args>
size_t unformat(csubstr buf, csubstr fmt, Arg & C4_RESTRICT a, Args & C4_RESTRICT ...more)
{
    size_t pos = fmt.find("{}");
    if(C4_UNLIKELY(pos == csubstr::npos))
    {
        return unformat(buf, fmt);
    }
    size_t num = pos;
    size_t out = num;
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num  = from_chars_first(buf, &a);
    if(C4_UNLIKELY(num == csubstr::npos)) return csubstr::npos;
    out += num;
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num  = unformat(buf, fmt.sub(pos + 2), more...);
    if(C4_UNLIKELY(num == csubstr::npos)) return csubstr::npos;
    out += num;
    return out;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** a tag type
 * @see catrs
 * @ingroup formatting_functions
 * */
struct append_t {};

/** a tag variable
 * @see catrs
 * @ingroup formatting_functions
 * */
constexpr const append_t append = {};

//-----------------------------------------------------------------------------

/** like cat(), but receives a container, and resizes it as needed to contain
 * the result. The container is overwritten. To append to it, use the append
 * overload.
 *
 * @see cat()
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline void catrs(CharOwningContainer * C4_RESTRICT cont, Args const& C4_RESTRICT ...args)
{
retry:
    substr buf = to_substr(*cont);
    size_t ret = cat(buf, args...);
    cont->resize(ret);
    if(ret > buf.len)
    {
        goto retry;
    }
}

/** like cat(), but creates and returns a new container sized as needed to contain
 * the result.
 *
 * @see cat()
 * @overload catrs
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline CharOwningContainer catrs(Args const& C4_RESTRICT ...args)
{
    CharOwningContainer cont;
    catrs(&cont, args...);
    return cont;
}

/** like cat(), but receives a container, and appends to it instead of
 * overwriting it. The container is resized as needed to contain the result.
 *
 * @return the region newly appended to the original container
 * @see cat()
 * @see catrs()
 * @overload catrs
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline csubstr catrs(append_t, CharOwningContainer * C4_RESTRICT cont, Args const& C4_RESTRICT ...args)
{
    const size_t pos = cont->size();
retry:
    substr buf = to_substr(*cont).sub(pos);
    size_t ret = cat(buf, args...);
    cont->resize(pos + ret);
    if(ret > buf.len)
    {
        goto retry;
    }
    return to_csubstr(*cont).range(pos, cont->size());
}

//-----------------------------------------------------------------------------

/** like catsep(), but receives a container, and resizes it as needed to contain the result.
 * The container is overwritten. To append to the container use the append overload.
 * @see catsep()
 * @ingroup formatting_functions */
template<class CharOwningContainer, class Sep, class... Args>
inline void catseprs(CharOwningContainer * C4_RESTRICT, Sep const& C4_RESTRICT)
{
    return;
}

/** like catsep(), but receives a container, and resizes it as needed to contain the result.
 * The container is overwritten. To append to the container use the append overload.
 * @see catsep()
 * @ingroup formatting_functions */
template<class CharOwningContainer, class Sep, class... Args>
inline void catseprs(CharOwningContainer * C4_RESTRICT cont, Sep const& C4_RESTRICT sep, Args const& C4_RESTRICT ...args)
{
retry:
    substr buf = to_substr(*cont);
    size_t ret = catsep(buf, sep, args...);
    cont->resize(ret);
    if(ret > buf.len)
    {
        goto retry;
    }
}

/** like catsep(), but create a container with the result.
 * @overload catseprs
 * @return the requested container
 * @ingroup formatting_functions */
template<class CharOwningContainer, class Sep, class... Args>
inline CharOwningContainer catseprs(Sep const& C4_RESTRICT sep, Args const& C4_RESTRICT ...args)
{
    CharOwningContainer cont;
    catseprs(&cont, std::cref(sep), args...);
    return cont;
}

/// @cond dev
// terminates the recursion
template<class CharOwningContainer, class Sep, class... Args>
inline csubstr catseprs(append_t, CharOwningContainer * C4_RESTRICT, Sep const& C4_RESTRICT)
{
    csubstr s;
    return s;
}
/// @endcond

/** like catsep(), but receives a container, and appends the arguments, resizing the
 * container as needed to contain the result. The buffer is appended to.
 * @return a csubstr of the appended part
 * @ingroup formatting_functions */
template<class CharOwningContainer, class Sep, class... Args>
inline csubstr catseprs(append_t, CharOwningContainer * C4_RESTRICT cont, Sep const& C4_RESTRICT sep, Args const& C4_RESTRICT ...args)
{
    const size_t pos = cont->size();
retry:
    substr buf = to_substr(*cont).sub(pos);
    size_t ret = catsep(buf, sep, args...);
    cont->resize(pos + ret);
    if(ret > buf.len)
    {
        goto retry;
    }
    return to_csubstr(*cont).range(pos, cont->size());
}


//-----------------------------------------------------------------------------

/** like format(), but receives a container, and resizes it as needed
 * to contain the result.  The container is overwritten. To append to
 * the container use the append overload.
 * @see format()
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline void formatrs(CharOwningContainer * C4_RESTRICT cont, csubstr fmt, Args const& C4_RESTRICT ...args)
{
retry:
    substr buf = to_substr(*cont);
    size_t ret = format(buf, fmt, args...);
    cont->resize(ret);
    if(ret > buf.len)
    {
        goto retry;
    }
}

/**
 * @overload formatrs
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline CharOwningContainer formatrs(csubstr fmt, Args const& C4_RESTRICT ...args)
{
    CharOwningContainer cont;
    formatrs(&cont, fmt, args...);
    return cont;
}

/** like format(), but receives a container, and appends the
 * arguments, resizing the container as needed to contain the
 * result. The buffer is appended to.
 * @return the region newly appended to the original container
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline csubstr formatrs(append_t, CharOwningContainer * C4_RESTRICT cont, csubstr fmt, Args const& C4_RESTRICT ...args)
{
    const size_t pos = cont->size();
retry:
    substr buf = to_substr(*cont).sub(pos);
    size_t ret = format(buf, fmt, args...);
    cont->resize(pos + ret);
    if(ret > buf.len)
    {
        goto retry;
    }
    return to_csubstr(*cont).range(pos, cont->size());
}

} // namespace c4

#ifdef _MSC_VER
#   pragma warning(pop)
#elif defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif

#endif /* _C4_FORMAT_HPP_ */
