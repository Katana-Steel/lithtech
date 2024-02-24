// ----------------------------------------------------------------------- //
//
// MODULE  : StoryElement.h
//
// PURPOSE : StoryElement definition
//
// CREATED : 05.22.2000
//
// ----------------------------------------------------------------------- //

#ifndef __STORYELEMENT_H
#define __STORYELEMENT_H

class CLTGUIFont;

class StoryElement
{
public:
	StoryElement();
	~StoryElement();

	void Init();
	void Update();

	HSTRING		m_hImage;
	HSTRING		m_hSoundFile;
	LTFLOAT		m_fDuration;
	LTFLOAT		m_fStartTime;
	LTFLOAT		m_fRangeSqr;
	int			m_nStringID;
	uint8		m_nFontId;	
	int			m_nYOffset;	
	int			m_nXOffset;	
	int			m_nTextWidth;	

private:

	HSURFACE	m_hSurface;
	HSTRING		m_hString;
	DDWORD		m_nWidth;
	DDWORD		m_nHeight;
	LTVector	m_vInitPos;
	LTBOOL		m_bInitUpdate;

	LTBOOL		CheckRange();
	CLTGUIFont* GetFont();
};

#endif