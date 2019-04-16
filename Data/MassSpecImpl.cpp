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
    case MassSpecVecType: return new MassSpecVec(ms);
    }
    return nullptr;
}

MassSpecImpl *MassSpecImpl::create(MassSpecImpl::Type type, const MassSpecImpl::Vec &ms)
{
    switch(type)
    {
    case MassSpecMapType: return new MassSpecMap(ms);
    case MassSpecVecType: return new MassSpecVec(ms);
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
    if(data.empty()) return;
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
    if(data.empty()) return;
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
    assert(isPacked());
    return *mData.begin();
}

MassSpecImpl::Map::value_type MassSpecMap::last() const
{
    assert(isPacked());
    return *mData.rbegin();
}

bool MassSpecMap::isEmpty() const
{
    return mData.empty() && mPackData.empty();
}

int MassSpecMap::tic(int t0, int t1) const
{
    if(t1 >= t0) return 0;
    int res = 0;
    Map::const_iterator _First = mData.lower_bound(t0);
    Map::const_iterator _Last = mData.upper_bound(t1);
    for(; _First != _Last; ++_First)
    {
        res += _First->second;
    }
    return res;
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



MassSpecVec::MassSpecVec(const MassSpecImpl::Map &ms, bool packData)
    :
      mPacker(new SimpleAndZlibPack)
{
    nTimeZero = ms.empty() ? 0 : ms.begin()->first;
    if(!ms.empty())
    {
        int nMaxTime = ms.rbegin()->first;
        mData.assign(nMaxTime - nTimeZero + 1, 0);
        for(Map::const_reference d : ms)
        {
            mData[d.first - nTimeZero] = d.second;
        }
    }
    if(packData) pack();
}

MassSpecVec::MassSpecVec(const MassSpecImpl::Vec &ms, bool packData)
    :
      mData(ms),
      mPacker(new SimpleAndZlibPack),
      nTimeZero(0)
{
    if(packData) pack();
}

MassSpecVec::~MassSpecVec()
{

}

MassSpecImpl::Type MassSpecVec::type() const
{
    return MassSpecVecType;
}

void MassSpecVec::pack()
{
    using DataVec = PackProc::DataVec;
    if(!isPacked())
    {
        DataVec data
        (
            reinterpret_cast<DataVec::pointer>(mData.data()),
            reinterpret_cast<DataVec::pointer>(mData.data() + mData.size())
        );
        mData.clear();
        mPackData = mPacker->pack(data);
    }
}

void MassSpecVec::unpack()
{
    using DataVec = PackProc::DataVec;
    if(isPacked())
    {
        DataVec data = mPacker->unpack(mPackData);
        mPackData.clear();
        mData.assign
        (
            reinterpret_cast<int*>(data.data()),
            reinterpret_cast<int*>(data.data() + data.size())
        );
    }
}

bool MassSpecVec::isPacked() const
{
    return !mPackData.empty();
}

int MassSpecVec::operator[](int idx) const
{
    assert(!isPacked());
    if(idx >= nTimeZero && idx < nTimeZero + mData.size())
    {
        return mData[idx - nTimeZero];
    }
    else
    {
        return 0;
    }
}

int& MassSpecVec::operator[](int idx)
{
    assert(!isPacked());
    if(idx >= nTimeZero && idx < nTimeZero + mData.size())
    {
        return mData[idx - nTimeZero];
    }
    else
    {
        throw std::out_of_range("Mass spec storage is out of range!");
    }
}

void MassSpecVec::addEvent(int evt)
{
    if(evt != 0)
    {
        if(isPacked()) unpack();
        extendDataToKeepEvent(evt);
        mData[evt - nTimeZero] ++;
    }
}

void MassSpecVec::addEvents(int time, int nEvents)
{
    if(time != 0)
    {
        addEvent(time);
        mData[time - nTimeZero] += nEvents - 1; //1 already added above
    }
}

MassSpecImpl::MapShrdPtr MassSpecVec::data()
{
    MassSpecImpl::PackerLock lock(this);
    return MassSpecMap(mData).data();
}

MassSpecImpl::VecShrdPtr MassSpecVec::vecData(int minTimeBin, int maxTimeBin)
{
    std::unique_ptr<MassSpecImpl::PackerLock> packerLock;
    if(isPacked()) packerLock.reset(new PackerLock(this));
    Vec res(maxTimeBin - minTimeBin + 1, 0);
    for(size_t i = 0; minTimeBin <= maxTimeBin; ++minTimeBin, ++i)
    {
        res[i] = static_cast<const MassSpecImpl*>(this)->operator[](minTimeBin);
    }
    return VecShrdPtr(new Vec(std::move(res)));
}

MassSpecImpl::Map::value_type MassSpecVec::first() const
{
    assert(!isPacked());
    return MassSpecImpl::Map::value_type(nTimeZero, mData[0]);
}

MassSpecImpl::Map::value_type MassSpecVec::last() const
{
    assert(!isPacked());
    return MassSpecImpl::Map::value_type(nTimeZero + mData.size() - 1, mData.back());
}

bool MassSpecVec::isEmpty() const
{
    return mPackData.empty() && mData.empty();
}

int MassSpecVec::tic(int t0, int t1) const
{
    t0 -= nTimeZero;
    t1 -= nTimeZero;
    if(t1 > mData.size()) t1 = mData.size();
    if(t0 >= t1) return 0;
    int res = 0;
    for(; t0 < t1; ++t0)
    {
        res += mData[t0];
    }
    return res;
}

void MassSpecVec::extendDataToKeepEvent(int evt)
{
    if(evt < nTimeZero)
    {
        Vec front(nTimeZero - evt, 0);
        front.insert(front.end(), mData.begin(), mData.end());
        mData = front;
        nTimeZero = evt;
    }
    else if(evt >= nTimeZero + mData.size())
    {
        mData.insert(mData.end(), evt - (nTimeZero + mData.size()) + 1, 0);
    }
}
