#ifndef MAYCHUWEB_H
#define MAYCHUWEB_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "CoSoDuLieu.h"
#include "AuthService.h"
#include "QuanLyBanHang.h"

#pragma comment(lib, "ws2_32.lib")

struct YeuCauHTTP {
    std::string phuongThuc; // GET, POST, OPTIONS
    std::string duongDan;
    std::string chuoiTruyVan;
    std::string body;
    std::map<std::string, std::string> tieuDe;
    std::map<std::string, std::string> thamSo;
};

struct PhanHoiHTTP {
    int maTrangThai;
    std::string loaiNoiDung;
    std::string noiDung;

    PhanHoiHTTP(int code = 200, const std::string& type = "application/json", const std::string& body = "")
        : maTrangThai(code), loaiNoiDung(type), noiDung(body) {}
};

class MayChuWeb {
private:
    int congMang;
    SOCKET socketLangNghe;
    std::atomic<bool> dangChay;
    std::shared_ptr<CoSoDuLieu> csdl;
    std::shared_ptr<AuthService> authService;
    std::shared_ptr<QuanLyBanHang> quanLyBanHang;
    std::string thuMucFrontend;
    std::vector<std::thread> workerThreads;

    void xuLyKetNoi(SOCKET clientSocket);
    YeuCauHTTP phanTichYeuCau(const std::string& rawRequest);
    PhanHoiHTTP dinhTuyenYeuCau(const YeuCauHTTP& req);
    std::shared_ptr<NguoiDung> layNguoiDungTuYeuCau(const YeuCauHTTP& req);
    std::string layMimeType(const std::string& duongDan);

    // Cac API Handlers
    PhanHoiHTTP xuLyDangNhap(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyDangKy(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyXacThucOTP(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyLayOTPHienTai(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyQuenMatKhau(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyDatLaiMatKhau(const YeuCauHTTP& req);

    PhanHoiHTTP xuLyLayDanhSachNguoiDung(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyThemNguoiDung(const YeuCauHTTP& req);
    PhanHoiHTTP xuLySuaNguoiDung(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyXoaNguoiDung(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyTrangThaiNguoiDung(const YeuCauHTTP& req);

    PhanHoiHTTP xuLyLayDanhSachSanPham(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyThemSanPham(const YeuCauHTTP& req);
    PhanHoiHTTP xuLySuaSanPham(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyXoaSanPham(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyNhaCungCap(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyTrangThaiNhaCungCap(const YeuCauHTTP& req);

    PhanHoiHTTP xuLyLayPhieuNhap(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyThemPhieuNhap(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyDuyetPhieuNhap(const YeuCauHTTP& req);

    PhanHoiHTTP xuLyLayKiemKe(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyThemKiemKe(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyLayGiaoDichKho(const YeuCauHTTP& req);

    PhanHoiHTTP xuLyLayCanhBao(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyDocTatCaCanhBao(const YeuCauHTTP& req);

    PhanHoiHTTP xuLyLayHoaDon(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyDatHang(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyXacNhanDon(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyGiaoHang(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyTraHang(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyDuyetTraHang(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyThanhToan(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyHuyDon(const YeuCauHTTP& req);
    PhanHoiHTTP xuLyHoanTacHuy(const YeuCauHTTP& req);

    PhanHoiHTTP xuLyBaoCao(const YeuCauHTTP& req);

public:
    MayChuWeb(int port = 8080, const std::string& frontendDir = "../frontend");
    ~MayChuWeb();

    bool khoiDong();
    void dung();
};

#endif // MAYCHUWEB_H

