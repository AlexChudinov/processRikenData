#ifndef MATH_OBJ_H
#define MATH_OBJ_H

#include <QString>
#include <QVariant>

//Declares some basic properties of math objects
class MathObject
{
	QVariantMap m_properties;
public:
	MathObject();
	virtual ~MathObject();

	//Returns the list of all objects that inherit the interface
	virtual const QStringList& objects() const = 0;

	//Returns properties set for a given object
	virtual const QVariantMap& properties() const = 0;

protected:
	
	virtual void setProperties(const QVariantMap& properties) = 0;
};

#endif