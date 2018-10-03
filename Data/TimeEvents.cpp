#include "TimeEvents.h"

TimeEvents::TimeEvents(QObject *parent) : QObject(parent)
{

}

TimeEvents::~TimeEvents()
{

}

void TimeEvents::addEvent(size_t event)
{
    m_timeEvents.push(event);
    emit timeEventAdd();
    emit timeEventsAdd(m_timeEvents.size());
}
