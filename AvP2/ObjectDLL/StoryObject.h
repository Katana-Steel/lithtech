// ----------------------------------------------------------------------- //
//
// MODULE  : StoryObject.h
//
// PURPOSE : CStoryObject - Definition
//
// CREATED : 5/22/2000
//
// ----------------------------------------------------------------------- //

#ifndef __STORY_OBJECT_H__
#define __STORY_OBJECT_H__

#include "ltengineobjects.h"
#include "SoundButeMgr.h"
#include "LTString.h"
#include "Prop.h"

class StoryObject : public Prop
{
	public :

		StoryObject();
		~StoryObject();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData);
		virtual DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual void  HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead);

	private:
		HSTRING		m_hBitmapFile;	//image file
		HSTRING		m_hSoundFile;	//sound file
		LTString	m_strViewCmd;	//on view command
		LTFLOAT		m_fTime;		//duration
		LTFLOAT		m_fRange;		//how far to move away before disable
		int			m_nNumViews;	//how many views before object is disabled
		int			m_nStringId;	//ID of the text string to be displayed.
		uint8		m_nFontId;		//ID of the font used for the stroy object
		int			m_nYOffset;		//Y offset of text display
		int			m_nXOffset;		//X offset of text display
		int			m_nWidth;		//Width of text display

		LTBOOL		ReadProp(ObjectCreateStruct *pData);
		void		ClearStrings();
		void		SendStoryToClient();
		void		Save(HMESSAGEWRITE pData, DDWORD fData);
		void		Load(HMESSAGEREAD pData, DDWORD fData);

};

class CStoryPlugin : public IObjectPlugin
{
public:
	
    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char* const * aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);
	
protected :
	
	CSoundButeMgrPlugin			m_SoundButePlugin;
};

#endif // __STORY_OBJECT_H__
