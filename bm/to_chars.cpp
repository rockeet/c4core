#include <benchmark/benchmark.h>
#include <string>
#include <sstream>
#include <c4/c4_push.hpp>
#include <c4/substr.hpp>
#include <c4/to_chars.hpp>
#include <inttypes.h>
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <vector>


namespace bm = benchmark;

namespace to_chars_alternatives {


template<class T> struct fmtspec;
template<> struct fmtspec< uint8_t> { constexpr static const char fmt[] = "%" PRIu8; };
template<> struct fmtspec<  int8_t> { constexpr static const char fmt[] = "%" PRId8; };
template<> struct fmtspec<uint16_t> { constexpr static const char fmt[] = "%" PRIu16; };
template<> struct fmtspec< int16_t> { constexpr static const char fmt[] = "%" PRId16; };
template<> struct fmtspec<uint32_t> { constexpr static const char fmt[] = "%" PRIu32; };
template<> struct fmtspec< int32_t> { constexpr static const char fmt[] = "%" PRId32; };
template<> struct fmtspec<uint64_t> { constexpr static const char fmt[] = "%" PRIu64; };
template<> struct fmtspec< int64_t> { constexpr static const char fmt[] = "%" PRId64; };
template<> struct fmtspec< float  > { constexpr static const char fmt[] = "%g"; };
template<> struct fmtspec< double > { constexpr static const char fmt[] = "%lg"; };

template<class T>
void sprintf(c4::substr buf, T val)
{
    ::snprintf(buf.str, buf.len, fmtspec<T>::fmt, val);
}


template<class StreamType, class T>
std::string sstream_naive(T const& C4_RESTRICT val)
{
    StreamType ss;
    ss << val;
    return ss.str();
}


template<class StreamType, class T>
std::string sstream_naive_reuse(StreamType &ss, T const& C4_RESTRICT val)
{
    ss.clear();
    ss.str("");
    ss << val;
    return ss.str();
}


} // to_chars_alternatives


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// utilities for use in benchmarks below

template<size_t Dim=128>
struct sbuf
{
    char buf_[Dim];
    c4::substr buf;
    sbuf() : buf_(), buf(buf_) {}
    inline operator c4::substr& () { return buf; }
};

template<class FloatType>
struct ranf
{
    std::vector<FloatType> v;
    size_t curr;
    FloatType next() { FloatType f = v[curr]; curr = (curr + 1) % v.size(); return f; }
    ranf(size_t sz=4096) : v(sz), curr(0) { std::generate(v.begin(), v.end(), std::rand); }
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class IntegralType>
void c4_itoa(bm::State& st)
{
    sbuf<> buf;
    IntegralType i = 0;
    for(auto _ : st)
    {
        ++i;
        c4::itoa(buf, i);
    }
    st.SetBytesProcessed(st.iterations() * sizeof(IntegralType));
}

template<class IntegralType>
void c4_utoa(bm::State& st)
{
    sbuf<> buf;
    IntegralType i = 0;
    for(auto _ : st)
    {
        ++i;
        c4::utoa(buf, i);
    }
    st.SetBytesProcessed(st.iterations() * sizeof(IntegralType));
}

template<class FloatType>
void c4_ftoa(bm::State& st)
{
    sbuf<> buf;
    ranf<FloatType> rans;
    for(auto _ : st)
    {
        c4::ftoa(buf, rans.next());
    }
    st.SetBytesProcessed(st.iterations() * sizeof(FloatType));
}


//-----------------------------------------------------------------------------

template<class IntegralType>
void sprintf_int(bm::State& st)
{
    sbuf<> buf;
    IntegralType i = 0;
    for(auto _ : st)
    {
        ++i;
        to_chars_alternatives::sprintf(buf, i);
    }
    st.SetBytesProcessed(st.iterations() * sizeof(IntegralType));
}

template<class FloatType>
void sprintf_real(bm::State& st)
{
    sbuf<> buf;
    ranf<FloatType> rans;
    for(auto _ : st)
    {
        to_chars_alternatives::sprintf(buf, rans.next());
    }
    st.SetBytesProcessed(st.iterations() * sizeof(FloatType));
}


//-----------------------------------------------------------------------------

template<class IntegralType, class StreamType>
void sstream_naive_int(bm::State& st)
{
    IntegralType i = 0;
    for(auto _ : st)
    {
        ++i;
        std::string out = to_chars_alternatives::sstream_naive<StreamType>(i);
        C4_UNUSED(out);
    }
    st.SetBytesProcessed(st.iterations() * sizeof(IntegralType));
}

template<class FloatType, class StreamType>
void sstream_naive_real(bm::State& st)
{
    ranf<FloatType> rans;
    for(auto _ : st)
    {
        std::string out = to_chars_alternatives::sstream_naive<StreamType>(rans.next());
        C4_UNUSED(out);
    }
    st.SetBytesProcessed(st.iterations() * sizeof(FloatType));
}


//-----------------------------------------------------------------------------

template<class IntegralType, class StreamType>
void sstream_naive_reuse_int(bm::State& st)
{
    IntegralType i = 0;
    StreamType ss;
    for(auto _ : st)
    {
        ++i;
        std::string out = to_chars_alternatives::sstream_naive_reuse(ss, i);
        C4_UNUSED(out);
    }
    st.SetBytesProcessed(st.iterations() * sizeof(IntegralType));
}

template<class FloatType, class StreamType>
void sstream_naive_reuse_real(bm::State& st)
{
    ranf<FloatType> rans;
    StreamType ss;
    for(auto _ : st)
    {
        std::string out = to_chars_alternatives::sstream_naive_reuse(ss, rans.next());
        C4_UNUSED(out);
    }
    st.SetBytesProcessed(st.iterations() * sizeof(FloatType));
}


//-----------------------------------------------------------------------------

BENCHMARK_TEMPLATE(c4_utoa,  uint8_t);
BENCHMARK_TEMPLATE(sprintf_int,  uint8_t);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,  uint8_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,  uint8_t, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,  uint8_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,  uint8_t, std::stringstream);

