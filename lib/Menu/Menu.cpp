#include "Menu.h"

bool MenuManager::handleInput(MenuEvent event) {
    if (event == NONE) return false;
    if (!currentMenu) return false;

    switch (event){
        case SELECT: {
            if (currentMenu->currentIndex >= currentMenu->menuItems.size()) return false;

            MenuItem &selectedItem = currentMenu->menuItems[currentMenu->currentIndex];
            if (selectedItem.type == SUBMENU && selectedItem.menu) {
                selectedItem.menu->parent = currentMenu;
                currentMenu = selectedItem.menu;
                return true;
            }
            if (selectedItem.type == FUNCTION && selectedItem.action) {
                selectedItem.action();
                return true;
            }
            return false;
        }

        case BACK:
            if (!currentMenu->parent) return false;
            currentMenu = currentMenu->parent;
            return true;

        case NEXT:
        case PREVIOUS:
            if (currentMenu->menuItems.empty()) return false;
            if (currentMenu->currentIndex >= currentMenu->menuItems.size()) {
                currentMenu->currentIndex = 0;
            }

            if (event == NEXT) {
                currentMenu->currentIndex = (currentMenu->currentIndex + 1) % currentMenu->menuItems.size();
            } else if (currentMenu->currentIndex == 0) {
                currentMenu->currentIndex = currentMenu->menuItems.size() - 1;
            } else {
                --currentMenu->currentIndex;
            }
            break;

        default:
            return false;
    }

    return true;
}
