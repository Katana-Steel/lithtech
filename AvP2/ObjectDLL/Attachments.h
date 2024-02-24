// ----------------------------------------------------------------- //
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------- //

#ifndef __ATTACHMENTS_H__
#define __ATTACHMENTS_H__

#include "iaggregate.h"
#include "ltengineobjects.h"
#include "Weapons.h"
#include "iobjectplugin.h"

// ----------------------------------------------------------------- //
#define DEFAULT_ATTACHMENT "<default>"
#define NO_ATTACHMENT	"<nothing>"

// ----------------------------------------------------------------- //

// ----------------------------------------------------------------- //

#define ADD_HUMANATTACHMENTS_AGGREGATE( group ) \
	ADD_STRINGPROP_FLAG(LeftHand,		DEFAULT_ATTACHMENT,		(group) | PF_STATICLIST) \
	ADD_STRINGPROP_FLAG(RightHand,		DEFAULT_ATTACHMENT,		(group) | PF_STATICLIST) \
	ADD_STRINGPROP_FLAG(Head,			DEFAULT_ATTACHMENT,		(group) | PF_STATICLIST) \
	ADD_STRINGPROP_FLAG(Helmet,			DEFAULT_ATTACHMENT,		(group) | PF_STATICLIST) \
	ADD_STRINGPROP_FLAG(LeftShoulder,	DEFAULT_ATTACHMENT,		(group) | PF_STATICLIST) \
	ADD_STRINGPROP_FLAG(RightShoulder,	DEFAULT_ATTACHMENT,		(group) | PF_STATICLIST) \
	ADD_STRINGPROP_FLAG(LeftFoot,		DEFAULT_ATTACHMENT,		(group) | PF_STATICLIST) \
	ADD_STRINGPROP_FLAG(RightFoot,		DEFAULT_ATTACHMENT,		(group) | PF_STATICLIST) \

// ----------------------------------------------------------------- //

#define ADD_CREATEATTACHMENTS_AGGREGATE( group ) \
	ADD_ATTACHMENTS_AGGREGATE( group ) \

// ----------------------------------------------------------------- //

#define MAX_ATTACHMENT_POSITIONS		16

#define ATTACHMENT_TYPE_PROP			0
#define ATTACHMENT_TYPE_OBJECT			1
#define ATTACHMENT_TYPE_WEAPON			100 // not directly specifiable in butes file

#define ATTACHMENTS_TYPE_HUMAN			100
#define ATTACHMENTS_TYPE_PLAYER			200
#define ATTACHMENTS_TYPE_CREATURE		300

// ----------------------------------------------------------------- //

class CHHWeaponModel;
class Prop;
class BodyProp;
class CAttachmentPosition;

// ----------------------------------------------------------------- //

class CAttachment
{
	public :

		// Ctors/dtors/etc

		CAttachment();
		virtual ~CAttachment();

		virtual void Init(HOBJECT hObject, HOBJECT hModel, int nAttachmentID);

		// Methods

		virtual void CreateSpawnString(char* szSpawn) = 0;

		// Handlers


		// Engine stuff

		virtual uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead) { return 0; }

		virtual void Save(HMESSAGEWRITE hWrite);
		virtual void Load(HMESSAGEREAD hRead);

		// Simple accessors

		HOBJECT GetObject() { return m_hObject; }
		HOBJECT GetModel() { return m_hModel; }
		int GetID() { return m_nAttachmentID; }

		virtual uint32 GetType() = 0;

	private :

		int				m_nAttachmentID;	// The ID in the attachments butes file.
		HOBJECT			m_hObject;			// The object we're attaching to

		// This cannot be a LTSmartLink.  The Attachments class will create
		// an inter-object link so that it can immediately remove the attachment
		// if the attachment's model gets destroyed.
		HOBJECT			m_hModel;			// The engine object handle to the attachment.
};

// ----------------------------------------------------------------- //

class CAttachmentProp : public CAttachment
{
	public :

		// Methods

		void CreateSpawnString(char* szSpawn);

		// Handlers


		// Simple accessors

		uint32 GetType() { return ATTACHMENT_TYPE_PROP; }
};

// ----------------------------------------------------------------- //

class CAttachmentObject : public CAttachment
{
	public :

		// Methods

		void CreateSpawnString(char* szSpawn);

		// Handlers


		// Simple accessors

		uint32 GetType() { return ATTACHMENT_TYPE_OBJECT; }
};

// ----------------------------------------------------------------- //

class CAttachmentWeapon : public CAttachment
{
	public :

		// Ctors/dtors/etc

		void Init(HOBJECT hObject, HOBJECT hWeaponModel, int nAttachmentID, int nWeaponID, int nBarrelID);

