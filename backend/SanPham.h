#ifndef SANPHAM_H
#define SANPHAM_H

#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <cmath>

inline bool ngayISOHopLe(const std::string& s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    for (size_t i = 0; i < s.size(); ++i) if (i != 4 && i != 7 && (s[i] < '0' || s[i] > '9')) return false;
    try {
        int y = std::stoi(s.substr(0,4)), m = std::stoi(s.substr(5,2)), d = std::stoi(s.substr(8,2));
        return std::chrono::year_month_day{std::chrono::year{y}, std::chrono::month{static_cast<unsigned>(m)}, std::chrono::day{static_cast<unsigned>(d)}}.ok();
    } catch (...) { return false; }
}
inline std::string ngayHienTaiISO() {
    std::time_t now = std::time(nullptr); std::tm tmNow = *std::localtime(&now);
    char buf[11]; std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tmNow); return buf;
}

// ========================================================
// TINH KET THUA VA DA HINH TRONG OOP (INHERITANCE & POLYMORPHISM)
// ========================================================

enum class LoaiSanPham {
    THUONG,         // San pham thong thuong (do gia dung, van phong pham...)
    HAN_SU_DUNG,    // San pham co han su dung (sua, banh mi, thuc pham...)
    THEO_CAN        // San pham ban theo khoi luong (thit, gao, rau cu...)
};

// Lop co so truu tuong (Abstract Base Class)
class SanPham {
protected:
    std::string maSP;
    std::string tenSP;
    std::string donViTinh;
    double giaNhap;
    double giaVonBinhQuan;
    double giaBan;
    double soLuongTon;
    double tonToiThieu;
    double tonToiDa;
    std::string viTriKho;
    std::string moTa;
    std::string hinhAnh;

public:
    SanPham(const std::string& ma, const std::string& ten, const std::string& dvt,
            double gNhap, double gBan, double slTon, double minTon, double maxTon,
            const std::string& viTri, const std::string& mTa = "", const std::string& img = "")
        : maSP(ma), tenSP(ten), donViTinh(dvt), giaNhap(gNhap), giaVonBinhQuan(gNhap), giaBan(gBan),
          soLuongTon(slTon), tonToiThieu(minTon), tonToiDa(maxTon),
          viTriKho(viTri), moTa(mTa), hinhAnh(img) {}

    virtual ~SanPham() {}

    // Getter & Setter
    std::string layMaSP() const { return maSP; }
    std::string layTenSP() const { return tenSP; }
    std::string layDonViTinh() const { return donViTinh; }
    double layGiaNhap() const { return giaNhap; }
    double layGiaVonBinhQuan() const { return giaVonBinhQuan; }
    void nhapThemGiaVon(double soLuongNhap, double donGia) {
        const double tonCu = soLuongTon;
        if (tonCu + soLuongNhap > 0) giaVonBinhQuan = (tonCu * giaVonBinhQuan + soLuongNhap * donGia) / (tonCu + soLuongNhap);
    }
    void datGiaVonBinhQuan(double gia) { if (gia >= 0) giaVonBinhQuan = gia; }
    double layGiaBan() const { return giaBan; }
    double laySoLuongTon() const { return soLuongTon; }
    double layTonToiThieu() const { return tonToiThieu; }
    double layTonToiDa() const { return tonToiDa; }
    std::string layViTriKho() const { return viTriKho; }
    std::string layMoTa() const { return moTa; }
    std::string layHinhAnh() const { return hinhAnh; }

    virtual void capNhatSoLuongTon(double delta) {
        soLuongTon += delta;
        if (soLuongTon < 0) soLuongTon = 0;
    }

    virtual void datSoLuongTon(double sl) {
        soLuongTon = (sl >= 0) ? sl : 0;
    }

