#include "App.h"

#include <Arduino.h>

#include <pins.h>

App::App(Emulator &emulator, Chip8Display &display, Chip8Storage &storage)
    : emulator(emulator),
      display(display),
      storage(storage),
      menuInput(ENCODER_A, ENCODER_B, ENCODER_BUTTON, BACK_BUTTON),
      menuManager(&mainMenu) {
    accelKeyMapping.up = 0x2;
    accelKeyMapping.down = 0x8;
    accelKeyMapping.left = 0x4;
    accelKeyMapping.right = 0x6;
    accelKeyMapping.shake = 0x5;
}

bool App::init() {
    if (!emulator.init()) return false;

    if (!emulator.setAccelKeyMapping(accelKeyMapping)) {
        Serial.println("Could not configure accelerometer key mapping.");
        return false;
    }
    if (!emulator.setTiltSensitivity(tiltSensitivity) ||
        !emulator.setShakeSensitivity(shakeSensitivity)) {
        Serial.println("Could not configure accelerometer sensitivity.");
        return false;
    }
    emulator.setAccelerometerEnabled(accelerometerEnabled);

    if (!dashboard.start()){
        Serial.println("Could not start dashboard.");
    }

    menuInput.init();
    menuManager.currentMenu = &mainMenu;

    gamesMenu.menuItems.clear();
    demosMenu.menuItems.clear();
    testsMenu.menuItems.clear();
    settingsMenu.menuItems.clear();

    gamePaths.clear();
    gameLabels.clear();
    demoPaths.clear();
    demoLabels.clear();
    testPaths.clear();
    testLabels.clear();

    mainMenu.title = "CHIP-8";
    mainMenu.currentIndex = 0;

    gamesMenu.title = "Games";
    gamesMenu.parent = &mainMenu;
    gamesMenu.currentIndex = 0;

    demosMenu.title = "Demos";
    demosMenu.parent = &mainMenu;
    demosMenu.currentIndex = 0;

    testsMenu.title = "Tests";
    testsMenu.parent = &mainMenu;
    testsMenu.currentIndex = 0;

    settingsMenu.title = "Settings";
    settingsMenu.parent = &mainMenu;
    settingsMenu.currentIndex = 0;
    settingsLabels.clear();
    settingsLabels.reserve(9);
    for (size_t index = 0; index < 9; ++index) {
        settingsLabels.push_back("");
        MenuItem settingItem;
        settingItem.type = FUNCTION;
        settingsMenu.menuItems.push_back(settingItem);
    }
    refreshSettingsMenu();

    mainMenu.menuItems.clear();
    const char *menuLabels[] = {
        "Games",
        "Demos",
        "Tests",
        "Settings"
    };
    Menu *submenus[] = {&gamesMenu, &demosMenu, &testsMenu, &settingsMenu};
    for (size_t index = 0; index < 4; ++index) {
        MenuItem item;
        item.label = menuLabels[index];
        item.menu = submenus[index];
        item.type = SUBMENU;
        mainMenu.menuItems.push_back(item);
    }

    if (!storage.init()) {
        Serial.println("SD card and flash filesystem are unavailable.");
    }

    populateRomMenu(gamesMenu, "/games", gamesSource, gamePaths, gameLabels);
    populateRomMenu(demosMenu, "/demos", FLASH_FILESYSTEM, demoPaths, demoLabels);
    populateRomMenu(testsMenu, "/test", FLASH_FILESYSTEM, testPaths, testLabels);

    initialized = true;
    display.drawStartScreen();
    display.drawMenu(mainMenu);

    lastDashUpdate = millis();

    return true;
}

