#include "bdefs.h"
#include "objectimporter.h"
#include "dedit.h"
#include "regiondoc.h"
#include "regionview.h"
#include "eventnames.h"
#include "stdrvtrackers.h"

void CRegionView::InitTracker()
{
	//the current tracker that is being added
	CRegionViewTracker *pRVTracker;
	
	//trackers that need to be preserved for overriding inside of other trackers
	//(primarily the drawing tracker)
	CRegionViewTracker *pZoomTracker, *pMouseZoomTracker, *pMoveTracker, *pDragTracker;
	CRegionViewTracker *pShrinkGridTracker, *pExpandGridTracker;

	// Set up the focus tracker
	pRVTracker = new CRVTrackerFocus(UIE_TRACKER_FOCUS, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the depth-specific selection callback
	pRVTracker = new CRVCallback(UIE_SELECT_DEPTH, this, &CRegionView::OnDepthSelect);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, 'Y'));
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the apply-texture callbacks
	pRVTracker = new CRVCallback(UIE_APPLY_TEXTURE_CLICK, this, &CRegionView::OnApplyTextureClick);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, 'Y'));
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerMenuItem( UIE_APPLY_TEXTURE_TO_SEL, this, &CRegionView::OnApplyTexture, &CRegionView::OnUpdateApplyTexture );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_REMOVE_TEXTURE, this, &CRegionView::OnRemoveTexture, &CRegionView::OnUpdateRemoveTexture );
	m_cTrackerMgr.AddTracker( pRVTracker );

	// Set up the speed test callback
	/*
	pRVTracker = new CRVCallback(UIE_TEST_RENDER_SPEED, this, CRegionView::OnTestRenderSpeed);
	((CRVCallback *)pRVTracker)->SetFireOnStart(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);
	*/

	// Set up the split callback
	pRVTracker = new CRVCallbackSimple(UIE_SPLIT_BRUSH, this, &CRegionView::OnBrushSplitBrush);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the unselect callback
	pRVTracker = new CRVCallback(UIE_SELECT_NONE, this, &CRegionView::OnSelectNone);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the grid direction callbacks
	pRVTracker = new CRVCallback(UIE_SET_GRID_FORWARD, this, &CRegionView::OnSetGridOrientation);
	((CRVCallback *)pRVTracker)->SetData(GRID_FORWARD);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_SET_GRID_RIGHT, this, &CRegionView::OnSetGridOrientation);
	((CRVCallback *)pRVTracker)->SetData(GRID_RIGHT);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_SET_GRID_UP, this, &CRegionView::OnSetGridOrientation);
	((CRVCallback *)pRVTracker)->SetData(GRID_UP);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the init texture space callback
	pRVTracker = new CRVCallback(UIE_INIT_TEXTURE_SPACE, this, &CRegionView::OnInitTextureSpace);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the delete callback
	pRVTracker = new CRVCallback(UIE_DELETE, this, &CRegionView::OnDelete);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the nudge callbacks
	pRVTracker = new CRVCallback(UIE_NUDGE_UP, this, &CRegionView::OnNudge);
	((CRVCallback *)pRVTracker)->SetData(VK_UP);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	((CRVCallback *)pRVTracker)->SetToggle(1, CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_NUDGE_DOWN, this, &CRegionView::OnNudge);
	((CRVCallback *)pRVTracker)->SetData(VK_DOWN);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	((CRVCallback *)pRVTracker)->SetToggle(1, CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_NUDGE_LEFT, this, &CRegionView::OnNudge);
	((CRVCallback *)pRVTracker)->SetData(VK_LEFT);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	((CRVCallback *)pRVTracker)->SetToggle(1, CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_NUDGE_RIGHT, this, &CRegionView::OnNudge);
	((CRVCallback *)pRVTracker)->SetData(VK_RIGHT);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	((CRVCallback *)pRVTracker)->SetToggle(1, CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	m_cTrackerMgr.AddTracker(pRVTracker);


	// Set up the delete edge callbacks
	pRVTracker = new CRVCallback(UIE_REMOVE_EDGES, this, &CRegionView::OnDeleteEdges);
	((CRVCallback *)pRVTracker)->SetData(TRUE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_DELETE_EDGES, this, &CRegionView::OnDeleteEdges);
	((CRVCallback *)pRVTracker)->SetData(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);


	// Set up the split edge callback
	pRVTracker = new CRVCallback(UIE_SPLIT_EDGE, this, &CRegionView::OnSplitEdges);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the delete tagged callback
	pRVTracker = new CRVCallback(UIE_DELETE_TAGGED_POLIES, this, &CRegionView::OnDeleteTaggedPolygons);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the grid on poly callback
	pRVTracker = new CRVCallback(UIE_GRID_TO_POLY, this, &CRegionView::OnGridToPoly);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the poly vert tag callback
	pRVTracker = new CRVCallback(UIE_TAG_POLY_VERTS, this, &CRegionView::OnTagPolyVerts);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the flip normal callback
	pRVTracker = new CRVCallback(UIE_FLIP_NORMAL, this, &CRegionView::OnFlipNormal);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the orbit vertex callback
	pRVTracker = new CRVCallback(UIE_ORBIT_VERTEX, this, &CRegionView::OnOrbitVertex);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the brush rotation tracker
	pRVTracker = new CRVTrackerNodeRotate(UIE_NODE_ROTATE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the texture movement tracker
	pRVTracker = new CRVTrackerTextureMove(UIE_TEXTURE_MOVE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the brush sizing tracker
	pRVTracker = new CRVTrackerBrushSize(UIE_BRUSH_SIZE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the object sizing tracker
	pRVTracker = new CRVTrackerObjectSize(UIE_BRUSH_SIZE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the curve editing tracker
	pRVTracker = new CRVTrackerCurveEdit(UIE_CURVE_EDIT, this);
	pRVTracker->SetPhantomEnd(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the far clipping plane tracker
	pRVTracker = new CRVTrackerFarDist(UIE_FAR_DIST, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the grid sizing tracker
	pRVTracker = new CRVTrackerGridSize(UIE_GRID_SIZE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the marker placement tracker
	pRVTracker = new CRVTrackerMarkerMove(UIE_MARKER_MOVE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the marker centering tracker
	pRVTracker = new CRVTrackerMenuItem(UIE_MARKER_CENTER, this, &CRegionView::OnCenterMarkerOnSelection, &CRegionView::OnUpdateCenterMarkerOnSelection);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the selection centering tracker
	pRVTracker = new CRVTrackerMenuItem(UIE_CENTER_SEL_ON_MARKER, this, &CRegionView::OnCenterSelectionOnMarker, &CRegionView::OnUpdateCenterSelectionOnMarker);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the vertex movement trackers
 
	pRVTracker = new CRVTrackerVertMove(UIE_MOVE_VERT, this, CRVTrackerVertMove::esm_Poly);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerVertMove(UIE_MOVE_SEL_VERT, this, CRVTrackerVertMove::esm_SelVert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerVertMove(UIE_MOVE_IMM_VERT, this, CRVTrackerVertMove::esm_ImmVert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the snapping vertex movement trackers
	// Note : I couldn't find any good key combinations to allow the poly snapping versions.. :(
	/*
	pRVTracker = new CRVTrackerVertSnap("VertSnap.Edge.Poly", this, CRVTrackerVertSnap::esm_Poly, CRVTrackerVertSnap::est_Edge);
	pRVTracker->AddStartEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'E'));
	pRVTracker->AddStartEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	pRVTracker->AddEndEvent(CUIKeyEvent(UIEVENT_KEYUP, 'E'));
	m_cTrackerMgr.AddTracker(pRVTracker);
	*/

	pRVTracker = new CRVTrackerVertSnap(UIE_VERT_SNAP_EDGE_SEL, this, CRVTrackerVertSnap::esm_SelVert, CRVTrackerVertSnap::est_Edge);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerVertSnap(UIE_VERT_SNAP_EDGE, this, CRVTrackerVertSnap::esm_ImmVert, CRVTrackerVertSnap::est_Edge);
	m_cTrackerMgr.AddTracker(pRVTracker);

	/*
	pRVTracker = new CRVTrackerVertSnap("VertSnap.Vert.Poly", this, CRVTrackerVertSnap::esm_Poly, CRVTrackerVertSnap::est_Vert);
	pRVTracker->AddStartEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'V'));
	pRVTracker->AddStartEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	pRVTracker->AddEndEvent(CUIKeyEvent(UIEVENT_KEYUP, 'V'));
	m_cTrackerMgr.AddTracker(pRVTracker);
	*/

	pRVTracker = new CRVTrackerVertSnap(UIE_VERT_SNAP_VERT_SEL, this, CRVTrackerVertSnap::esm_SelVert, CRVTrackerVertSnap::est_Vert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerVertSnap(UIE_VERT_SNAP_VERT, this, CRVTrackerVertSnap::esm_ImmVert, CRVTrackerVertSnap::est_Vert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the poly scaling tracker
	pRVTracker = new CRVTrackerPolyScale(UIE_SCALE_POLY, this, CRVTrackerPolyScale::esm_Poly);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the poly scaling tracker
	pRVTracker = new CRVTrackerVertScale(UIE_SCALE_POLY_DIVIDE, this, CRVTrackerVertScale::esm_Poly);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the vertex scaling tracker
	pRVTracker = new CRVTrackerVertScale(UIE_SCALE_VERT_SEL, this, CRVTrackerVertScale::esm_SelVert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the vertex scaling tracker
	pRVTracker = new CRVTrackerVertScale(UIE_SCALE_VERT_IMM, this, CRVTrackerVertScale::esm_ImmVert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the extrusion tracker
	pRVTracker = new CRVTrackerExtrudePoly(UIE_EXTRUDE_POLY, this, false);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the extrusion (new brush) tracker
	pRVTracker = new CRVTrackerExtrudePoly(UIE_EXTRUDE_POLY_BRUSH, this, true);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the node movement trackers
	pRVTracker = new CRVTrackerNodeMove(UIE_MOVE_NODE_HANDLE, this, CRVTrackerNodeMove::FLAG_HANDLE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerNodeMove(UIE_SNAP_NODE, this, CRVTrackerNodeMove::FLAG_SNAP);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerNodeMove(UIE_MOVE_NODE_PERP, this, CRVTrackerNodeMove::FLAG_PERP);
	m_cTrackerMgr.AddTracker(pRVTracker);

	//set up the zoom tracker
	pRVTracker = new CRVTrackerZoom(UIE_ZOOM, this);
	pRVTracker->SetPhantomEnd(FALSE);
	pMouseZoomTracker = pRVTracker;
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerNavDrag(UIE_NAV_DRAG, this);
	pRVTracker->SetPhantomEnd(FALSE);
	pDragTracker = pRVTracker;
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerNavArcRotate(UIE_NAV_ARC_ROTATE, this);
	pRVTracker->SetPhantomEnd(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the navigation movement tracker
	pRVTracker = new CRVTrackerNavMove(UIE_NAV_MOVE, this);
	pRVTracker->SetPhantomEnd(FALSE);
	pMoveTracker = pRVTracker;
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the navigation rotation tracker
	pRVTracker = new CRVTrackerNavRotate(UIE_NAV_ROTATE, this);
	pRVTracker->SetPhantomEnd(FALSE);
	pZoomTracker = pRVTracker;
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the navigation orbit tracker
	pRVTracker = new CRVTrackerNavOrbit(UIE_NAV_ORBIT, this);
	pRVTracker->SetPhantomEnd(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	//handle grid resizing
	pShrinkGridTracker = new CRVTrackerMenuItem( UIE_SHRINK_GRID_SPACING, this, &CRegionView::OnShrinkGridSpacing, &CRegionView::OnUpdateShrinkGridSpacing );
	m_cTrackerMgr.AddTracker( pShrinkGridTracker );
	pExpandGridTracker = new CRVTrackerMenuItem( UIE_EXPAND_GRID_SPACING, this, &CRegionView::OnExpandGridSpacing, &CRegionView::OnUpdateExpandGridSpacing );
	m_cTrackerMgr.AddTracker( pExpandGridTracker );


	// Set up the Draw Poly callback
	pRVTracker = new CRVTrackerDrawPoly(UIE_DRAW_POLY, this);
	pRVTracker->AddOverride(pZoomTracker);
	pRVTracker->AddOverride(pMouseZoomTracker);
	pRVTracker->AddOverride(pMoveTracker);
	pRVTracker->AddOverride(pDragTracker);
	pRVTracker->AddOverride(pShrinkGridTracker);
	pRVTracker->AddOverride(pExpandGridTracker);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the tagging trackers
	pRVTracker = new CRVTrackerTag(UIE_TAG_ALL, this, CRVTrackerTag::FLAG_ALL);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Un-Join selected brushes
	pRVTracker = new CRVCallbackSimple(UIE_UNJOIN_BRUSHES, this, &CRegionView::OnBrushUnJoin);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Join selected brushes
	pRVTracker = new CRVCallbackSimple(UIE_JOIN_BRUSHES, this, &CRegionView::OnBrushJoin);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Triangulate selected brushes..
	pRVTracker = new CRVCallbackSimple(UIE_TRI_BRUSHES, this, &CRegionView::OnAutoTriangulate);
	m_cTrackerMgr.AddTracker(pRVTracker);

	/*
	pRVTracker = new CRVTrackerTag(UIE_TAG_BRUSH, this, CRVTrackerTag::FLAG_BRUSH | CRVTrackerTag::FLAG_OBJECT);
	m_cTrackerMgr.AddTracker(pRVTracker);
	*/

	//maximize select viewports
	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_VIEW1, this, &CRegionView::OnViewMaximizeView1, &CRegionView::OnUpdateViewMaximizeView1 );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_VIEW2, this, &CRegionView::OnViewMaximizeView2, &CRegionView::OnUpdateViewMaximizeView2 );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_VIEW3, this, &CRegionView::OnViewMaximizeView3, &CRegionView::OnUpdateViewMaximizeView3 );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_VIEW4, this, &CRegionView::OnViewMaximizeView4, &CRegionView::OnUpdateViewMaximizeView4 );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_ACTIVE_VIEW, this, &CRegionView::OnViewMaximizeActiveView, &CRegionView::OnUpdateViewMaximizeActiveView, TRUE );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//reset the viewport configuration
	pRVTracker = new CRVTrackerMenuItem( UIE_RESET_VIEWPORTS, this, &CRegionView::On4ViewConfiguration, &CRegionView::OnUpdate4ViewConfiguration );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//freeze the selected nodes
	pRVTracker = new CRVTrackerMenuItem( UIE_FREEZE_SELECTED, this, &CRegionView::OnSelectionFreezeSelected, &CRegionView::OnUpdateSelectionFreezeSelected );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//unfreeze all the nodes
	pRVTracker = new CRVTrackerMenuItem( UIE_UNFREEZE_ALL, this, &CRegionView::OnSelectionUnfreezeAll, &CRegionView::OnUpdateSelectionUnfreezeAll );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//texture mirroring
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_TEXTURE_X, this, &CRegionView::OnMirrorTextureX, &CRegionView::OnUpdateMirrorTextureX );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_TEXTURE_Y, this, &CRegionView::OnMirrorTextureY, &CRegionView::OnUpdateMirrorTextureY );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//texture matching
	pRVTracker = new CRVTrackerMenuItem( UIE_MATCH_TEXTURE_COORDS, this, &CRegionView::OnMatchTextureCoords, &CRegionView::OnUpdateMatchTextureCoords);
	m_cTrackerMgr.AddTracker( pRVTracker );

	//mirroring operations
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_X, this, &CRegionView::OnSelectionMirrorX, &CRegionView::OnUpdateSelectionMirrorX );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_Y, this, &CRegionView::OnSelectionMirrorY, &CRegionView::OnUpdateSelectionMirrorY );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_Z, this, &CRegionView::OnSelectionMirrorZ, &CRegionView::OnUpdateSelectionMirrorZ );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_ALL, this, &CRegionView::OnSelectionSelectAll, &CRegionView::OnUpdateSelectionSelectAll );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_INVERSE, this, &CRegionView::OnSelectionSelectInverse, &CRegionView::OnUpdateSelectionSelectInverse );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_CONTAINER, this, &CRegionView::OnSelectionContainer, &CRegionView::OnUpdateSelectionContainer );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_ADVANCED_SELECT, this, &CRegionView::OnSelectionAdvanced, &CRegionView::OnUpdateSelectionAdvanced );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SCALE_SELECTION, this, &CRegionView::OnSelectionScale, &CRegionView::OnUpdateSelectionScale );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SAVE_AS_PREFAB, this, &CRegionView::OnSelectionSavePrefab, &CRegionView::OnUpdateSelectionSavePrefab );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_GENERATE_UNIQUE_NAMES, this, &CRegionView::OnSelectionGenerateUniqueNames, &CRegionView::OnUpdateSelectionGenerateUniqueNames );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_GROUP_SELECTION, this, &CRegionView::OnSelectionGroup, &CRegionView::OnUpdateSelectionGroup );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_FLIP_BRUSH, this, &CRegionView::OnBrushFlip, &CRegionView::OnUpdateBrushFlip );
	m_cTrackerMgr.AddTracker( pRVTracker );
	
	//handle redo/undo operations
	pRVTracker = new CRVTrackerMenuItem( UIE_EDIT_UNDO, this, &CRegionView::OnEditUndo, &CRegionView::OnUpdateEditUndo );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_EDIT_REDO, this, &CRegionView::OnEditRedo, &CRegionView::OnUpdateEditRedo );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//setup the trackers for the different views
	pRVTracker = new CRVTrackerMenuItem( UIE_TOP_VIEW, this, &CRegionView::OnTopView, &CRegionView::OnUpdateTopView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_BOTTOM_VIEW, this, &CRegionView::OnBottomView, &CRegionView::OnUpdateBottomView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_LEFT_VIEW, this, &CRegionView::OnLeftView, &CRegionView::OnUpdateLeftView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_RIGHT_VIEW, this, &CRegionView::OnRightView, &CRegionView::OnUpdateRightView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_FRONT_VIEW, this, &CRegionView::OnFrontView, &CRegionView::OnUpdateFrontView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_BACK_VIEW, this, &CRegionView::OnBackView, &CRegionView::OnUpdateBackView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_PERSPECTIVE_VIEW, this, &CRegionView::OnPerspectiveView, &CRegionView::OnUpdatePerspectiveView );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//navigator trackers
	pRVTracker = new CRVTrackerMenuItem( UIE_NAVIGATOR_STORE, this, &CRegionView::OnNavigatorStore, &CRegionView::OnUpdateNavigatorStore );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_NAVIGATOR_ORGANIZE, this, &CRegionView::OnNavigatorOrganize, &CRegionView::OnUpdateNavigatorOrganize );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_NAVIGATOR_NEXT, this, &CRegionView::OnNavigatorGotoNext, &CRegionView::OnUpdateNavigatorGotoNext );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_NAVIGATOR_PREVIOUS, this, &CRegionView::OnNavigatorGotoPrevious, &CRegionView::OnUpdateNavigatorGotoPrevious );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle adding objects
	pRVTracker = new CRVTrackerMenuItem( UIE_ADD_OBJECT, this, &CRegionView::OnAddObject, &CRegionView::OnUpdateAddObject );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle the different shade modes
	pRVTracker = new CRVTrackerMenuItem( UIE_SHADE_FLAT, this, &CRegionView::OnShadeFlat, &CRegionView::OnUpdateShadeFlat );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHADE_WIREFRAME, this, &CRegionView::OnShadeWireframe, &CRegionView::OnUpdateShadeWireframe );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHADE_TEXTURES, this, &CRegionView::OnShadeTextured, &CRegionView::OnUpdateShadeTextured );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHADE_LIGHTMAPS, this, &CRegionView::OnShadeLightmapsOnly, &CRegionView::OnUpdateShadeLightmapsOnly );
	m_cTrackerMgr.AddTracker( pRVTracker );


	//surface applicators
	pRVTracker = new CRVTrackerMenuItem( UIE_APPLY_COLOR, this, &CRegionView::OnApplyColor, &CRegionView::OnUpdateApplyColor );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_TEXTURE, this, &CRegionView::OnSelectTexture, &CRegionView::OnUpdateSelectTexture );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_BRUSH_COLOR, this, &CRegionView::OnSelectBrushColor, &CRegionView::OnUpdateSelectBrushColor );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_PREFAB, this, &CRegionView::OnSelectPrefab, &CRegionView::OnUpdateSelectPrefab );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle hiding
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_INVERSE, this, &CRegionView::OnSelectionHideInverse, &CRegionView::OnUpdateSelectionHideInverse );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_UNHIDE_INVERSE, this, &CRegionView::OnSelectionUnhideInverse, &CRegionView::OnUpdateSelectionUnhideInverse );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_SELECTED, this, &CRegionView::OnSelectionHideSelected, &CRegionView::OnUpdateSelectionHideSelected );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_UNHIDE_SELECTED, this, &CRegionView::OnSelectionUnhideSelected, &CRegionView::OnUpdateSelectionUnhideSelected );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle rotation of objects
	pRVTracker = new CRVTrackerMenuItem( UIE_ROTATE_SELECTION, this, &CRegionView::OnRotateSelection, &CRegionView::OnUpdateRotateSelection );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVCallback(UIE_ROTATE_SELECTION_LEFT, this, &CRegionView::OnNudgeRotate);
	((CRVCallback *)pRVTracker)->SetData(VK_LEFT);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVCallback(UIE_ROTATE_SELECTION_RIGHT, this, &CRegionView::OnNudgeRotate);
	((CRVCallback *)pRVTracker)->SetData(VK_RIGHT);
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle binding brushes to objects
	pRVTracker = new CRVTrackerMenuItem( UIE_BIND_TO_OBJECT, this, &CRegionView::OnBindToObject, &CRegionView::OnUpdateBindToObject );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle the model showing keys
	pRVTracker = new CRVTrackerMenuItem( UIE_SHOW_ALL_MODELS, this, &CRegionView::OnDisplayAllModels, &CRegionView::OnUpdateDisplayAllModels);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_ALL_MODELS, this, &CRegionView::OnHideAllModels, &CRegionView::OnUpdateHideAllModels);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHOW_SELECTED_MODELS, this, &CRegionView::OnDisplaySelectedModels, &CRegionView::OnUpdateDisplaySelectedModels);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_SELECTED_MODELS, this, &CRegionView::OnHideSelectedModels, &CRegionView::OnUpdateHideSelectedModels);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHOW_MODELS_OF_CLASS, this, &CRegionView::OnDisplayModelsOfClass, &CRegionView::OnUpdateDisplayModelsOfClass);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_MODELS_OF_CLASS, this, &CRegionView::OnHideModelsOfClass, &CRegionView::OnUpdateHideModelsOfClass);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHOW_MODEL_POLYCOUNT, this, &CRegionView::OnDisplayModelPolycount, &CRegionView::OnUpdateDisplayModelPolycount);
	m_cTrackerMgr.AddTracker( pRVTracker );

	//the trackers for creating primitives
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_BOX, this, &CRegionView::OnCreatePrimitiveBox, &CRegionView::OnUpdateCreatePrimitiveBox);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_DOME, this, &CRegionView::OnCreatePrimitiveDome, &CRegionView::OnUpdateCreatePrimitiveDome);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_SPHERE, this, &CRegionView::OnCreatePrimitiveSphere, &CRegionView::OnUpdateCreatePrimitiveSphere);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_PLANE, this, &CRegionView::OnCreatePrimitivePlane, &CRegionView::OnUpdateCreatePrimitivePlane);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_PYRAMID, this, &CRegionView::OnCreatePrimitivePyramid, &CRegionView::OnUpdateCreatePrimitivePyramid);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_CYLINDER, this, &CRegionView::OnCreatePrimitiveCylinder, &CRegionView::OnUpdateCreatePrimitiveCylinder);
	m_cTrackerMgr.AddTracker( pRVTracker );



	//setup the trackers for the different modes
	pRVTracker = new CRVCallbackSimple(UIE_OBJECT_EDIT_MODE, this, &CRegionView::SetObjectEditMode);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVCallbackSimple(UIE_GEOMETRY_EDIT_MODE, this, &CRegionView::SetGeometryEditMode);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVCallbackSimple(UIE_BRUSH_EDIT_MODE, this, &CRegionView::SetBrushEditMode);
	m_cTrackerMgr.AddTracker( pRVTracker );

	//texturing
	pRVTracker = new CRVCallbackSimple( UIE_NEXT_TEXTURE_LAYER, this, &CRegionView::NextTextureLayer);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerTextureWrap( UIE_TEXTURE_WRAP, this);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MAP_TEXTURE_COORDS, this, &CRegionView::OnMapTextureCoords, &CRegionView::OnUpdateMapTextureCoords );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MAP_TEXTURE_TO_VIEW, this, &CRegionView::OnMapTextureToView, &CRegionView::OnUpdateMapTextureToView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_RESET_TEXTURE_COORDS, this, &CRegionView::OnResetTextureCoords, &CRegionView::OnUpdateResetTextureCoords );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_REPLACE_TEXTURES, this, &CRegionView::OnReplaceTextures, &CRegionView::OnUpdateReplaceTextures );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_REPLACE_TEXTURES_IN_SEL, this, &CRegionView::OnReplaceTexturesInSel, &CRegionView::OnUpdateReplaceTexturesInSel );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_FIT_TEXTURE_TO_POLY, this, &CRegionView::OnFitTextureToPoly, &CRegionView::OnUpdateFitTextureToPoly );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_TOGGLE_CLASS_ICONS, this, &CRegionView::OnToggleClassIcons, &CRegionView::OnUpdateToggleClassIcons );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_FROZEN_NODES, this, &CRegionView::OnHideFrozenNodes, &CRegionView::OnUpdateHideFrozenNodes );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_CAMERA_TO_OBJECT, this, &CRegionView::OnCameraToObject, &CRegionView::OnUpdateCameraToObject );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_DISCONNECT_SELECTED_PREFABS, this, &CRegionView::OnDisconnectSelectedPrefabs, &CRegionView::OnUpdateDisconnectSelectedPrefabs );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//class icons
	pRVTracker = new CRVTrackerMenuItem( UIE_TOGGLE_CLASS_ICONS, this, &CRegionView::OnToggleClassIcons, &CRegionView::OnUpdateToggleClassIcons);
	m_cTrackerMgr.AddTracker( pRVTracker );

}
