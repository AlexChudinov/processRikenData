#ifndef PACKPROC_H
#define PACKPROC_H

#include <algorithm>
#include <vector>
#include <cassert>
#include <memory>

/**
 * @brief The PackProc class packing procedures
 */
class PackProc
{
public:
    using DataVec = std::vector<char>;

    PackProc();
    virtual ~PackProc();

    virtual DataVec pack(const DataVec& in) = 0;
    virtual DataVec unpack(const DataVec& in) = 0;
};

class ZlibPack : public PackProc
{
public:
    ZlibPack();
    virtual ~ZlibPack();

    virtual DataVec pack(const DataVec& in);
    virtual DataVec unpack(const DataVec& in);
};

/**
* @brief The SimplePack class pack array of integers
*/
template <typename Int> class SimplePack : public PackProc
{
public:
    using Array = std::vector<Int>;
    using BYTE = unsigned char;

    struct Header
    {
        const char tag[4]{ 'B', 'D', 'S', '\0' };
        uint32_t nElems; //number of elements in the init array
        uint32_t nBytes; //number of bytes in packed array
        Int minVal; //smallest element in init array
        Int maxVal; //biggest element in init array
        Int maxFreq; //most frequent element init array
    };

    SimplePack() {}
    virtual ~SimplePack() {}

    virtual DataVec pack(const DataVec& in);
    virtual DataVec unpack(const DataVec& in);

private:
    template<unsigned short N>
    struct SimplePackImpl
    {
        static inline void pack
        (
            const Header * h,
            const Array& in,
            DataVec& indexes,
            Array& values
        )
        {
            indexes.assign(in.size() / 8 + 1, 0x00);
            size_t k = 0;
            for (size_t i = 0; i < in.size(); ++i)
            {
                if (in[i] != h->maxFreq)
                {
                    const size_t n = i / 8;
                    const size_t r = i % 8;
                    indexes[n] |= (0x01 << r);
                    k++;
                }
            }
            if (N > 1)
            {
                const size_t frac = (sizeof(Int) * 8) / N;
                values.assign(k / frac + 1, 0);
                k = 0;
                for (size_t i = 0; i < in.size(); ++i)
                {
                    if (in[i] != h->maxFreq)
                    {
                        Int val = in[i] - h->minVal;
                        const size_t n = k / frac;
                        const size_t r = k % frac;
                        values[n] |= (val << (N * r));
                        k++;
                    }
                }
            }
        }

        static inline void unpack
        (
            const Header * h,
            Array& out,
            const DataVec& indexes,
            const Array& values
        )
        {
            Int setVal = h->minVal == h->maxFreq ? h->maxVal : h->minVal;
            out.assign(h->nElems, h->maxFreq);
            size_t k = 0;
            const size_t frac = sizeof(Int) * 8 / N ;
            const Int mask = (1 << N) - 1;
            for (size_t i = 0; i < out.size(); ++i)
            {
                const size_t n = i / 8;
                const size_t r = i % 8;
                if ((indexes[n] >> r) & 0x01)
                {
                    if (N > 1)
                    {
                        const size_t n = k / frac;
                        const size_t r = k % frac;
                        out[i] = h->minVal + ((values[n] >> (r * N)) & mask);
                    }
                    else
                    {
                        out[i] = setVal;
                    }
                    k++;
                }
            }
        }
    };

    //Found minimum number of bits that is enough to store range from
    //minVal to maxVal
    static inline BYTE bits(Int minVal, Int maxVal)
    {
        Int dI = maxVal - minVal;
        BYTE nBits = 0;
        while (dI != 0)
        {
            dI >>= 1;
            nBits++;
        }
        //Let bits will be the power of 2 in order bits of any integer type
        if (nBits == 0) return 0x00;
        else
        {
            BYTE pow2 = 0x01;
            while (pow2 < nBits)
            {
                pow2 <<= 1;
            }
            return pow2;
        }
    }

    //Found max frequent elem of the array
    static inline Int maxFrequentElem(const Array& a, Int minVal, Int maxVal)
    {
        std::vector<int> counters(maxVal - minVal + 1, 0);
        Int res = 0;
        for (const Int& aa : a)
        {
            counters[aa - minVal] ++;
            if (counters[aa - minVal] > res)
            {
                res = aa - minVal;
            }
        }
        return res + minVal;
    }
};

