#include "bdefs.h"
#include "input.h"
#include <SDL.h>
#ifdef WIN32
#include "dsys_interface.h"
#include "dinput.h"
#else
#include "linuxdsys.h"
#endif
#include "sdl2_scancode_to_dinput.h"
#include <algorithm>
#include <unordered_map>

#define SPECIAL_MOUSEX	-50000
#define SPECIAL_MOUSEY	-50001
#define SPECIAL_MOUSECLICK_LEFT	 -50002
#define SPECIAL_MOUSECLICK_RIGHT -50003
#define SPECIAL_MOUSEWHEEL_UP	 -50004
#define SPECIAL_MOUSEWHEEL_DOWN -50005

struct SDL2Key
{
	const char *m_pName;
	SDL_Keycode key;
	float m_Scale;
};

struct ISAction
{
	char m_Name[100];
	int m_Code;
	ISAction *m_pNext;
};

struct ISBinding
{
	char m_deviceName[32];
	float m_ranges[2];
	SDL2Key *m_pKey;
	ISAction *m_pAction;
	ISBinding *m_pNext;
};


std::unordered_map<std::string, ISAction*> g_sdl2_actions;
std::vector<ISBinding*> g_sdl2_bindings;

int g_key_state[SDL_NUM_SCANCODES];
int g_click_state[2];
int g_mousewheel;