    void capNhatThongTin(const std::string& ten, const std::string& dvt, double gNhap,
                         double gBan, double minTon, double maxTon, const std::string& viTri, const std::string& mTa) {
        tenSP = ten;
        donViTinh = dvt;
        giaNhap = gNhap;
        giaBan = gBan;
        tonToiThieu = minTon;
        tonToiDa = maxTon;
        viTriKho = viTri;
        moTa = mTa;
    }

    // Phuong thuc kiem tra ton kho
    bool sapHetHang() const {
        return soLuongTon <= tonToiThieu;
    }

    // Cac phuong thuc thuan ao (Pure Virtual Functions) - The hien Da hinh
    virtual LoaiSanPham layLoai() const = 0;
    virtual std::string layTenLoai() const = 0;
    virtual double tinhGiaBanThucTe(double soLuong = 1.0) const = 0;
    virtual double tinhLuongTonTieuHao(double soLuongBan) const { return soLuongBan; }
    virtual bool kiemTraXuatKho(double soLuongCanXuat, std::string& lyDoLoi) const = 0;
    virtual std::string xuatThongTinJSON() const = 0;
};

// 1. San Pham Thong Thuong
class SanPhamThuong : public SanPham {
public:
    SanPhamThuong(const std::string& ma, const std::string& ten, const std::string& dvt,
                  double gNhap, double gBan, double slTon, double minTon = 5, double maxTon = 200,
                  const std::string& viTri = "Khu A", const std::string& mTa = "", const std::string& img = "")
        : SanPham(ma, ten, dvt, gNhap, gBan, slTon, minTon, maxTon, viTri, mTa, img) {}

    LoaiSanPham layLoai() const override { return LoaiSanPham::THUONG; }
    std::string layTenLoai() const override { return "THUONG"; }

    double tinhGiaBanThucTe(double soLuong = 1.0) const override {
        return giaBan * soLuong;
    }

    bool kiemTraXuatKho(double soLuongCanXuat, std::string& lyDoLoi) const override {
        if (soLuongCanXuat <= 0) {
            lyDoLoi = "So luong xuat phai lon hon 0!";
            return false;
        }
        if (soLuongCanXuat > soLuongTon) {
            lyDoLoi = "Khong du ton kho! Hien con: " + std::to_string((int)soLuongTon);
            return false;
        }
        return true;
    }

    std::string xuatThongTinJSON() const override;
};

// 2. San Pham Co Han Su Dung (Quan ly HSD, FEFO, Canh bao can date)
class SanPhamHanSuDung : public SanPham {
public:
    struct LoHang { double soLuong; std::string ngaySanXuat; std::string hanSuDung; };
private:
    std::vector<LoHang> cacLoHang;
    std::string ngaySanXuat; // YYYY-MM-DD
    std::string hanSuDung;   // YYYY-MM-DD

public:
    SanPhamHanSuDung(const std::string& ma, const std::string& ten, const std::string& dvt,
                     double gNhap, double gBan, double slTon,
                     const std::string& nsx, const std::string& hsd,
                     double minTon = 10, double maxTon = 300,
                     const std::string& viTri = "Khu B (Mat/Lanh)", const std::string& mTa = "", const std::string& img = "")
        : SanPham(ma, ten, dvt, gNhap, gBan, slTon, minTon, maxTon, viTri, mTa, img),
          ngaySanXuat(nsx), hanSuDung(hsd) {
        if (slTon > 0 && ngayISOHopLe(hsd)) cacLoHang.push_back({slTon, nsx, hsd});
    }

    LoaiSanPham layLoai() const override { return LoaiSanPham::HAN_SU_DUNG; }
    std::string layTenLoai() const override { return "HAN_SU_DUNG"; }

