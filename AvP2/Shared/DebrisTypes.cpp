// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisTypes.cpp
//
// PURPOSE : Debris types
//
// CREATED : 7/2/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebrisTypes.h"
#include "CommonUtilities.h"

char* DebrisTypeToString[DBT_LAST] =
{
	"GENERIC",			// Generic debris
	"GENERIC_FLAT",		// Generic flat debris
	"BOARDS",			// Wood boards
	"BRANCHES_BIG",		// Big Tree/bush branches 
	"BRANCHES_SMALL",	// Small Tree/bush branches
	"WOODCHIPS",		// Small wood chips
	"PLASTIC",			// Plastic debris
	"GLASS_BIG",		// Big glass debris
	"GLASS_SMALL",		// Small glass debris
	"FEATHERS",			// Feathers (for furniture)
	"STONE_BIG",		// Big stone debris
	"STONE_SMALL",		// Small stone debris
	"METAL_BIG",		// Big metal debris
	"METAL_SMALL",		// Small metal debris
	"CAR_PARTS",		// Car parts
	"VEHICLE_PARTS",	// Vehicle parts
	"HUMAN_PARTS"		// Human parts
};


// Debris model related sheyot...

char* s_debrisGenericFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\generic1.abc",
	"SFX\\Debris\\Models\\generic2.abc",
	"SFX\\Debris\\Models\\generic3.abc",
	"SFX\\Debris\\Models\\generic4.abc",
	"SFX\\Debris\\Models\\generic5.abc",
	"SFX\\Debris\\Models\\generic6.abc",
	"SFX\\Debris\\Models\\generic7.abc"
#else
	"SFX/Debris/Models/generic1.abc",
	"SFX/Debris/Models/generic2.abc",
	"SFX/Debris/Models/generic3.abc",
	"SFX/Debris/Models/generic4.abc",
	"SFX/Debris/Models/generic5.abc",
	"SFX/Debris/Models/generic6.abc",
	"SFX/Debris/Models/generic7.abc"
#endif
};

char* s_debrisGernicFlatFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\generic_flat1.abc",
	"SFX\\Debris\\Models\\generic_flat2.abc",
	"SFX\\Debris\\Models\\generic_flat3.abc"
#else
	"SFX/Debris/Models/generic_flat1.abc",
	"SFX/Debris/Models/generic_flat2.abc",
	"SFX/Debris/Models/generic_flat3.abc"
#endif
};

char* s_debrisBoardFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\board1.abc",
	"SFX\\Debris\\Models\\board2.abc",
	"SFX\\Debris\\Models\\board3.abc"
#else
	"SFX/Debris/Models/board1.abc",
	"SFX/Debris/Models/board2.abc",
	"SFX/Debris/Models/board3.abc"
#endif
};

char* s_debrisBranchFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\branch1.abc",
	"SFX\\Debris\\Models\\branch2.abc",
	"SFX\\Debris\\Models\\branch3.abc"
#else
	"SFX/Debris/Models/branch1.abc",
	"SFX/Debris/Models/branch2.abc",
	"SFX/Debris/Models/branch3.abc"
#endif
};

char* s_debrisWoodChipFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\woodchip1.abc",
	"SFX\\Debris\\Models\\woodchip2.abc",
	"SFX\\Debris\\Models\\woodchip3.abc"
#else
	"SFX/Debris/Models/woodchip1.abc",
	"SFX/Debris/Models/woodchip2.abc",
	"SFX/Debris/Models/woodchip3.abc"
#endif
};

char* s_debrisPlasticFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\generic_flat1.abc",
	"SFX\\Debris\\Models\\generic_flat1.abc",
	"SFX\\Debris\\Models\\generic_flat1.abc"
#else
	"SFX/Debris/Models/generic_flat1.abc",
	"SFX/Debris/Models/generic_flat1.abc",
	"SFX/Debris/Models/generic_flat1.abc"
#endif
};

char* s_debrisGlassFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\Glass1.abc",
	"SFX\\Debris\\Models\\Glass2.abc",
	"SFX\\Debris\\Models\\Glass3.abc"
#else
	"SFX/Debris/Models/Glass1.abc",
	"SFX/Debris/Models/Glass2.abc",
	"SFX/Debris/Models/Glass3.abc"
#endif
};

char* s_debrisFeatherFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\Feather1.abc",
	"SFX\\Debris\\Models\\Feather2.abc",
	"SFX\\Debris\\Models\\Feather3.abc"
#else
	"SFX/Debris/Models/Feather1.abc",
	"SFX/Debris/Models/Feather2.abc",
	"SFX/Debris/Models/Feather3.abc"
#endif
};

char* s_debrisStoneFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\Stone1.abc",
	"SFX\\Debris\\Models\\Stone2.abc",
	"SFX\\Debris\\Models\\Stone3.abc"
