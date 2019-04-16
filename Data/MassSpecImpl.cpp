#include "MassSpecImpl.h"
#include "PackProc.h"
#include <algorithm>
#include <cassert>

MassSpecImpl::MassSpecImpl()
{

}

MassSpecImpl::~MassSpecImpl()
{

}

MassSpecImpl * MassSpecImpl::create(Type type, const Map& ms)
{
    switch(type)
    {
        case MassSpecMapType: return new MassSpecMap(ms);
    }
    return nullptr;
}

MassSpecImpl *MassSpecImpl::create(MassSpecImpl::Type type, const MassSpecImpl::Vec &ms)
{
    switch(type)
    {
        case MassSpecMapType: return new MassSpecMap(ms);
    }
    return nullptr;
}

void MassSpecImpl::release(MassSpecImpl *ptr)
{
    delete ptr;
}

MassSpecMap::MassSpecMap(const Map &data, bool packData)
    :
      mPacker(new ZlibPack)
{
    int minVal = std::min_element
    (
        data.begin(),
        data.end(),
        [](Map::const_reference a, Map::const_reference b)->bool
        {
            return a.second < b.second;
        }
    )->second;
    for(const auto& d : data)
    {
        if(d.second != minVal)
        {
            addEvents(d.first, d.second - minVal);
        }
    }
    if(packData) pack();
}

MassSpecMap::MassSpecMap(const Vec& data, bool packData)
    :
      mPacker(new ZlibPack)
{
    int minVal = *std::min_element(data.begin(), data.end());
    for(size_t i = 0; i < data.size(); i++)
    {
        if(data[i] != minVal)
        {
            addEvents(i+1, data[i] - minVal);
        }
    }
    if(packData) pack();
}

MassSpecMap::~MassSpecMap()
{

}

MassSpecImpl::Type MassSpecMap::type() const
{
    return MassSpecMapType;
}

void MassSpecMap::pack()
{
    if (isPacked()) return;
    PackProc::DataVec vec(mData.size() * sizeof (Map::value_type));
    char * _End = vec.data() + vec.size();
    Map::const_iterator it = mData.begin();
    for
    (
        char * _First = vec.data();
        _First != _End;
        _First += sizeof (Map::value_type), ++it
    )
    {
        *reinterpret_cast<std::pair<int, int>*>(_First) = *it;
    }
    mPackData = mPacker->pack(vec);
    mData.clear();
}

void MassSpecMap::unpack()
{
    if (!isPacked()) return;
    PackProc::DataVec unpackedData = mPacker->unpack(mPackData);
    char * _End = unpackedData.data() + unpackedData.size();
    for
    (
        char * _First = unpackedData.data();
        _First != _End;
        _First += sizeof (Map::value_type)
    )
    {
        mData.insert(*reinterpret_cast<Map::const_pointer>(_First));
    }
    mPackData.clear();
}

bool MassSpecMap::isPacked() const
{
    return !mPackData.empty();
}

int MassSpecMap::operator[](int idx) const
{
    assert(!isPacked());
    Map::const_iterator it = mData.find(idx);
    if(it != mData.end()) return it->second;
    else return 0;
}

int &MassSpecMap::operator[](int idx)
{
    assert(!isPacked());
    return mData[idx];
}

void MassSpecMap::addEvent(int evt)
{
    if(evt != 0) //Skip start events
    {
        if(isPacked()) unpack();
        Map::iterator it = mData.insert({evt, 0}).first;
        increaseEvtIter(it);
    }
}

void MassSpecMap::addEvents(int time, int nEvents)
{
    if(time != 0) //Skip start events
    {
        if(isPacked()) unpack();
        Map::iterator it = mData.insert({time, 0}).first;
        increaseEvtIter(it, nEvents);
    }
}

MassSpecImpl::MapShrdPtr MassSpecMap::data()
{
    if(isPacked())
    {
        PackerLock lock(this);
        return MapShrdPtr(new Map(mData));
    }
    return MapShrdPtr(new Map(mData));
}

MassSpecImpl::VecShrdPtr MassSpecMap::vecData(int minTimeBin, int maxTimeBin)
{
    VecShrdPtr res(new Vec(maxTimeBin - minTimeBin + 1));
    for(int i = 0; minTimeBin <= maxTimeBin; ++i, ++minTimeBin)
    {
        (*res)[i] = static_cast<const MassSpecImpl*>(this)->operator[](i);
    }
    return res;
}

MassSpecImpl::Map::value_type MassSpecMap::first() const
{
    return *mData.begin();
}

MassSpecImpl::Map::value_type MassSpecMap::last() const
{
    return *mData.rbegin();
}

bool MassSpecMap::isEmpty() const
{
    return mData.empty();
}

void MassSpecMap::increaseEvtIter(Map::iterator it, int nEvents)
{
    it->second += nEvents;
    if(it == mData.begin() || std::prev(it)->first != it->first - 1)
    {
        mData.insert(it, {it->first - 1, 0});
    }
    if(std::next(it) == mData.end() || std::next(it)->first != it->first + 1)
    {
        mData.insert(std::next(it), {it->first + 1, 0});
    }
}


