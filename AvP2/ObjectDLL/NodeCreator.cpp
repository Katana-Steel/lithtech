// ----------------------------------------------------------------------- //
//
// MODULE  : NodeCreator.cpp
//
// PURPOSE : 
//
// CREATED : 3/29/00
//
// ----------------------------------------------------------------------- //



#include "stdafx.h"


// This does not compile.  It requires ILTCommon::GetPolyInfo to be implemented, which
// was implemented but not checked into the engine.     PLH 3/29/00 


#ifdef _DEBUG
//#define USE_NODECREATOR
//#define NODE_DEBUG
#endif

#ifdef USE_NODECREATOR

#include <utility>
struct Node
{
		DVector position;
		DVector normal;
		DFLOAT  height;


	Node( const DVector & position_ = DVector(0.0f,0.0f,0.0f), 
		  const DVector & normal_   = DVector(0.0f,0.0f,0.0f),
		  DFLOAT height_ = FLT_MAX )
		: position(position_),
		  normal(normal_),
		  height(height_)  {}
};



class NodeContainer
{
public:

	typedef std::map<DFLOAT,Node>  XMap;
	typedef std::map<DFLOAT,XMap>  ZMap;
	typedef std::map<DFLOAT,ZMap>  YMap;
	typedef YMap  NodeMap;

public:


	bool Exists(const DVector & vPos) const
	{
		YMap::const_iterator y_iter = m_NodeMap.find(vPos.y);
		if( y_iter == m_NodeMap.end() ) return false;

		ZMap::const_iterator z_iter = (y_iter->second).find(vPos.z);
		if( z_iter == (y_iter->second).end() ) return false;

		XMap::const_iterator x_iter = (z_iter->second).find(vPos.x);
		if( x_iter == (z_iter->second).end() ) return false;

		return true;
	}

	void Insert(const Node & node);
	void Insert( const DVector & vPos ) { Insert(Node(vPos)); }

	void FindNeighbors(const DVector & vPos, std::vector<const Node*> * pNodeList)
	{
		{
			const YMap & y_map = m_NodeMap;
			for( YMap::const_iterator y_iter = y_map.begin();
			y_iter != y_map.end(); ++y_iter)
			{
				if( y_iter->first < 0.0f )
				{
					const ZMap & z_map = (y_iter->second);
					for( ZMap::const_iterator z_iter = z_map.begin();
					z_iter != z_map.end(); ++z_iter)
					{
						const XMap & x_map = (z_iter->second);
						for( XMap::const_iterator x_iter = x_map.begin();
						x_iter != x_map.end(); ++x_iter)
						{
							pNodeList->push_back( &(x_iter->second) );
						}
					}
				}
			}
		}
		return;

		if( !pNodeList ) return;

		const YMap & y_map = m_NodeMap;

		YMap::const_iterator y_iter = y_map.lower_bound(vPos.y);
		if( y_iter != y_map.end() ) 
		{
			const ZMap & z_map = (y_iter->second);
			ZMap::const_iterator z_iter = z_map.lower_bound(vPos.z);
			if( z_iter != z_map.end() ) 
			{
				const XMap & x_map = (z_iter->second);
				XMap::const_iterator x_iter = x_map.lower_bound(vPos.x);

				if( x_iter != x_map.end() )
				{
					pNodeList->push_back( &x_iter->second );
				}

				if( x_iter != x_map.begin() )
				{
					--x_iter;
					pNodeList->push_back( &x_iter->second );
				}
			}

			if( z_iter != z_map.begin() )
			{
				--z_iter;

				const XMap & x_map = (z_iter->second);
				XMap::const_iterator x_iter = x_map.lower_bound(vPos.x);

				if( x_iter != x_map.end() )
				{
					pNodeList->push_back( &x_iter->second );
				}

				if( x_iter != x_map.begin() )
				{
					--x_iter;
					pNodeList->push_back( &x_iter->second );
				}
			}
		}

		if( y_iter != y_map.begin() )
		{
			--y_iter;

			const ZMap & z_map = (y_iter->second);
			ZMap::const_iterator z_iter = z_map.lower_bound(vPos.z);
			if( z_iter != z_map.end() ) 
			{
				const XMap & x_map = (z_iter->second);
				XMap::const_iterator x_iter = x_map.lower_bound(vPos.x);

				if( x_iter != x_map.end() )
				{
					pNodeList->push_back( &x_iter->second );
				}

				if( x_iter != x_map.begin() )
				{
					--x_iter;
					pNodeList->push_back( &x_iter->second );
				}
			}

			if( z_iter != z_map.begin() )
			{
				--z_iter;

				const XMap & x_map = (z_iter->second);
				XMap::const_iterator x_iter = x_map.lower_bound(vPos.x);

				if( x_iter != x_map.end() )
				{
					pNodeList->push_back( &x_iter->second );
				}

				if( x_iter != x_map.begin() )
				{
					--x_iter;
					pNodeList->push_back( &x_iter->second );
				}
			}
		}
	}

