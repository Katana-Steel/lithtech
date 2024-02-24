// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLineSystem.h
//
// PURPOSE :
//
// CREATED : 3/29/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBUG_LINE_SYSTEM_H__
#define __DEBUG_LINE_SYSTEM_H__

// This whole file is empty for the final build!
#ifndef _FINAL

#include "DebugLine.h"
#include "ltsmartlink.h"
#include <deque>
#include <map>
#include <string>


class DebugLineSystem : public BaseClass
{
public:
	
	typedef std::deque<DebugLine> LineList;

	static DebugLineSystem * Spawn(const char * name, int max_lines = 300);

public:

	DebugLineSystem() 
		: BaseClass(OT_LINESYSTEM),
		  nextLineToSend(lines.begin()),
		  m_nNumClients(0),
		  maxLines(0),
		  m_bClearOldLines(false),
		  m_vVertexSum(0.0f,0.0f,0.0f),
		  m_fNumSummedVertices(0.0f) {}

	void UpdateClient(HCLIENT hClient, LTBOOL bUpdate);

	void AddLine(const DebugLine & new_line);

	void AddLine(const LTVector & vSource, const LTVector & vDest, 
		         const LTVector & vColor = LTVector(1.0f,1.0f,1.0f),
				 float fAlpha = 1.0f )
	{
		AddLine(DebugLine(vSource,vDest,vColor,fAlpha));
	}

	void AddBox( const LTVector & vBoxPos, const LTVector & vBoxDims, 
			     const LTVector & vColor = LTVector(1.0f,1.0f,1.0f),
				 float fAlpha = 1.0f );

	void Clear();


	void SetMaxLines(int max_lines) { maxLines = max_lines; }

protected:

	LTRESULT	EngineMessageFn (LTRESULT messageID, void *pData, float lData);

private:
	// These are called by EngineMessageFn.
	void ReadProp(ObjectCreateStruct *pStruct);
	void InitialUpdate(); 
	void	Update();  

	// Member Variables.
	LineList lines;
	LineList::iterator nextLineToSend;

	int		 m_nNumClients;
	int		 maxLines;
	bool     m_bClearOldLines;

	LTVector m_vVertexSum;
	float    m_fNumSummedVertices;
};


namespace Color
{
	const LTVector White(1.0f,1.0f,1.0f);
	
	const LTVector Red(1.0f,0.0f,0.0f);
	const LTVector Green(0.0f,1.0f,0.0f);
	const LTVector Blue(0.0f,0.0f,1.0f);

	const LTVector Yellow(1.0f,1.0f,0.0f);
	const LTVector Purple(1.0f,0.0f,1.0f);
	const LTVector Cyan(0.0f,1.0f,1.0f);
};


namespace LineSystem
{

	DebugLineSystem & GetSystem(const std::string & name);
	DebugLineSystem & GetSystem(const void * pOwner, const std::string & name);
	void ClearAll();

	class Clear {};

	inline DebugLineSystem & operator<<(DebugLineSystem & out, const LineSystem::Clear & /*unnused*/)
	{
		out.Clear();
		return out;
	}

	class Line
	{
	public :

		const LTVector start;
		const LTVector end;
		const LTVector color;
		const float    alpha;


		Line( const LTVector & new_start, 
			  const LTVector & new_end,
			  const LTVector & new_color = Color::White,
			  float new_alpha = 1.0f )
			: start(new_start),
			  end(new_end),
			  color(new_color),
			  alpha(new_alpha) {}
	};

	inline DebugLineSystem & operator<<( DebugLineSystem & out, const LineSystem::Line & line)
	{
		out.AddLine(line.start,line.end,line.color,line.alpha);
		return out;
	}



	class Box
	{
	public :

		const LTVector position;
		const LTVector dimensions;
		const LTVector color;
		const float    alpha;


		Box(const LTVector & new_position, 
			const LTVector & new_dimensions,
			const LTVector & new_color = Color::White,
			float new_alpha = 1.0f )
			: position(new_position),
			  dimensions(new_dimensions),
			  color(new_color),
			  alpha(new_alpha) {}
	};

	inline DebugLineSystem & operator<<( DebugLineSystem & out, const LineSystem::Box & box)
	{
		out.AddBox( box.position, box.dimensions, box.color, box.alpha );
		return out;
	}

	class Arrow
	{
	public :

		const LTVector start;
		const LTVector end;
		const LTVector color;
		const float    alpha;


		Arrow( const LTVector & new_start, 
			   const LTVector & new_end,
			   const LTVector & new_color = Color::White,
			   float new_alpha = 1.0f )
			 : start(new_start),
			   end(new_end),
			   color(new_color),
			   alpha(new_alpha) {}
	};

	inline DebugLineSystem & operator<<( DebugLineSystem & out, const LineSystem::Arrow & arrow)
	{
		const LTFLOAT head_size = 4.0f;
		LTVector vStartToEnd = arrow.end - arrow.start;
		vStartToEnd.Norm(head_size);

		out.AddLine(arrow.start,arrow.end,arrow.color,arrow.alpha);

		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(head_size/2.0f,0.0f,0.0f), arrow.color,arrow.alpha);
		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(-head_size/2.0f,0.0f,0.0f), arrow.color,arrow.alpha);

		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(0.0f,head_size/2.0f,0.0f), arrow.color,arrow.alpha);
		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(0.0f,-head_size/2.0f,0.0f), arrow.color,arrow.alpha);

		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(0.0f,0.0f,head_size/2.0f), arrow.color,arrow.alpha);
		out.AddLine(arrow.end,arrow.end - vStartToEnd + LTVector(0.0f,0.0f,-head_size/2.0f), arrow.color,arrow.alpha);

		return out;
	}

} //namespace LineSystem


#endif // !_FINAL



#endif //__DEBUG_LINE_SYSTEM_H__
