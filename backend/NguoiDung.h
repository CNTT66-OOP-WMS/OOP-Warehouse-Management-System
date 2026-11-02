#ifndef NGUOIDUNG_H
#define NGUOIDUNG_H

#include <string>
#include <sstream>
#include <memory>
#include "JsonUtil.h"
#include "PasswordUtil.h"

// Cac vai tro trong he thong WMS
enum class VaiTro {
    ADMIN,          // Quan tri vien he thong
    QUAN_LY_KHO,    // Thu kho / Nguoi quan ly kho
    BAN_HANG,       // Nhan vien ban hang & xu ly don online
    KHACH_HANG      // Khach mua hang
};

// Lop co so NguoiDung (Base Class)
class NguoiDung {
protected:
    std::string maNguoiDung;
    std::string tenDangNhap;
    std::string matKhau;
    std::string hoTen;
    std::string email;
    std::string soDienThoai;
    std::string trangThai; // KICH_HOAT, CHUA_KICH_HOAT, BI_KHOA

public:
    NguoiDung(const std::string& ma, const std::string& user, const std::string& pass,
              const std::string& ten, const std::string& mail, const std::string& sdt,
              const std::string& tThai = "KICH_HOAT")
        : maNguoiDung(ma), tenDangNhap(user), matKhau(hashPassword(pass)),
          hoTen(ten), email(mail), soDienThoai(sdt), trangThai(tThai) {}

    virtual ~NguoiDung() {}

    // Getters & Setters
    std::string layMaNguoiDung() const { return maNguoiDung; }
    std::string layTenDangNhap() const { return tenDangNhap; }
    std::string layMatKhau() const { return matKhau; }
    std::string layHoTen() const { return hoTen; }
    std::string layEmail() const { return email; }
    std::string laySoDienThoai() const { return soDienThoai; }
    std::string layTrangThai() const { return trangThai; }

    void datMatKhau(const std::string& mkMoi) { matKhau = hashPassword(mkMoi); }
    void datTrangThai(const std::string& tt) { trangThai = tt; }
    void capNhatThongTin(const std::string& ten, const std::string& mail, const std::string& sdt) {
        hoTen = ten;
        email = mail;
        soDienThoai = sdt;
    }

    bool kiemTraMatKhau(const std::string& mk) const {
        return verifyPassword(mk, matKhau);
    }

    // Cac phuong thuc ao kiem tra quyen han - Da hinh OOP
    virtual VaiTro layVaiTro() const = 0;
    virtual std::string layTenVaiTro() const = 0;
    virtual bool coQuyenQuanTri() const { return false; }
    virtual bool coQuyenKho() const { return false; }
    virtual bool coQuyenBanHang() const { return false; }
    virtual bool coQuyenMuaHang() const { return false; }

    virtual std::string xuatThongTinJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"maNguoiDung\":\"" << jsonEscape(maNguoiDung) << "\"," 
           << "\"tenDangNhap\":\"" << jsonEscape(tenDangNhap) << "\"," 
           << "\"hoTen\":\"" << jsonEscape(hoTen) << "\"," 
           << "\"email\":\"" << jsonEscape(email) << "\"," 
           << "\"soDienThoai\":\"" << jsonEscape(soDienThoai) << "\"," 
           << "\"vaiTro\":\"" << jsonEscape(layTenVaiTro()) << "\"," 
           << "\"trangThai\":\"" << jsonEscape(trangThai) << "\""
           << "}";
        return ss.str();
    }
};

// 1. Quan Tri Vien (Admin) - Toan quyen he thong
class QuanTriVien : public NguoiDung {
public:
    QuanTriVien(const std::string& ma, const std::string& user, const std::string& pass,
                const std::string& ten, const std::string& mail, const std::string& sdt)
        : NguoiDung(ma, user, pass, ten, mail, sdt) {}

    VaiTro layVaiTro() const override { return VaiTro::ADMIN; }
    std::string layTenVaiTro() const override { return "ADMIN"; }
    bool coQuyenQuanTri() const override { return true; }
    bool coQuyenKho() const override { return true; }
    bool coQuyenBanHang() const override { return true; }
    bool coQuyenMuaHang() const override { return true; }
};

// 2. Nguoi Quan Ly Kho (Warehouse Manager / Thu Kho)
class NguoiQuanLyKho : public NguoiDung {
public:
    NguoiQuanLyKho(const std::string& ma, const std::string& user, const std::string& pass,
                   const std::string& ten, const std::string& mail, const std::string& sdt)
        : NguoiDung(ma, user, pass, ten, mail, sdt) {}

    VaiTro layVaiTro() const override { return VaiTro::QUAN_LY_KHO; }
    std::string layTenVaiTro() const override { return "QUAN_LY_KHO"; }
    bool coQuyenKho() const override { return true; }
};

// 3. Nhan Vien Ban Hang - Xu ly don online & tai quay
class NhanVienBanHang : public NguoiDung {
public:
    NhanVienBanHang(const std::string& ma, const std::string& user, const std::string& pass,
                    const std::string& ten, const std::string& mail, const std::string& sdt)
        : NguoiDung(ma, user, pass, ten, mail, sdt) {}

    VaiTro layVaiTro() const override { return VaiTro::BAN_HANG; }
    std::string layTenVaiTro() const override { return "BAN_HANG"; }
    bool coQuyenBanHang() const override { return true; }
};

// 4. Khach Hang (Customer)
class KhachHang : public NguoiDung {
private:
    double diemTichLuy;
public:
    KhachHang(const std::string& ma, const std::string& user, const std::string& pass,
              const std::string& ten, const std::string& mail, const std::string& sdt,
              double diem = 0.0)
        : NguoiDung(ma, user, pass, ten, mail, sdt), diemTichLuy(diem) {}

    VaiTro layVaiTro() const override { return VaiTro::KHACH_HANG; }
    std::string layTenVaiTro() const override { return "KHACH_HANG"; }
    bool coQuyenMuaHang() const override { return true; }

    double layDiemTichLuy() const { return diemTichLuy; }
    void congDiem(double diem) { diemTichLuy += diem; }
};

#endif // NGUOIDUNG_H

