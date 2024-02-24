#ifndef PLUG_FACTORY_TEMPLATE_H
#define PLUG_FACTORY_TEMPLATE_H

#pragma warning(disable : 4786)
#include <map>

// Given a Product Family, this pluggable factory will create the
// correct product from an input stream.
template<class ProductFamilyT, class KeyT>
class PlugFactory
{
public:
	
	// Useful type information.
	typedef ProductFamilyT ProductFamily;
	typedef KeyT		   Key;

	// Register the names of the products that can be produced
	PlugFactory(const KeyT & className)
	{
		GetRegistry().insert(std::make_pair(className, this));
	}

	// Return the concrete product desired
	ProductFamilyT* NewAbstract(const KeyT & className)
	{
		PlugFactory<ProductFamilyT, KeyT>* plugFactory = GetFactory(className);
		if (plugFactory)
		{
			return plugFactory->MakeShape();
		}
		return 0;
	}

	PlugFactory<ProductFamilyT, KeyT>* GetFactory(const KeyT& className)
	{
		PlugIterator iter = GetRegistry().find(className);
		if (iter == GetRegistry().end())
		{
			return 0;
		}
		return (*iter).second;
	}

protected:
	virtual ProductFamilyT* MakeShape() const {return 0;}

private:
	// These are protected so the main family factory can make an instance of this
	typedef std::map<KeyT, PlugFactory*> PlugFactoryContainer;
	typedef PlugFactoryContainer::iterator PlugIterator;

	// This is needed to force the registry to be initialized before any
	// derived classes use it.
	PlugFactoryContainer& GetRegistry() 
	{
		static PlugFactoryContainer m_Registry;
		return m_Registry;
	}

	// By not having a default constructor, concrete classes can't
	// forget to register
	PlugFactory() {;}
};

#endif		// PLUG_FACTORY_TEMPLATE_H
