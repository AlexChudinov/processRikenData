#include "BaseObject.h"
#include "ThreadPool.h"
#include "Math\ParSplineCalc.h"

BaseObject::VectorStr BaseObject::s_objectsRegistryType;
BaseObject::MapStrUint BaseObject::s_objectsRegistryName;
BaseObject::VectorObj BaseObject::s_objects;

std::string BaseObject::objName(size_t objTypeId)
{
	if (objTypeId < s_objectsRegistryType.size())
	{
		return s_objectsRegistryType[objTypeId];
	}
	else
	{
		return std::string();
	}
}

void BaseObject::addObserver(BaseObserver * obs)
{
	m_observers.push_back(obs);
}

void BaseObject::clearObservers()
{
	for (BaseObserver * obs : m_observers)
	{
		delete obs;
	}
	m_observers.clear();
}

BaseObject::~BaseObject()
{
	clearObservers();
}

size_t BaseObject::typeId() const
{
	return s_objectsRegistryType.size();
}

bool BaseObject::isRegistered() const
{
	return typeId() < s_objectsRegistryName.size();
}

BaseObject * BaseObject::getObj(size_t typeId)
{
	if (typeId < s_objects.size())
	{
		return s_objects[typeId];
	}
	else
	{
		return nullptr;
	}
}

BaseObject * BaseObject::getObj(const std::string objName)
{
	MapStrUint::const_iterator it = s_objectsRegistryName.find(objName);
	if (it != s_objectsRegistryName.cend())
	{
		size_t typeId = it->second;
		return s_objects[typeId];
	}
	else
	{
		return nullptr;
	}
}

void BaseObject::clearObjRegistry()
{
	s_objectsRegistryName.clear();
	s_objectsRegistryType.clear();
	for (BaseObject * obj : s_objects)
	{
		delete obj;
	}
	s_objects.clear();
}

size_t BaseObject::registerObj(const std::string & objName, BaseObject * obj)
{
	MapStrUint::const_iterator it = s_objectsRegistryName.find(objName);
	if (it != s_objectsRegistryName.cend())
	{
		return it->second;
	}
	else
	{
		s_objectsRegistryType.push_back(objName);
		s_objects.push_back(obj);
		size_t objType = s_objectsRegistryType.size() - 1;
		s_objectsRegistryName[objName] = objType;
		return objType;
	}
}

void BaseObject::updateObservers() const
{
	for (BaseObserver* obs : m_observers)
	{
		obs->update(this);
	}
}

MyInit::MyInit()
{
	new ThreadPool;
	new ParSplineCalc;
}

MyInit::~MyInit()
{
	BaseObject::clearObjRegistry();
}
