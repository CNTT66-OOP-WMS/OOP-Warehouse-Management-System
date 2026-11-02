#ifndef COMMAND_IMPL_H
#define COMMAND_IMPL_H

#include "Command.h"
#include "CoSoDuLieu.h"

// Lenh Huy Don Hang (Command Pattern)
class LenhHuyHoaDon : public ILenh {
private:
    std::string maHoaDon;
    std::string trangThaiCu;
    std::string trangThaiThanhToanCu;
    bool daTungHoanKho;
    std::shared_ptr<CoSoDuLieu> csdl;

public:
    LenhHuyHoaDon(const std::string& maHD, std::shared_ptr<CoSoDuLieu> db)
        : maHoaDon(maHD), daTungHoanKho(false), csdl(db) {}

    std::string layTenLenh() const override { return "HuyHoaDon"; }
    std::string layMaDoiTuong() const override { return maHoaDon; }

    bool thucHien(std::string& thongBaoLoi) override {
        auto hd = csdl->timHoaDonTheoMa(maHoaDon);
        if (!hd) {
            thongBaoLoi = "Khong tim thay hoa don " + maHoaDon;
            return false;
        }

        if (!hd->coTheHuy()) {
            thongBaoLoi = "Don hang da hoan tat hoac da bi huy truoc do!";
            return false;
        }

        trangThaiCu = hd->layTrangThaiDon();
        trangThaiThanhToanCu = hd->layTrangThaiThanhToan();

        // Kiem tra don hang da xuat kho hay chua de hoan ton kho an toan
        // Neu don chua hoan kho, thi moi hoan kho khi huy!
        bool thanhCong = csdl->capNhatTrangThaiHoaDon(maHoaDon, "DA_HUY", 
                               (trangThaiThanhToanCu == "DA_THANH_TOAN" ? "DA_HOAN_TIEN" : "CHUA_THANH_TOAN"),
                               "System-Command");
        if (thanhCong) {
            daTungHoanKho = true;
            return true;
        }
        thongBaoLoi = "Khong the cap nhat trang thai hoa don!";
        return false;
    }

    bool hoanTac(std::string& thongBaoLoi) override {
        auto hd = csdl->timHoaDonTheoMa(maHoaDon);
        if (!hd) {
            thongBaoLoi = "Khong tim thay hoa don de hoan tac!";
            return false;
        }
        if (!daTungHoanKho) {
            thongBaoLoi = "Khong co thay doi ton kho de hoan tac.";
            return false;
        }
        if (!csdl->hoanTacHuyHoaDon(maHoaDon, trangThaiCu, trangThaiThanhToanCu)) {
            thongBaoLoi = csdl->layLoiLuuGanNhat();
            if (thongBaoLoi.empty()) thongBaoLoi = "Khong the hoan tac don hang; ton kho khong du hoac ghi du lieu that bai.";
            return false;
        }
        return true;
    }
};

#endif // COMMAND_IMPL_H

