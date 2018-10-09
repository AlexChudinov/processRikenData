#include "BaseObject.h"
#include "Math/ParSplineCalc.h"
#include "Data/TimeEvents.h"

const MyInit * MyInit::s_instance;

MyInit::MyInit(QObject * parent)
    :
      QObject(parent)
{
    if(s_instance) throw std::runtime_error
            ("Tryed to create second MyInit instance!");
    new ParSplineCalc(this);
    new TimeEvents(this);
    s_instance = this;
}

MyInit::~MyInit()
{
}

const MyInit &MyInit::instance()
{
    return *s_instance;
}