		// Methods

		void CreateSpawnString(char* szSpawn);

		// Engine stuff

		uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Simple accessors

		uint32 GetType() { return ATTACHMENT_TYPE_WEAPON; }
		CWeapons* GetWeapons() { return &m_Weapons; }

	private :

		CWeapons	m_Weapons;			// The weapons aggregate for this weapon attachment
};

// ----------------------------------------------------------------- //

class CAttachmentPosition
{
	public :

		// Ctors/dtors/etc

		CAttachmentPosition();
		~CAttachmentPosition();

		// Engine stuff

		uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Simple accessors

		void SetAttachment(CAttachment* pAttachment) { m_pAttachment = pAttachment; }
		CAttachment* GetAttachment() { return m_pAttachment; }
		LTBOOL HasAttachment() { return !!m_pAttachment; }

		const std::string & GetName() const { return m_sName; }
		void SetName(const std::string sName) { m_sName = sName; }

		void SetAttachmentName(const std::string sAttachmentName) { m_sAttachmentName = sAttachmentName; }
		const std::string & GetAttachmentName() const { return m_sAttachmentName; }
		LTBOOL HasAttachmentName() { return !m_sAttachmentName.empty(); }

	private :

		std::string		m_sName;				// The name of the position. Assumes pointing to a const global string.
		std::string		m_sAttachmentName;
		CAttachment*	m_pAttachment;			// The attachment, if any, on this position
};

// ----------------------------------------------------------------- //

class CAttachments : public IAggregate
{
	public :

		// Ctors/dtors/etc

		CAttachments(LTBOOL bAutoInit = LTTRUE);
		virtual ~CAttachments();

		CAttachmentPosition* GetAttachmentPosition(const char *szAttachmentPosition);

		int EnumerateWeapons(CWeapon** apWeapons, CAttachmentPosition** apAttachmentPositions, int cMaxWeapons);
		int EnumerateProps(Prop** apProps, CAttachmentPosition** apAttachmentPositions, int cMaxProps);
		int EnumerateObjects(BaseClass** apObjects, CAttachmentPosition** apAttachmentPositions, int cMaxObjects);

		LTBOOL HasWeapons() { return m_cWeapons > 0; }
		LTBOOL HasProps() { return m_cProps > 0; }
		LTBOOL HasObjects() { return m_cObjects > 0; }

		void GetInfiniteAmmo();

		virtual void HandleDeath();

		virtual void Init(HOBJECT hObject);
		virtual void ReInit(HOBJECT hObject);

		void ResetRequirements();
		void AddRequirements(int nCharacterSet);

		virtual void Attach(const char* szAttachmentPosition, const char* szAttachment);
		virtual void Detach(const char* szAttachmentPosition);
		virtual void Detach(CAttachmentPosition* pAttachmentPosition, LTBOOL bBreakLink = LTTRUE);
		virtual void DropAttachment(CAttachmentPosition* pAttachmentPosition, LTBOOL bRemove = LTTRUE);

		// Simple accessors

		virtual uint32 GetType() = 0;


		// Engine message functions

		uint32 EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
		uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		// Methods

		virtual void	Update();

	protected :

		virtual void	CreateAttachment(CAttachmentPosition *pAttachmentPosition);
		void			CreateWeaponAttachment(CAttachmentPosition *pAttachmentPosition, const char* szAttachmentName, uint8 nWeaponID, uint8 nBarrelID);
		void			CreateObjectAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID);
		void			CreatePropAttachment(CAttachmentPosition *pAttachmentPosition, int nAttachmentID);


		// Handlers

		virtual void	HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);

		// Engine stuff

		virtual LTBOOL	ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pInfo);
        virtual void    Save(HMESSAGEWRITE hWrite, uint8 nType);
        virtual void    Load(HMESSAGEREAD hRead, uint8 nType);
		virtual void    HandleLinkBroken( HOBJECT hRemovedObject );

		HOBJECT					m_hObject;	// The object I'm associated with


		CAttachmentPosition*	m_apAttachmentPositions[MAX_ATTACHMENT_POSITIONS];		// Array of pointers to attachment positions
		std::string				m_aAttachmentOverrides[MAX_ATTACHMENT_POSITIONS];		// An array of attachment over-rides, set in DEdit.
		int						m_cAttachmentPositions;									// This should equal the size of the arrow below

	private :

		int						m_cWeapons;												// A count of our weapon attachments
		int						m_cObjects;												// A count of our Object attachments
		int						m_cProps;												// A count of our Prop attachments
		uint32					m_dwFlags;												// All attachments have these flags set.
																						// If you want a particular attachment to have different flags, you will have to modify this code.

		LTBOOL					m_bAutoInit;											// Should the attachments init automatically on the initial update?
};

