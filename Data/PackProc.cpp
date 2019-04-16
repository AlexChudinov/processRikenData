#include "PackProc.h"
#include <zlib.h>

PackProc::PackProc()
{

}

PackProc::~PackProc()
{

}

ZlibPack::ZlibPack()
{

}

ZlibPack::~ZlibPack()
{

}

PackProc::DataVec ZlibPack::pack(const PackProc::DataVec &in)
{
    const size_t n = in.size();
    assert(std::numeric_limits<uLong>::max() / 12 > n);
    uLong destL = (static_cast<uLong>(n) * 12) / 10;
    DataVec res(destL + sizeof (size_t));
    *reinterpret_cast<size_t*>(res.data()) = n; //keep unpacked size
    char * destFirst = res.data() + sizeof (size_t);
    int err = compress
    (
        reinterpret_cast<Bytef*>(destFirst),
        &destL,
        reinterpret_cast<const Bytef*>(in.data()),
        static_cast<uLong>(n)
    );
    if(err != Z_OK)
        throw(std::runtime_error("Errors during packing!"));
    res.resize(destL + sizeof (size_t));
    return res;
}

PackProc::DataVec ZlibPack::unpack(const PackProc::DataVec &in)
{
    const size_t sourceL = in.size() - sizeof (size_t);
    const Bytef * src = reinterpret_cast<const Bytef*>(in.data() + sizeof (size_t));
    const uLong destL
            = static_cast<uLong>(*reinterpret_cast<const size_t*>(in.data()));
    DataVec res(destL);
    uLong destL2 = destL;
    int err = uncompress
    (
        reinterpret_cast<Bytef*>(res.data()),
        &destL2,
        src,
        static_cast<uLong>(sourceL)
    );
    if(err != Z_OK || destL2 != destL)
        throw(std::runtime_error("Errors during unpacking!"));
    return res;
}