BENCHMARK_TEMPLATE(c4_itoa,   int8_t);
BENCHMARK_TEMPLATE(sprintf_int,  int8_t);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,   int8_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,   int8_t, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,   int8_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,   int8_t, std::stringstream);

BENCHMARK_TEMPLATE(c4_utoa, uint16_t);
BENCHMARK_TEMPLATE(sprintf_int,  uint16_t);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int, uint16_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int, uint16_t, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_int, uint16_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_int, uint16_t, std::stringstream);

BENCHMARK_TEMPLATE(c4_itoa,  int16_t);
BENCHMARK_TEMPLATE(sprintf_int,  int16_t);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,  int16_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,  int16_t, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,  int16_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,  int16_t, std::stringstream);

BENCHMARK_TEMPLATE(c4_utoa, uint32_t);
BENCHMARK_TEMPLATE(sprintf_int,  uint32_t);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int, uint32_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int, uint32_t, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_int, uint32_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_int, uint32_t, std::stringstream);

BENCHMARK_TEMPLATE(c4_itoa,  int32_t);
BENCHMARK_TEMPLATE(sprintf_int,  int32_t);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,  int32_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,  int32_t, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,  int32_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,  int32_t, std::stringstream);

BENCHMARK_TEMPLATE(c4_utoa, uint64_t);
BENCHMARK_TEMPLATE(sprintf_int,  uint64_t);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int, uint64_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int, uint64_t, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_int, uint64_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_int, uint64_t, std::stringstream);

BENCHMARK_TEMPLATE(c4_itoa,  int64_t);
BENCHMARK_TEMPLATE(sprintf_int,  int64_t);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,  int64_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_int,  int64_t, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,  int64_t, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_int,  int64_t, std::stringstream);

BENCHMARK_TEMPLATE(c4_ftoa,  float);
BENCHMARK_TEMPLATE(sprintf_real,  float);
BENCHMARK_TEMPLATE(sstream_naive_reuse_real,  float, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_real,  float, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_real,  float, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_real,  float, std::stringstream);

BENCHMARK_TEMPLATE(c4_ftoa,  double);
BENCHMARK_TEMPLATE(sprintf_real,  double);
BENCHMARK_TEMPLATE(sstream_naive_reuse_real,  double, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_reuse_real,  double, std::stringstream);
BENCHMARK_TEMPLATE(sstream_naive_real,  double, std::ostringstream);
BENCHMARK_TEMPLATE(sstream_naive_real,  double, std::stringstream);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    bm::Initialize(&argc, argv);
    bm::RunSpecifiedBenchmarks();
    return 0;
}

#include <c4/c4_pop.hpp>