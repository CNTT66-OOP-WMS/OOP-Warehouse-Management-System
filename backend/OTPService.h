#ifndef OTPSERVICE_H
#define OTPSERVICE_H

#include <string>
#include <map>
#include <random>
#include <chrono>
#include <mutex>
#include <sstream>
#include <iomanip>

struct ThongTinOTP {
    std::string maOTP;
    std::string email;
    std::chrono::system_clock::time_point thoiGianTao;
    bool daDung;
};

class OTPService {
private:
    static std::map<std::string, ThongTinOTP> danhSachOTP;
    static std::mutex mutexOTP;

public:
    static std::string taoVaLuuOTP(const std::string& email) {
        std::lock_guard<std::mutex> lock(mutexOTP);

        // Sinh 6 chu so ngau nhien
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(100000, 999999);
        std::string otp = std::to_string(dis(gen));

        ThongTinOTP info;
        info.maOTP = otp;
        info.email = email;
        info.thoiGianTao = std::chrono::system_clock::now();
        info.daDung = false;

        danhSachOTP[email] = info;
        return otp;
    }

    static bool xacThucOTP(const std::string& email, const std::string& otpNhap) {
        std::lock_guard<std::mutex> lock(mutexOTP);
        auto it = danhSachOTP.find(email);
        if (it == danhSachOTP.end()) return false;

        if (it->second.daDung) return false;

        // Kiem tra thoi han (5 phut = 300 giay)
        auto now = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.thoiGianTao).count();
        if (diff > 300) {
            return false; // Het han
        }

        if (it->second.maOTP == otpNhap) {
            it->second.daDung = true;
            return true;
        }
        return false;
    }

    static std::string layOTPHienTai(const std::string& email) {
        std::lock_guard<std::mutex> lock(mutexOTP);
        auto it = danhSachOTP.find(email);
        if (it != danhSachOTP.end() && !it->second.daDung) {
            return it->second.maOTP;
        }
        return "";
    }
};

#endif // OTPSERVICE_H

