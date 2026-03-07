/*
 * Menu - Man hinh menu chinh
 */

#include <iostream>
#include <cstdlib>
#include "Menu/Menu.h"

int ShowMainMenu(void) {
    int choice;
    system("cls");
    std::cout << "\n";
    std::cout << "  ================================\n";
    std::cout << "  |      TRO CHOI CO CARO        |\n";
    std::cout << "  ================================\n";
    std::cout << "  |  1. New Game                 |\n";
    std::cout << "  |  2. Load Game                |\n";
    std::cout << "  |  3. Thoat                    |\n";
    std::cout << "  ================================\n";
    std::cout << "  Chon (1-3): ";
    std::cin >> choice;
    return choice;
}
