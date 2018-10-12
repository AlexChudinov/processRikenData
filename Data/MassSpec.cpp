#include "Base/BaseObject.h"
#include "MassSpec.h"
#include <QMessageBox>

MassSpec::MassSpec(QObject *parent)
    :
      QObject(parent)
{
    setObjectName("MassSpec");
}

void MassSpec::clear()
{
    mData.clear();
    Q_EMIT cleared();
    Q_EMIT massSpecsNumNotify(0);
}

void MassSpec::blockingClear()
{
    Locker lock(mMutex);
    clear();
}

void MassSpec::blockingNewHist(TimeEventsContainer evts)
{
    Locker lock(mMutex);
    mData.push_back(MapUintUint());
    HistCollection::reverse_iterator currHist = mData.rbegin();
    for(auto evt : evts)
    {
        if(evt)
        {
            MapUintUint::iterator it = currHist->find(evt);
            if(it == currHist->end())
            {
                currHist->operator[](evt) = 1;
            }
            else
            {
                it->second ++;
            }
        }
    }
    Q_EMIT massSpecsNumNotify(mData.size());
}
