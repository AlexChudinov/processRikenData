#ifndef MATH_OBJ_H
#define MATH_OBJ_H

#include <QVariant>
#include <string>
#include <vector>
#include <map>

//Initialises objects
class MyInit : public QObject
{
    Q_OBJECT

    static const MyInit * s_instance;

public:
    MyInit(QObject * parent = nullptr);
	~MyInit();

    static const MyInit & instance();
};

#endif