#else
	"SFX/Debris/Models/Stone1.abc",
	"SFX/Debris/Models/Stone2.abc",
	"SFX/Debris/Models/Stone3.abc"
#endif
};

char* s_debrisMetalFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\Metal1.abc",
	"SFX\\Debris\\Models\\Metal2.abc",
	"SFX\\Debris\\Models\\Metal3.abc"
#else
	"SFX/Debris/Models/Metal1.abc",
	"SFX/Debris/Models/Metal2.abc",
	"SFX/Debris/Models/Metal3.abc"
#endif
};

char* s_debrisCarPartFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\CarPart1.abc",
	"SFX\\Debris\\Models\\CarPart2.abc",
	"SFX\\Debris\\Models\\CarPart3.abc"
#else
	"SFX/Debris/Models/CarPart1.abc",
	"SFX/Debris/Models/CarPart2.abc",
	"SFX/Debris/Models/CarPart3.abc"
#endif
};

char* s_debrisVehiclePartFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\VehiclePart1.abc",
	"SFX\\Debris\\Models\\VehiclePart2.abc",
	"SFX\\Debris\\Models\\VehiclePart3.abc"
#else
	"SFX/Debris/Models/VehiclePart1.abc",
	"SFX/Debris/Models/VehiclePart2.abc",
	"SFX/Debris/Models/VehiclePart3.abc"
#endif
};

char* s_debrisHumanFilename[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Models\\HumanPart1.abc",
	"SFX\\Debris\\Models\\HumanPart2.abc",
	"SFX\\Debris\\Models\\HumanPart3.abc"
#else
	"SFX/Debris/Models/HumanPart1.abc",
	"SFX/Debris/Models/HumanPart2.abc",
	"SFX/Debris/Models/HumanPart3.abc"
#endif
};

char** s_pDebrisModels[] =
{
	s_debrisGenericFilename,
	s_debrisGernicFlatFilename,
	s_debrisBoardFilename,
	s_debrisBranchFilename,
	s_debrisBranchFilename,
	s_debrisWoodChipFilename,
	s_debrisPlasticFilename,
	s_debrisGlassFilename,
	s_debrisGlassFilename,
	s_debrisFeatherFilename,
	s_debrisStoneFilename,
	s_debrisStoneFilename,
	s_debrisMetalFilename,
	s_debrisMetalFilename,
	s_debrisCarPartFilename,
	s_debrisVehiclePartFilename,
	s_debrisHumanFilename
};


// Debris skins...

char* s_pDebrisSkins[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Skins\\generic.dtx",
	"SFX\\Debris\\Skins\\generic.dtx",
	"SFX\\Debris\\Skins\\board.dtx",
	"SFX\\Debris\\Skins\\branch.dtx",
	"SFX\\Debris\\Skins\\branch.dtx",
	"SFX\\Debris\\Skins\\woodchip.dtx",
	"SFX\\Debris\\Skins\\plastic.dtx",
	"SFX\\Debris\\Skins\\glass.dtx",
	"SFX\\Debris\\Skins\\glass.dtx",
	"SFX\\Debris\\Skins\\feather.dtx",
	"SFX\\Debris\\Skins\\stone.dtx",
	"SFX\\Debris\\Skins\\stone.dtx",
	"SFX\\Debris\\Skins\\metal.dtx",
	"SFX\\Debris\\Skins\\metal.dtx",
	"SFX\\Debris\\Skins\\car.dtx",
	"SFX\\Debris\\Skins\\vehicle.dtx", 
	"SFX\\Debris\\Skins\\human.dtx"
#else
	"SFX/Debris/Skins/generic.dtx",
	"SFX/Debris/Skins/generic.dtx",
	"SFX/Debris/Skins/board.dtx",
	"SFX/Debris/Skins/branch.dtx",
	"SFX/Debris/Skins/branch.dtx",
	"SFX/Debris/Skins/woodchip.dtx",
	"SFX/Debris/Skins/plastic.dtx",
	"SFX/Debris/Skins/glass.dtx",
	"SFX/Debris/Skins/glass.dtx",
	"SFX/Debris/Skins/feather.dtx",
	"SFX/Debris/Skins/stone.dtx",
	"SFX/Debris/Skins/stone.dtx",
	"SFX/Debris/Skins/metal.dtx",
	"SFX/Debris/Skins/metal.dtx",
	"SFX/Debris/Skins/car.dtx",
	"SFX/Debris/Skins/vehicle.dtx", 
	"SFX/Debris/Skins/human.dtx"
#endif
};


// Debris bounce sound related sheyot...

