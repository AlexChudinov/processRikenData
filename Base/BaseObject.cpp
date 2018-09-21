#include "BaseObject.h"
#include "Plot/BasePlot.h"
#include "Math/ParSplineCalc.h"

const MyInit * MyInit::s_instance;

MyInit::MyInit(QObject * parent)
    :
      QObject(parent)
{
    if(s_instance) throw std::runtime_error
            ("Try to create second MyInit instance!");
    new ParSplineCalc(this);
    new BasePlot(this);
    s_instance = this;
}

MyInit::~MyInit()
{
}

const MyInit &MyInit::instance()
{
    return *s_instance;
}
