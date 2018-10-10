#include <QThreadPool>
#include "BaseObject.h"
#include "Math/ParSplineCalc.h"
#include "Data/TimeEvents.h"
#include "Data/MassSpec.h"

const MyInit * MyInit::s_instance;

MyInit::MyInit(QObject * parent)
    :
      QObject(parent)
{
    if(s_instance) throw std::runtime_error
            ("Tryed to create second MyInit instance!");
    s_instance = this;
    new ParSplineCalc(this);
    new TimeEvents(this);
    new MassSpec(this);
}

MyInit::~MyInit()
{
}

const MyInit &MyInit::instance()
{
    return *s_instance;
}
