#ifndef MENU_H
#define MENU_H

#include <stddef.h>
#include <vector>
#include "MenuInput.h"

enum ItemType {
    SUBMENU,
    FUNCTION
};

struct Menu;

struct MenuItem {
    const char *label = nullptr;
    Menu *menu = nullptr;
    ItemType type = FUNCTION;
    void (*action)() = nullptr;
};

struct Menu {
    const char *title = nullptr;
    Menu *parent = nullptr;
    size_t currentIndex = 0;
    std::vector<MenuItem> menuItems;
};

class MenuManager {
    public:
        MenuManager(Menu *mainMenu = nullptr) : currentMenu(mainMenu) {}

        Menu *currentMenu = nullptr;
        bool handleInput(MenuEvent);
};

#endif