SDL2Key g_Keys[] =
{
	"X-axis", SPECIAL_MOUSEX, 1.0f,
	"Y-axis", SPECIAL_MOUSEY, 1.0f,

	"Button 0", SPECIAL_MOUSECLICK_LEFT, 1.0f,
	"Button 1", SPECIAL_MOUSECLICK_RIGHT, 1.0f,

	"Mousewheel Up", SPECIAL_MOUSEWHEEL_UP, 1.0f,
	"Mousewheel Down", SPECIAL_MOUSEWHEEL_DOWN, 1.0f,

	"Left Alt", SDLK_LALT,  1.0f,
	"Right Alt", SDLK_RALT, 1.0f,

	"A", SDLK_a, 1.0f,
	"B", SDLK_b, 1.0f,
	"C", SDLK_c, 1.0f,
	"D", SDLK_d, 1.0f,
	"E", SDLK_e, 1.0f,
	"F", SDLK_f, 1.0f,
	"G", SDLK_g, 1.0f,
	"H", SDLK_h, 1.0f,
	"I", SDLK_i, 1.0f,
	"J", SDLK_j, 1.0f,
	"K", SDLK_k, 1.0f,
	"L", SDLK_l, 1.0f,
	"M", SDLK_m, 1.0f,
	"N", SDLK_n, 1.0f,
	"O", SDLK_o, 1.0f,
	"P", SDLK_p, 1.0f,
	"Q", SDLK_q, 1.0f,
	"R", SDLK_r, 1.0f,
	"S", SDLK_s, 1.0f,
	"T", SDLK_t, 1.0f,
	"U", SDLK_u, 1.0f,
	"V", SDLK_v, 1.0f,
	"W", SDLK_w, 1.0f,
	"X", SDLK_x, 1.0f,
	"Y", SDLK_y, 1.0f,
	"Z", SDLK_z, 1.0f,

	"1", SDLK_1, 1.0f,
	"2", SDLK_2, 1.0f,
	"3", SDLK_3, 1.0f,
	"4", SDLK_4, 1.0f,
	"5", SDLK_5, 1.0f,
	"6", SDLK_6, 1.0f,
	"7", SDLK_7, 1.0f,
	"8", SDLK_8, 1.0f,
	"9", SDLK_9, 1.0f,
	"0", SDLK_0, 1.0f,

	"F1", SDLK_F1, 1.0f,
	"F2", SDLK_F2, 1.0f,
	"F3", SDLK_F3, 1.0f,
	"F4", SDLK_F4, 1.0f,
	"F5", SDLK_F5, 1.0f,
	"F6", SDLK_F6, 1.0f,
	"F7", SDLK_F7, 1.0f,
	"F8", SDLK_F8, 1.0f,
	"F9", SDLK_F9, 1.0f,
	"F10", SDLK_F10, 1.0f,
	"F11", SDLK_F11, 1.0f,
	"F12", SDLK_F12, 1.0f,

	// "Numpad 1", VK_NUMPAD1, DIK_NUMPAD1, 1.0f,
	// "Numpad 2", VK_NUMPAD2, DIK_NUMPAD2, 1.0f,
	// "Numpad 3", VK_NUMPAD3, DIK_NUMPAD3, 1.0f,
	// "Numpad 4", VK_NUMPAD4, DIK_NUMPAD4, 1.0f,
	// "Numpad 5", VK_NUMPAD5, DIK_NUMPAD5, 1.0f,
	// "Numpad 6", VK_NUMPAD6, DIK_NUMPAD6, 1.0f,
	// "Numpad 7", VK_NUMPAD7, DIK_NUMPAD7, 1.0f,
	// "Numpad 8", VK_NUMPAD8, DIK_NUMPAD8, 1.0f,
	// "Numpad 9", VK_NUMPAD9, DIK_NUMPAD9, 1.0f,
	// "Numpad +", VK_ADD, DIK_ADD, 1.0f,
	// "Numpad -", VK_SUBTRACT, DIK_SUBTRACT, 1.0f,
	// "Numpad *", VK_MULTIPLY, DIK_MULTIPLY, 1.0f,
	// "Numpad /", VK_DIVIDE, DIK_DIVIDE, 1.0f,

	"Up Arrow", SDLK_UP, 1.0f,
	"Left Arrow", SDLK_LEFT, 1.0f,
	"Down Arrow", SDLK_DOWN, 1.0f,
	"Right Arrow", SDLK_RIGHT, 1.0f,
	// "PgUp", VK_PRIOR, DIK_PRIOR, 1.0f,
	// "PgDn", VK_NEXT, DIK_NEXT, 1.0f,
	// "Home", VK_HOME, DIK_HOME, 1.0f,
	// "End", VK_END, DIK_END, 1.0f,

	"Backspace", SDLK_BACKSPACE, 1.0f,
	"Tab", SDLK_TAB, 1.0f,
	"Enter", SDLK_RETURN, 1.0f,
	"Left Ctrl", SDLK_LCTRL, 1.0f,
	"Right Ctrl", SDLK_RCTRL, 1.0f,
	"Left Shift", SDLK_LSHIFT, 1.0f,
	"Right Shift", SDLK_RSHIFT, 1.0f,
	"Space", SDLK_SPACE, 1.0f,
	"CapsLock", SDLK_CAPSLOCK, 1.0f
};
#define NUM_KEYS (sizeof(g_Keys)/sizeof(g_Keys[0]))

static int SDLScanCodeToKeyNum(SDL_Scancode sc)
{
	int idx = (int)sc;
	ASSERT(idx >= 0 && idx < SDL_NUM_SCANCODES);
	return scanCodeToKeyNum[idx];
}