char* s_debrisBoardBounce[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Board\\gib_1.wav",
	"SFX\\Debris\\Snd\\Board\\gib_2.wav",
	"SFX\\Debris\\Snd\\Board\\gib_3.wav",
	"SFX\\Debris\\Snd\\Board\\gib_4.wav",
	"SFX\\Debris\\Snd\\Board\\gib_5.wav"
#else
	"SFX/Debris/Snd/Board/gib_1.wav",
	"SFX/Debris/Snd/Board/gib_2.wav",
	"SFX/Debris/Snd/Board/gib_3.wav",
	"SFX/Debris/Snd/Board/gib_4.wav",
	"SFX/Debris/Snd/Board/gib_5.wav"
#endif
};

char* s_debrisBranchBounceBig[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\BigBranches\\gib_1.wav",
	"SFX\\Debris\\Snd\\BigBranches\\gib_2.wav",
	"SFX\\Debris\\Snd\\BigBranches\\gib_3.wav",
	"SFX\\Debris\\Snd\\BigBranches\\gib_4.wav",
	"SFX\\Debris\\Snd\\BigBranches\\gib_5.wav"
#else
	"SFX/Debris/Snd/BigBranches/gib_1.wav",
	"SFX/Debris/Snd/BigBranches/gib_2.wav",
	"SFX/Debris/Snd/BigBranches/gib_3.wav",
	"SFX/Debris/Snd/BigBranches/gib_4.wav",
	"SFX/Debris/Snd/BigBranches/gib_5.wav"
#endif
};

char* s_debrisBranchBounceSmall[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\SmallBranches\\gib_1.wav",
	"SFX\\Debris\\Snd\\SmallBranches\\gib_2.wav",
	"SFX\\Debris\\Snd\\SmallBranches\\gib_3.wav",
	"SFX\\Debris\\Snd\\SmallBranches\\gib_4.wav",
	"SFX\\Debris\\Snd\\SmallBranches\\gib_5.wav"
#else
	"SFX/Debris/Snd/SmallBranches/gib_1.wav",
	"SFX/Debris/Snd/SmallBranches/gib_2.wav",
	"SFX/Debris/Snd/SmallBranches/gib_3.wav",
	"SFX/Debris/Snd/SmallBranches/gib_4.wav",
	"SFX/Debris/Snd/SmallBranches/gib_5.wav"
#endif
};

char* s_debrisWoodChipBounce[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\WoodChips\\gib_1.wav",
	"SFX\\Debris\\Snd\\WoodChips\\gib_2.wav",
	"SFX\\Debris\\Snd\\WoodChips\\gib_3.wav",
	"SFX\\Debris\\Snd\\WoodChips\\gib_4.wav",
	"SFX\\Debris\\Snd\\WoodChips\\gib_5.wav"
#else
	"SFX/Debris/Snd/WoodChips/gib_1.wav",
	"SFX/Debris/Snd/WoodChips/gib_2.wav",
	"SFX/Debris/Snd/WoodChips/gib_3.wav",
	"SFX/Debris/Snd/WoodChips/gib_4.wav",
	"SFX/Debris/Snd/WoodChips/gib_5.wav"
#endif
};

char* s_debrisPlasticBounce[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Plastic\\gib_1.wav",
	"SFX\\Debris\\Snd\\Plastic\\gib_2.wav",
	"SFX\\Debris\\Snd\\Plastic\\gib_3.wav",
	"SFX\\Debris\\Snd\\Plastic\\gib_4.wav",
	"SFX\\Debris\\Snd\\Plastic\\gib_5.wav"
#else
	"SFX/Debris/Snd/Plastic/gib_1.wav",
	"SFX/Debris/Snd/Plastic/gib_2.wav",
	"SFX/Debris/Snd/Plastic/gib_3.wav",
	"SFX/Debris/Snd/Plastic/gib_4.wav",
	"SFX/Debris/Snd/Plastic/gib_5.wav"
#endif
};

char* s_debrisGlassBounceBig[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\BigGlass\\gib_1.wav",
	"SFX\\Debris\\Snd\\BigGlass\\gib_2.wav",
	"SFX\\Debris\\Snd\\BigGlass\\gib_3.wav",
	"SFX\\Debris\\Snd\\BigGlass\\gib_4.wav",
	"SFX\\Debris\\Snd\\BigGlass\\gib_5.wav"
#else
	"SFX/Debris/Snd/BigGlass/gib_1.wav",
	"SFX/Debris/Snd/BigGlass/gib_2.wav",
	"SFX/Debris/Snd/BigGlass/gib_3.wav",
	"SFX/Debris/Snd/BigGlass/gib_4.wav",
	"SFX/Debris/Snd/BigGlass/gib_5.wav"
#endif
};

