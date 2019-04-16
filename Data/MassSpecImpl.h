#ifndef MASSSPECIMPL_H
#define MASSSPECIMPL_H

#include <memory>
#include <map>
#include <vector>

/**
 * @brief The MassSpecImpl class represents base mass spectrum property:
 * return intensity by idx
 */

class PackProc;

class MassSpecImpl
{
public:
    using Map = std::map<int, int>;
    using Vec = std::vector<int>;
    using Pack = std::vector<char>;
    using Packer = std::shared_ptr<PackProc>;
    using MapShrdPtr = std::shared_ptr<Map>;
    using VecShrdPtr = std::shared_ptr<Vec>;

    enum Type
    {
        MassSpecMapType
    };

    /**
     * @brief The PackerLock class unpack/pack data auto
     */
    class PackerLock
    {
        MassSpecImpl * mLocked;
    public:
        PackerLock(MassSpecImpl * lock)
            :
              mLocked(lock)
        {
            mLocked->unpack();
        }
        ~PackerLock()
        {
            mLocked->pack();
        }
    };

    MassSpecImpl();
    virtual ~MassSpecImpl();

    static MassSpecImpl * create(Type type, const Map &ms = Map());
    static MassSpecImpl * create(Type type, const Vec &ms = Vec());
    static void release(MassSpecImpl * ptr);

    virtual Type type() const = 0;

    /**
     * @brief pack packs current mass spectrum
     */
    virtual void pack() = 0;
    /**
     * @brief unpack unpacks current mass spectrum
     */
    virtual void unpack() = 0;

    /**
     * @brief operator [] set/get intensity value at idx
     * @param idx
     * @return
     */
    virtual int operator[](int idx) const = 0;
    virtual int& operator[](int idx) = 0;

    /**
     * @brief isPacked true if array is packed
     * @return
     */
    virtual bool isPacked() const = 0;

    /**
     * @brief addEvent adds new time event into mass spectrum
     * @param evt
     */
    virtual void addEvent(int evt) = 0;
    virtual void addEvents(int time, int nEvents) = 0;

    /**
     * @brief data returns inner data representation
     * @return
     */
    virtual MapShrdPtr data() = 0;
    virtual VecShrdPtr vecData(int minTimeBin, int maxTimeBin) = 0;

    /**
     * @brief first returns first and last elements of the container
     * @return
     */
    virtual Map::value_type first() const = 0;
    virtual Map::value_type last() const = 0;

    /**
     * @brief isEmpty true if inner collection is empty
     * @return
     */
    virtual bool isEmpty() const = 0;
};

/**
 * @brief The MassSpecMap class stores mass spec data in map structure
 */
class MassSpecMap : public MassSpecImpl
{
    Map mData;
    Pack mPackData;
    Packer mPacker;
public:

    MassSpecMap(const Map& data = Map(), bool packData = false);
    MassSpecMap(const Vec& data = Vec(), bool packData = false);
    ~MassSpecMap();

    Type type() const;

    void pack();
    void unpack();

    bool isPacked() const;

    int operator[](int idx) const;
    int& operator[](int idx);

    void addEvent(int evt);
    void addEvents(int time, int nEvents);

    MapShrdPtr data();
    VecShrdPtr vecData(int minTimeBin, int maxTimeBin);

    Map::value_type first() const;
    Map::value_type last() const;

    bool isEmpty() const;
private:
    void increaseEvtIter(Map::iterator it, int nEvents = 1);
};

#endif // MASSSPECIMPL_H