SDL2Key* input_sdl2_FindKey(const char *pName, const char* deviceName, float rangelow, float rangehigh)
{
	uint32 i;
	int testCode;
	int dik;
	SDL_Scancode scancode;

	if(strlen(pName) > 2 && pName[0] == '#' && pName[1] == '#')
	{
		pName += 2;

		if(stricmp(pName, "x-axis") == 0)
		{
			return &g_Keys[0];
		}
		if(stricmp(pName, "y-axis") == 0)
		{
			return &g_Keys[1];
		}
		if(!strcmp(pName, "3") && !strcmp(deviceName, "##mouse"))
		{
			return &g_Keys[2];
		}
		if(!strcmp(pName, "4") && !strcmp(deviceName, "##mouse"))
		{
			return &g_Keys[3];
		}
		if(!strcmp(pName, "z-axis"))
		{
			if (rangelow > 0.0f && rangehigh > 0.0f)
			{
				return &g_Keys[4];
			}
			if (rangelow < 0.0f && rangehigh < 0.0f)
			{
				return &g_Keys[5];
			}
			ASSERT(0);
		}

		testCode = atoi(pName);
		for(i=0; i < NUM_KEYS; i++)
		{
			scancode = SDL_GetScancodeFromKey(g_Keys[i].key);
			dik = SDLScanCodeToKeyNum(scancode);

			if(dik == testCode)
			{
				return &g_Keys[i];
			}
		}

		// Fall below where it looks without the ##.
	}

	for(i=0; i < NUM_KEYS; i++)
	{
		if(stricmp(pName, g_Keys[i].m_pName) == 0)
		{
			return &g_Keys[i];
		}
	}

	return LTNULL;
}

ISAction* input_sdl2_FindAction(const char *pName)
{
	std::string tmp{pName};
	if (g_sdl2_actions.find(tmp) != g_sdl2_actions.end())
		return g_sdl2_actions.at(tmp);

	return nullptr;
}


bool input_sdl2_init(InputMgr *pMgr, ConsoleState *pState)
{
	size_t i;
	for (i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		g_key_state[i] = 0;
	}
	g_click_state[0] = g_click_state[1] = 0;
	g_mousewheel = 0;
	return true;
}

void input_sdl2_term(InputMgr *pMgr)
{
	for (auto &bind : g_sdl2_bindings)
		dfree(bind);

	for(auto &act : g_sdl2_actions)
		dfree(act.second);

	g_sdl2_bindings.clear();
	g_sdl2_actions.clear();
}

bool input_sdl2_IsInitted(InputMgr *pMgr)
{
	return true;
}

void input_sdl2_ListDevices(InputMgr *pMgr)
{
	dsi_ConsolePrint("SIMULATED INPUT ------------------");
	dsi_ConsolePrint("Only devices are keyboard and mouse");
}

long input_sdl2_PlayJoystickEffect(InputMgr *pMgr, const char* strEffectName, float x, float y)
{
	return 0;
}

void input_sdl2_ReadInput(InputMgr *pMgr, uint8 *pActionsOn, float axisOffsets[3], void* keyDowns, int* mouseclick, int* mouserel, int mousewheel)
{
	SDL_Keycode* downs = (SDL_Keycode*)keyDowns;
	memcpy(g_key_state, keyDowns, sizeof(int) * SDL_NUM_SCANCODES);
	memcpy(g_click_state, mouseclick, sizeof(int) * 2);
	g_mousewheel = mousewheel;

	axisOffsets[0] = axisOffsets[1] = axisOffsets[2] = 0.0f;

	for (auto &pBinding: g_sdl2_bindings)
	{
		float value = 0.0f;

		switch(pBinding->m_pKey->key)
		{
		case SPECIAL_MOUSEX:
			value = float(mouserel[0]) * pBinding->m_pKey->m_Scale;
			break;
		case SPECIAL_MOUSEY:
			value = float(mouserel[1]) * pBinding->m_pKey->m_Scale;
			break;
		case SPECIAL_MOUSECLICK_LEFT:
			if(mouseclick[0])
				value = 1.0f;
			break;
		case SPECIAL_MOUSECLICK_RIGHT:
			if(mouseclick[1])
				value = 1.0f;
			break;
		case SPECIAL_MOUSEWHEEL_UP:
			if(mousewheel > 0)
				value = 1.0f;
			break;
		case SPECIAL_MOUSEWHEEL_DOWN:
			if(mousewheel < 0)
				value = 1.0f;
			break;
		default:
			auto scancode = SDL_GetScancodeFromKey(pBinding->m_pKey->key);
			if(downs[scancode] == 1)
			{
				value = 1.0f;
			}
			break;
		}

		switch(pBinding->m_pAction->m_Code)
		{
		case -1:
			axisOffsets[0] += value;
			break;
		case -2:
			axisOffsets[1] += value;
			break;
		case -3:
			axisOffsets[2] += value;
			break;
		default:
			pActionsOn[pBinding->m_pAction->m_Code] |= (uint8)value;
			break;

		}
	}
}

