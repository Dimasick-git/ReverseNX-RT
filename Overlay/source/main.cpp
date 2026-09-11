#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#include <exception_wrap.hpp>
#include <tesla.hpp>    // The Tesla Header
#include "SaltyNX.h"
#include "rnxs_lang.hpp"
#include <dirent.h>

bool _isDocked = false;
bool _def = true;
bool PluginRunning = false;
bool state = false;
bool closed = false;
bool check = false;
bool SaltySD = false;
bool bak = false;
bool plugin = true;
std::string saveText;
std::string dockedText;
std::string systemText;
std::string handheldDdrText;
std::string dockedDdrText;
uint64_t PID = 0;
Handle remoteSharedMemory = INVALID_HANDLE;
SharedMemory _sharedmemory = {};
bool SharedMemoryUsed = false;

enum res_mode {
	res_mode_default = 0,
	res_mode_480p = 1,
	res_mode_540p = 2,
	res_mode_630p = 3,
	res_mode_720p = 4,
	res_mode_810p = 5,
	res_mode_900p = 6,
	res_mode_1080p = 7,
	res_mode_amount = 8
};

std::pair<int, int> resolutions[] = {{0 ,0}, {854, 480}, {960, 540}, {1120, 630}, {1280, 720}, {1440, 810}, {1600, 900}, {1920, 1080}};

struct Shared {
	uint32_t MAGIC;
	bool isDocked;
	bool def;
	bool pluginActive;
	struct {
		res_mode handheld_res: 4;
		res_mode docked_res: 4;
	} NX_PACKED res;
	bool wasDDRused;
} NX_PACKED;

static_assert(sizeof(Shared) == 9);

Shared* ReverseNX_RT = nullptr;

bool writeSave() {
	if (!PluginRunning || ReverseNX_RT == nullptr) {
		return false;
	}

	uint64_t titid = 0;
	if (R_FAILED(pmdmntGetProgramId(&titid, PID))) {
		return false;
	}
	char path[128];
	DIR* dir = opendir("sdmc:/SaltySD/plugins/ReverseNX-RT/");
	if (!dir) {
		mkdir("sdmc:/SaltySD/plugins/", 0777);
		mkdir("sdmc:/SaltySD/plugins/ReverseNX-RT/", 0777);
	}
	else closedir(dir);
	snprintf(path, sizeof(path), "sdmc:/SaltySD/plugins/ReverseNX-RT/%016lX.dat", titid);
	if (_def) {
		remove(path);
		return true;
	}
	FILE* save_file = fopen(path, "wb");
	if (!save_file)
		return false;
	fprintf(save_file, "NXRT");
	uint8_t version = 2;
	fwrite(&version, 1, 1, save_file);
	fwrite(&_isDocked, 1, 1, save_file);
	uint8_t resolutionModeH = (uint8_t)(ReverseNX_RT->res.handheld_res);
	uint8_t resolutionModeD = (uint8_t)(ReverseNX_RT->res.docked_res);
	fwrite(&resolutionModeH, 1, 1, save_file);
	fwrite(&resolutionModeD, 1, 1, save_file);
	fclose(save_file);
	return true;
}

bool LoadSharedMemory() {
	if (R_FAILED(SaltySD_Connect()))
		return false;

	remoteSharedMemory = INVALID_HANDLE;
	Result rc = SaltySD_GetSharedMemoryHandle(&remoteSharedMemory);
	SaltySD_Term();
	if (R_FAILED(rc) || remoteSharedMemory == INVALID_HANDLE)
		return false;

	shmemLoadRemote(&_sharedmemory, remoteSharedMemory, 0x1000, Perm_Rw);
	rc = shmemMap(&_sharedmemory);
	if (R_SUCCEEDED(rc)) {
		SharedMemoryUsed = true;
		return true;
	}

	shmemClose(&_sharedmemory);
	remoteSharedMemory = INVALID_HANDLE;
	return false;
}

