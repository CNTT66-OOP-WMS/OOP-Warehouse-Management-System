#ifndef QUANLYBANHANG_H
#define QUANLYBANHANG_H

#include <string>
#include <vector>
#include <memory>
#include "HoaDon.h"
#include "CoSoDuLieu.h"
#include "Command.h"

struct MonHangDatMua {
    std::string maSP;
    double soLuong;
};

class QuanLyBanHang {
private:
    std::shared_ptr<CoSoDuLieu> csdl;
    QuanLyLenh quanLyLenh;

public:
    QuanLyBanHang(std::shared_ptr<CoSoDuLieu> db) : csdl(db) {}

    // Dat hang truc tuyen hoac tai quay
    bool taoDonHangMoi(const std::string& maKH, const std::string& tenKH, const std::string& sdt,
                       const std::string& diaChi, const std::vector<MonHangDatMua>& danhSachMon,
                       const std::string& loaiGiamGia, const std::string& loaiThue,
                       const std::string& phuongThucTT, const std::string& ghiChu,
                       std::string& maHDMoi, std::string& thongBaoLoi);

    // Xu ly duyet don
    bool xacNhanDonHang(const std::string& maHD, const std::string& nguoiXuLy, std::string& thongBaoLoi);
    bool capNhatGiaoHang(const std::string& maHD, const std::string& trangThaiMoi, std::string& thongBaoLoi);
    bool xacNhanThanhToan(const std::string& maHD, std::string& thongBaoLoi);

    // Xu ly huy don ap dung Command Pattern
    bool huyDonHang(const std::string& maHD, std::string& thongBaoLoi);
    bool hoanTacHuyDon(std::string& thongBaoLoi);
};

#endif // QUANLYBANHANG_H

