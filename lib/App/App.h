#ifndef APP_H
#define APP_H

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <Arduino.h>
#include <Chip8Display.h>
#include <Chip8Storage.h>
#include <Emulator.h>
#include <Menu.h>
#include <MenuInput.h>
#include <Dashboard.h>


class App {
    public:
        App(Emulator &emulator, Chip8Display &display, Chip8Storage &storage);
        bool init();
        void loop();

    private:
        Emulator &emulator;
        Chip8Display &display;
        Chip8Storage &storage;
        MenuInput menuInput;
        Dashboard dashboard;

        Menu mainMenu;
        Menu gamesMenu;
        Menu demosMenu;
        Menu testsMenu;
        Menu settingsMenu;
        MenuManager menuManager;

        std::vector<String> gamePaths;
        std::vector<String> gameLabels;
        std::vector<String> demoPaths;
        std::vector<String> demoLabels;
        std::vector<String> testPaths;
        std::vector<String> testLabels;
        std::vector<String> settingsLabels;
        String currentRomPath;

        AccelKeyMapping accelKeyMapping;
        Sensitivity tiltSensitivity = NORMAL;
        Sensitivity shakeSensitivity = NORMAL;
        
        RomSource gamesSource = SD_CARD;
        RomSource selectedSource = SD_CARD;
        bool accelerometerEnabled = true;
        
        bool initialized = false;
        bool playing = false;

        const uint32_t DASH_UPDATE_INTERVAL = 100;
        uint32_t lastDashUpdate;

        void populateRomMenu(Menu &romMenu, const char *romDirectory, RomSource romSource, std::vector<String> &romPaths, std::vector<String> &romLabels);
        void refreshSettingsMenu();
        void cycleSelectedKeyMap();
        void cycleSelectedSensitivity();
        bool runSelectedGame();
};

#endif