ptrdiff_t searchSharedMemoryBlock(uintptr_t base) {
	if (base == 0)
		return -1;

	ptrdiff_t search_offset = 0;
	while (search_offset <= static_cast<ptrdiff_t>(0x1000 - sizeof(Shared))) {
		uint32_t* MAGIC_shared = (uint32_t*)(base + search_offset);
		if (*MAGIC_shared == 0x5452584E) {
			return search_offset;
		}
		else search_offset += 4;
	}
	return -1;
}

bool CheckPort () {
	Handle saltysd;
	for (int i = 0; i < 67; i++) {
		if (R_SUCCEEDED(svcConnectToNamedPort(&saltysd, "InjectServ"))) {
			svcCloseHandle(saltysd);
			break;
		}
		else {
			if (i == 66) return false;
			svcSleepThread(1'000'000);
		}
	}
	for (int i = 0; i < 67; i++) {
		if (R_SUCCEEDED(svcConnectToNamedPort(&saltysd, "InjectServ"))) {
			svcCloseHandle(saltysd);
			return true;
		}
		else svcSleepThread(1'000'000);
	}
	return false;
}

class ResolutionModeMenu : public tsl::Gui {
public:
	bool _isDocked = false;
	ResolutionModeMenu (bool isDocked) {
		_isDocked = isDocked;
	}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame(rnxs::OVERLAY_TITLE, _isDocked ? rnxs::SUBTITLE_DOCKED_RES : rnxs::SUBTITLE_HANDHELD_RES);

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();

