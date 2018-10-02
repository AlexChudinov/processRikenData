#ifndef MATH_OBJ_H
#define MATH_OBJ_H

#include <QVariant>
#include <string>
#include <vector>
#include <map>

//Base object properties
class Props
{
public:
    virtual ~Props();
    virtual QVariantMap getProps() const = 0;
    virtual void setProps(const QVariantMap& props) const = 0;
};

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
