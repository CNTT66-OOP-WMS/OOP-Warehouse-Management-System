#include "QuanLyBanHang.h"
#include "Command.cpp" // De dung LenhHuyHoaDon
#include <chrono>
#include <ctime>
#include <iomanip>

static std::string layNgayGioHienTai() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool QuanLyBanHang::taoDonHangMoi(const std::string& maKH, const std::string& tenKH, const std::string& sdt,
                                  const std::string& diaChi, const std::vector<MonHangDatMua>& danhSachMon,
                                  const std::string& loaiGiamGia, const std::string& loaiThue,
                                  const std::string& phuongThucTT, const std::string& ghiChu,
                                  std::string& maHDMoi, std::string& thongBaoLoi) {
    if (danhSachMon.empty()) {
        thongBaoLoi = "Gio hang dang trong, khong the dat hang!";
        return false;
    }

    if (tenKH.empty() || sdt.empty()) {
        thongBaoLoi = "Vui long nhap day du ten nguoi nhan va so dien thoai!";
        return false;
    }

    // Khong tin cac ma uu dai do trinh duyet gui len. VIP chi ap dung cho
    // tai khoan khach hang da tich luy du diem; thue mien giam can duoc
    // xac nhan theo tung mat hang nen tam thoi khong nhan tu request.
    if (loaiGiamGia == "GIAM_VIP") {
        auto kh = std::dynamic_pointer_cast<KhachHang>(csdl->timNguoiDungTheoMa(maKH));
        if (!kh || kh->layDiemTichLuy() < 1000.0) {
            thongBaoLoi = "Khach hang chua du dieu kien VIP (can toi thieu 1000 diem).";
            return false;
        }
    } else if (loaiGiamGia != "" && loaiGiamGia != "KHONG_GIAM") {
        thongBaoLoi = "Ma giam gia khong hop le hoac khong duoc phep su dung.";
        return false;
    }
    if (loaiThue == "MIEN_THUE" || loaiThue == "VAT_5") {
        thongBaoLoi = "Khong the tu chon muc thue; thue phai duoc xac dinh tu danh muc hang hoa.";
        return false;
    }
    if (loaiThue != "" && loaiThue != "VAT_10") {
        thongBaoLoi = "Loai thue khong hop le.";
        return false;
    }

    maHDMoi = csdl->sinhMaHoaDon();
    auto hd = std::make_shared<HoaDon>(
        maHDMoi, layNgayGioHienTai(), maKH, tenKH, sdt, diaChi, phuongThucTT,
        "CHO_XAC_NHAN", (phuongThucTT == "CHUYEN_KHOAN" ? "CHUA_THANH_TOAN" : "CHUA_THANH_TOAN"),
        "Online-Web", ghiChu);

    int demCT = 1;
    for (const auto& mon : danhSachMon) {
        if (mon.soLuong <= 0) {
            thongBaoLoi = "So luong moi san pham phai lon hon 0!";
            return false;
        }
        auto sp = csdl->timSanPhamTheoMa(mon.maSP);
        if (!sp) {
            thongBaoLoi = "Khong tim thay san pham co ma " + mon.maSP;
            return false;
        }

        std::string loiKho;
        if (!sp->kiemTraXuatKho(mon.soLuong, loiKho)) {
            thongBaoLoi = "San pham " + sp->layTenSP() + ": " + loiKho;
            return false;
        }

        // Tinh don gia tai thoi diem ban
        double donGia = sp->tinhGiaBanThucTe(1.0);
        std::string maCT = "CT" + std::to_string(demCT++) + "_" + maHDMoi;
        ChiTietHoaDon ct(maCT, maHDMoi, sp->layMaSP(), sp->layTenSP(), mon.soLuong, donGia);
        hd->themChiTiet(ct);
    }

    // Ap dung Strategy giam gia & thue
    auto clGiamGia = BoChonChienLuoc::taoChienLuocGiamGia(loaiGiamGia);
    auto clThue = BoChonChienLuoc::taoChienLuocThue(loaiThue);
    hd->apDungChienLuoc(clGiamGia, clThue);

    if (!csdl->themHoaDon(hd)) {
        thongBaoLoi = "Loi ton kho hoac khong the luu hoa don!";
        return false;
    }

    return true;
}