template<typename Int>
PackProc::DataVec SimplePack<Int>::pack(const PackProc::DataVec &in)
{
    assert(in.size() % sizeof(Int) == 0);
    DataVec res;
    const Int * _SourceFirst = reinterpret_cast<const Int*>(in.data());
    const Int * _SourceLast = reinterpret_cast<const Int*>(in.data() + in.size());
    Array data(_SourceFirst, _SourceLast), packedData;
    DataVec indexes;
    const auto minMax = std::minmax_element(data.begin(), data.end());
    Header h;
    h.nElems = data.size();
    h.minVal = *minMax.first;
    h.maxVal = *minMax.second;
    h.maxFreq = maxFrequentElem(data, h.minVal, h.maxVal);

    switch (bits(h.minVal, h.maxVal))
    {
    case 0: break;
#define CASE(N)\
    case N: \
        SimplePackImpl<N>::pack(&h, data, indexes, packedData);\
        break;
        CASE(1) CASE(2) CASE(4) CASE(8) CASE(16) CASE(32)
#undef CASE
    default:
        throw (std::runtime_error("Incorrect bit number in Simple packing"));
    }
    res.resize(sizeof(Header) + indexes.size() + packedData.size() * sizeof (Int));
    h.nBytes = res.size();
    char * _Cur = res.data();
    std::copy
    (
        reinterpret_cast<char*>(&h),
        reinterpret_cast<char*>(&h) + sizeof(Header),
        _Cur
    );
    _Cur += sizeof(Header);
    std::copy(indexes.begin(), indexes.end(), _Cur);
    _Cur += indexes.size();
    std::copy
    (
        reinterpret_cast<char*>(packedData.data()),
        reinterpret_cast<char*>(packedData.data() + packedData.size()),
        _Cur
    );
    return res;
}

template<typename Int>
PackProc::DataVec SimplePack<Int>::unpack(const PackProc::DataVec &in)
{
    const char * _Cur = in.data();
    const Header* h = reinterpret_cast<const Header*>(_Cur);
    if (h->tag != std::string("BDS"))
        throw (std::runtime_error("Unknown file!"));
    _Cur += sizeof(Header);
    DataVec indexes(h->nElems / 8 + 1);
    std::copy(_Cur, _Cur + indexes.size(), indexes.begin());
    _Cur += indexes.size();
    const size_t nValues = h->nBytes - indexes.size() - sizeof(Header);
    Array values(nValues), res;
    std::copy(_Cur, _Cur + nValues, reinterpret_cast<char*>(values.data()));
    switch (bits(h->minVal, h->maxVal)) {
    case 0: res.assign(h->nElems, h->maxFreq);
#define CASE(N)\
    case N: \
        SimplePackImpl<N>::unpack(h, res, indexes, values);\
        break;
        CASE(1) CASE(2) CASE(4) CASE(8) CASE(16) CASE(32)
#undef CASE
    default:
        throw (std::runtime_error("Incorrect bit number in Simple packing"));
    }

    return DataVec
    (
        reinterpret_cast<char*>(res.data()),
        reinterpret_cast<char*>(res.data() + res.size())
    );
}

/**
 * @brief The SimpleAndZlibPack class packs data using both
 * simple for int values and zlib algorithms
 */
class SimpleAndZlibPack : public PackProc
{
    std::unique_ptr<PackProc> mZlibPack;
    std::unique_ptr<PackProc> mSimplePack;
public:
    SimpleAndZlibPack();
    ~SimpleAndZlibPack();

    DataVec pack(const DataVec& in);
    DataVec unpack(const DataVec& in);
};

#endif // PACKPROC_H
