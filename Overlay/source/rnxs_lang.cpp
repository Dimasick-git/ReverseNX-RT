#include "rnxs_lang.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cJSON.h>

namespace rnxs {

    // English defaults — preserved verbatim from upstream main.cpp so
    // a user without any lang/ JSON file gets exactly the upstream UI.
    std::string OVERLAY_TITLE               = "ReverseNX-RT";
    std::string SUBTITLE_DOCKED_RES         = "Change Docked Default Display Resolution";
    std::string SUBTITLE_HANDHELD_RES       = "Change Handheld Default Display Resolution";
    std::string RES_DEFAULT                 = "Default";

    std::string ERR_SALTYNX_NOT_WORKING     = "SaltyNX is not working!";
    std::string ERR_GAME_CLOSED             = "Game was closed! Overlay disabled!";
    std::string ERR_GAME_NOT_RUNNING        = "Game is not running! Overlay disabled!";
    std::string INFO_GAME_RUNNING           = "Game is running.";
    std::string ERR_PLUGIN_NOT_RUNNING      = "ReverseNX-RT is not running!";
    std::string INFO_PLUGIN_RUNNING         = "ReverseNX-RT is running.";
    std::string ERR_NO_MODE_CHECKED         = "Game didn't check any mode!";
    std::string INFO_DDR_NOT_CHECKED_LINE1  = "Default Display Resolution";
    std::string INFO_DDR_NOT_CHECKED_LINE2  = "was not checked!";

    std::string ITEM_CHANGE_SYSTEM          = "Change system control";
    std::string ITEM_CHANGE_MODE            = "Change mode";
    std::string ITEM_CHANGE_HANDHELD_DDR    = "Change Handheld DDR";
    std::string ITEM_CHANGE_DOCKED_DDR      = "Change Docked DDR";
    std::string ITEM_SAVE                   = "Save current settings";

    std::string MSG_SAVED_OK                = "Settings saved successfully!";
    std::string MSG_SAVED_FAIL              = "Saving settings failed!";

    std::string STATUS_SYS_YES              = "Controlled by system: Yes";
    std::string STATUS_SYS_NO               = "Controlled by system: No";
    std::string MODE_DOCKED                 = "Mode: Docked";
    std::string MODE_HANDHELD               = "Mode: Handheld";
    std::string MODE_FAKE_DOCKED            = "Mode: Fake Docked";
    std::string MODE_FAKE_HANDHELD          = "Mode: Fake Handheld";
    std::string HANDHELD_DDR_DEFAULT        = "Handheld DDR: Default";
    std::string HANDHELD_DDR_PREFIX         = "Handheld DDR: ";
    std::string DOCKED_DDR_DEFAULT          = "Docked DDR: Default";
    std::string DOCKED_DDR_PREFIX           = "Docked DDR: ";

    namespace {

        struct Entry { const char *key; std::string *dst; };
        const Entry kTable[] = {
            { "OVERLAY_TITLE",               &OVERLAY_TITLE               },
            { "SUBTITLE_DOCKED_RES",         &SUBTITLE_DOCKED_RES         },
            { "SUBTITLE_HANDHELD_RES",       &SUBTITLE_HANDHELD_RES       },
            { "RES_DEFAULT",                 &RES_DEFAULT                 },
            { "ERR_SALTYNX_NOT_WORKING",     &ERR_SALTYNX_NOT_WORKING     },
            { "ERR_GAME_CLOSED",             &ERR_GAME_CLOSED             },
            { "ERR_GAME_NOT_RUNNING",        &ERR_GAME_NOT_RUNNING        },
            { "INFO_GAME_RUNNING",           &INFO_GAME_RUNNING           },
            { "ERR_PLUGIN_NOT_RUNNING",      &ERR_PLUGIN_NOT_RUNNING      },
            { "INFO_PLUGIN_RUNNING",         &INFO_PLUGIN_RUNNING         },
            { "ERR_NO_MODE_CHECKED",         &ERR_NO_MODE_CHECKED         },
            { "INFO_DDR_NOT_CHECKED_LINE1",  &INFO_DDR_NOT_CHECKED_LINE1  },
            { "INFO_DDR_NOT_CHECKED_LINE2",  &INFO_DDR_NOT_CHECKED_LINE2  },
            { "ITEM_CHANGE_SYSTEM",          &ITEM_CHANGE_SYSTEM          },
            { "ITEM_CHANGE_MODE",            &ITEM_CHANGE_MODE            },
            { "ITEM_CHANGE_HANDHELD_DDR",    &ITEM_CHANGE_HANDHELD_DDR    },
            { "ITEM_CHANGE_DOCKED_DDR",      &ITEM_CHANGE_DOCKED_DDR      },
            { "ITEM_SAVE",                   &ITEM_SAVE                   },
            { "MSG_SAVED_OK",                &MSG_SAVED_OK                },
            { "MSG_SAVED_FAIL",              &MSG_SAVED_FAIL              },
            { "STATUS_SYS_YES",              &STATUS_SYS_YES              },
            { "STATUS_SYS_NO",               &STATUS_SYS_NO               },
            { "MODE_DOCKED",                 &MODE_DOCKED                 },
            { "MODE_HANDHELD",               &MODE_HANDHELD               },
            { "MODE_FAKE_DOCKED",            &MODE_FAKE_DOCKED            },
            { "MODE_FAKE_HANDHELD",          &MODE_FAKE_HANDHELD          },
            { "HANDHELD_DDR_DEFAULT",        &HANDHELD_DDR_DEFAULT        },
            { "HANDHELD_DDR_PREFIX",         &HANDHELD_DDR_PREFIX         },
            { "DOCKED_DDR_DEFAULT",          &DOCKED_DDR_DEFAULT          },
            { "DOCKED_DDR_PREFIX",           &DOCKED_DDR_PREFIX           },
        };

        bool loadFile(const char *path) {
            std::FILE *fp = std::fopen(path, "rb");
            if (!fp) return false;
            std::fseek(fp, 0, SEEK_END);
            long size = std::ftell(fp);
            if (size <= 0 || size > 64 * 1024) { std::fclose(fp); return false; }
            std::fseek(fp, 0, SEEK_SET);

            char *buf = static_cast<char *>(std::malloc(size + 1));
            if (!buf) { std::fclose(fp); return false; }
            size_t got = std::fread(buf, 1, size, fp);
            std::fclose(fp);
            if (got != static_cast<size_t>(size)) { std::free(buf); return false; }
            buf[size] = 0;

            cJSON *root = cJSON_ParseWithLength(buf, size);
            std::free(buf);
            if (!root) return false;

            for (const Entry &e : kTable) {
                cJSON *item = cJSON_GetObjectItem(root, e.key);
                if (item && cJSON_IsString(item) && item->valuestring && item->valuestring[0]) {
                    *e.dst = item->valuestring;
                }
            }
            cJSON_Delete(root);
            return true;
        }

    } // namespace

    void loadLanguage(const std::string &langCode) {
        if (langCode.empty()) return;
        char path[256];
        std::snprintf(path, sizeof(path),
                      "sdmc:/config/ReverseNX-RT/lang/%s.json",
                      langCode.c_str());
        loadFile(path);
    }

} // namespace rnxs