		auto *clickableListItem2 = new tsl::elm::ListItem(rnxs::RES_DEFAULT);
		clickableListItem2->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning && ReverseNX_RT != nullptr) {
				if (_isDocked) ReverseNX_RT->res.docked_res = res_mode_default;
				else ReverseNX_RT->res.handheld_res = res_mode_default;
				tsl::goBack();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem2);
		
		for (uint32_t i = 1; i < res_mode_amount; i++) {
			char Hz[] = "1920x1080";
			snprintf(Hz, sizeof(Hz), "%dx%d", resolutions[i].first, resolutions[i].second);
			auto *clickableListItem = new tsl::elm::ListItem(Hz);
			clickableListItem->setClickListener([this, i](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning && ReverseNX_RT != nullptr) {
					if (_isDocked) ReverseNX_RT->res.docked_res = (res_mode)i;
					else ReverseNX_RT->res.handheld_res = (res_mode)i;
					tsl::goBack();
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem);
		}

		frame->setContent(list);

		return frame;
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (!PluginRunning) {
			tsl::goBack();
			return true;
		}
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class GuiTest : public tsl::Gui {
public:
	GuiTest(u8 arg1, u8 arg2, bool arg3) {}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame(rnxs::OVERLAY_TITLE, APP_VERSION);

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();
		
		list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
			if (!SaltySD) {
				renderer->drawString(rnxs::ERR_SALTYNX_NOT_WORKING.c_str(), false, x, y+50, 20, renderer->a(0xF33F));
			}
			else if (!check) {
				if (closed) {
					renderer->drawString(rnxs::ERR_GAME_CLOSED.c_str(), false, x, y+20, 19, renderer->a(0xF33F));
				}
				else {
					renderer->drawString(rnxs::ERR_GAME_NOT_RUNNING.c_str(), false, x, y+20, 19, renderer->a(0xF33F));
				}
			}
			else if (!PluginRunning) {
				renderer->drawString(rnxs::INFO_GAME_RUNNING.c_str(), false, x, y+20, 20, renderer->a(0xFFFF));
				renderer->drawString(rnxs::ERR_PLUGIN_NOT_RUNNING.c_str(), false, x, y+40, 20, renderer->a(0xF33F));
			}
			else {
				renderer->drawString(rnxs::INFO_PLUGIN_RUNNING.c_str(), false, x, y+20, 20, renderer->a(0xFFFF));
				if (ReverseNX_RT == nullptr) return;
				if (!(ReverseNX_RT->pluginActive)) renderer->drawString(rnxs::ERR_NO_MODE_CHECKED.c_str(), false, x, y+40, 18, renderer->a(0xF33F));
				else {
					renderer->drawString(systemText.c_str(), false, x, y+42, 20, renderer->a(0xFFFF));
					renderer->drawString(dockedText.c_str(), false, x, y+64, 20, renderer->a(0xFFFF));
					if (!(ReverseNX_RT->def)) {
						if (ReverseNX_RT->wasDDRused) {
							renderer->drawString(handheldDdrText.c_str(), false, x, y+86, 20, renderer->a(0xFFFF));
							renderer->drawString(dockedDdrText.c_str(), false, x, y+108, 20, renderer->a(0xFFFF));
						}
						else {
							renderer->drawString(rnxs::INFO_DDR_NOT_CHECKED_LINE1.c_str(), false, x, y+86, 20, renderer->a(0xFFFF));
							renderer->drawString(rnxs::INFO_DDR_NOT_CHECKED_LINE2.c_str(), false, x, y+108, 20, renderer->a(0xFFFF));							
						}
					}
				}
				renderer->drawString(saveText.c_str(), false, x, y+130, 20, renderer->a(0xFFFF));
			}
		}), 150);

		if (PluginRunning && ReverseNX_RT != nullptr && ReverseNX_RT->pluginActive) {

			auto *clickableListItem = new tsl::elm::ListItem(rnxs::ITEM_CHANGE_SYSTEM);
			clickableListItem->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning && ReverseNX_RT != nullptr) {
					ReverseNX_RT->def = !(ReverseNX_RT->def);
					tsl::swapTo<GuiTest>(1, 2, true);
					return true;
				}

				return false;
			});

			list->addItem(clickableListItem);

			if (!(ReverseNX_RT->def)) {

				auto *clickableListItem2 = new tsl::elm::ListItem(rnxs::ITEM_CHANGE_MODE);
				clickableListItem2->setClickListener([](u64 keys) { 
					if ((keys & HidNpadButton_A) && PluginRunning && ReverseNX_RT != nullptr) {
						ReverseNX_RT->isDocked = !(ReverseNX_RT->isDocked);
						return true;
					}
					
					return false;
				});
				list->addItem(clickableListItem2);

				if (ReverseNX_RT->wasDDRused) {
					auto *clickableListItem3 = new tsl::elm::ListItem(rnxs::ITEM_CHANGE_HANDHELD_DDR);
					clickableListItem3->setClickListener([](u64 keys) { 
						if ((keys & HidNpadButton_A) && PluginRunning && ReverseNX_RT != nullptr) {
							tsl::changeTo<ResolutionModeMenu>(false);
							return true;
						}
						
						return false;
					});
					list->addItem(clickableListItem3);

					auto *clickableListItem4 = new tsl::elm::ListItem(rnxs::ITEM_CHANGE_DOCKED_DDR);
					clickableListItem4->setClickListener([](u64 keys) { 
						if ((keys & HidNpadButton_A) && PluginRunning && ReverseNX_RT != nullptr) {
							tsl::changeTo<ResolutionModeMenu>(true);
							return true;
						}
						
						return false;
					});
					list->addItem(clickableListItem4);
				}
			}

			auto *clickableListItem3 = new tsl::elm::ListItem(rnxs::ITEM_SAVE);
			clickableListItem3->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning && ReverseNX_RT != nullptr) {
					saveText = writeSave() ? rnxs::MSG_SAVED_OK : rnxs::MSG_SAVED_FAIL;
					return true;
				}
				
				return false;
			});
			list->addItem(clickableListItem3);
		}

		// Add the list to the frame for it to be drawn
		frame->setContent(list);
        
		// Return the frame to have it become the top level element of this Gui
		return frame;
	}

	// Called once every frame to update values
	virtual void update() override {
		static uint8_t i = 10;
		Result rc = pmdmntGetApplicationProcessId(&PID);
		if (R_FAILED(rc) && PluginRunning) {
			PluginRunning = false;
			ReverseNX_RT = nullptr;
			check = false;
			closed = true;
		}

		if (PluginRunning && ReverseNX_RT != nullptr) {
			if (i > 9) {
				_def = ReverseNX_RT->def;
				_isDocked = ReverseNX_RT->isDocked;
				i = 0;
				
				systemText = _def ? rnxs::STATUS_SYS_YES : rnxs::STATUS_SYS_NO;

				if (_def) {
					dockedText = _isDocked ? rnxs::MODE_DOCKED : rnxs::MODE_HANDHELD;
				}
				else {
					dockedText = _isDocked ? rnxs::MODE_FAKE_DOCKED : rnxs::MODE_FAKE_HANDHELD;
				}

				const auto formatResolution = [](res_mode mode, const std::string &fallback, const std::string &prefix) {
					const auto index = static_cast<unsigned int>(mode);
					if (index == res_mode_default || index >= res_mode_amount)
						return fallback;
					return prefix + std::to_string(resolutions[index].first) + "x" + std::to_string(resolutions[index].second);
				};

				handheldDdrText = formatResolution(ReverseNX_RT->res.handheld_res, rnxs::HANDHELD_DDR_DEFAULT, rnxs::HANDHELD_DDR_PREFIX);
				dockedDdrText = formatResolution(ReverseNX_RT->res.docked_res, rnxs::DOCKED_DDR_DEFAULT, rnxs::DOCKED_DDR_PREFIX);
			}
			else i++;
		}
	
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (keysDown & HidNpadButton_B) {
			tsl::goBack();
			return true;
		}
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class Dummy : public tsl::Gui {
public:
	Dummy(u8 arg1, u8 arg2, bool arg3) {}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		auto frame = new tsl::elm::OverlayFrame(rnxs::OVERLAY_TITLE, APP_VERSION);
		return frame;
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		tsl::swapTo<GuiTest>(0, 1, true);
		return true;   // Return true here to singal the inputs have been consumed
	}
};

