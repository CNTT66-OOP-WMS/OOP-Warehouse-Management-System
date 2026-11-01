#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "MayChuWeb.h"

// Doc cong tu file cau hinh .env
int docCongMangTuEnv(const std::string& pathEnv = "config/.env") {
    std::ifstream file(pathEnv);
    if (!file.is_open()) {
        file.open("../config/.env");
    }
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("PORT=", 0) == 0) {
                try {
                    return std::stoi(line.substr(5));
                } catch (...) {
                    return 8080;
                }
            }
        }
    }
    return 8080;
}

int main() {
    // Thiet lap console ho tro UTF-8 tren Windows
    system("chcp 65001 > nul");

    int port = docCongMangTuEnv();

    std::cout << "================================================================" << std::endl;
    std::cout << "    HE THONG QUAN LY KHO HANG VA BAN HANG (WMS) - OOP C++      " << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << " 1. TINH DA HINH & KE THUA: SanPham -> SanPhamThuong,           " << std::endl;
    std::cout << "    SanPhamHanSuDung (FEFO/Date), SanPhamTheoCan (Kg/Hao hut)    " << std::endl;
    std::cout << " 2. DESIGN PATTERNS:                                           " << std::endl;
    std::cout << "    - Strategy Pattern: Chien luoc khuyen mai, chien luoc thue " << std::endl;
    std::cout << "    - Observer Pattern: Canh bao het hang, canh bao can date   " << std::endl;
    std::cout << "    - Command Pattern:  Huy hoa don & Hoan tac (Undo/Redo)     " << std::endl;
    std::cout << " 3. TAI KHOAN THU NGHIEM:                                      " << std::endl;
    std::cout << "    * Admin (Quan tri):        user: admin     | pass: admin123" << std::endl;
    std::cout << "    * Quan ly kho (Thu kho):   user: thukho    | pass: kho123  " << std::endl;
    std::cout << "    * Nhan vien ban hang:      user: banhang   | pass: banhang123" << std::endl;
    std::cout << "    * Khach hang mua online:   user: khachhang | pass: khach123" << std::endl;
    std::cout << "================================================================" << std::endl;

    MayChuWeb server(port, "frontend");
    if (!server.khoiDong()) {
        std::cerr << "Khong the khoi dong may chu web!" << std::endl;
        return 1;
    }

    return 0;
}

