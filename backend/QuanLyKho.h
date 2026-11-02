#ifndef QUANLYKHO_H
#define QUANLYKHO_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "SanPham.h"
#include "Observer.h"
#include "JsonUtil.h"

struct GiaoDichKho {
    std::string maGiaoDich, thoiGian, loai, maThamChieu, nguoiThucHien, maSP, tenSP, lyDo;
    double soLuong;

    std::string xuatJSON() const {
        std::ostringstream ss;
        ss << "{\"maGiaoDich\":\"" << jsonEscape(maGiaoDich) << "\",\"thoiGian\":\"" << jsonEscape(thoiGian)
           << "\",\"loai\":\"" << jsonEscape(loai) << "\",\"maThamChieu\":\"" << jsonEscape(maThamChieu)
           << "\",\"nguoiThucHien\":\"" << jsonEscape(nguoiThucHien) << "\",\"maSP\":\"" << jsonEscape(maSP)
           << "\",\"tenSP\":\"" << jsonEscape(tenSP) << "\",\"soLuong\":" << soLuong
           << ",\"lyDo\":\"" << jsonEscape(lyDo) << "\"}";
        return ss.str();
    }
};

struct NhaCungCap {
    std::string maNCC, ten, soDienThoai, diaChi, email, trangThai;
    std::string xuatJSON() const {
        std::ostringstream ss;
        ss << "{\"maNCC\":\"" << jsonEscape(maNCC) << "\",\"ten\":\"" << jsonEscape(ten)
           << "\",\"soDienThoai\":\"" << jsonEscape(soDienThoai) << "\",\"diaChi\":\"" << jsonEscape(diaChi)
           << "\",\"email\":\"" << jsonEscape(email) << "\",\"trangThai\":\"" << jsonEscape(trangThai) << "\"}";
        return ss.str();
    }
};

// Dong chi tiet trong phieu nhap kho
struct ChiTietNhapKho {
    std::string maChiTiet;
    std::string maPhieu;
    std::string maSP;
    std::string tenSP;
    double soLuong;
    double donGiaNhap;
    double thanhTien;
    std::string hanSuDung;

    std::string xuatJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"maChiTiet\":\"" << jsonEscape(maChiTiet) << "\"," 
           << "\"maPhieu\":\"" << jsonEscape(maPhieu) << "\"," 
           << "\"maSP\":\"" << jsonEscape(maSP) << "\"," 
           << "\"tenSP\":\"" << jsonEscape(tenSP) << "\"," 
           << "\"soLuong\":" << soLuong << ","
           << "\"donGiaNhap\":" << std::fixed << std::setprecision(0) << donGiaNhap << ","
           << "\"thanhTien\":" << std::fixed << std::setprecision(0) << thanhTien << ","
           << "\"hanSuDung\":\"" << jsonEscape(hanSuDung) << "\""
           << "}";
        return ss.str();
    }
};

// Phieu nhap kho tu Nha Cung Cap (PO)
class PhieuNhapKho {
private:
    std::string maPhieu;
    std::string ngayNhap;
    std::string nhaCungCap;
    std::string nguoiLapPhieu;
    double tongTien;
    std::string ghiChu;
    std::string trangThai;
    std::vector<ChiTietNhapKho> danhSachChiTiet;

public:
    PhieuNhapKho(const std::string& ma, const std::string& ngay, const std::string& ncc,
                 const std::string& nguoiLap, const std::string& note = "")
        : maPhieu(ma), ngayNhap(ngay), nhaCungCap(ncc), nguoiLapPhieu(nguoiLap),
          tongTien(0.0), ghiChu(note), trangThai("DA_DUYET") {}

    std::string layMaPhieu() const { return maPhieu; }
    std::string layNgayNhap() const { return ngayNhap; }
    std::string layNhaCungCap() const { return nhaCungCap; }
    std::string layNguoiLapPhieu() const { return nguoiLapPhieu; }
    double layTongTien() const { return tongTien; }
    std::string layGhiChu() const { return ghiChu; }
    std::string layTrangThai() const { return trangThai; }
    void datTrangThai(const std::string& tt) { trangThai = tt; }
    const std::vector<ChiTietNhapKho>& layDanhSachChiTiet() const { return danhSachChiTiet; }

    void themChiTiet(const ChiTietNhapKho& ct) {
        danhSachChiTiet.push_back(ct);
        tongTien += ct.thanhTien;
    }

    std::string xuatJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"maPhieu\":\"" << jsonEscape(maPhieu) << "\"," 
           << "\"ngayNhap\":\"" << jsonEscape(ngayNhap) << "\"," 
           << "\"nhaCungCap\":\"" << jsonEscape(nhaCungCap) << "\"," 
           << "\"nguoiLapPhieu\":\"" << jsonEscape(nguoiLapPhieu) << "\"," 
           << "\"tongTien\":" << std::fixed << std::setprecision(0) << tongTien << ","
           << "\"ghiChu\":\"" << jsonEscape(ghiChu) << "\"," 
           << "\"trangThai\":\"" << trangThai << "\","
           << "\"danhSachChiTiet\":[";
        for (size_t i = 0; i < danhSachChiTiet.size(); ++i) {
            ss << danhSachChiTiet[i].xuatJSON();
            if (i + 1 < danhSachChiTiet.size()) ss << ",";
        }
        ss << "]}";
        return ss.str();
    }
};

// Phieu kiem ke kho (Doi chieu thuc te & so sach)
struct PhieuKiemKe {
    std::string maKiemKe;
    std::string ngayKiemKe;
    std::string nguoiKiemKe;
    std::string maSP;
    std::string tenSP;
    double tonSoSach;
    double tonThucTe;
    double chenhLech; // thucTe - soSach
    std::string lyDo;
    std::string cachXuLy; // DIEU_CHINH_TANG, DIEU_CHINH_GIAM, GHI_NHAN_HU_HAI

    std::string xuatJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"maKiemKe\":\"" << jsonEscape(maKiemKe) << "\"," 
           << "\"ngayKiemKe\":\"" << jsonEscape(ngayKiemKe) << "\"," 
           << "\"nguoiKiemKe\":\"" << jsonEscape(nguoiKiemKe) << "\"," 
           << "\"maSP\":\"" << jsonEscape(maSP) << "\"," 
           << "\"tenSP\":\"" << jsonEscape(tenSP) << "\"," 
           << "\"tonSoSach\":" << tonSoSach << ","
           << "\"tonThucTe\":" << tonThucTe << ","
           << "\"chenhLech\":" << chenhLech << ","
           << "\"lyDo\":\"" << jsonEscape(lyDo) << "\"," 
           << "\"cachXuLy\":\"" << jsonEscape(cachXuLy) << "\""
           << "}";
        return ss.str();
    }
};

#endif // QUANLYKHO_H