bool input_sdl2_FlushInputBuffers(InputMgr *pMgr)
{
	return true;
}

LTRESULT input_sdl2_ClearInput()
{
	return LT_OK;
}

void input_sdl2_AddAction(InputMgr *pMgr, const char *pActionName, int actionCode)
{
	ISAction *pAction = input_sdl2_FindAction(pActionName);

	if(pAction)
		return;

	LT_MEM_TRACK_ALLOC(pAction = (ISAction*)dalloc(sizeof(ISAction)),LT_MEM_TYPE_INPUT);
	LTStrCpy(pAction->m_Name, pActionName, sizeof(pAction->m_Name));
	pAction->m_Code = actionCode;
	pAction->m_pNext = nullptr;
	g_sdl2_actions[pActionName] = pAction;
}

bool input_sdl2_EnableDevice(InputMgr *pMgr, const char *pDeviceName)
{
	return true;
}

bool input_sdl2_ClearBindings(InputMgr *pMgr, const char *pDeviceName, const char *pTriggerName)
{
	return true;
}

bool input_sdl2_AddBinding(InputMgr *pMgr,
	const char *pDeviceName, const char *pTriggerName, const char *pActionName,
	float rangeLow, float rangeHigh)
{
	ISBinding *pBinding, *bind_cur;
	if (strlen(pDeviceName) == 0 || strlen(pActionName) == 0)
	{
		return false;
	}

	ISAction *pAction = input_sdl2_FindAction(pActionName);
	if(!pAction)
		return false;

	SDL2Key *pKey = input_sdl2_FindKey(pTriggerName, pDeviceName, rangeLow, rangeHigh);
	if(!pKey)
		return false;
    auto bind = std::find_if(g_sdl2_bindings.begin(),g_sdl2_bindings.end(),
		[pAction, pDeviceName](auto b) { return (b->m_pAction == pAction && !strcmp(b->m_deviceName, pDeviceName)); }
	);
	if (bind != g_sdl2_bindings.end())
	{
		bind_cur = *bind;
		if(bind_cur->m_pKey != pKey)
		{
			bind_cur->m_pKey = pKey;
			bind_cur->m_ranges[0] = rangeLow;
			bind_cur->m_ranges[1] = rangeHigh;
			return true;
		} else {
			// no duplicates
			return false;
		}
	}

	LT_MEM_TRACK_ALLOC(pBinding = (ISBinding*)dalloc(sizeof(ISBinding)),LT_MEM_TYPE_INPUT);
	pBinding->m_pKey = pKey;
	pBinding->m_pAction = pAction;
	strcpy(pBinding->m_deviceName, pDeviceName);
	pBinding->m_ranges[0] = rangeLow;
	pBinding->m_ranges[1] = rangeHigh;

	g_sdl2_bindings.push_back(pBinding);
	return true;
}

bool input_sdl2_ScaleTrigger(InputMgr *pMgr, const char *pDeviceName, const char *pTriggerName, float scale, float fRangeScaleMin, float fRangeScaleMax, float fRangeScalePreCenterOffset )
{
	SDL2Key *pKey = input_sdl2_FindKey(pTriggerName, pDeviceName, fRangeScaleMin, fRangeScaleMax);

	if(pKey)
	{
		pKey->m_Scale = scale;
		return true;
	}
	else
	{
		return false;
	}
}

