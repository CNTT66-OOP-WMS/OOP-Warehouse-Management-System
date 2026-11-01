#ifndef COSODULIEU_H
#define COSODULIEU_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "NguoiDung.h"
#include "SanPham.h"
#include "HoaDon.h"
#include "QuanLyKho.h"
#include "Observer.h"

// Quan ly du lieu trong bo nho va doc/ghi trang thai vao file JSON.
class CoSoDuLieu {
private:
    static std::shared_ptr<CoSoDuLieu> instance;
    static std::mutex mutexKhoa;

    std::string duongDanFile;
    std::string loiLuuGanNhat;
    std::recursive_mutex mutexDuLieu;

    std::vector<std::shared_ptr<NguoiDung>> danhSachNguoiDung;
    std::vector<std::shared_ptr<SanPham>> danhSachSanPham;
    std::vector<std::shared_ptr<HoaDon>> danhSachHoaDon;
    std::vector<PhieuNhapKho> danhSachPhieuNhap;
    std::vector<PhieuKiemKe> danhSachKiemKe;
    std::vector<GiaoDichKho> danhSachGiaoDichKho;
    std::vector<NhaCungCap> danhSachNhaCungCap;
    std::vector<ThongBaoCanhBao> danhSachCanhBao;
    TrungTamCanhBaoKho trungTamCanhBao;

    int demCanhBao;
    int demHoaDon;
    int demPhieuNhap;
    int demKiemKe;
    int demNguoiDung;
    int demGiaoDichKho;
    int demNhaCungCap;

    void ghiNhanGiaoDichKho(const std::string& loai, const std::string& maThamChieu,
                           const std::string& nguoiThucHien, const std::string& maSP,
                           const std::string& tenSP, double soLuong, const std::string& lyDo);

    CoSoDuLieu(const std::string& filePath = "database/wms.json");
    void khoiTaoQuanSatKho();

public:
    static std::shared_ptr<CoSoDuLieu> layInstance(const std::string& filePath = "database/wms.json");

    // Khoi tao va luu tru du lieu (Persistence)
    void khoiTao();
    void napDuLieuMau();
    bool luuXuongDia();
    std::string layLoiLuuGanNhat();
    void docTuDia();

    // Sinh ma tu dong
    std::string sinhMaHoaDon();
    std::string sinhMaPhieuNhap();
    std::string sinhMaKiemKe();
    std::string sinhMaNguoiDung();
    std::string sinhMaCanhBao();

    // 1. Quan ly Nguoi Dung
    std::vector<std::shared_ptr<NguoiDung>> layDanhSachNguoiDung();
    std::shared_ptr<NguoiDung> timNguoiDung(const std::string& username);
    std::shared_ptr<NguoiDung> timNguoiDungTheoEmail(const std::string& email);
    std::shared_ptr<NguoiDung> timNguoiDungTheoMa(const std::string& ma);
    bool themNguoiDung(std::shared_ptr<NguoiDung> nd);
    bool capNhatNguoiDung(const std::string& ma, const std::string& hoTen, const std::string& email, const std::string& sdt, const std::string& matKhau);
    bool xoaNguoiDung(const std::string& ma);
    bool datTrangThaiNguoiDung(const std::string& ma, const std::string& trangThai);
    bool datMatKhauNguoiDung(const std::string& ma, const std::string& matKhau);

    // 2. Quan ly San Pham
    std::vector<std::shared_ptr<SanPham>> layDanhSachSanPham();
    std::shared_ptr<SanPham> timSanPhamTheoMa(const std::string& ma);
    bool themSanPham(std::shared_ptr<SanPham> sp);
    bool capNhatSanPham(const std::string& ma, const std::string& ten, const std::string& dvt,
                         double gNhap, double gBan, double minTon, double maxTon,
                         const std::string& viTri, const std::string& moTa,
                         const std::string& nsx = "", const std::string& hsd = "");
    bool capNhatTonKho(const std::string& maSP, double delta);
    bool xoaSanPham(const std::string& ma);

    // 3. Quan ly Nhap Kho
    std::vector<PhieuNhapKho> layDanhSachPhieuNhap();
    bool themPhieuNhap(const PhieuNhapKho& phieu);
    bool duyetPhieuNhap(const std::string& maPhieu);

    // 4. Quan ly Ban Hang & Hoa Don
    std::vector<std::shared_ptr<HoaDon>> layDanhSachHoaDon();
    std::shared_ptr<HoaDon> timHoaDonTheoMa(const std::string& ma);
    bool themHoaDon(std::shared_ptr<HoaDon> hd);
    bool capNhatTrangThaiHoaDon(const std::string& maHD, const std::string& ttDon,
                                const std::string& ttThanhToan, const std::string& nguoiXuLy);
    bool hoanTacHuyHoaDon(const std::string& maHD, const std::string& trangThaiDonCu,
                          const std::string& trangThaiThanhToanCu);
    bool yeuCauTraHang(const std::string& maHD, const std::string& maKhachHang, std::string& thongBao);
    bool xacNhanTraHang(const std::string& maHD, const std::string& nguoiDuyet, std::string& thongBao);

    // 5. Quan ly Kiem Ke
    std::vector<PhieuKiemKe> layDanhSachKiemKe();
    bool themPhieuKiemKe(const PhieuKiemKe& kk);
    std::vector<GiaoDichKho> layDanhSachGiaoDichKho();
    std::vector<NhaCungCap> layDanhSachNhaCungCap();
    bool themNhaCungCap(NhaCungCap ncc);
    bool capNhatNhaCungCap(const NhaCungCap& ncc);
    bool datTrangThaiNhaCungCap(const std::string& maNCC, const std::string& trangThai);

    // 6. Quan ly Canh Bao (Observer)
    std::vector<ThongBaoCanhBao> layDanhSachCanhBao();
    void themCanhBao(const std::string& loai, const std::string& maSP, const std::string& tenSP, const std::string& noiDung);
    void danhDauDaDocCanhBao();
    void kiemTraVaPhatCanhBaoTuDong();

    // 7. Thong ke Bao Cao
    std::string taoBaoCaoTongHopJSON();
};

#endif // COSODULIEU_H