    std::string layNgaySanXuat() const { return ngaySanXuat; }
    std::string layHanSuDung() const { return hanSuDung; }
    std::string layHanLoFEFO() const {
        std::string minDate;
        for (const auto& lo : cacLoHang) if (lo.soLuong > 1e-9 && ngayISOHopLe(lo.hanSuDung) && (minDate.empty() || lo.hanSuDung < minDate)) minDate = lo.hanSuDung;
        return minDate;
    }
    void capNhatHSD(const std::string& nsx, const std::string& hsd) {
        ngaySanXuat = nsx;
        hanSuDung = hsd;
    }
    const std::vector<LoHang>& layCacLoHang() const { return cacLoHang; }
    void datCacLoHang(const std::vector<LoHang>& lots) {
        cacLoHang = lots;
        std::sort(cacLoHang.begin(), cacLoHang.end(), [](const LoHang& a, const LoHang& b) { return a.hanSuDung < b.hanSuDung; });
        if (!cacLoHang.empty()) { ngaySanXuat = cacLoHang.front().ngaySanXuat; hanSuDung = cacLoHang.front().hanSuDung; }
    }
    void nhapLo(double soLuong, const std::string& nsx, const std::string& hsd) {
        if (soLuong <= 0 || !ngayISOHopLe(hsd)) return;
        cacLoHang.push_back({soLuong, nsx, hsd});
        std::sort(cacLoHang.begin(), cacLoHang.end(), [](const LoHang& a, const LoHang& b) { return a.hanSuDung < b.hanSuDung; });
        ngaySanXuat = cacLoHang.front().ngaySanXuat; hanSuDung = cacLoHang.front().hanSuDung;
        SanPham::capNhatSoLuongTon(soLuong);
    }
    void capNhatSoLuongTon(double delta) override {
        if (delta > 0) {
            if (ngayISOHopLe(hanSuDung)) cacLoHang.push_back({delta, ngaySanXuat, hanSuDung});
            std::sort(cacLoHang.begin(), cacLoHang.end(), [](const LoHang& a, const LoHang& b) { return a.hanSuDung < b.hanSuDung; });
            SanPham::capNhatSoLuongTon(delta); return;
        }
        double canXuat = -delta;
        const std::string today = ngayHienTaiISO();
        for (auto& lo : cacLoHang) { if (lo.hanSuDung < today) continue; double lay = std::min(canXuat, lo.soLuong); lo.soLuong -= lay; canXuat -= lay; if (canXuat <= 1e-9) break; }
        cacLoHang.erase(std::remove_if(cacLoHang.begin(), cacLoHang.end(), [](const LoHang& lo) { return lo.soLuong <= 1e-9; }), cacLoHang.end());
        SanPham::capNhatSoLuongTon(delta);
        if (!cacLoHang.empty()) { ngaySanXuat = cacLoHang.front().ngaySanXuat; hanSuDung = cacLoHang.front().hanSuDung; }
    }
    void datSoLuongTon(double sl) override {
        if (sl < 0) return;
        if (std::abs(sl - soLuongTon) <= 1e-9) return;

        if (sl > soLuongTon) {
            const double delta = sl - soLuongTon;
            if (ngayISOHopLe(hanSuDung)) {
                cacLoHang.push_back({delta, ngaySanXuat, hanSuDung});
                std::sort(cacLoHang.begin(), cacLoHang.end(), [](const LoHang& a, const LoHang& b) { return a.hanSuDung < b.hanSuDung; });
            }
            soLuongTon = sl;
            if (!cacLoHang.empty()) { ngaySanXuat = cacLoHang.front().ngaySanXuat; hanSuDung = cacLoHang.front().hanSuDung; }
            return;
        }

        double canGiam = soLuongTon - sl;
        for (auto& lo : cacLoHang) {
            if (lo.hanSuDung < ngayHienTaiISO()) continue;
            const double lay = std::min(canGiam, lo.soLuong);
            lo.soLuong -= lay;
            canGiam -= lay;
            if (canGiam <= 1e-9) break;
        }
        cacLoHang.erase(std::remove_if(cacLoHang.begin(), cacLoHang.end(), [](const LoHang& lo) { return lo.soLuong <= 1e-9; }), cacLoHang.end());
        soLuongTon = sl;
        if (!cacLoHang.empty()) { ngaySanXuat = cacLoHang.front().ngaySanXuat; hanSuDung = cacLoHang.front().hanSuDung; }
    }

