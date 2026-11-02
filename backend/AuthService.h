#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
#include "CoSoDuLieu.h"
#include "EmailService.h"
#include "OTPService.h"

struct KetQuaXacThuc {
    bool thanhCong;
    std::string thongBao;
    std::shared_ptr<NguoiDung> nguoiDung;
    std::string token;
};

class AuthService {
private:
    std::shared_ptr<CoSoDuLieu> csdl;
    std::unordered_map<std::string, std::string> tokenNguoiDung;
    std::mutex mutexToken;

public:
    AuthService(std::shared_ptr<CoSoDuLieu> db) : csdl(db) {}

    KetQuaXacThuc dangNhap(const std::string& username, const std::string& password);
    std::shared_ptr<NguoiDung> timNguoiDungTheoToken(const std::string& token);
    KetQuaXacThuc dangKy(const std::string& username, const std::string& password,
                         const std::string& hoTen, const std::string& email, const std::string& sdt);
    bool xacThucTaiKhoanOTP(const std::string& email, const std::string& otp, std::string& thongBao);
    bool yeuCauQuenMatKhau(const std::string& email, std::string& thongBao);
    bool datLaiMatKhau(const std::string& email, const std::string& otp, const std::string& matKhauMoi, std::string& thongBao);
};

#endif // AUTHSERVICE_H