// ----------------------------------------------------------------- //

class CHumanAttachments : public CAttachments
{
	public :

		// Ctor/dtor/etc

		CHumanAttachments(LTBOOL bAutoInit = LTTRUE);

		// Handlers

		void HandleCheatBigGuns();

		// Simple accessors

		CAttachmentPosition& GetRightHand() { return m_RightHand; }

		uint32 GetType() { return ATTACHMENTS_TYPE_HUMAN; }

	protected :

		CAttachmentPosition		m_LeftHand;			// Our attachment positions...
		CAttachmentPosition		m_RightHand;
		CAttachmentPosition		m_Head;
		CAttachmentPosition		m_Helmet;
		CAttachmentPosition		m_LeftShoulder;
		CAttachmentPosition		m_RightShoulder;
		CAttachmentPosition		m_LeftFoot;
		CAttachmentPosition		m_RightFoot;
};

// ----------------------------------------------------------------- //

class CPlayerAttachments : public CHumanAttachments
{
	public :

		// Ctors/dtors/etc

		CPlayerAttachments(LTBOOL bAutoInit = LTTRUE);

		// Handlers

		void HandleDeath();

		// Weapons aggregate methods

		inline CWeapons * GetWeapons ()
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				return pAttachmentWeapon->GetWeapons();
			}
			return LTNULL;
		}

        inline CWeapon* GetWeapon(uint8 nWeaponId)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				return pAttachmentWeapon->GetWeapons()->GetWeapon(nWeaponId);
			}
			else
			{
				return NULL;
			}
		}

		inline CWeapon* GetWeapon()
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				return pAttachmentWeapon->GetWeapons()->GetCurWeapon();
			}
			else
			{
				return NULL;
			}
		}

		inline int GetAmmoCount(int iAmmoType)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				return pAttachmentWeapon->GetWeapons()->GetAmmoCount(iAmmoType);
			}
			else
			{
				return 0;
			}
		}

		inline int GetAmmoPoolCount(int iAmmoPool) 
		{ 
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				return pAttachmentWeapon->GetWeapons()->GetAmmoPoolCount(iAmmoPool); 
			}
			else
			{
				return 0;
			}
		}

		inline void DeselectWeapon()
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->DeselectCurWeapon();
			}
		}

		inline void ChangeWeapon(int nWeaponID)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->ChangeWeapon(nWeaponID);
			}
		}

		inline void ObtainWeapon(int nWeaponID, int nBarrelID = BARREL_DEFAULT_ID, int nAmmoPoolID = AMMO_DEFAULT_ID, 
			int nDefaultAmmo = -1, LTBOOL bNotifyClient=LTFALSE, LTBOOL bShowHud=LTTRUE)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->ObtainWeapon(nWeaponID, nBarrelID, nAmmoPoolID, nDefaultAmmo, bNotifyClient, bShowHud);
			}
		}

		inline void AddAmmo(int nAmmoId, int nAmount)
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->AddAmmo(nAmmoId, nAmount);
			}
		}

		inline void ResetAllWeapons()
		{
			CAttachmentWeapon* pAttachmentWeapon = GetDefaultAttachmentWeapon();
			if ( pAttachmentWeapon )
			{
				pAttachmentWeapon->GetWeapons()->Reset();
			}
		}

		// Handlers

		void HandleCheatFullAmmo();
		void HandleCheatFullWeapon();
		void HandleCheatFullClips();

		// Simple accessors

		uint32 GetType() { return ATTACHMENTS_TYPE_PLAYER; }

	protected :

		inline CAttachmentWeapon* GetDefaultAttachmentWeapon()
		{
			return (CAttachmentWeapon*)m_RightHand.GetAttachment();
		}

};

// ----------------------------------------------------------------- //

class CCreatureAttachments : public CAttachments
{
	public :

		// Ctor/dtor/etc

		CCreatureAttachments() {}

		// Simple accessors

		uint32 GetType() { return ATTACHMENTS_TYPE_CREATURE; }
};

// ----------------------------------------------------------------- //

// Attachments plugin definition

class CAttachmentsPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

	protected :

		void PopulateStringList(
			char* const * aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

};

// ----------------------------------------------------------------- //

class CHumanAttachmentsPlugin : public CAttachmentsPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

};

// ----------------------------------------------------------------- //

class CCreatureAttachmentsPlugin : public CAttachmentsPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

};

// ----------------------------------------------------------------- //

#endif // __ATTACHMENTS_H__