	NodeMap::size_type NumNodes() const 
	{
		int sum = 0;
		for( YMap::const_iterator y_iter = m_NodeMap.begin(); y_iter != m_NodeMap.end(); ++y_iter)
		{
			for( ZMap::const_iterator z_iter = (y_iter->second).begin(); 
			     z_iter != (y_iter->second).end(); ++z_iter )
			{
				sum += (z_iter->second).size();
			}
		}
		return sum;
	}


private:


	NodeMap		m_NodeMap;
	DFLOAT		m_fSeperation;
};


void NodeContainer::Insert(const Node & node)
{
	std::pair<XMap::iterator,bool> insertion = m_NodeMap[node.position.y][node.position.z].insert( std::make_pair(node.position.x,node) );

	// Handle a shared node.
	if( insertion.second )
	{
		// The node already exists, interpolate the node data.
		Node & orig_node = insertion.first->second;

		orig_node.normal += node.normal;
		orig_node.normal.Norm();

		if( node.height < orig_node.height )
		{
			orig_node.height = node.height;
		}
	}
}


class NodeCreator : public BaseClass
{
public:

	NodeCreator() {}

protected:

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

private:

	void ReadProp(ObjectCreateStruct * pStruct);

	void MakeNodes();
	Node * CreateNode(const DVector & vPos);
	void FillPolygon(HPOLY hPoly, DPlane * pPlane, DVector * aVertexList, DDWORD nNumVertices);

#ifdef NODE_DEBUG
	void ShowNodes();
#endif

private:

	NodeContainer nodeContainer;

	std::vector<HOBJECT> m_DebugBoxes;
};


BEGIN_CLASS(NodeCreator)
	ADD_REALPROP_FLAG(Seperation, 8.0f, 0)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(NodeCreator, BaseClass, NULL, NULL )

DDWORD NodeCreator::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}
			}			

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			DCounter counter;
			g_pServerDE->StartCounter(&counter);
			MakeNodes();
			DDWORD dwTicks = g_pServerDE->EndCounter(&counter);

			
			g_pServerDE->CPrint("MakeNodes took %u ms. for %d points.", 
				dwTicks, nodeContainer.NumNodes() );
#ifdef NODE_DEBUG
//			g_pServerDE->SetDeactivationTime(m_hObject, 10000.0f);
//			g_pServerDE->SetNextUpdate(m_hObject,0.5f);
			ShowNodes();
#endif
		}
		break;


		case MID_UPDATE :
		{
#ifdef NODE_DEBUG
//			ShowNodes();
//			g_pServerDE->SetNextUpdate(m_hObject,0.5f);
#endif
		}


		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);;
}


void NodeCreator::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;

}


/*
void NodeCreator::FillPolygon(HPOLY hPoly, DPlane * pPlane, DVector * aVertexList, DDWORD nNumVertices)
{
	ASSERT( nNumVertices >= 3 );
	ASSERT( pPlane && aVertexList );
	if( nNumVertices < 3 || !pPlane || !aVertexList ) return;

	// Find the center point of the polygon.
	DVector vCenter = DVector(0.0f,0.0f,0.0f);
	{ for(int i = 0; i < nNumVertices; ++i )
	{
		vCenter += aVertexList[i];
	}}

	vCenter /= float(nNumVertices);

	// Get the plane's normal.
	const DVector & vNormal = pPlane->Normal();

	// Find the height of the center point.
	DFLOAT fHeight = FLT_MAX;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vCenter;
	IQuery.m_To = IQuery.m_From + vNormal*1000.0f;
	IQuery.m_Flags	  = IGNORE_NONSOLID;

	if (g_pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		fHeight = (IInfo.m_Point - IQuery.m_From).MagSqr();
	}

	// Insert the center point.
	nodeContainer.Insert( Node(vCenter,vNormal,fHeight) );

	// Insert the vertices.
	{ for(int i = 0; i < nNumVertices; ++i)
	{
		nodeContainer.Insert( Node(aVertexList[i],vNormal, fHeight) );
	}}

}
*/

void NodeCreator::MakeNodes()
{
#ifdef NODE_DEBUG
	int nNumPolies = 0;
#endif

	// Variables to be used while iterating through polygons.
	HPOLY hPoly = INVALID_HPOLY;

	DPlane * pPlane;
	const DDWORD nVertexListMax = 100;
	DVector aVertexList[nVertexListMax];
	DDWORD nNumVertices;

	// Walk through all the polygons.
	while(g_pServerDE->GetNextPoly(&hPoly) == LT_OK)
	{
		if( LT_OK == g_pServerDE->Common()->GetPolyInfo(hPoly,&pPlane,aVertexList,nVertexListMax,&nNumVertices) )
		{
			ASSERT( nNumVertices <= nVertexListMax );

			FillPolygon(hPoly,pPlane,aVertexList,nNumVertices);

#ifdef NODE_DEBUG
			++nNumPolies;
#endif
		}
	}

#ifdef NODE_DEBUG
	g_pServerDE->CPrint("Walked through %d polies.", nNumPolies);
#endif
}

