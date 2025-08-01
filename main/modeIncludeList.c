//==============================================================================
// Includes
//==============================================================================

#include "modeIncludeList.h"

//==============================================================================
// Variables
//==============================================================================

/*
 Quickly regenerate with:
   grep -hirE '^extern swadgeMode_t (.*);' main/modes/ | awk '{print $3}' \
     | sed -E 's/(.*);/\&\1,/g' | grep -v quickSettings | sort
*/

/// @brief Add swadgeMode_t pointers to this struct to include them in emulator and main menu
swadgeMode_t* const allSwadgeModes[] = {
    &accelTestMode,   &bouncyMode,     &colorchordMode,     &cosCrunchMode, &danceMode,      &danceNetworkMode,
    &factoryTestMode, &gamepadMode,    &introMode,          &jukeboxMode,   &keebTestMode,   &mainMenuMode,
    &modeCredits,     &modeDiceRoller, &modePicross,        &nameTestMode,  &roboRunnerMode, &sequencerMode,
    &swadgedokuMode,  &swadgeItMode,   &swadgePassTestMode, &synthMode,     &tCaseMode,      &touchTestMode,
    &trophyTestMode,  &tunernomeMode,
};

//==============================================================================
// Functions
//==============================================================================

int modeListGetCount()
{
    return ARRAY_SIZE(allSwadgeModes);
}

void modeListSetMenu(menu_t* menu)
{
    // Games sub menu
    menu = startSubMenu(menu, "Games");
    addSingleItemToMenu(menu, cosCrunchMode.modeName);
    addSingleItemToMenu(menu, swadgeItMode.modeName);
    addSingleItemToMenu(menu, swadgedokuMode.modeName);
    addSingleItemToMenu(menu, danceNetworkMode.modeName);
    addSingleItemToMenu(menu, roboRunnerMode.modeName);
    addSingleItemToMenu(menu, modePicross.modeName);
    menu = endSubMenu(menu);

    // Music sub menu
    menu = startSubMenu(menu, "Music");
    addSingleItemToMenu(menu, sequencerMode.modeName);
    addSingleItemToMenu(menu, colorchordMode.modeName);
    addSingleItemToMenu(menu, tunernomeMode.modeName);
    addSingleItemToMenu(menu, jukeboxMode.modeName);
    addSingleItemToMenu(menu, synthMode.modeName);
    menu = endSubMenu(menu);

    // Utilities sub menu
    menu = startSubMenu(menu, "Utilities");
    addSingleItemToMenu(menu, gamepadMode.modeName);
    addSingleItemToMenu(menu, danceMode.modeName);
    addSingleItemToMenu(menu, introMode.modeName);
    addSingleItemToMenu(menu, modeDiceRoller.modeName);
    addSingleItemToMenu(menu, bouncyMode.modeName);
    menu = endSubMenu(menu);

    // Trophy Case
    addSingleItemToMenu(menu, tCaseMode.modeName);

    // Credits
    addSingleItemToMenu(menu, modeCredits.modeName);
}

int32_t getModeIdx(const swadgeMode_t* mode)
{
    for (uint32_t idx = 0; idx < ARRAY_SIZE(allSwadgeModes); idx++)
    {
        if (allSwadgeModes[idx] == mode)
        {
            return idx;
        }
    }
    return -1;
}