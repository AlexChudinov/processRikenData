#ifndef _FACTORY_
#define _FACTORY_

#include <memory>
#include <vector>
#include <unordered_map>

#include "BaseDefs.h"

template <class ClassName, class Params> 
class Factory
{
public:
    using Pointer = std::shared_ptr<ClassName>;
	class Constructor
	{
		DEF_NOT_COPYABLE(Constructor)
	public:
		Constructor(){}
		virtual ~Constructor(){}
		virtual Pointer create(const Params& params) const = 0;
		virtual Pointer create(Params&& params) const = 0;
	};
	using ConstructorPtr = const Constructor*;
	using String = const char*;
	using StringArray = std::vector<String>;
	using Map = std::unordered_map<String, ConstructorPtr>;

	static Pointer create(const String& strProductName, const Params& params)
	{
		return create(constructorPtr(strProductName), params);
	}

	static Pointer create(ConstructorPtr pConstructor, const Params& params)
	{
		return pConstructor ? pConstructor->create(params) : Pointer();
	}

	static ConstructorPtr constructorPtr(const String& strProductName)
	{
		typename Map::const_iterator it = s_products.find(strProductName);
		return it != s_products.end() ? it->second : nullptr;
	}

	static StringArray products()
	{
		StringArray ret(s_products.size());
		StringArray::iterator it = ret.begin();
		for (const auto& prod : s_products) *it++ = prod.first;
		return ret;
	}

private:
	static Map s_products;
};

#define DEF_PRODUCTS_LIST(ClassName, Params)\
template<> typename Factory<ClassName, Params>::Map Factory<ClassName, Params>::s_products

#define DEF_PRODUCTS_LIST_ENTRY(ClassName)\
{#ClassName, ClassName::constructor()}

#define DEF_ADD_FACTORY_DEFS(ClassName, Params)\
    using FactoryInstance = Factory<ClassName, Params>;\
    using Constructor = FactoryInstance::Constructor; \
    using ConstructorPtr = FactoryInstance::ConstructorPtr; \
    using Pointer = FactoryInstance::Pointer;

#define DEF_ADD_CONSTRUCTOR_INSTANCE \
static ConstructorPtr constructor() \
{  \
	static Constructor s_constructorInstance;\
	return &s_constructorInstance;\
}

#endif