void App::loop() {
    if (!initialized) return;

    MenuEvent event = menuInput.getInputEvent();
    if (playing) {
        dashboard.loop();

        if (millis()-lastDashUpdate >= DASH_UPDATE_INTERVAL){
            lastDashUpdate = millis();
            dashboard.sendState(emulator.getState());
        }
        if (event == SELECT) {
            if (!emulator.start(currentRomPath.c_str(), selectedSource)) {
                Serial.print("Could not restart game: ");
                Serial.println(currentRomPath);
                emulator.stop();
                playing = false;
                menuManager.currentMenu = &mainMenu;
                display.drawMenu(mainMenu);
                emulator.buzzerBeep();
            }
        } else if (event == BACK) {
            emulator.stop();
            emulator.buzzerBeep();
            playing = false;
            menuManager.currentMenu = &mainMenu;
            display.drawMenu(mainMenu);
        } else {
            emulator.loop();
        }
        return;
    }

    emulator.updateAccelerometer();

    if (event == NONE) {
        delay(1);
        return;
    }

    if (event == SELECT && menuManager.currentMenu == &settingsMenu) {
        if (settingsMenu.currentIndex == 0) {
            accelerometerEnabled = !accelerometerEnabled;
            emulator.setAccelerometerEnabled(accelerometerEnabled);

        } else if (settingsMenu.currentIndex <= 6) {
            cycleSelectedKeyMap();

        } else cycleSelectedSensitivity();

        refreshSettingsMenu();
        display.drawMenu(settingsMenu);
        emulator.buzzerBeep();
        return;
    }
    if (event == SELECT &&
        (menuManager.currentMenu == &gamesMenu ||
         menuManager.currentMenu == &demosMenu ||
         menuManager.currentMenu == &testsMenu)) {
        if (!runSelectedGame()) {
            display.drawMenu(*menuManager.currentMenu);
            emulator.buzzerBeep();
        }
        return;
    }

    if (menuManager.handleInput(event) && menuManager.currentMenu) {
        display.drawMenu(*menuManager.currentMenu);
        if (event == SELECT || event == BACK) {
            emulator.buzzerBeep();
        }
    }
}

void App::refreshSettingsMenu() {
    static char *directionNames[] = {
        "Center", "Up", "Down", "Left", "Right", "Shake"
    };
    int8_t keyMappings[] = {
        accelKeyMapping.center,
        accelKeyMapping.up,
        accelKeyMapping.down,
        accelKeyMapping.left,
        accelKeyMapping.right,
        accelKeyMapping.shake
    };
    static char keyHexDigits[] = "0123456789ABCDEF";

    settingsLabels[0] = String("Accelerometer: ") + (accelerometerEnabled ? "ON" : "OFF");

    for (size_t mappingIndex = 0; mappingIndex < 6; ++mappingIndex) {
        String settingLabel = directionNames[mappingIndex];
        settingLabel += ": ";
        settingLabel += (keyMappings[mappingIndex] < 0)
            ? "-"
            : String(keyHexDigits[(uint8_t)keyMappings[mappingIndex]]);
        settingsLabels[mappingIndex + 1] = settingLabel;
    }

    settingsLabels[7] = String("Tilt sensitivity: ") +
        (tiltSensitivity == LESS_SENSITIVE ? "Low" :
         tiltSensitivity == MORE_SENSITIVE ? "High" : "Normal");
    settingsLabels[8] = String("Shake sensitivity: ") +
        (shakeSensitivity == LESS_SENSITIVE ? "Low" :
         shakeSensitivity == MORE_SENSITIVE ? "High" : "Normal");

    for (size_t index = 0; index < settingsMenu.menuItems.size(); ++index) {
        settingsMenu.menuItems[index].label = settingsLabels[index].c_str();
    }
}

void App::cycleSelectedKeyMap() {
    AccelKeyMapping updatedMapping = accelKeyMapping;
    int8_t *mappingValue = nullptr;
    switch (settingsMenu.currentIndex) {
        case 1: mappingValue = &updatedMapping.center; break;
        case 2: mappingValue = &updatedMapping.up; break;
        case 3: mappingValue = &updatedMapping.down; break;
        case 4: mappingValue = &updatedMapping.left; break;
        case 5: mappingValue = &updatedMapping.right; break;
        case 6: mappingValue = &updatedMapping.shake; break;
        default: return;
    }

    *mappingValue = (*mappingValue < 0) ? 0 :
                    (*mappingValue == 0x0F) ? -1 : *mappingValue + 1;
    if (!emulator.setAccelKeyMapping(updatedMapping)) {
        Serial.println("Could not update accelerometer key mapping.");
        return;
    }
    accelKeyMapping = updatedMapping;
}