char* s_debrisGlassBounceSmall[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\SmallGlass\\gib_1.wav",
	"SFX\\Debris\\Snd\\SmallGlass\\gib_2.wav",
	"SFX\\Debris\\Snd\\SmallGlass\\gib_3.wav",
	"SFX\\Debris\\Snd\\SmallGlass\\gib_4.wav",
	"SFX\\Debris\\Snd\\SmallGlass\\gib_5.wav"
#else
	"SFX/Debris/Snd/SmallGlass/gib_1.wav",
	"SFX/Debris/Snd/SmallGlass/gib_2.wav",
	"SFX/Debris/Snd/SmallGlass/gib_3.wav",
	"SFX/Debris/Snd/SmallGlass/gib_4.wav",
	"SFX/Debris/Snd/SmallGlass/gib_5.wav"
#endif
};

char* s_debrisStoneBounceBig[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\BigStone\\gib_1.wav",
	"SFX\\Debris\\Snd\\BigStone\\gib_2.wav",
	"SFX\\Debris\\Snd\\BigStone\\gib_3.wav",
	"SFX\\Debris\\Snd\\BigStone\\gib_4.wav",
	"SFX\\Debris\\Snd\\BigStone\\gib_5.wav"
#else
	"SFX/Debris/Snd/BigStone/gib_1.wav",
	"SFX/Debris/Snd/BigStone/gib_2.wav",
	"SFX/Debris/Snd/BigStone/gib_3.wav",
	"SFX/Debris/Snd/BigStone/gib_4.wav",
	"SFX/Debris/Snd/BigStone/gib_5.wav"
#endif
};

char* s_debrisStoneBounceSmall[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\SmallStone\\gib_1.wav",
	"SFX\\Debris\\Snd\\SmallStone\\gib_2.wav",
	"SFX\\Debris\\Snd\\SmallStone\\gib_3.wav",
	"SFX\\Debris\\Snd\\SmallStone\\gib_4.wav",
	"SFX\\Debris\\Snd\\SmallStone\\gib_5.wav"
#else
	"SFX/Debris/Snd/SmallStone/gib_1.wav",
	"SFX/Debris/Snd/SmallStone/gib_2.wav",
	"SFX/Debris/Snd/SmallStone/gib_3.wav",
	"SFX/Debris/Snd/SmallStone/gib_4.wav",
	"SFX/Debris/Snd/SmallStone/gib_5.wav"
#endif
};

char* s_debrisMetalBounceBig[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\BigMetal\\gib_1.wav",
	"SFX\\Debris\\Snd\\BigMetal\\gib_2.wav",
	"SFX\\Debris\\Snd\\BigMetal\\gib_3.wav",
	"SFX\\Debris\\Snd\\BigMetal\\gib_4.wav",
	"SFX\\Debris\\Snd\\BigMetal\\gib_5.wav"
#else
	"SFX/Debris/Snd/BigMetal/gib_1.wav",
	"SFX/Debris/Snd/BigMetal/gib_2.wav",
	"SFX/Debris/Snd/BigMetal/gib_3.wav",
	"SFX/Debris/Snd/BigMetal/gib_4.wav",
	"SFX/Debris/Snd/BigMetal/gib_5.wav"
#endif
};

char* s_debrisMetalBounceSmall[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\SmallMetal\\gib_1.wav",
	"SFX\\Debris\\Snd\\SmallMetal\\gib_2.wav",
	"SFX\\Debris\\Snd\\SmallMetal\\gib_3.wav",
	"SFX\\Debris\\Snd\\SmallMetal\\gib_4.wav",
	"SFX\\Debris\\Snd\\SmallMetal\\gib_5.wav"
#else
	"SFX/Debris/Snd/SmallMetal/gib_1.wav",
	"SFX/Debris/Snd/SmallMetal/gib_2.wav",
	"SFX/Debris/Snd/SmallMetal/gib_3.wav",
	"SFX/Debris/Snd/SmallMetal/gib_4.wav",
	"SFX/Debris/Snd/SmallMetal/gib_5.wav"
#endif
};

char* s_debrisCarPartBounce[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Car\\gib_1.wav",
	"SFX\\Debris\\Snd\\Car\\gib_2.wav",
	"SFX\\Debris\\Snd\\Car\\gib_3.wav",
	"SFX\\Debris\\Snd\\Car\\gib_4.wav",
	"SFX\\Debris\\Snd\\Car\\gib_5.wav"
#else
	"SFX/Debris/Snd/Car/gib_1.wav",
	"SFX/Debris/Snd/Car/gib_2.wav",
	"SFX/Debris/Snd/Car/gib_3.wav",
	"SFX/Debris/Snd/Car/gib_4.wav",
	"SFX/Debris/Snd/Car/gib_5.wav"
#endif
};

