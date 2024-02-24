#ifndef C_SPECIAL_FX_FACTORY_H
#define C_SPECIAL_FX_FACTORY_H

#pragma warning(disable : 4786)
#include <string>

#ifndef PLUG_FACTORY_TEMPLATE_H
#include "PlugFactoryTemplate.h"
#endif

#ifndef __SPECIAL_FX_H__
#include "SpecialFX.h"
#endif

// This makes the base for a whole family of products
// This class can also define other creation methods
class CSpecialFXFactory : public PlugFactory<CSpecialFX, int>
{
public:
	CSpecialFXFactory(int className) : PlugFactory<CSpecialFX, int>(className){;}

	// Return the concrete product desired
	CSpecialFX* NewSFX(int className)
	{
		return NewAbstract(className);
	}

	CSpecialFX* NewSFX(const int& className, SFXCREATESTRUCT* p_createStruct)
	{
		CSpecialFXFactory* sfxFactory = dynamic_cast<CSpecialFXFactory*>(GetFactory(className));
		if(sfxFactory)
		{
			return sfxFactory->MakeShape(p_createStruct);
		}
		return 0;
	}

	CSpecialFX* NewSFX (const int& className, 
								const HMESSAGEREAD& hMessage, 
								const HOBJECT& hServerObj)
	{
		CSpecialFXFactory* sfxFactory = dynamic_cast<CSpecialFXFactory*>(GetFactory(className));
		if(sfxFactory)
		{
			return sfxFactory->MakeShape(hMessage, hServerObj);
		}
		return 0;
	}

protected:
	
	virtual CSpecialFX* MakeShape() const {return 0;}

	virtual CSpecialFX* MakeShape(SFXCREATESTRUCT* p_createStruct) const
	{
		CSpecialFX* sfx = MakeShape();
		if (!sfx->Init(p_createStruct))
		{
			delete sfx;
			sfx = DNULL;
		}
		return sfx;
	}

	virtual CSpecialFX* MakeShape(const HMESSAGEREAD& hMessage, const HOBJECT& hServerObj) const
	{
		CSpecialFX* sfx = MakeShape();

		if (!sfx->Init(hServerObj, hMessage))
		{
			delete sfx;
			sfx = DNULL;
		}
		return sfx;
	}
};

#endif		// C_SPECIAL_FX_FACTORY_H