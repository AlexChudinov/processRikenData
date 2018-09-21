#include "BaseObject.h"
#include "Plot/BasePlot.h"
#include "Math/ParSplineCalc.h"

MyInit MyInit::s_instance;

MyInit::MyInit(QObject * parent)
    :
      QObject(parent)
{
    new ParSplineCalc(this);
    new BasePlot(this);
}

MyInit::~MyInit()
{
}

const MyInit &MyInit::instance()
{
    return  s_instance;
}