char* s_debrisVehiclePartBounce[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Vehicle\\gib_1.wav",
	"SFX\\Debris\\Snd\\Vehicle\\gib_2.wav",
	"SFX\\Debris\\Snd\\Vehicle\\gib_3.wav",
	"SFX\\Debris\\Snd\\Vehicle\\gib_4.wav",
	"SFX\\Debris\\Snd\\Vehicle\\gib_5.wav"
#else
	"SFX/Debris/Snd/Vehicle/gib_1.wav",
	"SFX/Debris/Snd/Vehicle/gib_2.wav",
	"SFX/Debris/Snd/Vehicle/gib_3.wav",
	"SFX/Debris/Snd/Vehicle/gib_4.wav",
	"SFX/Debris/Snd/Vehicle/gib_5.wav"
#endif
};

char* s_debrisHumanBounce[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Human\\gib_1.wav",
	"SFX\\Debris\\Snd\\Human\\gib_2.wav",
	"SFX\\Debris\\Snd\\Human\\gib_3.wav",
	"SFX\\Debris\\Snd\\Human\\gib_4.wav",
	"SFX\\Debris\\Snd\\Human\\gib_5.wav"
#else
	"SFX/Debris/Snd/Human/gib_1.wav",
	"SFX/Debris/Snd/Human/gib_2.wav",
	"SFX/Debris/Snd/Human/gib_3.wav",
	"SFX/Debris/Snd/Human/gib_4.wav",
	"SFX/Debris/Snd/Human/gib_5.wav"
#endif
};

char** s_pDebrisBounceSounds[] =
{
	s_debrisStoneBounceBig,
	s_debrisStoneBounceBig,
	s_debrisBoardBounce,
	s_debrisBranchBounceBig,
	s_debrisBranchBounceSmall,
	s_debrisWoodChipBounce,
	s_debrisPlasticBounce,
	s_debrisGlassBounceBig,
	s_debrisGlassBounceSmall,
	DNULL,
	s_debrisStoneBounceBig,
	s_debrisStoneBounceSmall,
	s_debrisMetalBounceBig,
	s_debrisMetalBounceSmall,
	s_debrisCarPartBounce,
	s_debrisVehiclePartBounce,
	s_debrisHumanBounce
};



// Debris explosion sound related sheyot...

char* s_debrisBoardExplode[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Board\\Explode_1.wav",
	"SFX\\Debris\\Snd\\Board\\Explode_2.wav",
#else
	"SFX/Debris/Snd/Board/Explode_1.wav",
	"SFX/Debris/Snd/Board/Explode_2.wav",
#endif
};

char* s_debrisBranchExplodeBig[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\BigBranches\\Explode_1.wav",
	"SFX\\Debris\\Snd\\BigBranches\\Explode_2.wav"
#else
	"SFX/Debris/Snd/BigBranches/Explode_1.wav",
	"SFX/Debris/Snd/BigBranches/Explode_2.wav"
#endif
};

char* s_debrisBranchExplodeSmall[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\SmallBranches\\Explode_1.wav",
	"SFX\\Debris\\Snd\\SmallBranches\\Explode_2.wav"
#else
	"SFX/Debris/Snd/SmallBranches/Explode_1.wav",
	"SFX/Debris/Snd/SmallBranches/Explode_2.wav"
#endif
};

char* s_debrisWoodChipExplode[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\WoodChips\\Explode_1.wav",
	"SFX\\Debris\\Snd\\WoodChips\\Explode_2.wav",
#else
	"SFX/Debris/Snd/WoodChips/Explode_1.wav",
	"SFX/Debris/Snd/WoodChips/Explode_2.wav",
#endif
};

char* s_debrisPlasticExplode[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Plastic\\Explode_1.wav",
	"SFX\\Debris\\Snd\\Plastic\\Explode_2.wav"
#else
	"SFX/Debris/Snd/Plastic/Explode_1.wav",
	"SFX/Debris/Snd/Plastic/Explode_2.wav"
#endif
};

char* s_debrisGlassExplodeBig[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\BigGlass\\Explode_1.wav",
	"SFX\\Debris\\Snd\\BigGlass\\Explode_2.wav"
#else
	"SFX/Debris/Snd/BigGlass/Explode_1.wav",
	"SFX/Debris/Snd/BigGlass/Explode_2.wav"
#endif
};

char* s_debrisGlassExplodeSmall[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\SmallGlass\\Explode_1.wav",
	"SFX\\Debris\\Snd\\SmallGlass\\Explode_2.wav"
#else
	"SFX/Debris/Snd/SmallGlass/Explode_1.wav",
	"SFX/Debris/Snd/SmallGlass/Explode_2.wav"
#endif
};

char* s_debrisStoneExplodeBig[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\BigStone\\Explode_1.wav",
	"SFX\\Debris\\Snd\\BigStone\\Explode_2.wav"
#else
	"SFX/Debris/Snd/BigStone/Explode_1.wav",
	"SFX/Debris/Snd/BigStone/Explode_2.wav"
#endif
};