bool QuanLyBanHang::xacNhanDonHang(const std::string& maHD, const std::string& nguoiXuLy, std::string& thongBaoLoi) {
    auto hd = csdl->timHoaDonTheoMa(maHD);
    if (!hd) {
        thongBaoLoi = "Khong tim thay don hang " + maHD;
        return false;
    }
    if (hd->layTrangThaiDon() != "CHO_XAC_NHAN") {
        thongBaoLoi = "Don hang da o trang thai: " + hd->layTrangThaiDon();
        return false;
    }

    return csdl->capNhatTrangThaiHoaDon(maHD, "DA_XAC_NHAN", "", nguoiXuLy);
}

bool QuanLyBanHang::capNhatGiaoHang(const std::string& maHD, const std::string& trangThaiMoi, std::string& thongBaoLoi) {
    auto hd = csdl->timHoaDonTheoMa(maHD);
    if (!hd) {
        thongBaoLoi = "Khong tim thay don hang " + maHD;
        return false;
    }

    std::string ttThanhToan = "";
    if (hd->layTrangThaiDon() == "DA_XAC_NHAN" && trangThaiMoi != "DANG_GIAO") {
        thongBaoLoi = "Don da xac nhan chi co the chuyen sang dang giao.";
        return false;
    }
    if (hd->layTrangThaiDon() == "DANG_GIAO" && trangThaiMoi != "DA_GIAO") {
        thongBaoLoi = "Don dang giao chi co the chuyen sang da giao.";
        return false;
    }
    if (trangThaiMoi == "DA_GIAO") {
        // COD duoc thu khi giao; chuyen khoan phai duoc ghi nhan qua thao tac thanh toan rieng.
        if (hd->layPhuongThucThanhToan() == "TIEN_MAT")
        ttThanhToan = "DA_THANH_TOAN";
    } else if (trangThaiMoi != "DANG_GIAO") {
        thongBaoLoi = "Trang thai giao hang khong hop le.";
        return false;
    }

    return csdl->capNhatTrangThaiHoaDon(maHD, trangThaiMoi, ttThanhToan, "");
}

bool QuanLyBanHang::xacNhanThanhToan(const std::string& maHD, std::string& thongBaoLoi) {
    auto hd = csdl->timHoaDonTheoMa(maHD);
    if (!hd) {
        thongBaoLoi = "Khong tim thay don hang " + maHD;
        return false;
    }
    if (hd->layPhuongThucThanhToan() != "CHUYEN_KHOAN" || hd->layTrangThaiDon() != "DA_GIAO") {
        thongBaoLoi = "Chi ghi nhan thanh toan chuyen khoan cho don da giao.";
        return false;
    }
    if (hd->layTrangThaiDon() == "DA_HUY" || hd->layTrangThaiDon() == "DA_TRA_HANG" ||
        hd->layTrangThaiThanhToan() == "DA_THANH_TOAN" || hd->layTrangThaiThanhToan() == "DA_HOAN_TIEN") {
        thongBaoLoi = "Don khong o trang thai co the ghi nhan thanh toan.";
        return false;
    }
    return csdl->capNhatTrangThaiHoaDon(maHD, "", "DA_THANH_TOAN", "");
}

bool QuanLyBanHang::huyDonHang(const std::string& maHD, std::string& thongBaoLoi) {
    auto lenhHuy = std::make_shared<LenhHuyHoaDon>(maHD, csdl);
    return quanLyLenh.thucThiLenh(lenhHuy, thongBaoLoi);
}

bool QuanLyBanHang::hoanTacHuyDon(std::string& thongBaoLoi) {
    return quanLyLenh.hoanTacLenhGanNhat(thongBaoLoi);
}

