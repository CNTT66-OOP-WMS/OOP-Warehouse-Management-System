#ifndef OBSERVER_H
#define OBSERVER_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <ctime>
#include <sstream>

// ========================================================
// DESIGN PATTERN: OBSERVER PATTERN (NGUOI QUAN SAT)
// Canh bao het hang, sap het han, chenh lech kiem ke kho
// ========================================================

struct ThongBaoCanhBao {
    std::string maCanhBao;
    std::string thoiGian;
    std::string loaiCanhBao; // HET_HANG, SAP_HET_HAN, CHENH_LECH_KHO
    std::string maSP;
    std::string tenSP;
    std::string noiDung;
    bool daDoc;

    std::string xuatJSON() const {
        std::ostringstream ss;
        ss << "{"
           << "\"maCanhBao\":\"" << maCanhBao << "\","
           << "\"thoiGian\":\"" << thoiGian << "\","
           << "\"loaiCanhBao\":\"" << loaiCanhBao << "\","
           << "\"maSP\":\"" << maSP << "\","
           << "\"tenSP\":\"" << tenSP << "\","
           << "\"noiDung\":\"" << noiDung << "\","
           << "\"daDoc\":" << (daDoc ? "true" : "false")
           << "}";
        return ss.str();
    }
};

// Interface Nguoi Quan Sat (Observer Interface)
class INguoiQuanSatKho {
public:
    virtual ~INguoiQuanSatKho() {}
    virtual void capNhatCanhBao(const ThongBaoCanhBao& canhBao) = 0;
    virtual std::string layTenNguoiQuanSat() const = 0;
};

// Chu the phat su kien (Subject)
class IChuTheKho {
public:
    virtual ~IChuTheKho() {}
    virtual void dangKyQuanSat(std::shared_ptr<INguoiQuanSatKho> quanSat) = 0;
    virtual void huyDangKyQuanSat(std::shared_ptr<INguoiQuanSatKho> quanSat) = 0;
    virtual void phatThongBao(const ThongBaoCanhBao& canhBao) = 0;
};

// He thong thong bao kho trung tam (Concrete Subject)
class TrungTamCanhBaoKho : public IChuTheKho {
private:
    std::vector<std::shared_ptr<INguoiQuanSatKho>> danhSachQuanSat;
    std::vector<ThongBaoCanhBao> lichSuCanhBao;

public:
    void dangKyQuanSat(std::shared_ptr<INguoiQuanSatKho> quanSat) override {
        danhSachQuanSat.push_back(quanSat);
    }

    void huyDangKyQuanSat(std::shared_ptr<INguoiQuanSatKho> quanSat) override {
        // Xoa khoi danh sach quan sat
        for (auto it = danhSachQuanSat.begin(); it != danhSachQuanSat.end(); ++it) {
            if (it->get() == quanSat.get()) {
                danhSachQuanSat.erase(it);
                break;
            }
        }
    }

    void phatThongBao(const ThongBaoCanhBao& canhBao) override {
        lichSuCanhBao.push_back(canhBao);
        for (auto& qs : danhSachQuanSat) {
            if (qs) {
                qs->capNhatCanhBao(canhBao);
            }
        }
    }

    const std::vector<ThongBaoCanhBao>& layDanhSachCanhBao() const {
        return lichSuCanhBao;
    }

    void danhDauDaDocTatCa() {
        for (auto& cb : lichSuCanhBao) {
            cb.daDoc = true;
        }
    }
};

// 1. Concrete Observer: Thu Kho Quan Sat (Nhan canh bao het hang de lap PO)
class QuanSatThuKho : public INguoiQuanSatKho {
private:
    std::string tenThuKho;
public:
    QuanSatThuKho(const std::string& ten = "Thu Kho") : tenThuKho(ten) {}

    std::string layTenNguoiQuanSat() const override { return tenThuKho; }

    void capNhatCanhBao(const ThongBaoCanhBao& canhBao) override {
        std::cout << "[OBSERVER - " << tenThuKho << "] Nhan canh bao: " 
                  << canhBao.loaiCanhBao << " - " << canhBao.noiDung << std::endl;
    }
};

// 2. Concrete Observer: Bo Phan Ban Hang Quan Sat (Nhan canh bao de ngung nhan don online)
class QuanSatBanHang : public INguoiQuanSatKho {
private:
    std::string tenBoPhan;
public:
    QuanSatBanHang(const std::string& ten = "Bo Phan Ban Hang") : tenBoPhan(ten) {}

    std::string layTenNguoiQuanSat() const override { return tenBoPhan; }

    void capNhatCanhBao(const ThongBaoCanhBao& canhBao) override {
        if (canhBao.loaiCanhBao == "HET_HANG" || canhBao.loaiCanhBao == "SAP_HET_HAN") {
            std::cout << "[OBSERVER - " << tenBoPhan << "] Cap nhat trang thai san pham: " 
                      << canhBao.tenSP << " (" << canhBao.noiDung << ")" << std::endl;
        }
    }
};

#endif // OBSERVER_H