char* s_debrisStoneExplodeSmall[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\SmallStone\\Explode_1.wav",
	"SFX\\Debris\\Snd\\SmallStone\\Explode_2.wav"
#else
	"SFX/Debris/Snd/SmallStone/Explode_1.wav",
	"SFX/Debris/Snd/SmallStone/Explode_2.wav"
#endif
};

char* s_debrisMetalExplodeBig[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\BigMetal\\Explode_1.wav",
	"SFX\\Debris\\Snd\\BigMetal\\Explode_2.wav"
#else
	"SFX/Debris/Snd/BigMetal/Explode_1.wav",
	"SFX/Debris/Snd/BigMetal/Explode_2.wav"
#endif
};

char* s_debrisMetalExplodeSmall[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\SmallMetal\\Explode_1.wav",
	"SFX\\Debris\\Snd\\SmallMetal\\Explode_2.wav",
#else
	"SFX/Debris/Snd/SmallMetal/Explode_1.wav",
	"SFX/Debris/Snd/SmallMetal/Explode_2.wav",
#endif
};

char* s_debrisCarPartExplode[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Car\\Explode_1.wav",
	"SFX\\Debris\\Snd\\Car\\Explode_2.wav"
#else
	"SFX/Debris/Snd/Car/Explode_1.wav",
	"SFX/Debris/Snd/Car/Explode_2.wav"
#endif
};

char* s_debrisVehiclePartExplode[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Vehicle\\Explode_1.wav",
	"SFX\\Debris\\Snd\\Vehicle\\Explode_2.wav"
#else
	"SFX/Debris/Snd/Vehicle/Explode_1.wav",
	"SFX/Debris/Snd/Vehicle/Explode_2.wav"
#endif
};

char* s_debrisHumanExplode[] =
{
#ifdef _WIN32
	"SFX\\Debris\\Snd\\Human\\Explode_1.wav",
	"SFX\\Debris\\Snd\\Human\\Explode_2.wav",
	"SFX\\Debris\\Snd\\Human\\Explode_3.wav"
#else
	"SFX/Debris/Snd/Human/Explode_1.wav",
	"SFX/Debris/Snd/Human/Explode_2.wav",
	"SFX/Debris/Snd/Human/Explode_3.wav"
#endif
};

char** s_pDebrisExplodeSounds[] =
{
	DNULL,
	DNULL,
	s_debrisBoardExplode,
	s_debrisBranchExplodeBig,
	s_debrisBranchExplodeSmall,
	s_debrisWoodChipExplode,
	s_debrisPlasticExplode,
	s_debrisGlassExplodeBig,
	s_debrisGlassExplodeSmall,
	DNULL,
	s_debrisStoneExplodeBig,
	s_debrisStoneExplodeSmall,
	s_debrisMetalExplodeBig,
	s_debrisMetalExplodeSmall,
	s_debrisCarPartExplode,
	s_debrisVehiclePartExplode,
	s_debrisHumanExplode
};

