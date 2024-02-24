// ----------------------------------------------------------------------- //
//
// MODULE  : ObstacleMgr.h
//
// PURPOSE : AI Obstacle avoidance system
//
// CREATED : 4/17/00
//
// ----------------------------------------------------------------------- //

#ifndef __OBSTACLEMGR_H__
#define __OBSTACLEMGR_H__

#include "ltsmartlink.h"
#include "ltvector.h"
#include "ltbasetypes.h"
#include "Timer.h"
#include <deque>
#include <list>


class  AvoidancePath; // defined in ObstacleMgr.cpp
class  CharacterMovement;


struct Obstacle
{
	friend ILTMessage & operator<<(ILTMessage & out, /*const*/ Obstacle & x);
	friend ILTMessage & operator>>(ILTMessage & in, Obstacle & x);

	LTSmartLink objectHandle;
	LTVector position;
	LTFLOAT  radius;
	LTFLOAT  height;
	LTBOOL   ignorable;

	Obstacle(HOBJECT my_handle, const LTVector & my_position, LTFLOAT my_radius, LTFLOAT my_height, LTBOOL i_am_ignorable)
		: objectHandle(my_handle),
		  position(my_position),
		  radius( my_radius ),
		  height( my_height ),
		  ignorable(i_am_ignorable) {}

	Obstacle(HOBJECT my_handle, LTBOOL i_am_ignorable)
		: objectHandle(my_handle),
		  position(0,0,0),
		  radius(0),
		  height(0),
		  ignorable(i_am_ignorable)
	{
		// Get our position.
		Update();
	}

	void Update()
	{
		if( objectHandle )
		{
			g_pServerDE->GetObjectPos(objectHandle,&position);
		}
	}

	bool CoversPoint(const LTVector vPoint) const
	{
		const LTFLOAT x_dist = position.x - vPoint.x;
		const LTFLOAT z_dist = position.z - vPoint.z;
				
		return (x_dist*x_dist + z_dist*z_dist) < radius*radius;
	}

	const Obstacle & GetObstacle() const { return *this; }
};


class ObstacleMgr
{
public :

	typedef std::list<Obstacle> ObstacleList;

	friend ILTMessage & operator<<(ILTMessage & out, /*const*/ ObstacleMgr & x);
	friend ILTMessage & operator>>(ILTMessage & in, ObstacleMgr & x);


public :

	ObstacleMgr();
	~ObstacleMgr();

	bool Update( LTVector * goal_ptr, LTVector * next_goal_ptr, const CharacterMovement * pMovement);

	bool IsStuck() const;

	void SetGoal(const LTVector & new_goal, const CharacterMovement * pMovement);

	void  SetPathingFrequency(LTFLOAT val) { m_fPathingFrequency = val; }

	void   AvoidIgnorable(LTBOOL bVal)  { m_bAvoidIgnorable = bVal; }
	LTBOOL IsAvoidingIgnorable() const { return m_bAvoidIgnorable; }
private:


	AvoidancePath * m_pAvoidancePath;

	LTFLOAT      m_fPathingFrequency;
	CTimer       m_tmrPathingFrequency;

	LTBOOL       m_bAvoidIgnorable;

	ObstacleList    potentialObstacleList;
};



#endif //__OBSTACLEMGR_H__
