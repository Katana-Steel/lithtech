#include "StdAfx.h"
#include "TO2GameClientShell.h"

// These macros create a bunch of static variables whose constructors do initialization
// for hooking into the engine.  They need to get created before all other static variables
// in the project.

ILTModelClient*		g_pILTModelClient = NULL;
define_holder(ILTModelClient, g_pILTModelClient);

ILTDrawPrim*		g_pDrawPrim;
define_holder(ILTDrawPrim,g_pDrawPrim);

ILTFontManager*		g_pFontManager;
define_holder(ILTFontManager,g_pFontManager);

ILTTexInterface*	g_pTexInterface;
define_holder(ILTTexInterface,g_pTexInterface);

SETUP_CLIENTSHELL();

#ifndef __LINUX
define_interface(CTO2GameClientShell, IClientShell);
#else
static CAPIInstanceDefines<CTO2GameClientShell>
	global_static_impl_CTO2GameClientShell_defines_IClientShell_Default_{
		new CTO2GameClientShell,
		"IClientShell" "." "Default",
		IClientShell::_IClientShell_VERSION_};

static SStaticSearchInterface
	global_static_var_search_IClientShell_CTO2GameClientShell_Default_ = {
		SEARCH_MARKER_INTERFACE, SEARCH_MARKER_INT, "IClientShell",
		"CTO2GameClientShell", "Default",
		IClientShell::_IClientShell_VERSION_};
#endif