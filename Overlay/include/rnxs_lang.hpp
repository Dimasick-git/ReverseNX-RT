#pragma once
// ReverseNX-RT (Ryazhenka) overlay-local i18n.
//
// All user-visible strings used by main.cpp live as extern globals here
// so they can be swapped at startup based on system language. Loaded
// JSON files override the English defaults compiled into rnxs_lang.cpp.
//
// JSON path on SD: /config/ReverseNX-RT/lang/<code>.json
//   where <code> is one of: en, ru.
//
// Falls back silently to compiled English defaults if a file is missing
// or malformed — the overlay never crashes due to a localisation issue.

#include <string>

namespace rnxs {

    // Title and subtitles
    extern std::string OVERLAY_TITLE;
    extern std::string SUBTITLE_DOCKED_RES;
    extern std::string SUBTITLE_HANDHELD_RES;
    extern std::string RES_DEFAULT;

    // Status messages (drawn into the CustomDrawer)
    extern std::string ERR_SALTYNX_NOT_WORKING;
    extern std::string ERR_GAME_CLOSED;
    extern std::string ERR_GAME_NOT_RUNNING;
    extern std::string INFO_GAME_RUNNING;
    extern std::string ERR_PLUGIN_NOT_RUNNING;
    extern std::string INFO_PLUGIN_RUNNING;
    extern std::string ERR_NO_MODE_CHECKED;
    extern std::string INFO_DDR_NOT_CHECKED_LINE1;
    extern std::string INFO_DDR_NOT_CHECKED_LINE2;

    // List items
    extern std::string ITEM_CHANGE_SYSTEM;
    extern std::string ITEM_CHANGE_MODE;
    extern std::string ITEM_CHANGE_HANDHELD_DDR;
    extern std::string ITEM_CHANGE_DOCKED_DDR;
    extern std::string ITEM_SAVE;

    // Save feedback
    extern std::string MSG_SAVED_OK;
    extern std::string MSG_SAVED_FAIL;

    // Status line composers — prefix + value built at runtime in update()
    extern std::string STATUS_SYS_YES;            // "Controlled by system: Yes"
    extern std::string STATUS_SYS_NO;             // "Controlled by system: No"
    extern std::string MODE_DOCKED;               // "Mode: Docked"
    extern std::string MODE_HANDHELD;             // "Mode: Handheld"
    extern std::string MODE_FAKE_DOCKED;          // "Mode: Fake Docked"
    extern std::string MODE_FAKE_HANDHELD;        // "Mode: Fake Handheld"
    extern std::string HANDHELD_DDR_DEFAULT;      // "Handheld DDR: Default"
    extern std::string HANDHELD_DDR_PREFIX;       // "Handheld DDR: "      → printf "%s%dx%d"
    extern std::string DOCKED_DDR_DEFAULT;        // "Docked DDR: Default"
    extern std::string DOCKED_DDR_PREFIX;         // "Docked DDR: "

    // Call once at GUI construction. langCode is a two-letter ISO code
    // ("en", "ru"). If unrecognised or the file is missing, English
    // defaults remain in effect.
    void loadLanguage(const std::string &langCode);

} // namespace rnxs
