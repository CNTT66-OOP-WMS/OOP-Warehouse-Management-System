#ifndef HOADON_H
#define HOADON_H

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include "Strategy.h"
#include "JsonUtil.h"

// Chi tiet mot dong trong hoa don
class ChiTietHoaDon {
private:
    std::string maChiTiet;
    std::string maHoaDon;
    std::string maSP;
    std::string tenSP;
    double soLuong;
    double donGia;
    double thanhTien;

public:
    ChiTietHoaDon(const std::string& maCT, const std::string& maHD, const std::string& maHang,
                  const std::string& tenHang, double sl, double gia)
        : maChiTiet(maCT), maHoaDon(maHD), maSP(maHang), tenSP(tenHang),
          soLuong(sl), donGia(gia) {
        thanhTien = soLuong * donGia;
    }

    std::string layMaChiTiet() const { return maChiTiet; }
    std::string layMaHoaDon() const { return maHoaDon; }
    std::string layMaSP() const { return maSP; }
    std::string layTenSP() const { return tenSP; }
    double laySoLuong() const { return soLuong; }
    double layDonGia() const { return donGia; }
    double layThanhTien() const { return thanhTien; }

    std::string xuatJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"maChiTiet\":\"" << jsonEscape(maChiTiet) << "\"," 
           << "\"maHoaDon\":\"" << jsonEscape(maHoaDon) << "\"," 
           << "\"maSP\":\"" << jsonEscape(maSP) << "\"," 
           << "\"tenSP\":\"" << jsonEscape(tenSP) << "\"," 
           << "\"soLuong\":" << soLuong << ","
           << "\"donGia\":" << std::fixed << std::setprecision(0) << donGia << ","
           << "\"thanhTien\":" << std::fixed << std::setprecision(0) << thanhTien
           << "}";
        return ss.str();
    }
};

// Lop HoaDon - Nghiep vu quan ly ban hang
class HoaDon {
private:
    std::string maHoaDon;
    std::string ngayTao;
    std::string maKhachHang;
    std::string tenKhachHang;
    std::string soDienThoai;
    std::string diaChiGiaoHang;

    std::vector<ChiTietHoaDon> danhSachChiTiet;

    double tongTienGoc;
    std::string loaiGiamGia;
    double tienGiamGia;
    std::string loaiThue;
    double tienThue;
    double tongTienThanhToan;

    std::string phuongThucThanhToan; // TIEN_MAT, CHUYEN_KHOAN
    std::string trangThaiDon;        // CHO_XAC_NHAN, DA_XAC_NHAN, DANG_GIAO, DA_GIAO, DA_HUY, DA_TRA_HANG
    std::string trangThaiThanhToan; // CHUA_THANH_TOAN, DA_THANH_TOAN, THAT_BAI, DA_HOAN_TIEN
    std::string nguoiXuLy;
    std::string ghiChu;
    std::int64_t thoiDiemGiaoHang = 0;
    bool daHoanKhoTraHang = false;

public:
    HoaDon(const std::string& maHD, const std::string& ngay, const std::string& maKH,
           const std::string& tenKH, const std::string& sdt, const std::string& diaChi,
           const std::string& ptThanhToan = "TIEN_MAT", const std::string& ttDon = "CHO_XAC_NHAN",
           const std::string& ttTT = "CHUA_THANH_TOAN", const std::string& ngXuLy = "Online",
           const std::string& note = "")
        : maHoaDon(maHD), ngayTao(ngay), maKhachHang(maKH), tenKhachHang(tenKH),
          soDienThoai(sdt), diaChiGiaoHang(diaChi), tongTienGoc(0.0),
          loaiGiamGia("KHONG_GIAM"), tienGiamGia(0.0),
          loaiThue("VAT_10"), tienThue(0.0), tongTienThanhToan(0.0),
          phuongThucThanhToan(ptThanhToan), trangThaiDon(ttDon),
          trangThaiThanhToan(ttTT), nguoiXuLy(ngXuLy), ghiChu(note) {}

