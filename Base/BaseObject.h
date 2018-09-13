#ifndef MATH_OBJ_H
#define MATH_OBJ_H

#include <QVariant>
#include <string>
#include <vector>
#include <map>

class BaseObject;

//Declares updatable objects that can observe the state of the base object
class BaseObserver
{
public:
	virtual void update(const BaseObject* obj) = 0;
};

//Declares some basic properties of math objects
class BaseObject
{
	using VectorObserver = std::vector<BaseObserver*>;
	using VectorObj = std::vector<BaseObject*>;
public:
	using VectorStr = std::vector<std::string>;
	using MapStrUint = std::map<std::string, size_t>;

	static std::string objName(size_t objTypeId);

	void addObserver(BaseObserver * obs);
	void clearObservers();

	BaseObject() {}
	virtual ~BaseObject();

	virtual size_t typeId() const;
	bool isRegistered() const;
	//Returns implemented interface that was registered under typeId
	static BaseObject * getObj(size_t typeId);
	//Returns implemented interface that was registered under name objName
	static BaseObject * getObj(const std::string objName);

	//Returns default state of the object
	virtual QVariantMap defState() const = 0;

	//Returns properties set for a given object
	virtual QVariantMap state() const = 0;

	//Sets properties for a given current object
	virtual void setState(const QVariantMap& properties) = 0;

	static void clearObjRegistry();
protected:
	//Registers new object
	static size_t registerObj(const std::string& objName, BaseObject * obj);

	void updateObservers() const;

private:
	//Observers that watching given object
	VectorObserver m_observers;

	//Registrates all new objects in a table
	static VectorStr s_objectsRegistryType;
	static MapStrUint s_objectsRegistryName;
	static VectorObj s_objects;
};

#define BASE_OBJECT_DUMMY_STATE \
	virtual QVariantMap defState() const { return QVariantMap(); }\
	virtual QVariantMap state() const { return QVariantMap(); }\
	virtual void setState(const QVariantMap&) {}

//Initialises objects
class MyInit
{
public:
	MyInit();
	~MyInit();
};

#endif