#ifndef MATH_OBJ_H
#define MATH_OBJ_H

#include <QVariant>
#include <string>
#include <vector>
#include <map>

/**
 * @brief The Props class reflects properties
 */
class Props
{
public:
    virtual ~Props();

    /**
     * @brief props get object properties
     * @return
     */
    virtual QVariantMap props() const = 0;

    /**
     * @brief setProps set object properties
     * @param props
     */
    virtual void setProps(const QVariantMap& props) = 0;

    /**
     * @brief propType property type
     * @param name
     * @return
     */
    virtual QVariant::Type propType(const QString& name) const = 0;

    /**
     * @brief prop returns particular property
     * @param name
     * @return
     */
    virtual QVariant prop(const QString& name) const = 0;

    /**
     * @brief setProp sets rarticular property
     * @param name
     * @param val
     */
    virtual void setProp(const QString& name, const QVariant& val) = 0;
};

/**
 * @brief The ObjProp class interface to get properties from an object
 */
template<class Obj> class ObjProp
{
    QScopedPointer<Props> m_props;
public:

    ObjProp(const Obj&){}

    virtual ~ObjProp(){}

    Props * props(){ return m_props.get(); }
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