    // Getters
    std::string layMaHoaDon() const { return maHoaDon; }
    std::string layNgayTao() const { return ngayTao; }
    std::string layMaKhachHang() const { return maKhachHang; }
    std::string layTenKhachHang() const { return tenKhachHang; }
    std::string laySoDienThoai() const { return soDienThoai; }
    std::string layDiaChiGiaoHang() const { return diaChiGiaoHang; }
    double layTongTienGoc() const { return tongTienGoc; }
    std::string layLoaiGiamGia() const { return loaiGiamGia; }
    double layTienGiamGia() const { return tienGiamGia; }
    std::string layLoaiThue() const { return loaiThue; }
    double layTienThue() const { return tienThue; }
    double layTongTienThanhToan() const { return tongTienThanhToan; }
    std::string layPhuongThucThanhToan() const { return phuongThucThanhToan; }
    std::string layTrangThaiDon() const { return trangThaiDon; }
    std::string layTrangThaiThanhToan() const { return trangThaiThanhToan; }
    std::string layNguoiXuLy() const { return nguoiXuLy; }
    std::string layGhiChu() const { return ghiChu; }
    std::int64_t layThoiDiemGiaoHang() const { return thoiDiemGiaoHang; }
    bool layDaHoanKhoTraHang() const { return daHoanKhoTraHang; }
    const std::vector<ChiTietHoaDon>& layDanhSachChiTiet() const { return danhSachChiTiet; }

    void datTrangThaiDon(const std::string& tt) { trangThaiDon = tt; }
    void datTrangThaiThanhToan(const std::string& tt) { trangThaiThanhToan = tt; }
    void datNguoiXuLy(const std::string& ng) { nguoiXuLy = ng; }
    void datGhiChu(const std::string& gc) { ghiChu = gc; }
    void datThoiDiemGiaoHang(std::int64_t tg) { thoiDiemGiaoHang = tg; }
    void datDaHoanKhoTraHang(bool daHoan) { daHoanKhoTraHang = daHoan; }
    void datGiaTriTrucTiep(double goc, const std::string& loaiGG, double tienGG,
                           const std::string& loaiTh, double tienTh, double tongTT) {
        tongTienGoc = goc;
        loaiGiamGia = loaiGG;
        tienGiamGia = tienGG;
        loaiThue = loaiTh;
        tienThue = tienTh;
        tongTienThanhToan = tongTT;
    }

    void themChiTiet(const ChiTietHoaDon& ct) {
        danhSachChiTiet.push_back(ct);
        tinhLaiTongTien();
    }

    // Tinh tien ap dung Strategy Pattern (Khuyen mai & Thue)
    void apDungChienLuoc(std::shared_ptr<IChienLuocGiamGia> clGiamGia,
                         std::shared_ptr<IChienLuocThue> clThue) {
        tongTienGoc = 0.0;
        for (const auto& ct : danhSachChiTiet) {
            tongTienGoc += ct.layThanhTien();
        }

        if (clGiamGia) {
            loaiGiamGia = clGiamGia->layMaChienLuoc();
            tienGiamGia = clGiamGia->tinhSoTienGiam(tongTienGoc);
        } else {
            loaiGiamGia = "KHONG_GIAM";
            tienGiamGia = 0.0;
        }

        double sauGiam = tongTienGoc - tienGiamGia;
        if (sauGiam < 0) sauGiam = 0;

        if (clThue) {
            loaiThue = clThue->layMaThue();
            tienThue = clThue->tinhThue(sauGiam);
        } else {
            loaiThue = "VAT_10";
            tienThue = sauGiam * 0.1;
        }

        tongTienThanhToan = sauGiam + tienThue;
    }

    void tinhLaiTongTien() {
        auto clGiam = BoChonChienLuoc::taoChienLuocGiamGia(loaiGiamGia);
        auto clThue = BoChonChienLuoc::taoChienLuocThue(loaiThue);
        apDungChienLuoc(clGiam, clThue);
    }

    // Kiem tra nghiep vu
    bool coTheHuy() const {
        // Chi huy truoc khi ban giao cho don vi van chuyen.
        return trangThaiDon == "CHO_XAC_NHAN" || trangThaiDon == "DA_XAC_NHAN";
    }

    bool daXuatKho() const {
        // Don hang da xuat kho khi o trang thai DANG_GIAO hoac DA_GIAO
        return trangThaiDon == "DANG_GIAO" || trangThaiDon == "DA_GIAO";
    }

    std::string xuatJSON() const;
};

#endif // HOADON_H

