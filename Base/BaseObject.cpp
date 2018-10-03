#include "BaseObject.h"
#include "Math/ParSplineCalc.h"

const MyInit * MyInit::s_instance;

MyInit::MyInit(QObject * parent)
    :
      QObject(parent)
{
    if(s_instance) throw std::runtime_error
            ("Tryed to create second MyInit instance!");
    new ParSplineCalc(this);
    s_instance = this;
}

MyInit::~MyInit()
{
}

const MyInit &MyInit::instance()
{
    return *s_instance;
}

Props::~Props()
{
}
