#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// Forward declarations
class HoaDon;
class CoSoDuLieu;

// ========================================================
// DESIGN PATTERN: COMMAND PATTERN (MENH LENH)
// Xu ly Huy hoa don, Hoan tac (Undo/Redo), Hoan tien
// Dam bao kiem tra trang thai kho, tranh cong ton kho 2 lan!
// ========================================================

class ILenh {
public:
    virtual ~ILenh() {}
    virtual bool thucHien(std::string& thongBaoLoi) = 0;
    virtual bool hoanTac(std::string& thongBaoLoi) = 0;
    virtual std::string layTenLenh() const = 0;
    virtual std::string layMaDoiTuong() const = 0;
};

// Quan ly lich su goi lenh (Invoker & History)
class QuanLyLenh {
private:
    std::vector<std::shared_ptr<ILenh>> lichSuLenh;
    std::vector<std::shared_ptr<ILenh>> lichSuHoanTac;

public:
    bool thucThiLenh(std::shared_ptr<ILenh> lenh, std::string& thongBaoLoi) {
        if (!lenh) return false;
        if (lenh->thucHien(thongBaoLoi)) {
            lichSuLenh.push_back(lenh);
            lichSuHoanTac.clear(); // Reset redo stack
            return true;
        }
        return false;
    }

    bool hoanTacLenhGanNhat(std::string& thongBaoLoi) {
        if (lichSuLenh.empty()) {
            thongBaoLoi = "Khong co thao tac nao de hoan tac!";
            return false;
        }
        auto lenh = lichSuLenh.back();
        lichSuLenh.pop_back();

        if (lenh->hoanTac(thongBaoLoi)) {
            lichSuHoanTac.push_back(lenh);
            return true;
        } else {
            // Neu hoan tac that bai, giu lai lenh vao lich su
            lichSuLenh.push_back(lenh);
            return false;
        }
    }

    size_t soLuongLenhDaChay() const { return lichSuLenh.size(); }
};

#endif // COMMAND_H

