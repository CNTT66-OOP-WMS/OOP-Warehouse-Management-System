#include "SanPham.h"
#include "JsonUtil.h"
#include <iostream>
#include <sstream>
#include <iomanip>

int SanPhamHanSuDung::tinhSoNgayConLai() const {
    const std::string hsd = layHanLoFEFO();
    if (!ngayISOHopLe(hsd)) return -1;
    try {
        int y = std::stoi(hsd.substr(0, 4));
        int m = std::stoi(hsd.substr(5, 2));
        int d = std::stoi(hsd.substr(8, 2));

        std::tm tmHsd = {};
        tmHsd.tm_year = y - 1900;
        tmHsd.tm_mon = m - 1;
        tmHsd.tm_mday = d;

        std::time_t tHsd = std::mktime(&tmHsd);
        std::time_t tNow = std::time(nullptr);

        double seconds = std::difftime(tHsd, tNow);
        return (int)(seconds / (60 * 60 * 24));
    } catch (...) {
        return 999;
    }
}

bool SanPhamHanSuDung::daHetHan() const {
    for (const auto& lo : cacLoHang) if (lo.soLuong > 1e-9 && ngayISOHopLe(lo.hanSuDung) && lo.hanSuDung >= ngayHienTaiISO()) return false;
    return true;
}

bool SanPhamHanSuDung::sapHetHan(int nguongNgay) const {
    int ngay = tinhSoNgayConLai();
    return ngay >= 0 && ngay <= nguongNgay;
}

std::string SanPhamThuong::xuatThongTinJSON() const {
    std::ostringstream ss;
    ss << "{"
       << "\"maSP\":\"" << jsonEscape(maSP) << "\"," 
       << "\"tenSP\":\"" << jsonEscape(tenSP) << "\"," 
       << "\"loaiSP\":\"" << layTenLoai() << "\","
       << "\"donViTinh\":\"" << jsonEscape(donViTinh) << "\"," 
       << "\"giaNhap\":" << std::fixed << std::setprecision(0) << giaNhap << ","
       << "\"giaVonBinhQuan\":" << std::fixed << std::setprecision(2) << giaVonBinhQuan << ","
       << "\"giaBan\":" << std::fixed << std::setprecision(0) << giaBan << ","
       << "\"soLuongTon\":" << soLuongTon << ","
       << "\"tonToiThieu\":" << tonToiThieu << ","
       << "\"tonToiDa\":" << tonToiDa << ","
       << "\"viTriKho\":\"" << jsonEscape(viTriKho) << "\"," 
       << "\"moTa\":\"" << jsonEscape(moTa) << "\"," 
       << "\"hinhAnh\":\"" << jsonEscape(hinhAnh) << "\"," 
       << "\"canhBaoTon\":" << (sapHetHang() ? "true" : "false")
       << "}";
    return ss.str();
}

std::string SanPhamHanSuDung::xuatThongTinJSON() const {
    std::ostringstream ss;
    int ngayConLai = tinhSoNgayConLai();
    ss << "{"
       << "\"maSP\":\"" << jsonEscape(maSP) << "\"," 
       << "\"tenSP\":\"" << jsonEscape(tenSP) << "\"," 
       << "\"loaiSP\":\"" << layTenLoai() << "\","
       << "\"donViTinh\":\"" << jsonEscape(donViTinh) << "\"," 
       << "\"giaNhap\":" << std::fixed << std::setprecision(0) << giaNhap << ","
       << "\"giaVonBinhQuan\":" << std::fixed << std::setprecision(2) << giaVonBinhQuan << ","
       << "\"giaBan\":" << std::fixed << std::setprecision(0) << giaBan << ","
       << "\"soLuongTon\":" << soLuongTon << ","
       << "\"tonToiThieu\":" << tonToiThieu << ","
       << "\"tonToiDa\":" << tonToiDa << ","
       << "\"viTriKho\":\"" << jsonEscape(viTriKho) << "\"," 
       << "\"moTa\":\"" << jsonEscape(moTa) << "\"," 
       << "\"hinhAnh\":\"" << jsonEscape(hinhAnh) << "\"," 
       << "\"ngaySanXuat\":\"" << jsonEscape(ngaySanXuat) << "\"," 
       << "\"hanSuDung\":\"" << jsonEscape(hanSuDung) << "\"," 
       << "\"danhSachLo\":[";
    for (size_t i = 0; i < cacLoHang.size(); ++i) {
        if (i) ss << ",";
        ss << "{\"soLuong\":" << cacLoHang[i].soLuong << ",\"ngaySanXuat\":\"" << jsonEscape(cacLoHang[i].ngaySanXuat)
           << "\",\"hanSuDung\":\"" << jsonEscape(cacLoHang[i].hanSuDung) << "\"}";
    }
    ss << "],"
       << "\"ngayConLai\":" << ngayConLai << ","
       << "\"daHetHan\":" << (daHetHan() ? "true" : "false") << ","
       << "\"sapHetHan\":" << (sapHetHan(7) ? "true" : "false") << ","
       << "\"canhBaoTon\":" << (sapHetHang() ? "true" : "false")
       << "}";
    return ss.str();
}

std::string SanPhamTheoCan::xuatThongTinJSON() const {
    std::ostringstream ss;
    ss << "{"
       << "\"maSP\":\"" << jsonEscape(maSP) << "\"," 
       << "\"tenSP\":\"" << jsonEscape(tenSP) << "\"," 
       << "\"loaiSP\":\"" << layTenLoai() << "\","
       << "\"donViTinh\":\"" << jsonEscape(donViTinh) << "\"," 
       << "\"giaNhap\":" << std::fixed << std::setprecision(0) << giaNhap << ","
       << "\"giaVonBinhQuan\":" << std::fixed << std::setprecision(2) << giaVonBinhQuan << ","
       << "\"giaBan\":" << std::fixed << std::setprecision(0) << giaBan << ","
       << "\"soLuongTon\":" << soLuongTon << ","
       << "\"tonToiThieu\":" << tonToiThieu << ","
       << "\"tonToiDa\":" << tonToiDa << ","
       << "\"viTriKho\":\"" << jsonEscape(viTriKho) << "\"," 
       << "\"moTa\":\"" << jsonEscape(moTa) << "\"," 
       << "\"hinhAnh\":\"" << jsonEscape(hinhAnh) << "\"," 
       << "\"trongLuong\":" << trongLuongMacDinh << ","
       << "\"heSoHaoHut\":" << heSoHaoHut << ","
       << "\"canhBaoTon\":" << (sapHetHang() ? "true" : "false")
       << "}";
    return ss.str();
}