static HOBJECT CreateBox(const DVector & vPos)
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;

#ifdef _WIN32
	SAFE_STRCPY(theStruct.m_Filename, "Models\\1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Skins\\1x1_square.dtx");
#else
	SAFE_STRCPY(theStruct.m_Filename, "Models/1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "Skins/1x1_square.dtx");
#endif

	theStruct.m_Flags = FLAG_VISIBLE;
	theStruct.m_ObjectType = OT_MODEL;

	theStruct.m_Scale = DVector(10.0f,10.0f,10.0f);

	HCLASS hClass = g_pServerDE->GetClass("BaseClass");
	LPBASECLASS pBox = g_pServerDE->CreateObject(hClass, &theStruct);
	if( pBox ) return pBox->m_hObject;

	return DNULL;
}

#ifdef NODE_DEBUG
void NodeCreator::ShowNodes()
{
	if( !g_pCharacterMgr ) return;

//	CPlayerObj * pPlayer = g_pCharacterMgr->FindPlayer();
//	DVector vPlayerPos;
//	g_pServerDE->GetObjectPos(pPlayer->m_hObject,&vPlayerPos);

	DVector vOrigin(0.0f,0.0f,0.0f);
	std::vector<const Node *> neighbors;
	nodeContainer.FindNeighbors(vOrigin,&neighbors);

	while( m_DebugBoxes.size() > neighbors.size() )
	{
		g_pServerDE->RemoveObject(m_DebugBoxes.back());
		m_DebugBoxes.pop_back();
	}

	for( int i = 0; i < neighbors.size(); ++i )
	{
		while( i >= m_DebugBoxes.size() )
		{
			m_DebugBoxes.push_back( CreateBox(vOrigin) );
		}

		g_pServerDE->TeleportObject(m_DebugBoxes[i], const_cast<DVector*>(&neighbors[i]->position) );

	}
}
#endif

// not used, but here in case is needed.
static BOOL InsideConvex(const DVector & test_pt,
						 const DVector & normal, 
						 const DVector * aVertices, DDWORD nNumVertices )
{
	DWORD i, nextI;
	DPlane edgePlane;
	DFLOAT edgeDot;

	for( i=0; i < nNumVertices; i++ )
	{
		nextI = (i+1) % nNumVertices;

		edgePlane.m_Normal = normal.Cross( aVertices[i] - aVertices[nextI] );
		edgePlane.m_Normal.Norm();
		edgePlane.m_Dist = edgePlane.m_Normal.Dot(aVertices[i]);
		
		edgeDot = edgePlane.m_Normal.Dot(test_pt) - edgePlane.m_Dist;
		
		if( edgeDot < 0.0f )
			return FALSE;
	}

	return TRUE;
}

static DVector MakeMin(const DVector & v1, const DVector & v2)
{
	return DVector( v1.x < v2.x ? v1.x : v2.x,
					v1.y < v2.y ? v1.y : v2.y,
					v1.z < v2.z ? v1.z : v2.z );
}

static DVector MakeMax(const DVector & v1, const DVector & v2)
{
	return DVector( v1.x > v2.x ? v1.x : v2.x,
					v1.y > v2.y ? v1.y : v2.y,
					v1.z > v2.z ? v1.z : v2.z );
}

void NodeCreator::FillPolygon(HPOLY hPoly, DPlane * pPlane, DVector * aVertexList, DDWORD nNumVertices)
{
	ASSERT( nNumVertices >= 3 );
	ASSERT( pPlane && aVertexList );
	if( nNumVertices < 3 || !pPlane || !aVertexList ) return;

	// This is clumsy and slow.  Please optimize it.

	// Get the min and max corners of the polygon.
	DVector vMinExtent = aVertexList[0];
	DVector vMaxExtent = aVertexList[0];

	{ for(int i = 0; i < nNumVertices; ++i )
	{
		vMinExtent = MakeMin(vMinExtent,aVertexList[i]);
		vMaxExtent = MakeMax(vMaxExtent,aVertexList[i]);
	}}


	// walk the square that circumscribes the polygon and insert nodes on each grid point.
	{ const DFLOAT grid_size = 32.0f;
	  for(int i = vMinExtent.x; i <= vMaxExtent.x; i += grid_size )
		for( int j = vMinExtent.y; j <= vMaxExtent.y; j += grid_size )
			for( int  k = vMinExtent.z; k <= vMaxExtent.z; k += grid_size )
			{
				if( InsideConvex(DVector(i,j,k),pPlane->Normal(),aVertexList,nNumVertices) )
				{
					nodeContainer.Insert( DVector(i,j,k) );
				}
			}
	}
}

#endif