void make_keyboard_binding(DeviceBinding* pBinding, ISBinding *bind)
{
	LTStrCpy( pBinding->strDeviceName, "Keyboard", sizeof(pBinding->strDeviceName) );
	LTStrCpy( pBinding->strTriggerName, bind->m_pKey->m_pName, sizeof(pBinding->strTriggerName) );
	pBinding->m_nObjectId = (uint32)bind->m_pKey->key;
}

void make_mouse_binding(DeviceBinding* pBinding, ISBinding *bind)
{
	LTStrCpy( pBinding->strDeviceName, "Mouse", sizeof(pBinding->strDeviceName) );
	LTStrCpy( pBinding->strTriggerName, bind->m_pKey->m_pName, sizeof(pBinding->strTriggerName) );
	switch (bind->m_pKey->key) {
	case SPECIAL_MOUSECLICK_LEFT:
		pBinding->m_nObjectId = 1;
		break;
	case SPECIAL_MOUSECLICK_RIGHT:
		pBinding->m_nObjectId = 2;
		break;
	case SPECIAL_MOUSEWHEEL_UP:
		pBinding->m_nObjectId = 3;
		pBinding->pActionHead->nRangeLow = bind->m_ranges[0];
		pBinding->pActionHead->nRangeHigh = bind->m_ranges[1];
		break;
	case SPECIAL_MOUSEWHEEL_DOWN:
		pBinding->m_nObjectId = 4;
		pBinding->pActionHead->nRangeLow = bind->m_ranges[0];
		pBinding->pActionHead->nRangeHigh = bind->m_ranges[1];
		break;
	}
}

DeviceBinding* input_sdl2_GetDeviceBindings ( uint32 nDevice )
{
	DeviceBinding* pBindingsHead = nullptr;

	for (auto &cur_bind: g_sdl2_bindings)
	{
		if (nDevice == DEVICETYPE_MOUSE && strcmp(cur_bind->m_deviceName, "##mouse"))
		{
			continue;
		}
		if (nDevice == DEVICETYPE_KEYBOARD && strcmp(cur_bind->m_deviceName, "##keyboard"))
		{
			continue;
		}
		GameAction *Action;
		LT_MEM_TRACK_ALLOC(Action = new GameAction,LT_MEM_TYPE_INPUT);
		memset( Action, 0, sizeof(GameAction) );

		Action->nActionCode = cur_bind->m_pAction->m_Code;
		LTStrCpy( Action->strActionName, cur_bind->m_pAction->m_Name, sizeof( Action->strActionName) );

		DeviceBinding* pBinding;
		LT_MEM_TRACK_ALLOC(pBinding = new DeviceBinding,LT_MEM_TYPE_INPUT);
		if( !pBinding ) break;
		memset( pBinding, 0, sizeof(DeviceBinding) );

		LTStrCpy( pBinding->strDeviceName, "Unknown", sizeof(pBinding->strDeviceName) );
		pBinding->pActionHead = Action;
		switch(nDevice)
		{
		case DEVICETYPE_MOUSE:
			make_mouse_binding(pBinding, cur_bind);
			break;
		case DEVICETYPE_KEYBOARD:
			 make_keyboard_binding(pBinding, cur_bind);
			break;
		}

		pBinding->pNext = pBindingsHead;
		pBindingsHead = pBinding;
	}

	return pBindingsHead;
}

