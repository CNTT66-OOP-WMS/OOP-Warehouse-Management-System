#include "HoaDon.h"
#include "JsonUtil.h"

std::string HoaDon::xuatJSON() const {
    std::ostringstream ss;
    ss << "{"
       << "\"maHoaDon\":\"" << jsonEscape(maHoaDon) << "\"," 
       << "\"ngayTao\":\"" << jsonEscape(ngayTao) << "\"," 
       << "\"maKhachHang\":\"" << jsonEscape(maKhachHang) << "\"," 
       << "\"tenKhachHang\":\"" << jsonEscape(tenKhachHang) << "\"," 
       << "\"soDienThoai\":\"" << jsonEscape(soDienThoai) << "\"," 
       << "\"diaChiGiaoHang\":\"" << jsonEscape(diaChiGiaoHang) << "\"," 
       << "\"tongTienGoc\":" << std::fixed << std::setprecision(0) << tongTienGoc << ","
       << "\"loaiGiamGia\":\"" << jsonEscape(loaiGiamGia) << "\"," 
       << "\"tienGiamGia\":" << std::fixed << std::setprecision(0) << tienGiamGia << ","
       << "\"loaiThue\":\"" << jsonEscape(loaiThue) << "\"," 
       << "\"tienThue\":" << std::fixed << std::setprecision(0) << tienThue << ","
       << "\"tongTienThanhToan\":" << std::fixed << std::setprecision(0) << tongTienThanhToan << ","
       << "\"phuongThucThanhToan\":\"" << jsonEscape(phuongThucThanhToan) << "\"," 
       << "\"trangThaiDon\":\"" << jsonEscape(trangThaiDon) << "\"," 
       << "\"trangThaiThanhToan\":\"" << jsonEscape(trangThaiThanhToan) << "\"," 
       << "\"nguoiXuLy\":\"" << jsonEscape(nguoiXuLy) << "\"," 
       << "\"ghiChu\":\"" << jsonEscape(ghiChu) << "\"," 
       << "\"thoiDiemGiaoHang\":" << thoiDiemGiaoHang << ","
       << "\"daHoanKhoTraHang\":" << (daHoanKhoTraHang ? "true" : "false") << ","
       << "\"danhSachChiTiet\":[";

    for (size_t i = 0; i < danhSachChiTiet.size(); ++i) {
        ss << danhSachChiTiet[i].xuatJSON();
        if (i + 1 < danhSachChiTiet.size()) {
            ss << ",";
        }
    }
    ss << "]}";
    return ss.str();
}