DFLOAT s_fDebrisScale[] =
{
	1.5f,	// Generic
	2.0f,	// Generic flat
	0.13f,	// Board
	0.2f,	// big branch
	0.1f,	// small branch
	0.05f,	// woodchip
	0.5f,	// plastic
	0.2f,	// big glass
	0.1f,   // small glass 
	0.2f,	// feather 
	0.2f,	// big stone
	0.1f,	// small stone
	0.15f,	// big metal
	0.05f,	// small metal
	0.02f,	// car
	0.015f,	// vehicle
	0.25f	// human
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisSurfaceType
//
//	PURPOSE:	Return the surface type of the debris
//
// ----------------------------------------------------------------------- //

SurfaceType GetDebrisSurfaceType(DebrisType eType)
{
	SurfaceType eSurfType = ST_UNKNOWN;
/*
	switch (eType)
	{
		case DBT_BOARDS:
		case DBT_BRANCHES_BIG:
			eSurfType = ST_WOOD;
		break;

		case DBT_BRANCHES_SMALL:
		case DBT_WOODCHIPS:
			eSurfType = ST_LIGHT_WOOD;
		break;

		case DBT_PLASTIC:
			eSurfType = ST_PLASTIC;
		break;

		case DBT_GLASS_BIG:
		case DBT_GLASS_SMALL:
			eSurfType = ST_GLASS;
		break;

		case DBT_FEATHERS:
			eSurfType = ST_CLOTH;
		break;

		case DBT_STONE_BIG:
			eSurfType = ST_STONE_HEAVY;
		break;

		case DBT_STONE_SMALL:
			eSurfType = ST_STONE_LIGHT;
		break;

		case DBT_METAL_BIG:
			eSurfType = ST_METAL;
		break;

		case DBT_METAL_SMALL:
			eSurfType = ST_METAL_LIGHT;
		break;

		case DBT_CAR_PARTS:
			eSurfType = ST_METAL_HOLLOW_HEAVY;
		break;

		case DBT_HUMAN_PARTS:
			eSurfType = ST_FLESH;
		break;

		case DBT_GENERIC:
		case DBT_GENERIC_FLAT:
		default :
			eSurfType = ST_STONE;
		break;
	}
*/
	return eSurfType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisType
//
//	PURPOSE:	Return the debris type of the surface
//
// ----------------------------------------------------------------------- //

DebrisType GetDebrisType(SurfaceType eType)
{
	DebrisType eDebrisType = DBT_GENERIC;
/*
	switch (eType)
	{
		case ST_STONE_HEAVY:
			eDebrisType = DBT_STONE_BIG;
		break;

		case ST_STONE:
		case ST_STONE_LIGHT:
			eDebrisType = DBT_STONE_SMALL;
		break;

		case ST_METAL:
		case ST_METAL_HEAVY:
		case ST_METAL_HOLLOW_HEAVY:
			eDebrisType = DBT_METAL_BIG;
		break;

		case ST_METAL_LIGHT:
		case ST_METAL_HOLLOW:
		case ST_METAL_HOLLOW_LIGHT:
			eDebrisType = DBT_METAL_SMALL;
		break;

		case ST_DENSE_WOOD:
			eDebrisType = DBT_BOARDS;
		break;

		case ST_WOOD:
		case ST_LIGHT_WOOD:
			eDebrisType = DBT_WOODCHIPS;
		break;

		case ST_PLASTIC:
			eDebrisType = DBT_PLASTIC;
		break;

		case ST_GLASS:
			eDebrisType = DBT_GLASS_SMALL;
		break;
		
		case ST_CLOTH:
			eDebrisType = DBT_FEATHERS;
		break;

		case ST_CHAINFENCE:
		case ST_PLASTIC_HEAVY:
		case ST_PLASTIC_LIGHT:
			eDebrisType = DBT_GENERIC_FLAT;
		break;
		
		case ST_FLESH:
			eDebrisType = DBT_HUMAN_PARTS;
		break;

		case ST_TERRAIN:
		case ST_SNOW:
		case ST_LIQUID:
		case ST_UNKNOWN:
		case ST_SKY:
		default : break;
	}
*/
	return eDebrisType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetVectorDebrisType
//
//	PURPOSE:	Return the debris type of the surface (for vector weapons)
//
// ----------------------------------------------------------------------- //

DebrisType GetVectorDebrisType(SurfaceType eType)
{
	DebrisType eDebrisType = DBT_GENERIC;
/*
	switch (eType)
	{
		case ST_STONE_HEAVY:
		case ST_STONE:
		case ST_STONE_LIGHT:
			eDebrisType = DBT_STONE_SMALL;
		break;

		case ST_METAL:
		case ST_METAL_HEAVY:
		case ST_METAL_HOLLOW_HEAVY:
		case ST_METAL_LIGHT:
		case ST_METAL_HOLLOW:
		case ST_METAL_HOLLOW_LIGHT:
			eDebrisType = DBT_METAL_SMALL;
		break;

		case ST_DENSE_WOOD:
		case ST_WOOD:
		case ST_LIGHT_WOOD:
			eDebrisType = DBT_WOODCHIPS;
		break;

		case ST_GLASS:
			eDebrisType = DBT_GLASS_SMALL;
		break;
		
		case ST_CLOTH:
			eDebrisType = DBT_FEATHERS;
		break;

		case ST_PLASTIC:
		case ST_CHAINFENCE:
		case ST_PLASTIC_HEAVY:
		case ST_PLASTIC_LIGHT:
			eDebrisType = DBT_GENERIC_FLAT;
		break;
		
		case ST_FLESH:
			eDebrisType = DBT_HUMAN_PARTS;
		break;

		case ST_TERRAIN:
		case ST_SNOW:
		case ST_LIQUID:
		case ST_UNKNOWN:
		case ST_SKY:
		default : break;
	}
*/
	return eDebrisType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsSmallDebris
//
//	PURPOSE:	Return  true if the debris type is small
//
// ----------------------------------------------------------------------- //

DBOOL IsSmallDebris(DebrisType eType)
{
	if (eType == DBT_GLASS_SMALL || 
		eType == DBT_STONE_SMALL ||
		eType == DBT_METAL_SMALL)
	{
		return DTRUE;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumDebrisModels
//
//	PURPOSE:	Return the number of debris models associated with a 
//				particular id
//
// ----------------------------------------------------------------------- //

int GetNumDebrisModels(DebrisType eType)
{

	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		switch (eType)
		{
			case DBT_GENERIC:
				return (sizeof (s_debrisGenericFilename) / sizeof (s_debrisGenericFilename[0]));
			break;

			case DBT_GENERIC_FLAT:
				return (sizeof (s_debrisGernicFlatFilename) / sizeof (s_debrisGernicFlatFilename[0]));
			break;

			case DBT_BOARDS:
				return (sizeof (s_debrisBoardFilename) / sizeof (s_debrisBoardFilename[0]));
			break;

			case DBT_BRANCHES_BIG:
			case DBT_BRANCHES_SMALL:
				return (sizeof (s_debrisBranchFilename) / sizeof (s_debrisBranchFilename[0]));
			break;

			case DBT_WOODCHIPS:
				return (sizeof (s_debrisWoodChipFilename) / sizeof (s_debrisWoodChipFilename[0]));
			break;

			case DBT_PLASTIC:
				return (sizeof (s_debrisPlasticFilename) / sizeof (s_debrisPlasticFilename[0]));
			break;

			case DBT_GLASS_BIG:
			case DBT_GLASS_SMALL:
				return (sizeof (s_debrisGlassFilename) / sizeof (s_debrisGlassFilename[0]));
			break;

			case DBT_FEATHERS:
				return (sizeof (s_debrisFeatherFilename) / sizeof (s_debrisFeatherFilename[0]));
			break;

			case DBT_STONE_BIG:
			case DBT_STONE_SMALL:
				return (sizeof (s_debrisStoneFilename) / sizeof (s_debrisStoneFilename[0]));
			break;

			case DBT_METAL_BIG:
			case DBT_METAL_SMALL:
				return (sizeof (s_debrisMetalFilename) / sizeof (s_debrisMetalFilename[0]));
			break;

			case DBT_CAR_PARTS:
				return (sizeof (s_debrisCarPartFilename) / sizeof (s_debrisCarPartFilename[0]));
			break;

			case DBT_VEHICLE_PARTS:
				return (sizeof (s_debrisVehiclePartFilename) / sizeof (s_debrisVehiclePartFilename[0]));
			break;

			case DBT_HUMAN_PARTS:
				return (sizeof (s_debrisHumanFilename) / sizeof (s_debrisHumanFilename[0]));
			break;

			default :
			break;
		}
	}

	return 0;
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisModel
//
//	PURPOSE:	Return the model associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisModel(DebrisType eType, DVector & vScale, int nIndex)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		VEC_MULSCALAR(vScale, vScale, s_fDebrisScale[eType]);

		int nNum = GetNumDebrisModels(eType);
		int i = 0;

		if (0 <= nIndex && nIndex < nNum)
		{
			i = nIndex;
		}
		else
		{
			i = GetRandom(0, nNum-1);
		}

		return s_pDebrisModels[eType][i];
	}

	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisSkin
//
//	PURPOSE:	Return the skin associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisSkin(DebrisType eType)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		return s_pDebrisSkins[eType];
	}

	return DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumDebrisBounceSounds
//
//	PURPOSE:	Return the number of debris bounce sounds associated with a 
//				particular id
//
// ----------------------------------------------------------------------- //

int GetNumDebrisBounceSounds(DebrisType eType)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		if (s_pDebrisBounceSounds[eType])
		{
			return (sizeof (s_pDebrisBounceSounds[eType]) / sizeof (s_pDebrisBounceSounds[eType][0]));
		}
	}

	return 0;
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisBounceSound
//
//	PURPOSE:	Return the bounce sound associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisBounceSound(DebrisType eType, int nIndex)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		int nNum = GetNumDebrisBounceSounds(eType);
		int i = 0;

		if (0 <= nIndex && nIndex < nNum)
		{
			i = nIndex;
		}
		else
		{
			i = GetRandom(0, nNum-1);
		}

		if (s_pDebrisBounceSounds[eType])
		{
			return s_pDebrisBounceSounds[eType][i];
		}
	}

	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumDebrisExplodeSounds
//
//	PURPOSE:	Return the number of debris explode sounds associated with a 
//				particular id
//
// ----------------------------------------------------------------------- //

int GetNumDebrisExplodeSounds(DebrisType eType)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		if (s_pDebrisExplodeSounds[eType])
		{
			return (sizeof (s_pDebrisExplodeSounds[eType]) / sizeof (s_pDebrisExplodeSounds[eType][0]));
		}
	}

	return 0;
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisExplodeSound
//
//	PURPOSE:	Return the explode sound associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisExplodeSound(DebrisType eType, int nIndex)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		int nNum = GetNumDebrisExplodeSounds(eType);
		int i = 0;

		if (0 <= nIndex && nIndex < nNum)
		{
			i = nIndex;
		}
		else
		{
			i = GetRandom(0, nNum-1);
		}

		if (s_pDebrisExplodeSounds[eType])
		{
			return s_pDebrisExplodeSounds[eType][i];
		}
	}

	return DNULL;
}