void input_sdl2_SaveBindings(FILE *fp)
{
	for( auto &pCur : g_sdl2_actions)
		fprintf(fp, "AddAction %s %d\n", pCur.second->m_Name, pCur.second->m_Code);

	fprintf(fp, "\nenabledevice \"##keyboard\"\n");

	for (auto &cur_bind: g_sdl2_bindings)
	{
		if(!strcmp(cur_bind->m_deviceName, "##keyboard"))
		{
			auto dik = SDLScanCodeToKeyNum(
				SDL_GetScancodeFromKey(cur_bind->m_pKey->key)
			);
			fprintf(fp, "rangebind \"##keyboard\" \"##%d\" %f %f \"%s\"\n",
				dik,
				cur_bind->m_ranges[0], cur_bind->m_ranges[1],
				cur_bind->m_pAction->m_Name
			);
		}
	}

	fprintf(fp, "\nenabledevice \"##mouse\"\n");
	for(auto &cur_bind: g_sdl2_bindings)
	{
		const char *axis = nullptr;
		if(strcmp(cur_bind->m_deviceName, "##mouse")) // if not mouse binding, continue
			continue;

		if (!strcmp(cur_bind->m_pAction->m_Name, "Axis1"))
		{
			axis = "x-axis";
		}
		else if (!strcmp(cur_bind->m_pAction->m_Name, "Axis2"))
		{
			axis = "y-axis";
		} else {
			switch (cur_bind->m_pKey->key) {
			case SPECIAL_MOUSECLICK_LEFT:
				axis = "3";
				break;
			case SPECIAL_MOUSECLICK_RIGHT:
				axis = "4";
				break;
			case SPECIAL_MOUSEWHEEL_DOWN:
			case SPECIAL_MOUSEWHEEL_UP:
				axis = "z-axis";
				break;
			}
		}
		if (!axis)
			continue;

		fprintf(fp, "rangebind \"##mouse\" \"##%s\" %f %f \"%s\"\n",
			axis, cur_bind->m_ranges[0], cur_bind->m_ranges[1], cur_bind->m_pAction->m_Name);
	}

	for(int i = 0; i < NUM_KEYS; i++)
	{
		if(!stricmp(g_Keys[i].m_pName, "x-axis"))
		{
			fprintf(fp, "scale \"##mouse\" \"##x-axis\" %f\n", g_Keys[i].m_Scale);
		}
		if(!stricmp(g_Keys[i].m_pName, "y-axis"))
		{
			fprintf(fp, "scale \"##mouse\" \"##y-axis\" %f\n", g_Keys[i].m_Scale);
		}
	}
}

void input_sdl2_FreeDeviceBindings ( DeviceBinding* pBindings )
{
	DeviceBinding* pBinding = pBindings;
	while( pBinding )
	{
		GameAction* pAction = pBinding->pActionHead;
		while( pAction )
		{
			GameAction* pNext = pAction->pNext;
			delete pAction;
			pAction = pNext;
		}

		DeviceBinding* pNext = pBinding->pNext;
		delete pBinding;
		pBinding = pNext;
	}
}

bool input_sdl2_StartDeviceTrack(InputMgr *pMgr, uint32 nDeviceFlags, uint32 nBufferSize)
{
	return true;
}

