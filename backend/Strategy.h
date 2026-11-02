#ifndef STRATEGY_H
#define STRATEGY_H

#include <string>
#include <memory>
#include <algorithm>

// ========================================================
// DESIGN PATTERN: STRATEGY PATTERN (CHIEN LUOC)
// 1. Chien luoc tinh khuyen mai / giam gia (Discount Strategy)
// 2. Chien luoc tinh thue gia tri gia tang (Tax Strategy)
// ========================================================

// --------------------------------------------------------
// 1. STRATEGY: KHUYEN MAI / GIAM GIA
// --------------------------------------------------------
class IChienLuocGiamGia {
public:
    virtual ~IChienLuocGiamGia() {}
    virtual std::string layMaChienLuoc() const = 0;
    virtual std::string layTenChienLuoc() const = 0;
    virtual double tinhSoTienGiam(double tongTienGoc) const = 0;
};

// Khong ap dung giam gia
class KhongGiamGia : public IChienLuocGiamGia {
public:
    std::string layMaChienLuoc() const override { return "KHONG_GIAM"; }
    std::string layTenChienLuoc() const override { return "Khong giam gia"; }
    double tinhSoTienGiam(double /*tongTienGoc*/) const override {
        return 0.0;
    }
};

// Giam gia theo phan tram (%)
class GiamGiaPhanTram : public IChienLuocGiamGia {
private:
    double phanTram; // Vi du 10.0 cho 10%
    double giamToiDa;
public:
    GiamGiaPhanTram(double pt = 10.0, double maxGiam = 500000.0) 
        : phanTram(pt), giamToiDa(maxGiam) {}

    std::string layMaChienLuoc() const override { return "GIAM_PHAN_TRAM"; }
    std::string layTenChienLuoc() const override { 
        return "Giam " + std::to_string((int)phanTram) + "% tong don hang"; 
    }
    double tinhSoTienGiam(double tongTienGoc) const override {
        double tienGiam = tongTienGoc * (phanTram / 100.0);
        return std::min(tienGiam, giamToiDa);
    }
};

// Giam gia theo so tien co dinh khi don vuot nguong toi thieu
class GiamGiaSoTienCoDinh : public IChienLuocGiamGia {
private:
    double soTienGiam;
    double donToiThieu;
public:
    GiamGiaSoTienCoDinh(double tienGiam = 30000.0, double minDon = 200000.0)
        : soTienGiam(tienGiam), donToiThieu(minDon) {}

    std::string layMaChienLuoc() const override { return "GIAM_SO_TIEN"; }
    std::string layTenChienLuoc() const override { 
        return "Giam co dinh 30.000d cho don tu 200.000d"; 
    }
    double tinhSoTienGiam(double tongTienGoc) const override {
        if (tongTienGoc >= donToiThieu) {
            return soTienGiam;
        }
        return 0.0;
    }
};

// Giam gia danh cho khach hang than thiet / VIP (15%)
class GiamGiaKhachHangVIP : public IChienLuocGiamGia {
public:
    std::string layMaChienLuoc() const override { return "GIAM_VIP"; }
    std::string layTenChienLuoc() const override { return "Tri an Khach hang VIP (Giam 15%)"; }
    double tinhSoTienGiam(double tongTienGoc) const override {
        return tongTienGoc * 0.15;
    }
};

// --------------------------------------------------------
// 2. STRATEGY: TINH THUE (TAX STRATEGY)
// --------------------------------------------------------
class IChienLuocThue {
public:
    virtual ~IChienLuocThue() {}
    virtual std::string layMaThue() const = 0;
    virtual std::string layTenThue() const = 0;
    virtual double tinhThue(double tongTienSauGiam) const = 0;
};

// Thue VAT 10% mac dinh
class ThueVAT10 : public IChienLuocThue {
public:
    std::string layMaThue() const override { return "VAT_10"; }
    std::string layTenThue() const override { return "Thue VAT (10%)"; }
    double tinhThue(double tongTienSauGiam) const override {
        return tongTienSauGiam * 0.10;
    }
};

// Thue VAT 5% (Hang hoa uu dai)
class ThueVAT5 : public IChienLuocThue {
public:
    std::string layMaThue() const override { return "VAT_5"; }
    std::string layTenThue() const override { return "Thue VAT uu dai (5%)"; }
    double tinhThue(double tongTienSauGiam) const override {
        return tongTienSauGiam * 0.05;
    }
};

// Mien thue (0%) - Hang nong san, thiet yeu
class ThueMienGiam : public IChienLuocThue {
public:
    std::string layMaThue() const override { return "MIEN_THUE"; }
    std::string layTenThue() const override { return "Mien thue (0%)"; }
    double tinhThue(double /*tongTienSauGiam*/) const override {
        return 0.0;
    }
};

// Factory ho tro tao chien luoc linh hoat
class BoChonChienLuoc {
public:
    static std::shared_ptr<IChienLuocGiamGia> taoChienLuocGiamGia(const std::string& loai) {
        if (loai == "GIAM_PHAN_TRAM") return std::make_shared<GiamGiaPhanTram>(10.0);
        if (loai == "GIAM_SO_TIEN") return std::make_shared<GiamGiaSoTienCoDinh>(30000.0, 200000.0);
        if (loai == "GIAM_VIP") return std::make_shared<GiamGiaKhachHangVIP>();
        return std::make_shared<KhongGiamGia>();
    }

    static std::shared_ptr<IChienLuocThue> taoChienLuocThue(const std::string& loai) {
        if (loai == "VAT_5") return std::make_shared<ThueVAT5>();
        if (loai == "MIEN_THUE") return std::make_shared<ThueMienGiam>();
        return std::make_shared<ThueVAT10>();
    }
};

#endif // STRATEGY_H