void App::cycleSelectedSensitivity() {
    Sensitivity &selectedSensitivity = settingsMenu.currentIndex == 7 ? tiltSensitivity : shakeSensitivity;
    Sensitivity nextSensitivity = (selectedSensitivity == LESS_SENSITIVE) ? NORMAL :
                                  (selectedSensitivity == NORMAL) ? MORE_SENSITIVE : LESS_SENSITIVE;

    bool updateSucceeded = (settingsMenu.currentIndex == 7)
        ? emulator.setTiltSensitivity(nextSensitivity)
        : emulator.setShakeSensitivity(nextSensitivity);
    
    if (!updateSucceeded) {
        Serial.println("Could not update accelerometer sensitivity.");
        return;
    }
    selectedSensitivity = nextSensitivity;
}

void App::populateRomMenu(Menu &romMenu, const char *romDirectory, RomSource romSource, std::vector<String> &romPaths, std::vector<String> &romLabels) {
    bool directoryListed = storage.listRoms(romDirectory, romPaths, romSource);

    if (!directoryListed) {
        romMenu.menuItems.clear();
        MenuItem unavailableItem;
        unavailableItem.label = (romSource != SD_CARD) ? "Flash directory unavailable" :
                                (storage.isReady(romSource) ? "No /games folder on SD" : "SD card unavailable");

        romMenu.menuItems.push_back(unavailableItem);
        Serial.println(unavailableItem.label);
        return;
    }

    romLabels.clear();
    romLabels.reserve(romPaths.size());
    romMenu.menuItems.clear();
    romMenu.menuItems.reserve(romPaths.size());

    for (String &romPath : romPaths) {
        int filenameStart = romPath.lastIndexOf('/');
        int extensionStart = romPath.lastIndexOf('.');
        String romLabel = extensionStart > filenameStart
            ? romPath.substring(filenameStart + 1, extensionStart)
            : romPath.substring(filenameStart + 1);
        romLabels.push_back(romLabel);
    }

    for (size_t romIndex = 0; romIndex < romPaths.size(); ++romIndex) {
        MenuItem romMenuItem;
        romMenuItem.label = romLabels[romIndex].c_str();
        romMenuItem.type = FUNCTION;
        romMenu.menuItems.push_back(romMenuItem);
    }

    if (romPaths.empty()) {
        MenuItem emptyItem;
        emptyItem.label = romSource == SD_CARD
            ? "No .ch8 files on SD"
            : "No .ch8 games found";
        romMenu.menuItems.push_back(emptyItem);
    }
}

bool App::runSelectedGame() {
    Menu *selectedMenu = menuManager.currentMenu;
    std::vector<String> *selectedRomPaths = nullptr;

    if (selectedMenu == &gamesMenu) {
        selectedRomPaths = &gamePaths;
        selectedSource = gamesSource;

    } else if (selectedMenu == &demosMenu) {
        selectedRomPaths = &demoPaths;
        selectedSource = FLASH_FILESYSTEM;

    } else if (selectedMenu == &testsMenu) {
        selectedRomPaths = &testPaths;
        selectedSource = FLASH_FILESYSTEM;
    }

    if (!selectedRomPaths || selectedMenu->currentIndex >= selectedRomPaths->size()) {
        Serial.println("No game is selected.");
        return false;
    }

    String &selectedRomPath = selectedRomPaths->at(selectedMenu->currentIndex);
    if (!emulator.start(selectedRomPath.c_str(), selectedSource)) {
        Serial.print("Could not load game: ");
        Serial.println(selectedRomPath);
        return false;
    }

    currentRomPath = selectedRomPath;
    playing = true;
    return true;
}
