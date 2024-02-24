// ----------------------------------------------------------------------- //
//
// MODULE  : LightObject.h
//
// PURPOSE : LightObject.
//
// CREATED : 02/03/98
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHT_OBJECT_H__
#define __LIGHT_OBJECT_H__


// If you want a light to be recognized by AI visibility, have it inherit from
// this class, and provide the appropriate function.
class LightObject : public BaseClass
{
public:

	virtual ~LightObject() {}

	 // The position of the light.
	virtual LTVector GetPos() const = 0; 

	// The light's radius, squared.
	virtual LTFLOAT  GetRadiusSqr() const = 0;  
	
	// The color the light gives at vLocation.  Returns a vector
	// going from 0.0 - 255.0 for Red, Green, Blue.
	virtual LTVector GetColor(const LTVector & vLocation) const = 0; 
};

#include "LightGroup.h"
#include "ClientLightFX.h"

inline LTVector GetModelLighting( const LTVector & vPoint)
{
	LTVector result(0,0,0);

	g_pLTServer->Common()->GetPointShade(const_cast<LTVector*>(&vPoint),&result);

	{for( LightGroup::LightGroupIterator iter = LightGroup::BeginLightGroups();
	     iter != LightGroup::EndLightGroups(); ++iter )
	{
		const LightGroup * const pLightG = *iter;

		if( pLightG )
		{
			if( (vPoint - pLightG->GetPos()).MagSqr() < pLightG->GetRadiusSqr() )
			{
				result += pLightG->GetColor(vPoint);
			}
		}
	}}

	{for( ClientLightFX::ClientLightFXIterator iter = ClientLightFX::BeginClientLightFXs();
	     iter != ClientLightFX::EndClientLightFXs(); ++iter )
	{
		const ClientLightFX* const pLightFX = *iter;

		if( pLightFX )
		{
			if( (vPoint - pLightFX->GetPos()).MagSqr() < pLightFX->GetRadiusSqr() )
			{
				result += pLightFX->GetColor(vPoint);
			}
		}
	}}

	if( result.x > 255.0f ) result.x = 255.0f;
	if( result.y > 255.0f ) result.y = 255.0f;
	if( result.z > 255.0f ) result.z = 255.0f;

	return result;
}



#endif // __LIGHT_OBJECT_H__
