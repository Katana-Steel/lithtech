// -------------------------------------------------------------------- //
//
// MODULE  : MPClientMgrChart.cpp
//
// PURPOSE : A client-side multiplayer statistics chart
//
// CREATED : 3/27/01
//
// -------------------------------------------------------------------- //


#include "stdafx.h"
#include "MPClientMgrChart.h"
#include "InterfaceMgr.h"
#include "LithFontMgr.h"


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::Init()
//
// PURPOSE:		Initial setup
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::Init(uint8 nColumns, uint8 nRows, LTIntPt pt, HSURFACE hBG)
{
	Clear();

	m_nColumns = (nColumns < MAX_CHART_COLUMNS) ? nColumns : MAX_CHART_COLUMNS;
	m_nRows = (nRows < MAX_CHART_ROWS) ? nRows : MAX_CHART_ROWS;

	m_pt = pt;

	m_hBackground = hBG;

	m_bInitted = LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::Clear()
//
// PURPOSE:		Clear everything
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::Clear()
{
	m_nColumns = 0;
	m_nRows = 0;

	m_hBackground = LTNULL;

	m_bInitted = LTFALSE;

	// Clear the individual elements
	for(int i = 0; i < MAX_CHART_COLUMNS; i++)
		for(int j = 0; j < MAX_CHART_ROWS; j++)
			m_pChart[i][j].Clear();
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::GetElementAtPt()
//
// PURPOSE:		Retrieve the column and row of a particular screen point
//
// -------------------------------------------------------------------- //

LTIntPt MPClientMgrChart::GetElementAtPt(LTIntPt pt)
{
	// Setup the draw points for each chart element
	LTIntPt pPts[MAX_CHART_COLUMNS][MAX_CHART_ROWS];
	uint8 i, j;

	for(i = 0; i < m_nColumns; i++)
	{
		for(j = 0; j < m_nRows; j++)
		{
			pPts[i][j].x = (i == 0) ? m_pt.x : (pPts[i - 1][j].x + m_nColumnSize[i - 1]);
			pPts[i][j].y = (j == 0) ? m_pt.y : (pPts[i][j - 1].y + m_nRowSize[j - 1]);
		}
	}


	// Alright... let's draw the bastards!
	for(i = 0; i < m_nColumns; i++)
	{
		for(j = 0; j < m_nRows; j++)
		{
			// Check to see if the screen point is within this elements box
			if(	(pt.x >= pPts[i][j].x) && (pt.y >= pPts[i][j].y) && 
				(pt.x < pPts[i][j].x + m_nColumnSize[i]) && (pt.y < pPts[i][j].y + m_nRowSize[j]))
			{
				// This is the first one we found... so return it
				return LTIntPt(i, j);
			}
		}
	}

	// This is the point value we return if nothing is found
	return LTIntPt(-1, -1);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetColumnSize()
//
// PURPOSE:		Set the size of a particular column
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetColumnSize(uint8 nColumn, int nSize)
{
	if(nColumn < m_nColumns)
	{
		m_nColumnSize[nColumn] = nSize;
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			m_nColumnSize[i] = nSize;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetRowSize()
//
// PURPOSE:		Set the size of a particular row
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetRowSize(uint8 nRow, int nSize)
{
	if(nRow < m_nRows)
	{
		m_nRowSize[nRow] = nSize;
	}
	else
	{
		for(int i = 0; i < m_nRows; i++)
			m_nRowSize[i] = nSize;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::GetChartDims()
//
// PURPOSE:		Get the dimensions of the chart
//
// -------------------------------------------------------------------- //

LTIntPt MPClientMgrChart::GetChartDims(uint8 nColumns, uint8 nRows)
{
	LTIntPt pt(0, 0);
	int i;

	uint8 nC = (nColumns < m_nColumns) ? nColumns : m_nColumns;
	uint8 nR = (nRows < m_nRows) ? nRows : m_nRows;

	for(i = 0; i < nC; i++)
		pt.x += m_nColumnSize[i];

	for(i = 0; i < nR; i++)
		pt.y += m_nRowSize[i];

	return pt;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetElement()
//
// PURPOSE:		Set a particular element
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetElement(uint8 nColumn, uint8 nRow, MPClientMgrChartElement *pElement)
{
	if(pElement && (nColumn < m_nColumns) && (nRow < m_nRows))
		memcpy(&m_pChart[nColumn][nRow], pElement, sizeof(MPClientMgrChartElement));
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::GetElement()
//
// PURPOSE:		Get a particular element
//
// -------------------------------------------------------------------- //

MPClientMgrChartElement* MPClientMgrChart::GetElement(uint8 nColumn, uint8 nRow)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
		return &m_pChart[nColumn][nRow];

	return LTNULL;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::ClearString()
//
// PURPOSE:		Sets the string for this element to null
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::ClearString(uint8 nColumn, uint8 nRow)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
	{
		m_pChart[nColumn][nRow].m_szStr[0] = '\0';
	}
	else if(nColumn < m_nColumns)
	{
		for(int i = 0; i < m_nRows; i++)
			m_pChart[nColumn][i].m_szStr[0] = '\0';
	}
	else if(nRow < m_nRows)
	{
		for(int i = 0; i < m_nColumns; i++)
			m_pChart[i][nRow].m_szStr[0] = '\0';
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			for(int j = 0; j < m_nRows; j++)
				m_pChart[i][j].m_szStr[0] = '\0';
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetFont()
//
// PURPOSE:		Sets the font of a particular element, an entire
//				column or row, or the entire chart
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetFont(uint8 nColumn, uint8 nRow, LithFont *pFont)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
	{
		m_pChart[nColumn][nRow].m_pFont = pFont;
	}
	else if(nColumn < m_nColumns)
	{
		for(int i = 0; i < m_nRows; i++)
			m_pChart[nColumn][i].m_pFont = pFont;
	}
	else if(nRow < m_nRows)
	{
		for(int i = 0; i < m_nColumns; i++)
			m_pChart[i][nRow].m_pFont = pFont;
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			for(int j = 0; j < m_nRows; j++)
				m_pChart[i][j].m_pFont = pFont;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetColor()
//
// PURPOSE:		Sets the color of a particular element, an entire
//				column or row, or the entire chart
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetColor(uint8 nColumn, uint8 nRow, HLTCOLOR hColor)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
	{
		m_pChart[nColumn][nRow].m_hColor = hColor;
	}
	else if(nColumn < m_nColumns)
	{
		for(int i = 0; i < m_nRows; i++)
			m_pChart[nColumn][i].m_hColor = hColor;
	}
	else if(nRow < m_nRows)
	{
		for(int i = 0; i < m_nColumns; i++)
			m_pChart[i][nRow].m_hColor = hColor;
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			for(int j = 0; j < m_nRows; j++)
				m_pChart[i][j].m_hColor = hColor;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetBitmap()
//
// PURPOSE:		Sets the bitmap of a particular element, an entire
//				column or row, or the entire chart
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetBitmap(uint8 nColumn, uint8 nRow, HSURFACE hBitmap)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
	{
		m_pChart[nColumn][nRow].m_hBitmap = hBitmap;
	}
	else if(nColumn < m_nColumns)
	{
		for(int i = 0; i < m_nRows; i++)
			m_pChart[nColumn][i].m_hBitmap = hBitmap;
	}
	else if(nRow < m_nRows)
	{
		for(int i = 0; i < m_nColumns; i++)
			m_pChart[i][nRow].m_hBitmap = hBitmap;
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			for(int j = 0; j < m_nRows; j++)
				m_pChart[i][j].m_hBitmap = hBitmap;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetDrawBG()
//
// PURPOSE:		Sets the background state of a particular element, an entire
//				column or row, or the entire chart
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetDrawBG(uint8 nColumn, uint8 nRow, LTBOOL bDraw)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
	{
		m_pChart[nColumn][nRow].m_bDrawBG = bDraw;
	}
	else if(nColumn < m_nColumns)
	{
		for(int i = 0; i < m_nRows; i++)
			m_pChart[nColumn][i].m_bDrawBG = bDraw;
	}
	else if(nRow < m_nRows)
	{
		for(int i = 0; i < m_nColumns; i++)
			m_pChart[i][nRow].m_bDrawBG = bDraw;
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			for(int j = 0; j < m_nRows; j++)
				m_pChart[i][j].m_bDrawBG = bDraw;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetBGColor()
//
// PURPOSE:		Sets the background color of a particular element, an entire
//				column or row, or the entire chart
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetBGColor(uint8 nColumn, uint8 nRow, HLTCOLOR hColor)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
	{
		m_pChart[nColumn][nRow].m_hBGColor = hColor;
	}
	else if(nColumn < m_nColumns)
	{
		for(int i = 0; i < m_nRows; i++)
			m_pChart[nColumn][i].m_hBGColor = hColor;
	}
	else if(nRow < m_nRows)
	{
		for(int i = 0; i < m_nColumns; i++)
			m_pChart[i][nRow].m_hBGColor = hColor;
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			for(int j = 0; j < m_nRows; j++)
				m_pChart[i][j].m_hBGColor = hColor;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetHJustify()
//
// PURPOSE:		Sets the horizontal justification of a particular element,
//				an entire column or row, or the entire chart
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetHJustify(uint8 nColumn, uint8 nRow, uint8 nHJustify)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
	{
		m_pChart[nColumn][nRow].m_nHJustify = nHJustify;
	}
	else if(nColumn < m_nColumns)
	{
		for(int i = 0; i < m_nRows; i++)
			m_pChart[nColumn][i].m_nHJustify = nHJustify;
	}
	else if(nRow < m_nRows)
	{
		for(int i = 0; i < m_nColumns; i++)
			m_pChart[i][nRow].m_nHJustify = nHJustify;
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			for(int j = 0; j < m_nRows; j++)
				m_pChart[i][j].m_nHJustify = nHJustify;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetVJustify()
//
// PURPOSE:		Sets the vertical justification of a particular element,
//				an entire column or row, or the entire chart
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetVJustify(uint8 nColumn, uint8 nRow, uint8 nVJustify)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
	{
		m_pChart[nColumn][nRow].m_nVJustify = nVJustify;
	}
	else if(nColumn < m_nColumns)
	{
		for(int i = 0; i < m_nRows; i++)
			m_pChart[nColumn][i].m_nVJustify = nVJustify;
	}
	else if(nRow < m_nRows)
	{
		for(int i = 0; i < m_nColumns; i++)
			m_pChart[i][nRow].m_nVJustify = nVJustify;
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			for(int j = 0; j < m_nRows; j++)
				m_pChart[i][j].m_nVJustify = nVJustify;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::SetOffset()
//
// PURPOSE:		Sets the draw offset of a particular element,
//				an entire column or row, or the entire chart
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::SetOffset(uint8 nColumn, uint8 nRow, LTIntPt ptOffset)
{
	if((nColumn < m_nColumns) && (nRow < m_nRows))
	{
		m_pChart[nColumn][nRow].m_ptOffset = ptOffset;
	}
	else if(nColumn < m_nColumns)
	{
		for(int i = 0; i < m_nRows; i++)
			m_pChart[nColumn][i].m_ptOffset = ptOffset;
	}
	else if(nRow < m_nRows)
	{
		for(int i = 0; i < m_nColumns; i++)
			m_pChart[i][nRow].m_ptOffset = ptOffset;
	}
	else
	{
		for(int i = 0; i < m_nColumns; i++)
			for(int j = 0; j < m_nRows; j++)
				m_pChart[i][j].m_ptOffset = ptOffset;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MPClientMgrChart::Draw()
//
// PURPOSE:		Draw the chart
//
// -------------------------------------------------------------------- //

void MPClientMgrChart::Draw(HSURFACE hSurf)
{
	// If the surface isn't valid... just return
	if(!hSurf) return;


	// Setup the draw points for each chart element
	LTIntPt pPts[MAX_CHART_COLUMNS][MAX_CHART_ROWS];
	uint8 i, j;

	for(i = 0; i < m_nColumns; i++)
	{
		for(j = 0; j < m_nRows; j++)
		{
			pPts[i][j].x = (i == 0) ? m_pt.x : (pPts[i - 1][j].x + m_nColumnSize[i - 1]);
			pPts[i][j].y = (j == 0) ? m_pt.y : (pPts[i][j - 1].y + m_nRowSize[j - 1]);
		}
	}


	// Setup how to draw the font
	int x, y;
	LITHFONTDRAWDATA dd;
	dd.dwFlags = LTF_DRAW_SOLID;


	// Alright... let's draw the bastards!
	for(i = 0; i < m_nColumns; i++)
	{
		for(j = 0; j < m_nRows; j++)
		{
			// If this element should draw a background... do that
			if(m_hBackground && m_pChart[i][j].m_bDrawBG)
			{
				x = pPts[i][j].x + m_pChart[i][j].m_ptOffset.x;
				y = pPts[i][j].y + m_pChart[i][j].m_ptOffset.y;

				LTRect rect(x, y, x + m_nColumnSize[i], y + m_nRowSize[j]);
				g_pLTClient->ScaleSurfaceToSurfaceSolidColor(hSurf, m_hBackground, &rect, LTNULL, 0, m_pChart[i][j].m_hBGColor);
			}

			// Draw the bitmap if this element has one
			if(m_pChart[i][j].m_hBitmap)
			{
				x = pPts[i][j].x + m_pChart[i][j].m_ptOffset.x;
				y = pPts[i][j].y + m_pChart[i][j].m_ptOffset.y;

				g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurf, m_pChart[i][j].m_hBitmap, LTNULL, x, y, kTransBlack);
			}


			// Make sure the needed stuff is valid
			if(m_pChart[i][j].m_pFont && m_pChart[i][j].m_szStr[0])
			{
				switch(m_pChart[i][j].m_nHJustify)
				{
					case CHART_JUSTIFY_LEFT:	x = pPts[i][j].x; dd.byJustify = LTF_JUSTIFY_LEFT; break;
					case CHART_JUSTIFY_CENTER:	x = pPts[i][j].x + (m_nColumnSize[i] / 2); dd.byJustify = LTF_JUSTIFY_CENTER; break;
					case CHART_JUSTIFY_RIGHT:	x = pPts[i][j].x + m_nColumnSize[i]; dd.byJustify = LTF_JUSTIFY_RIGHT; break;
				}

				switch(m_pChart[i][j].m_nVJustify)
				{
					case CHART_JUSTIFY_LEFT:	y = pPts[i][j].y; break;
					case CHART_JUSTIFY_CENTER:	y = pPts[i][j].y + (m_nRowSize[i] / 2) - (m_pChart[i][j].m_pFont->Height() / 2); break;
					case CHART_JUSTIFY_RIGHT:	y = pPts[i][j].y + m_nRowSize[i] - m_pChart[i][j].m_pFont->Height(); break;
				}

				// Adjust by the offsets
				x += m_pChart[i][j].m_ptOffset.x;
				y += m_pChart[i][j].m_ptOffset.y;

				// Draw this element
				dd.hColor = m_pChart[i][j].m_hColor;
				m_pChart[i][j].m_pFont->Draw(hSurf, m_pChart[i][j].m_szStr, &dd, x, y);
			}
		}
	}
}

