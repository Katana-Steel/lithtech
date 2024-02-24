// -------------------------------------------------------------------- //
//
// MODULE  : MPClientMgrChart.h
//
// PURPOSE : A client-side multiplayer statistics chart
//
// CREATED : 3/27/01
//
// -------------------------------------------------------------------- //

#ifndef __MULTIPLAYER_CLIENT_MGR_CHART_H__
#define __MULTIPLAYER_CLIENT_MGR_CHART_H__

// -------------------------------------------------------------------- //

class LithFont;

// -------------------------------------------------------------------- //

#define MAX_CHART_STRING_LENGTH			128
#define MAX_CHART_COLUMNS				16
#define MAX_CHART_ROWS					32

#define CHART_JUSTIFY_LEFT				0
#define CHART_JUSTIFY_TOP				0
#define CHART_JUSTIFY_CENTER			1
#define CHART_JUSTIFY_RIGHT				2
#define CHART_JUSTIFY_BOTTOM			2

// -------------------------------------------------------------------- //

struct MPClientMgrChartElement
{
	MPClientMgrChartElement()
	{
		Clear();
	}

	void Clear()
	{
		memset(this, 0, sizeof(MPClientMgrChartElement));
		m_hColor = SETRGB(255, 255, 255);
		m_hBGColor = SETRGB(64, 64, 0);
	}

	char			m_szStr[MAX_CHART_STRING_LENGTH];

	LithFont		*m_pFont;						// The font to draw with
	HLTCOLOR		m_hColor;						// The color to draw the text

	HSURFACE		m_hBitmap;						// A bitmap to draw at this element location

	LTBOOL			m_bDrawBG;						// Draw a ground behind this element
	HLTCOLOR		m_hBGColor;						// The color to draw the background

	uint8			m_nHJustify;					// The horizontal justification
	uint8			m_nVJustify;					// The vertical justification

	LTIntPt			m_ptOffset;						// An offset for this element
};

// -------------------------------------------------------------------- //
// A class used to format a chart of string data to be drawn

class MPClientMgrChart
{
	public:

		MPClientMgrChart()				{ m_bInitted = LTFALSE; }
		~MPClientMgrChart()				{}


		// Chart setup control

		void	Init(uint8 nColumns, uint8 nRows, LTIntPt pt, HSURFACE hBG = LTNULL);
		void	Clear();

		LTBOOL	IsInitted()				{ return m_bInitted; }


		// Chart control functions

		void	SetPt(LTIntPt pt)		{ m_pt = pt; }
		LTIntPt	GetPt()					{ return m_pt; }
		LTIntPt	GetElementAtPt(LTIntPt pt);

		void	SetColumnSize(uint8 nColumn, int nSize);
		void	SetRowSize(uint8 nRow, int nSize);

		LTIntPt	GetChartDims(uint8 nColumns, uint8 nRows);


		// Element control functions

		void	SetElement(uint8 nColumn, uint8 nRow, MPClientMgrChartElement *pElement);
		MPClientMgrChartElement* GetElement(uint8 nColumn, uint8 nRow);

		void	ClearString(uint8 nColumn, uint8 nRow);

		void	SetFont(uint8 nColumn, uint8 nRow, LithFont *pFont);
		void	SetColor(uint8 nColumn, uint8 nRow, HLTCOLOR hColor);

		void	SetBitmap(uint8 nColumn, uint8 nRow, HSURFACE hBitmap);

		void	SetDrawBG(uint8 nColumn, uint8 nRow, LTBOOL bDraw);
		void	SetBGColor(uint8 nColumn, uint8 nRow, HLTCOLOR hColor);

		void	SetHJustify(uint8 nColumn, uint8 nRow, uint8 nHJustify);
		void	SetVJustify(uint8 nColumn, uint8 nRow, uint8 nVJustify);

		void	SetOffset(uint8 nColumn, uint8 nRow, LTIntPt ptOffset);


		// Chart rendering control

		void	Draw(HSURFACE hSurf);


	private:


		// The chart elements
		MPClientMgrChartElement m_pChart[MAX_CHART_COLUMNS][MAX_CHART_ROWS];

		HSURFACE	m_hBackground;						// The surface to use to draw backgrounds

		LTIntPt		m_pt;								// The upper left corner of the chart

		int			m_nColumnSize[MAX_CHART_COLUMNS];	// The sizes of the columns
		int			m_nRowSize[MAX_CHART_ROWS];			// The sizes of the rows

		uint8		m_nColumns;							// The number of columns being used
		uint8		m_nRows;							// The number of rows being used

		LTBOOL		m_bInitted;							// Is this chart initted?
};

// -------------------------------------------------------------------- //

#endif