class OverlayTest : public tsl::Overlay {
public:
	// libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
	virtual void initServices() override {

		// Ryazhenka: resolve system language and load lang/<code>.json
		// before any UI element is drawn. libtesla already mounted sdmc,
		// and initServices itself runs inside an active sm session.
		u64 langCode = 0;
		SetLanguage lang = SetLanguage_ENUS;
		if (R_SUCCEEDED(setInitialize())) {
			if (R_SUCCEEDED(setGetSystemLanguage(&langCode))) {
				setMakeLanguage(langCode, &lang);
			}
			setExit();
		}
		switch (lang) {
			case SetLanguage_RU: rnxs::loadLanguage("ru"); break;
			default:             rnxs::loadLanguage("en"); break;
		}

		SaltySD = CheckPort();
		if (!SaltySD) return;

		if (R_FAILED(pmdmntGetApplicationProcessId(&PID))) return;
		check = true;

		if (!LoadSharedMemory()) return;

		uintptr_t base = reinterpret_cast<uintptr_t>(shmemGetAddr(&_sharedmemory));
		ptrdiff_t rel_offset = searchSharedMemoryBlock(base);
		if (rel_offset >= 0) {
			ReverseNX_RT = reinterpret_cast<Shared*>(base + rel_offset);
			PluginRunning = true;
		}
	
	}  // Called at the start to initialize all services necessary for this Overlay
	
	virtual void exitServices() override {
		PluginRunning = false;
		ReverseNX_RT = nullptr;
		if (SharedMemoryUsed) {
			shmemClose(&_sharedmemory);
			SharedMemoryUsed = false;
			remoteSharedMemory = INVALID_HANDLE;
		}
	}  // Callet at the end to clean up all services previously initialized

	virtual void onShow() override {}    // Called before overlay wants to change from invisible to visible state
	
	virtual void onHide() override {}    // Called before overlay wants to change from visible to invisible state

	virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
		return initially<Dummy>(1, 2, true);  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
	}
};

int main(int argc, char **argv) {
    return tsl::loop<OverlayTest>(argc, argv);
}