    // Tinh so ngay con lai den han su dung
    int tinhSoNgayConLai() const;
    bool daHetHan() const;
    bool sapHetHan(int nguongNgay = 7) const;

    double tinhGiaBanThucTe(double soLuong = 1.0) const override {
        // Neu sap het han trong vong 3 ngay, giam gia 30% de xa kho FEFO
        if (sapHetHan(3) && !daHetHan()) {
            return giaBan * 0.7 * soLuong;
        }
        return giaBan * soLuong;
    }

    bool kiemTraXuatKho(double soLuongCanXuat, std::string& lyDoLoi) const override {
        if (soLuongCanXuat <= 0) {
            lyDoLoi = "So luong xuat phai lon hon 0!";
            return false;
        }
        double tonConHan = 0;
        const std::string today = ngayHienTaiISO();
        for (const auto& lo : cacLoHang) if (ngayISOHopLe(lo.hanSuDung) && lo.hanSuDung >= today) tonConHan += lo.soLuong;
        if (soLuongCanXuat > tonConHan) {
            lyDoLoi = "San pham da het han su dung (" + hanSuDung + "), khong duoc xuat ban!";
            return false;
        }
        if (soLuongCanXuat > soLuongTon) {
            lyDoLoi = "Ton kho khong du de xuat!";
            return false;
        }
        return true;
    }

    std::string xuatThongTinJSON() const override;
};

// 3. San Pham Ban Theo Can (Khoi luong, ty le hao hut)
class SanPhamTheoCan : public SanPham {
private:
    double trongLuongMacDinh; // Vi du tui 5kg hoac 1kg
    double heSoHaoHut;        // Ty le hao hut tu nhien (vi du 0.01 = 1%)

public:
    SanPhamTheoCan(const std::string& ma, const std::string& ten, const std::string& dvt,
                   double gNhap, double gBan, double slTon,
                   double trongLuong = 1.0, double haoHut = 0.01,
                   double minTon = 20, double maxTon = 500,
                   const std::string& viTri = "Khu C (Kho can)", const std::string& mTa = "", const std::string& img = "")
        : SanPham(ma, ten, dvt, gNhap, gBan, slTon, minTon, maxTon, viTri, mTa, img),
          trongLuongMacDinh(trongLuong), heSoHaoHut(haoHut) {}

    LoaiSanPham layLoai() const override { return LoaiSanPham::THEO_CAN; }
    std::string layTenLoai() const override { return "THEO_CAN"; }

    double layTrongLuongMacDinh() const { return trongLuongMacDinh; }
    double layHeSoHaoHut() const { return heSoHaoHut; }

    double tinhGiaBanThucTe(double soLuongKg = 1.0) const override {
        // Tinh theo khoi luong thuc te
        return giaBan * soLuongKg;
    }

    double tinhLuongTonTieuHao(double soLuongBan) const override {
        return soLuongBan * (1.0 + heSoHaoHut);
    }

    bool kiemTraXuatKho(double soLuongCanXuat, std::string& lyDoLoi) const override {
        if (soLuongCanXuat <= 0.0) {
            lyDoLoi = "Khoi luong xuat phai lon hon 0!";
            return false;
        }
        // Tinh ca ty le hao hut khi xuat
        double tongCanXuat = tinhLuongTonTieuHao(soLuongCanXuat);
        if (tongCanXuat > soLuongTon) {
            lyDoLoi = "Khoi luong trong kho khong du (tinh ca hao hut)!";
            return false;
        }
        return true;
    }

    std::string xuatThongTinJSON() const override;
};

#endif // SANPHAM_H