bool input_sdl2_TrackDevice(DeviceInput *pInputArray, uint32 *pnInOut)
{
	size_t i, j;
	uint32 cnt = 0;
	SDL_Keycode keycode;
	int found_keycode;

	for (i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		if (g_key_state[i] == 1)
		{
			found_keycode = 0;
			pInputArray[cnt].m_DeviceType = DEVICETYPE_KEYBOARD;
			LTStrCpy(pInputArray[cnt].m_DeviceName, "##keyboard", sizeof(pInputArray[cnt].m_DeviceName) );
			pInputArray[cnt].m_ControlType = CONTROLTYPE_KEY;
			pInputArray[cnt].m_ControlCode = 0;
			pInputArray[cnt].m_nObjectId = i;
			pInputArray[cnt].m_InputValue = 1;
			for (j = 0; j < NUM_KEYS; j++)
			{
				keycode = SDL_GetKeyFromScancode((SDL_Scancode)i);
				if (g_Keys[j].key == keycode)
				{
					LTStrCpy(pInputArray[cnt].m_ControlName, g_Keys[j].m_pName, sizeof(pInputArray[cnt].m_ControlName) );
					found_keycode = 1;
				}
			}
			ASSERT(found_keycode);
			cnt++;
		}
	}

	for (i = 0; i < 2; i++)
	{
		if (g_click_state[i] == 1)
		{
			pInputArray[cnt].m_DeviceType = DEVICETYPE_MOUSE;
			LTStrCpy(pInputArray[cnt].m_DeviceName, "##mouse", sizeof(pInputArray[cnt].m_DeviceName) );
			pInputArray[cnt].m_ControlType = CONTROLTYPE_BUTTON;
			pInputArray[cnt].m_ControlCode = 0;
			pInputArray[cnt].m_nObjectId = i + 1;
			pInputArray[cnt].m_InputValue = 1;
			LTStrCpy(pInputArray[cnt].m_ControlName, g_Keys[i + 2].m_pName, sizeof(pInputArray[cnt].m_ControlName) );
			cnt++;
		}
	}

	if (g_mousewheel != 0)
	{
		pInputArray[cnt].m_DeviceType = DEVICETYPE_MOUSE;
		LTStrCpy(pInputArray[cnt].m_DeviceName, "##mouse", sizeof(pInputArray[cnt].m_DeviceName) );
		pInputArray[cnt].m_ControlType = CONTROLTYPE_ZAXIS;
		pInputArray[cnt].m_ControlCode = 0;
		pInputArray[cnt].m_nObjectId = g_mousewheel > 0 ? 3 : 4;
		pInputArray[cnt].m_InputValue = g_mousewheel > 0 ? 1 : -1;
		LTStrCpy(pInputArray[cnt].m_ControlName, g_Keys[g_mousewheel > 0 ? 4 : 5].m_pName, sizeof(pInputArray[cnt].m_ControlName) );
		cnt++;
	}
	*pnInOut = cnt;
	return true;
}

bool input_sdl2_EndDeviceTrack()
{
	return true;
}

DeviceObject* input_sdl2_GetDeviceObjects ( uint32 nDeviceFlags )
{
	return LTNULL;
}

void input_sdl2_FreeDeviceObjects ( DeviceObject* pList )
{
}

bool input_sdl2_GetDeviceName ( uint32 nDeviceType, char* pStrBuffer, uint32 nBufferSize )
{
	switch (nDeviceType)
	{
	case DEVICETYPE_KEYBOARD:
		LTStrCpy( pStrBuffer, "##keyboard", nBufferSize );
		break;
	case DEVICETYPE_MOUSE:
		LTStrCpy( pStrBuffer, "##mouse", nBufferSize );
		break;
	default:
		return false;
		break;
	}

	return true;
}

bool input_sdl2_IsDeviceEnabled ( const char* pDeviceName )
{
	return true;
}

static bool input_sdl2_GetDeviceObjectName( char const* pszDeviceName, uint32 nDeviceObjectId,
	char* pszDeviceObjectName, uint32 nDeviceObjectNameLen )
{
	return true;
}

InputMgr g_InputSDL2Mgr =
{
	input_sdl2_init,
	input_sdl2_term,
	input_sdl2_IsInitted,
	input_sdl2_ListDevices,
	input_sdl2_PlayJoystickEffect,
	input_sdl2_ReadInput,
	input_sdl2_FlushInputBuffers,
	input_sdl2_ClearInput,
	input_sdl2_AddAction,
	input_sdl2_EnableDevice,
	input_sdl2_ClearBindings,
	input_sdl2_AddBinding,
	input_sdl2_ScaleTrigger,
	input_sdl2_GetDeviceBindings,
	input_sdl2_FreeDeviceBindings,
	input_sdl2_StartDeviceTrack,
	input_sdl2_TrackDevice,
	input_sdl2_EndDeviceTrack,
	input_sdl2_GetDeviceObjects,
	input_sdl2_FreeDeviceObjects,
	input_sdl2_GetDeviceName,
	input_sdl2_GetDeviceObjectName,
	input_sdl2_IsDeviceEnabled
};
