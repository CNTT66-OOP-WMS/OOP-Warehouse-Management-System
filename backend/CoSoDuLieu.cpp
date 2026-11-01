#include "CoSoDuLieu.h"
#include "JsonUtil.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <unordered_map>

std::shared_ptr<CoSoDuLieu> CoSoDuLieu::instance = nullptr;
std::mutex CoSoDuLieu::mutexKhoa;

static std::string jsonString(const std::string& obj, const std::string& key, const std::string& def = "") {
    std::regex re("\\\"" + key + "\\\"\\s*:\\s*\\\"((?:\\\\.|[^\\\"\\\\])*)\\\"");
    std::smatch m;
    if (!std::regex_search(obj, m, re)) return def;
    std::string s = m[1].str(), out;
    for (size_t i=0; i<s.size(); ++i) {
        if (s[i]=='\\' && i+1<s.size()) { char c=s[++i]; out += c=='n'?'\n':(c=='r'?'\r':(c=='t'?'\t':c)); }
        else out += s[i];
    }
    return out;
}
static double jsonNumber(const std::string& obj, const std::string& key, double def = 0) {
    std::regex re("\\\"" + key + "\\\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?)"); std::smatch m;
    if (!std::regex_search(obj,m,re)) return def;
    try { return std::stod(m[1].str()); } catch (...) { return def; }
}
static bool jsonBool(const std::string& obj, const std::string& key) {
    std::regex re("\\\"" + key + "\\\"\\s*:\\s*true"); return std::regex_search(obj,re);
}
static std::vector<std::string> jsonObjects(const std::string& json, const std::string& key) {
    std::vector<std::string> out; auto p=json.find("\""+key+"\""); if(p==std::string::npos)return out;
    p=json.find('[',p); if(p==std::string::npos)return out; int depth=0; bool quoted=false,esc=false; size_t start=0;
    for(size_t i=p+1;i<json.size();++i){char c=json[i];if(quoted){if(esc)esc=false;else if(c=='\\')esc=true;else if(c=='\"')quoted=false;continue;}if(c=='\"'){quoted=true;continue;}if(c=='{'){if(depth++==0)start=i;}else if(c=='}'&&depth>0&&--depth==0)out.push_back(json.substr(start,i-start+1));else if(c==']'&&depth==0)break;}
    return out;
}

static std::string layThoiGianHienTai() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

static std::string layNgayHienTai() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d");
    return ss.str();
}

CoSoDuLieu::CoSoDuLieu(const std::string& filePath)
    : duongDanFile(filePath), demCanhBao(10), demHoaDon(10), demPhieuNhap(10), demKiemKe(10), demNguoiDung(10), demGiaoDichKho(0), demNhaCungCap(0) {
    trungTamCanhBao.dangKyQuanSat(std::make_shared<QuanSatThuKho>());
    trungTamCanhBao.dangKyQuanSat(std::make_shared<QuanSatBanHang>());
}

void CoSoDuLieu::ghiNhanGiaoDichKho(const std::string& loai, const std::string& maThamChieu,
                                   const std::string& nguoiThucHien, const std::string& maSP,
                                   const std::string& tenSP, double soLuong, const std::string& lyDo) {
    std::ostringstream id;
    id << "GDK" << std::setw(5) << std::setfill('0') << ++demGiaoDichKho;
    danhSachGiaoDichKho.push_back({id.str(), layThoiGianHienTai(), loai, maThamChieu,
                                   nguoiThucHien, maSP, tenSP, lyDo, soLuong});
}

std::shared_ptr<CoSoDuLieu> CoSoDuLieu::layInstance(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutexKhoa);
    if (!instance) {
        instance = std::shared_ptr<CoSoDuLieu>(new CoSoDuLieu(filePath));
        instance->khoiTao();
    }
    return instance;
}

void CoSoDuLieu::khoiTao() {
    std::string targetPath = duongDanFile;
    std::ifstream current(targetPath);
    if (!current.good()) {
        const std::string fallback = "../" + targetPath;
        std::ifstream parent(fallback);
        if (parent.good()) { targetPath = fallback; duongDanFile = fallback; parent.close(); }
        else { targetPath = fallback; duongDanFile = fallback; }
    }
    current.close();
    current.open(targetPath);
    if (!current.good()) {
        auto slash = targetPath.find_last_of("/\\");
        const std::string legacyPath = targetPath.substr(0, slash == std::string::npos ? 0 : slash + 1) + "wms.db";
        std::ifstream legacy(legacyPath);
        if (legacy.good()) { duongDanFile = legacyPath; legacy.close(); docTuDia(); duongDanFile = targetPath; luuXuongDia(); }
        else { napDuLieuMau(); luuXuongDia(); }
    } else {
        current.close();
        docTuDia();
    }
    kiemTraVaPhatCanhBaoTuDong();
}

void CoSoDuLieu::napDuLieuMau() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    danhSachNguoiDung.clear();
    danhSachSanPham.clear();
    danhSachHoaDon.clear();
    danhSachPhieuNhap.clear();
    danhSachKiemKe.clear();
    danhSachCanhBao.clear();
    danhSachGiaoDichKho.clear();
    danhSachNhaCungCap.clear();

    danhSachNhaCungCap.push_back({"NCC001", "Cong ty CP Sua Viet Nam", "02838236688",
        "TP. Ho Chi Minh", "lienhe@vinamilk.com.vn", "DANG_HOP_TAC"});
    demNhaCungCap = 1;

    // 1. Nguoi dung mau
    danhSachNguoiDung.push_back(std::make_shared<QuanTriVien>(
        "ND01", "admin", "admin123", "Quan Tri Vien He Thong", "admin@wms.com", "0901234567"));
    danhSachNguoiDung.push_back(std::make_shared<NguoiQuanLyKho>(
        "ND02", "thukho", "kho123", "Nguyen Van Kho", "thukho@wms.com", "0912345678"));
    danhSachNguoiDung.push_back(std::make_shared<NhanVienBanHang>(
        "ND03", "banhang", "banhang123", "Tran Thi Ban Hang", "banhang@wms.com", "0923456789"));
    danhSachNguoiDung.push_back(std::make_shared<KhachHang>(
        "ND04", "khachhang", "khach123", "Le Hoang Khach Mua", "khachhang@gmail.com", "0934567890", 150.0));

    // 2. San pham mau - Da hinh (Polymorphism)
    // SP Thuong
    danhSachSanPham.push_back(std::make_shared<SanPhamThuong>(
        "SP01", "Binh giu nhiet Inox 800ml Lock&Lock", "Cai", 120000, 185000, 44, 10, 200, "Khu A - Ke 01",
        "Binh inox 304 cao cap giu nhiet nong lanh 24 gio"));
    danhSachSanPham.push_back(std::make_shared<SanPhamThuong>(
        "SP02", "Ban phim co gaming E-Dra EK387", "Chiec", 450000, 690000, 8, 10, 100, "Khu A - Ke 02",
        "Ban phim co switch Brown cao cap"));

    // SP Han su dung
    danhSachSanPham.push_back(std::make_shared<SanPhamHanSuDung>(
        "SP03", "Sua tuoi tiet trung Vinamilk 1L", "Hop", 28000, 39000, 58,
        "2026-10-10", "2026-11-20", 15, 300, "Khu B (Lanh) - Ke 01", "Bao quan nhiet do 4-6 do C"));
    danhSachSanPham.push_back(std::make_shared<SanPhamHanSuDung>(
        "SP04", "Banh mi sandwich bo Kinh Do", "Tui", 15000, 24000, 4,
        "2026-10-20", "2026-10-28", 10, 100, "Khu B - Ke 02", "Banh mi tuoi - Sap het han trong tuan"));

    // SP Theo can
    danhSachSanPham.push_back(std::make_shared<SanPhamTheoCan>(
        "SP05", "Gao thom dac san ST25 Thuong Hang", "Kg", 32000, 42000, 110.4, 5.0, 0.01,
        20, 500, "Khu C - Ke 01", "Gao deo ngon nhat the gioi"));
    danhSachSanPham.push_back(std::make_shared<SanPhamTheoCan>(
        "SP06", "Thit ba chi bo Uc nhap khau", "Kg", 180000, 250000, 3.5, 1.0, 0.02,
        10, 100, "Khu B (Dong lanh) - Ke 03", "Thit bo cuon thai lat dong lanh"));

    // 3. Phieu nhap kho mau
    PhieuNhapKho pn("PN001", "2026-10-24 09:30:00", "Cong ty CP Sua Viet Nam", "Nguyen Van Kho", "Nhap hang dau thang");
    ChiTietNhapKho ct1 = {"CTN01", "PN001", "SP03", "Sua tuoi tiet trung Vinamilk 1L", 50, 28000, 1400000, "2026-11-20"};
    ChiTietNhapKho ct2 = {"CTN02", "PN001", "SP01", "Binh giu nhiet Inox 800ml Lock&Lock", 20, 120000, 2400000, ""};
    pn.themChiTiet(ct1);
    pn.themChiTiet(ct2);
    danhSachPhieuNhap.push_back(pn);

    // 4. Hoa don mau
    auto hd1 = std::make_shared<HoaDon>(
        "HD001", "2026-10-24 14:15:00", "ND04", "Le Hoang Khach Mua", "0934567890",
        "123 Duong Nguyen Trai, Q. Thanh Xuan, Ha Noi", "CHUYEN_KHOAN", "DA_GIAO", "DA_THANH_TOAN",
        "Tran Thi Ban Hang", "Giao gio hanh chinh");
    hd1->themChiTiet(ChiTietHoaDon("CTHD01", "HD001", "SP01", "Binh giu nhiet Inox 800ml Lock&Lock", 1, 185000));
    hd1->themChiTiet(ChiTietHoaDon("CTHD02", "HD001", "SP03", "Sua tuoi tiet trung Vinamilk 1L", 2, 39000));
    hd1->tinhLaiTongTien();
    danhSachHoaDon.push_back(hd1);

    auto hd2 = std::make_shared<HoaDon>(
        "HD002", "2026-10-25 10:00:00", "ND04", "Le Hoang Khach Mua", "0934567890",
        "123 Duong Nguyen Trai, Q. Thanh Xuan, Ha Noi", "TIEN_MAT", "CHO_XAC_NHAN", "CHUA_THANH_TOAN",
        "Online", "Don dat qua web");
    hd2->themChiTiet(ChiTietHoaDon("CTHD03", "HD002", "SP05", "Gao thom dac san ST25 Thuong Hang", 10, 42000));
    hd2->tinhLaiTongTien();
    danhSachHoaDon.push_back(hd2);

    // 5. Phieu kiem ke mau
    PhieuKiemKe kk1 = {"KK001", "2026-10-23 17:00:00", "Nguyen Van Kho", "SP02", "Ban phim co gaming E-Dra EK387", 9, 8, -1, "Hao hut do trung bay", "DIEU_CHINH_GIAM"};
    danhSachKiemKe.push_back(kk1);
}

bool CoSoDuLieu::luuXuongDia() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    loiLuuGanNhat.clear();
    std::error_code fsError;
    auto parentDir = std::filesystem::path(duongDanFile).parent_path();
    if (!parentDir.empty()) std::filesystem::create_directories(parentDir, fsError);
    if (fsError) { loiLuuGanNhat = "Khong tao duoc thu muc du lieu: " + fsError.message(); return false; }
    const std::filesystem::path target(duongDanFile);
    const std::filesystem::path temp = target.string() + ".tmp";
    const std::filesystem::path backup = target.string() + ".bak";
    std::filesystem::remove(temp, fsError); fsError.clear();
    std::ofstream out(temp, std::ios::trunc | std::ios::binary);
    if (!out.is_open()) { loiLuuGanNhat = "Khong mo duoc tep du lieu de ghi: " + duongDanFile; return false; }

    // Luu dang JSON toan dien
    out << "{\n";
    
    // Nguoi dung
    out << "\"nguoiDung\": [\n";
    for (size_t i = 0; i < danhSachNguoiDung.size(); ++i) {
        auto nd = danhSachNguoiDung[i];
        out << "  {\"ma\":\"" << jsonEscape(nd->layMaNguoiDung()) << "\","
            << "\"user\":\"" << jsonEscape(nd->layTenDangNhap()) << "\","
            << "\"pass\":\"" << jsonEscape(nd->layMatKhau()) << "\","
            << "\"ten\":\"" << jsonEscape(nd->layHoTen()) << "\","
            << "\"email\":\"" << jsonEscape(nd->layEmail()) << "\","
            << "\"sdt\":\"" << jsonEscape(nd->laySoDienThoai()) << "\","
            << "\"vaiTro\":\"" << nd->layTenVaiTro() << "\"," 
            << "\"trangThai\":\"" << nd->layTrangThai() << "\",";
        auto kh = std::dynamic_pointer_cast<KhachHang>(nd);
        if (kh) out << "\"diemTichLuy\":" << kh->layDiemTichLuy(); else out << "\"diemTichLuy\":0";
        out << "}";
        if (i + 1 < danhSachNguoiDung.size()) out << ",";
        out << "\n";
    }
    out << "],\n";

    // San pham
    out << "\"sanPham\": [\n";
    for (size_t i = 0; i < danhSachSanPham.size(); ++i) {
        out << "  " << danhSachSanPham[i]->xuatThongTinJSON();
        if (i + 1 < danhSachSanPham.size()) out << ",";
        out << "\n";
    }
    out << "],\n";

    // Phieu nhap kho
    out << "\"phieuNhap\": [\n";
    for (size_t i = 0; i < danhSachPhieuNhap.size(); ++i) {
        out << "  " << danhSachPhieuNhap[i].xuatJSON();
        if (i + 1 < danhSachPhieuNhap.size()) out << ",";
        out << "\n";
    }
    out << "],\n";

    // Hoa don
    out << "\"hoaDon\": [\n";
    for (size_t i = 0; i < danhSachHoaDon.size(); ++i) {
        out << "  " << danhSachHoaDon[i]->xuatJSON();
        if (i + 1 < danhSachHoaDon.size()) out << ",";
        out << "\n";
    }
    out << "],\n";

    // Kiem ke
    out << "\"kiemKe\": [\n";
    for (size_t i = 0; i < danhSachKiemKe.size(); ++i) {
        out << "  " << danhSachKiemKe[i].xuatJSON();
        if (i + 1 < danhSachKiemKe.size()) out << ",";
        out << "\n";
    }
    out << "],\n\"giaoDichKho\": [\n";
    for (size_t i = 0; i < danhSachGiaoDichKho.size(); ++i) {
        out << "  " << danhSachGiaoDichKho[i].xuatJSON();
        if (i + 1 < danhSachGiaoDichKho.size()) out << ",";
        out << "\n";
    }
    out << "],\n\"nhaCungCap\": [\n";
    for (size_t i = 0; i < danhSachNhaCungCap.size(); ++i) {
        out << "  " << danhSachNhaCungCap[i].xuatJSON();
        if (i + 1 < danhSachNhaCungCap.size()) out << ",";
        out << "\n";
    }
    out << "]\n";

    out << "}\n";
    out.close();
    if (out.fail()) { std::filesystem::remove(temp, fsError); loiLuuGanNhat = "Ghi tep du lieu that bai: " + duongDanFile; return false; }
    const bool hadTarget = std::filesystem::exists(target, fsError) && !fsError;
    if (hadTarget) {
        std::filesystem::remove(backup, fsError); fsError.clear();
        std::filesystem::rename(target, backup, fsError);
        if (fsError) { std::filesystem::remove(temp, fsError); loiLuuGanNhat = "Khong the thay tep du lieu hien tai: " + duongDanFile; return false; }
    }
    std::filesystem::rename(temp, target, fsError);
    if (fsError) {
        if (hadTarget) { std::error_code restoreError; std::filesystem::rename(backup, target, restoreError); }
        std::filesystem::remove(temp, fsError);
        loiLuuGanNhat = "Khong the thay tep du lieu tam vao tep chinh: " + duongDanFile;
        return false;
    }
    if (hadTarget) std::filesystem::remove(backup, fsError);
    return true;
}

std::string CoSoDuLieu::layLoiLuuGanNhat() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    return loiLuuGanNhat;
}

void CoSoDuLieu::docTuDia() {
    std::ifstream in(duongDanFile, std::ios::binary);
    std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    auto users=jsonObjects(json,"nguoiDung"), products=jsonObjects(json,"sanPham"), orders=jsonObjects(json,"hoaDon");
    if (users.empty() && products.empty() && orders.empty()) { napDuLieuMau(); luuXuongDia(); return; }
    danhSachNguoiDung.clear(); danhSachSanPham.clear(); danhSachHoaDon.clear();
    danhSachPhieuNhap.clear(); danhSachKiemKe.clear(); danhSachCanhBao.clear(); danhSachGiaoDichKho.clear(); danhSachNhaCungCap.clear();
    auto bump=[](const std::string& id,int& counter){try{size_t p=id.find_first_of("0123456789");if(p!=std::string::npos)counter=std::max(counter,std::stoi(id.substr(p)));}catch(...){}};
    for(const auto& o:users){
        auto role=jsonString(o,"vaiTro"); std::shared_ptr<NguoiDung> u;
        auto ma=jsonString(o,"ma"), user=jsonString(o,"user"), pass=jsonString(o,"pass"), ten=jsonString(o,"ten"), email=jsonString(o,"email"), sdt=jsonString(o,"sdt");
        if(role=="ADMIN")u=std::make_shared<QuanTriVien>(ma,user,pass,ten,email,sdt);
        else if(role=="QUAN_LY_KHO")u=std::make_shared<NguoiQuanLyKho>(ma,user,pass,ten,email,sdt);
        else if(role=="BAN_HANG")u=std::make_shared<NhanVienBanHang>(ma,user,pass,ten,email,sdt);
        else u=std::make_shared<KhachHang>(ma,user,pass,ten,email,sdt,jsonNumber(o,"diemTichLuy"));
        u->datTrangThai(jsonString(o,"trangThai","KICH_HOAT")); danhSachNguoiDung.push_back(u); bump(ma,demNguoiDung);
    }
    for(const auto& o:products){
        auto ma=jsonString(o,"maSP"), ten=jsonString(o,"tenSP"), dvt=jsonString(o,"donViTinh"), vitri=jsonString(o,"viTriKho"), mota=jsonString(o,"moTa"), loai=jsonString(o,"loaiSP");
        double gn=jsonNumber(o,"giaNhap"), gb=jsonNumber(o,"giaBan"), ton=jsonNumber(o,"soLuongTon"), mn=jsonNumber(o,"tonToiThieu"), mx=jsonNumber(o,"tonToiDa");
        std::shared_ptr<SanPham> sp;
        if(loai=="HAN_SU_DUNG")sp=std::make_shared<SanPhamHanSuDung>(ma,ten,dvt,gn,gb,ton,jsonString(o,"ngaySanXuat"),jsonString(o,"hanSuDung"),mn,mx,vitri,mota,jsonString(o,"hinhAnh"));
        else if(loai=="THEO_CAN")sp=std::make_shared<SanPhamTheoCan>(ma,ten,dvt,gn,gb,ton,jsonNumber(o,"trongLuong",1),jsonNumber(o,"heSoHaoHut",0.01),mn,mx,vitri,mota,jsonString(o,"hinhAnh"));
        else sp=std::make_shared<SanPhamThuong>(ma,ten,dvt,gn,gb,ton,mn,mx,vitri,mota,jsonString(o,"hinhAnh"));
        sp->datGiaVonBinhQuan(jsonNumber(o,"giaVonBinhQuan",gn));
        if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp)) {
            std::vector<SanPhamHanSuDung::LoHang> lots;
            for (const auto& lot : jsonObjects(o, "danhSachLo")) lots.push_back({jsonNumber(lot,"soLuong"),jsonString(lot,"ngaySanXuat"),jsonString(lot,"hanSuDung")});
            if (!lots.empty()) hsd->datCacLoHang(lots);
        }
        danhSachSanPham.push_back(sp);
    }
    for(const auto& o:orders){
        auto ma=jsonString(o,"maHoaDon"); auto hd=std::make_shared<HoaDon>(ma,jsonString(o,"ngayTao"),jsonString(o,"maKhachHang"),jsonString(o,"tenKhachHang"),jsonString(o,"soDienThoai"),jsonString(o,"diaChiGiaoHang"),jsonString(o,"phuongThucThanhToan"),jsonString(o,"trangThaiDon"),jsonString(o,"trangThaiThanhToan"),jsonString(o,"nguoiXuLy"),jsonString(o,"ghiChu"));
        for(const auto& c:jsonObjects(o,"danhSachChiTiet"))hd->themChiTiet(ChiTietHoaDon(jsonString(c,"maChiTiet"),ma,jsonString(c,"maSP"),jsonString(c,"tenSP"),jsonNumber(c,"soLuong"),jsonNumber(c,"donGia")));
        hd->datGiaTriTrucTiep(jsonNumber(o,"tongTienGoc"),jsonString(o,"loaiGiamGia"),jsonNumber(o,"tienGiamGia"),jsonString(o,"loaiThue"),jsonNumber(o,"tienThue"),jsonNumber(o,"tongTienThanhToan"));
        hd->datThoiDiemGiaoHang(static_cast<std::int64_t>(jsonNumber(o,"thoiDiemGiaoHang"))); hd->datDaHoanKhoTraHang(jsonBool(o,"daHoanKhoTraHang"));
        danhSachHoaDon.push_back(hd); bump(ma,demHoaDon);
    }
    for(const auto& o:jsonObjects(json,"phieuNhap")){
        auto id=jsonString(o,"maPhieu"); PhieuNhapKho pn(id,jsonString(o,"ngayNhap"),jsonString(o,"nhaCungCap"),jsonString(o,"nguoiLapPhieu"),jsonString(o,"ghiChu"));
        for(const auto& c:jsonObjects(o,"danhSachChiTiet")){ChiTietNhapKho ct={jsonString(c,"maChiTiet"),id,jsonString(c,"maSP"),jsonString(c,"tenSP"),jsonNumber(c,"soLuong"),jsonNumber(c,"donGiaNhap"),jsonNumber(c,"thanhTien"),jsonString(c,"hanSuDung")};pn.themChiTiet(ct);}
        pn.datTrangThai(jsonString(o,"trangThai","DA_DUYET"));danhSachPhieuNhap.push_back(pn);bump(id,demPhieuNhap);
    }
    for(const auto& o:jsonObjects(json,"kiemKe")){
        PhieuKiemKe kk={jsonString(o,"maKiemKe"),jsonString(o,"ngayKiemKe"),jsonString(o,"nguoiKiemKe"),jsonString(o,"maSP"),jsonString(o,"tenSP"),jsonNumber(o,"tonSoSach"),jsonNumber(o,"tonThucTe"),jsonNumber(o,"chenhLech"),jsonString(o,"lyDo"),jsonString(o,"cachXuLy")};
        danhSachKiemKe.push_back(kk);bump(kk.maKiemKe,demKiemKe);
    }
    for(const auto& o:jsonObjects(json,"giaoDichKho")){
        GiaoDichKho gd{jsonString(o,"maGiaoDich"),jsonString(o,"thoiGian"),jsonString(o,"loai"),
            jsonString(o,"maThamChieu"),jsonString(o,"nguoiThucHien"),jsonString(o,"maSP"),
            jsonString(o,"tenSP"),jsonString(o,"lyDo"),jsonNumber(o,"soLuong")};
        danhSachGiaoDichKho.push_back(gd); bump(gd.maGiaoDich,demGiaoDichKho);
    }
    for(const auto& o:jsonObjects(json,"nhaCungCap")){
        NhaCungCap ncc{jsonString(o,"maNCC"),jsonString(o,"ten"),jsonString(o,"soDienThoai"),
            jsonString(o,"diaChi"),jsonString(o,"email"),jsonString(o,"trangThai","DANG_HOP_TAC")};
        danhSachNhaCungCap.push_back(ncc); bump(ncc.maNCC,demNhaCungCap);
    }
    if (danhSachNhaCungCap.empty()) {
        danhSachNhaCungCap.push_back({"NCC001", "Cong ty CP Sua Viet Nam", "02838236688",
            "TP. Ho Chi Minh", "lienhe@vinamilk.com.vn", "DANG_HOP_TAC"});
        demNhaCungCap = std::max(demNhaCungCap, 1);
        luuXuongDia();
    }
    // Password plaintext legacy records have been upgraded in-memory by NguoiDung construction.
    if (!luuXuongDia()) std::cerr << "Canh bao: khong the luu du lieu da chuyen doi mat khau.\n";
}

std::string CoSoDuLieu::sinhMaHoaDon() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    demHoaDon++;
    std::ostringstream ss;
    ss << "HD" << std::setw(3) << std::setfill('0') << demHoaDon;
    return ss.str();
}

std::string CoSoDuLieu::sinhMaPhieuNhap() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    demPhieuNhap++;
    std::ostringstream ss;
    ss << "PN" << std::setw(3) << std::setfill('0') << demPhieuNhap;
    return ss.str();
}

std::string CoSoDuLieu::sinhMaKiemKe() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    demKiemKe++;
    std::ostringstream ss;
    ss << "KK" << std::setw(3) << std::setfill('0') << demKiemKe;
    return ss.str();
}

std::string CoSoDuLieu::sinhMaNguoiDung() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    demNguoiDung++;
    std::ostringstream ss;
    ss << "ND" << std::setw(2) << std::setfill('0') << demNguoiDung;
    return ss.str();
}

std::string CoSoDuLieu::sinhMaCanhBao() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    demCanhBao++;
    std::ostringstream ss;
    ss << "CB" << std::setw(3) << std::setfill('0') << demCanhBao;
    return ss.str();
}

// 1. Nguoi Dung
std::vector<std::shared_ptr<NguoiDung>> CoSoDuLieu::layDanhSachNguoiDung() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    return danhSachNguoiDung;
}

std::shared_ptr<NguoiDung> CoSoDuLieu::timNguoiDung(const std::string& username) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& nd : danhSachNguoiDung) {
        if (nd->layTenDangNhap() == username) return nd;
    }
    return nullptr;
}

std::shared_ptr<NguoiDung> CoSoDuLieu::timNguoiDungTheoEmail(const std::string& email) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& nd : danhSachNguoiDung) {
        if (nd->layEmail() == email) return nd;
    }
    return nullptr;
}

std::shared_ptr<NguoiDung> CoSoDuLieu::timNguoiDungTheoMa(const std::string& ma) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& nd : danhSachNguoiDung) {
        if (nd->layMaNguoiDung() == ma) return nd;
    }
    return nullptr;
}

bool CoSoDuLieu::themNguoiDung(std::shared_ptr<NguoiDung> nd) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& item : danhSachNguoiDung) {
        if (item->layTenDangNhap() == nd->layTenDangNhap() || item->layEmail() == nd->layEmail()) {
            return false;
        }
    }
    danhSachNguoiDung.push_back(nd);
    if (!luuXuongDia()) { danhSachNguoiDung.pop_back(); return false; }
    return true;
}

bool CoSoDuLieu::capNhatNguoiDung(const std::string& ma, const std::string& hoTen,
                                 const std::string& email, const std::string& sdt, const std::string& matKhau) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& nd : danhSachNguoiDung) {
        if (nd->layMaNguoiDung() == ma) {
            const std::string tenCu = nd->layHoTen(), emailCu = nd->layEmail(), sdtCu = nd->laySoDienThoai(), matKhauCu = nd->layMatKhau();
            nd->capNhatThongTin(hoTen, email, sdt);
            if (!matKhau.empty()) {
                nd->datMatKhau(matKhau);
            }
            if (!luuXuongDia()) { nd->capNhatThongTin(tenCu, emailCu, sdtCu); nd->datMatKhau(matKhauCu); return false; }
            return true;
        }
    }
    return false;
}

bool CoSoDuLieu::xoaNguoiDung(const std::string& ma) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto it = danhSachNguoiDung.begin(); it != danhSachNguoiDung.end(); ++it) {
        if ((*it)->layMaNguoiDung() == ma) {
            auto nguoiDungCu = *it;
            const size_t viTri = static_cast<size_t>(std::distance(danhSachNguoiDung.begin(), it));
            danhSachNguoiDung.erase(it);
            if (!luuXuongDia()) { danhSachNguoiDung.insert(danhSachNguoiDung.begin() + viTri, nguoiDungCu); return false; }
            return true;
        }
    }
    return false;
}

bool CoSoDuLieu::datTrangThaiNguoiDung(const std::string& ma, const std::string& trangThai) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    if (trangThai != "KICH_HOAT" && trangThai != "BI_KHOA") return false;
    for (auto& nd : danhSachNguoiDung) if (nd->layMaNguoiDung() == ma) {
        if (nd->coQuyenQuanTri() && trangThai == "BI_KHOA") return false;
        const std::string cu = nd->layTrangThai();
        nd->datTrangThai(trangThai); if (!luuXuongDia()) { nd->datTrangThai(cu); return false; } return true;
    }
    return false;
}

bool CoSoDuLieu::datMatKhauNguoiDung(const std::string& ma, const std::string& matKhau) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& nd : danhSachNguoiDung) if (nd->layMaNguoiDung() == ma) {
        const std::string cu = nd->layMatKhau();
        nd->datMatKhau(matKhau);
        if (!luuXuongDia()) { nd->datMatKhau(cu); return false; }
        return true;
    }
    return false;
}

// 2. San Pham
std::vector<std::shared_ptr<SanPham>> CoSoDuLieu::layDanhSachSanPham() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    return danhSachSanPham;
}

std::shared_ptr<SanPham> CoSoDuLieu::timSanPhamTheoMa(const std::string& ma) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& sp : danhSachSanPham) {
        if (sp->layMaSP() == ma) return sp;
    }
    return nullptr;
}

bool CoSoDuLieu::themSanPham(std::shared_ptr<SanPham> sp) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& item : danhSachSanPham) {
        if (item->layMaSP() == sp->layMaSP()) return false;
    }
    const size_t gdCu = danhSachGiaoDichKho.size(); const int demGdCu = demGiaoDichKho;
    danhSachSanPham.push_back(sp);
    if (sp->laySoLuongTon() > 0) ghiNhanGiaoDichKho("TON_DAU_KY", "KHOI_TAO", "System", sp->layMaSP(), sp->layTenSP(), sp->laySoLuongTon(), "Ton kho ban dau khi tao san pham");
    if (!luuXuongDia()) { danhSachSanPham.pop_back(); danhSachGiaoDichKho.resize(gdCu); demGiaoDichKho = demGdCu; return false; }
    kiemTraVaPhatCanhBaoTuDong();
    return true;
}

bool CoSoDuLieu::capNhatSanPham(const std::string& ma, const std::string& ten, const std::string& dvt,
                               double gNhap, double gBan, double minTon, double maxTon,
                               const std::string& viTri, const std::string& moTa,
                               const std::string& nsx, const std::string& hsd) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& sp : danhSachSanPham) {
        if (sp->layMaSP() == ma) {
            const std::string tenCu = sp->layTenSP(), dvtCu = sp->layDonViTinh(), viTriCu = sp->layViTriKho(), moTaCu = sp->layMoTa();
            const double gNhapCu = sp->layGiaNhap(), gBanCu = sp->layGiaBan(), minCu = sp->layTonToiThieu(), maxCu = sp->layTonToiDa();
            auto hsdCu = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp);
            const std::string nsxCu = hsdCu ? hsdCu->layNgaySanXuat() : "", hanCu = hsdCu ? hsdCu->layHanSuDung() : "";
            sp->capNhatThongTin(ten, dvt, gNhap, gBan, minTon, maxTon, viTri, moTa);
            if (sp->layLoai() == LoaiSanPham::HAN_SU_DUNG && !hsd.empty()) {
                auto spHSD = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp);
                if (spHSD) {
                    spHSD->capNhatHSD(nsx, hsd);
                }
            }
            if (!luuXuongDia()) {
                sp->capNhatThongTin(tenCu, dvtCu, gNhapCu, gBanCu, minCu, maxCu, viTriCu, moTaCu);
                if (hsdCu) hsdCu->capNhatHSD(nsxCu, hanCu);
                return false;
            }
            kiemTraVaPhatCanhBaoTuDong();
            return true;
        }
    }
    return false;
}

bool CoSoDuLieu::capNhatTonKho(const std::string& maSP, double delta) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& sp : danhSachSanPham) {
        if (sp->layMaSP() == maSP) {
            const double tonCu = sp->laySoLuongTon();
            auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp);
            const std::vector<SanPhamHanSuDung::LoHang> loCu = hsd ? hsd->layCacLoHang() : std::vector<SanPhamHanSuDung::LoHang>{};
            sp->capNhatSoLuongTon(delta);
            ghiNhanGiaoDichKho("HOAN_TAC_HUY", "UNDO", "System-Command", maSP,
                               sp->layTenSP(), delta, "Hoan tac lenh huy don");
            const size_t gdCu = danhSachGiaoDichKho.size() - 1;
            const int demCu = demGiaoDichKho - 1;
            if (!luuXuongDia()) {
                sp->capNhatSoLuongTon(tonCu - sp->laySoLuongTon());
                if (hsd) hsd->datCacLoHang(loCu);
                danhSachGiaoDichKho.resize(gdCu); demGiaoDichKho = demCu;
                return false;
            }
            kiemTraVaPhatCanhBaoTuDong();
            return true;
        }
    }
    return false;
}

bool CoSoDuLieu::xoaSanPham(const std::string& ma) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (const auto& hd : danhSachHoaDon)
        for (const auto& ct : hd->layDanhSachChiTiet()) if (ct.layMaSP() == ma) return false;
    for (const auto& pn : danhSachPhieuNhap)
        for (const auto& ct : pn.layDanhSachChiTiet()) if (ct.maSP == ma) return false;
    for (const auto& gd : danhSachGiaoDichKho) if (gd.maSP == ma) return false;
    for (const auto& kk : danhSachKiemKe) if (kk.maSP == ma) return false;
    for (auto it = danhSachSanPham.begin(); it != danhSachSanPham.end(); ++it) {
        if ((*it)->layMaSP() == ma) {
            auto sanPhamCu = *it;
            const size_t viTri = static_cast<size_t>(std::distance(danhSachSanPham.begin(), it));
            danhSachSanPham.erase(it);
            if (!luuXuongDia()) { danhSachSanPham.insert(danhSachSanPham.begin() + viTri, sanPhamCu); return false; }
            return true;
        }
    }
    return false;
}

// 3. Phieu Nhap
std::vector<PhieuNhapKho> CoSoDuLieu::layDanhSachPhieuNhap() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    return danhSachPhieuNhap;
}

bool CoSoDuLieu::themPhieuNhap(const PhieuNhapKho& phieu) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    if (phieu.layTrangThai() == "DA_DUYET") {
        for (const auto& ct : phieu.layDanhSachChiTiet()) for (const auto& sp : danhSachSanPham)
            if (sp->layMaSP() == ct.maSP && sp->laySoLuongTon() + ct.soLuong > sp->layTonToiDa()) return false;
    }
    std::vector<std::pair<std::shared_ptr<SanPham>, double>> tonCu, vonCu;
    std::vector<std::pair<std::shared_ptr<SanPhamHanSuDung>, std::vector<SanPhamHanSuDung::LoHang>>> loCu;
    const size_t gdCu = danhSachGiaoDichKho.size(); const int demGdCu = demGiaoDichKho;
    danhSachPhieuNhap.push_back(phieu);

    if (phieu.layTrangThai() == "DA_DUYET") {
        for (const auto& ct : phieu.layDanhSachChiTiet()) {
            for (auto& sp : danhSachSanPham) {
                if (sp->layMaSP() == ct.maSP) {
                    tonCu.push_back({sp, sp->laySoLuongTon()}); vonCu.push_back({sp, sp->layGiaVonBinhQuan()});
                    if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp)) loCu.push_back({hsd, hsd->layCacLoHang()});
                    sp->nhapThemGiaVon(ct.soLuong, ct.donGiaNhap);
                    if (auto spHsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp)) spHsd->nhapLo(ct.soLuong, layNgayHienTai(), ct.hanSuDung);
                    else sp->capNhatSoLuongTon(ct.soLuong);
                    break;
                }
            }
        }
    }
    if (!luuXuongDia()) {
        danhSachPhieuNhap.pop_back();
        for (auto& x : tonCu) {
            if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(x.first)) hsd->capNhatSoLuongTon(x.second - hsd->laySoLuongTon());
            else x.first->datSoLuongTon(x.second);
        }
        for (auto& x : vonCu) x.first->datGiaVonBinhQuan(x.second);
        for (auto& x : loCu) x.first->datCacLoHang(x.second);
        danhSachGiaoDichKho.resize(gdCu); demGiaoDichKho = demGdCu;
        return false;
    }
    kiemTraVaPhatCanhBaoTuDong();
    return true;
}

bool CoSoDuLieu::duyetPhieuNhap(const std::string& maPhieu) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& phieu : danhSachPhieuNhap) {
        if (phieu.layMaPhieu() != maPhieu) continue;
        if (phieu.layTrangThai() != "CHO_DUYET") return false;
        std::vector<std::pair<std::shared_ptr<SanPham>, double>> tonCu, vonCu;
        std::vector<std::pair<std::shared_ptr<SanPhamHanSuDung>, std::vector<SanPhamHanSuDung::LoHang>>> loCu;
        const size_t gdCu = danhSachGiaoDichKho.size(); const int demGdCu = demGiaoDichKho;
        for (const auto& ct : phieu.layDanhSachChiTiet()) {
            for (const auto& sp : danhSachSanPham) if (sp->layMaSP() == ct.maSP && sp->laySoLuongTon() + ct.soLuong > sp->layTonToiDa()) return false;
        }
        for (const auto& ct : phieu.layDanhSachChiTiet()) {
            for (auto& sp : danhSachSanPham) {
                if (sp->layMaSP() != ct.maSP) continue;
                tonCu.push_back({sp, sp->laySoLuongTon()}); vonCu.push_back({sp, sp->layGiaVonBinhQuan()});
                if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp)) loCu.push_back({hsd, hsd->layCacLoHang()});
                sp->nhapThemGiaVon(ct.soLuong, ct.donGiaNhap);
                if (auto spHsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp)) spHsd->nhapLo(ct.soLuong, layNgayHienTai(), ct.hanSuDung);
                else sp->capNhatSoLuongTon(ct.soLuong);
                ghiNhanGiaoDichKho("NHAP", maPhieu, phieu.layNguoiLapPhieu(), ct.maSP, ct.tenSP, ct.soLuong, phieu.layGhiChu());
                break;
            }
        }
        const std::string trangThaiCu = phieu.layTrangThai();
        phieu.datTrangThai("DA_DUYET");
        if (!luuXuongDia()) {
            phieu.datTrangThai(trangThaiCu);
            for (auto& x : tonCu) {
                if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(x.first)) hsd->capNhatSoLuongTon(x.second - hsd->laySoLuongTon());
                else x.first->datSoLuongTon(x.second);
            }
            for (auto& x : vonCu) x.first->datGiaVonBinhQuan(x.second);
            for (auto& x : loCu) x.first->datCacLoHang(x.second);
            danhSachGiaoDichKho.resize(gdCu); demGiaoDichKho = demGdCu;
            return false;
        }
        kiemTraVaPhatCanhBaoTuDong();
        return true;
    }
    return false;
}

// 4. Hoa Don
std::vector<std::shared_ptr<HoaDon>> CoSoDuLieu::layDanhSachHoaDon() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    return danhSachHoaDon;
}

std::shared_ptr<HoaDon> CoSoDuLieu::timHoaDonTheoMa(const std::string& ma) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& hd : danhSachHoaDon) {
        if (hd->layMaHoaDon() == ma) return hd;
    }
    return nullptr;
}

bool CoSoDuLieu::themHoaDon(std::shared_ptr<HoaDon> hd) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    std::vector<std::pair<std::shared_ptr<SanPham>, double>> tonCu;
    std::vector<std::pair<std::shared_ptr<SanPhamHanSuDung>, std::vector<SanPhamHanSuDung::LoHang>>> loCu;
    const size_t soGiaoDichCu = danhSachGiaoDichKho.size();
    const int demGiaoDichCu = demGiaoDichKho;
    std::unordered_map<std::string, double> soLuongTheoMa;
    for (const auto& ct : hd->layDanhSachChiTiet()) soLuongTheoMa[ct.layMaSP()] += ct.laySoLuong();

    // Kiem tra va giam ton kho an toan
    for (const auto& [maSP, soLuong] : soLuongTheoMa) {
        bool timThay = false;
        for (auto& sp : danhSachSanPham) {
            if (sp->layMaSP() == maSP) {
                timThay = true;
                std::string loi;
                if (!sp->kiemTraXuatKho(soLuong, loi)) {
                    return false; // Loi ton kho hoac het han!
                }
                break;
            }
        }
        if (!timThay) return false;
    }

    // Tru so luong ton
    for (const auto& [maSP, soLuong] : soLuongTheoMa) {
        for (auto& sp : danhSachSanPham) {
            if (sp->layMaSP() == maSP) {
                tonCu.push_back({sp, sp->laySoLuongTon()});
                if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp)) loCu.push_back({hsd, hsd->layCacLoHang()});
                const double luongTieuHao = sp->tinhLuongTonTieuHao(soLuong);
                sp->capNhatSoLuongTon(-luongTieuHao);
                ghiNhanGiaoDichKho("XUAT_BAN", hd->layMaHoaDon(), hd->layMaKhachHang(), maSP,
                                   sp->layTenSP(), -luongTieuHao, "Dat don hang");
                break;
            }
        }
    }

    danhSachHoaDon.push_back(hd);
    if (!luuXuongDia()) {
        danhSachHoaDon.pop_back();
        for (auto& entry : tonCu) {
            if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(entry.first)) {
                hsd->capNhatSoLuongTon(entry.second - hsd->laySoLuongTon());
            } else entry.first->datSoLuongTon(entry.second);
        }
        for (auto& entry : loCu) entry.first->datCacLoHang(entry.second);
        danhSachGiaoDichKho.resize(soGiaoDichCu); demGiaoDichKho = demGiaoDichCu;
        return false;
    }
    kiemTraVaPhatCanhBaoTuDong();
    return true;
}

bool CoSoDuLieu::capNhatTrangThaiHoaDon(const std::string& maHD, const std::string& ttDon,
                                       const std::string& ttThanhToan, const std::string& nguoiXuLy) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& hd : danhSachHoaDon) {
        if (hd->layMaHoaDon() == maHD) {
            std::string trangThaiCu = hd->layTrangThaiDon();
            const std::string thanhToanCu = hd->layTrangThaiThanhToan();
            const std::string nguoiXuLyCu = hd->layNguoiXuLy();
            const std::int64_t thoiDiemGiaoCu = hd->layThoiDiemGiaoHang();
            const bool chuyenDonHopLe = ttDon.empty() ||
                (trangThaiCu == "CHO_XAC_NHAN" && (ttDon == "DA_XAC_NHAN" || ttDon == "DA_HUY")) ||
                (trangThaiCu == "DA_XAC_NHAN" && (ttDon == "DANG_GIAO" || ttDon == "DA_HUY")) ||
                (trangThaiCu == "DANG_GIAO" && ttDon == "DA_GIAO");
            if (!chuyenDonHopLe) return false;
            if (ttDon.empty() && (trangThaiCu == "DA_HUY" || trangThaiCu == "DA_TRA_HANG" || ttThanhToan.empty())) return false;
            if (ttThanhToan == "DA_THANH_TOAN" &&
                (hd->layTrangThaiThanhToan() == "DA_THANH_TOAN" || hd->layTrangThaiThanhToan() == "DA_HOAN_TIEN" ||
                 trangThaiCu == "DA_HUY" || trangThaiCu == "DA_TRA_HANG")) return false;
            if (ttThanhToan == "DA_THANH_TOAN" && ttDon.empty() &&
                (hd->layPhuongThucThanhToan() != "CHUYEN_KHOAN" || trangThaiCu != "DA_GIAO")) return false;
            if (ttThanhToan == "DA_HOAN_TIEN" &&
                (ttDon != "DA_HUY" || hd->layTrangThaiThanhToan() != "DA_THANH_TOAN")) return false;
            if (ttThanhToan != "" && ttThanhToan != "DA_THANH_TOAN" &&
                ttThanhToan != "DA_HOAN_TIEN" && ttThanhToan != "CHUA_THANH_TOAN") return false;
            if (!ttDon.empty()) hd->datTrangThaiDon(ttDon);
            if (ttDon == "DA_GIAO" && trangThaiCu != "DA_GIAO") {
                hd->datThoiDiemGiaoHang(static_cast<std::int64_t>(std::time(nullptr)));
            }
            if (!ttThanhToan.empty()) {
                hd->datTrangThaiThanhToan(ttThanhToan);
            }
            if (!nguoiXuLy.empty()) {
                hd->datNguoiXuLy(nguoiXuLy);
            }
            std::vector<std::pair<std::shared_ptr<SanPham>, double>> tonCu;
            std::vector<std::pair<std::shared_ptr<SanPhamHanSuDung>, std::vector<SanPhamHanSuDung::LoHang>>> loCu;
            const size_t gdCu = danhSachGiaoDichKho.size(); const int demGdCu = demGiaoDichKho;

            // Neu chuyen sang DA_HUY va chua hoan kho -> hoan lai ton kho an toan
            if (ttDon == "DA_HUY" && trangThaiCu != "DA_HUY") {
                for (const auto& ct : hd->layDanhSachChiTiet()) {
                    for (auto& sp : danhSachSanPham) {
                        if (sp->layMaSP() == ct.layMaSP()) {
                            tonCu.push_back({sp, sp->laySoLuongTon()});
                            if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp)) loCu.push_back({hsd, hsd->layCacLoHang()});
                            sp->capNhatSoLuongTon(sp->tinhLuongTonTieuHao(ct.laySoLuong()));
                            ghiNhanGiaoDichKho("HOAN_HUY", maHD, nguoiXuLy, ct.layMaSP(), ct.layTenSP(),
                                               sp->tinhLuongTonTieuHao(ct.laySoLuong()), "Hoan ton khi huy don");
                            break;
                        }
                    }
                }
            }
            if (!luuXuongDia()) {
                hd->datTrangThaiDon(trangThaiCu); hd->datTrangThaiThanhToan(thanhToanCu);
                hd->datNguoiXuLy(nguoiXuLyCu); hd->datThoiDiemGiaoHang(thoiDiemGiaoCu);
                for (auto& x : tonCu) {
                    if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(x.first)) hsd->capNhatSoLuongTon(x.second - hsd->laySoLuongTon());
                    else x.first->datSoLuongTon(x.second);
                }
                for (auto& x : loCu) x.first->datCacLoHang(x.second);
                danhSachGiaoDichKho.resize(gdCu); demGiaoDichKho = demGdCu;
                return false;
            }
            kiemTraVaPhatCanhBaoTuDong();
            return true;
        }
    }
    return false;
}

bool CoSoDuLieu::hoanTacHuyHoaDon(const std::string& maHD, const std::string& trangThaiDonCu,
                                  const std::string& trangThaiThanhToanCu) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    auto hdIt = std::find_if(danhSachHoaDon.begin(), danhSachHoaDon.end(), [&](const auto& hd) { return hd->layMaHoaDon() == maHD; });
    if (hdIt == danhSachHoaDon.end() || (*hdIt)->layTrangThaiDon() != "DA_HUY") return false;
    auto& hd = *hdIt;

    std::unordered_map<std::string, double> soLuongTheoMa;
    for (const auto& ct : hd->layDanhSachChiTiet()) soLuongTheoMa[ct.layMaSP()] += ct.laySoLuong();
    std::vector<std::pair<std::shared_ptr<SanPham>, double>> tonCu;
    std::vector<std::pair<std::shared_ptr<SanPhamHanSuDung>, std::vector<SanPhamHanSuDung::LoHang>>> loCu;
    std::vector<std::pair<std::shared_ptr<SanPham>, double>> luongTru;
    for (const auto& [maSP, soLuong] : soLuongTheoMa) {
        auto spIt = std::find_if(danhSachSanPham.begin(), danhSachSanPham.end(), [&](const auto& sp) { return sp->layMaSP() == maSP; });
        if (spIt == danhSachSanPham.end()) return false;
        const double canTru = (*spIt)->tinhLuongTonTieuHao(soLuong);
        if ((*spIt)->laySoLuongTon() < canTru) return false;
        luongTru.push_back({*spIt, canTru});
        tonCu.push_back({*spIt, (*spIt)->laySoLuongTon()});
        if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(*spIt)) loCu.push_back({hsd, hsd->layCacLoHang()});
    }
    const size_t gdCu = danhSachGiaoDichKho.size(); const int demGdCu = demGiaoDichKho;
    const std::string trangThaiHienTai = hd->layTrangThaiDon();
    const std::string thanhToanHienTai = hd->layTrangThaiThanhToan();
    for (const auto& [sp, canTru] : luongTru) {
        sp->capNhatSoLuongTon(-canTru);
        ghiNhanGiaoDichKho("HOAN_TAC_HUY", maHD, "System-Command", sp->layMaSP(), sp->layTenSP(), -canTru, "Hoan tac lenh huy don");
    }
    hd->datTrangThaiDon(trangThaiDonCu);
    hd->datTrangThaiThanhToan(trangThaiThanhToanCu);
    if (!luuXuongDia()) {
        hd->datTrangThaiDon(trangThaiHienTai); hd->datTrangThaiThanhToan(thanhToanHienTai);
        for (auto& x : tonCu) {
            if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(x.first)) hsd->capNhatSoLuongTon(x.second - hsd->laySoLuongTon());
            else x.first->datSoLuongTon(x.second);
        }
        for (auto& x : loCu) x.first->datCacLoHang(x.second);
        danhSachGiaoDichKho.resize(gdCu); demGiaoDichKho = demGdCu;
        return false;
    }
    kiemTraVaPhatCanhBaoTuDong();
    return true;
}

bool CoSoDuLieu::yeuCauTraHang(const std::string& maHD, const std::string& maKhachHang, std::string& thongBao) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& hd : danhSachHoaDon) {
        if (hd->layMaHoaDon() != maHD) continue;
        if (hd->layMaKhachHang() != maKhachHang) { thongBao = "Don hang khong thuoc tai khoan cua ban."; return false; }
        if (hd->layTrangThaiDon() != "DA_GIAO" || hd->layDaHoanKhoTraHang()) { thongBao = "Don hang khong o trang thai co the tra."; return false; }
        if (hd->layTrangThaiThanhToan() != "DA_THANH_TOAN") { thongBao = "Don hang chua duoc thanh toan, khong the hoan tien."; return false; }
        auto giao = hd->layThoiDiemGiaoHang();
        if (giao == 0 || std::time(nullptr) - giao > 24 * 60 * 60) { thongBao = "Da qua han tra hang 24 gio ke tu luc giao hang."; return false; }
        const std::string trangThaiCu = hd->layTrangThaiDon();
        hd->datTrangThaiDon("CHO_KIEM_TRA_TRA_HANG");
        if (!luuXuongDia()) { hd->datTrangThaiDon(trangThaiCu); return false; }
        thongBao = "Da tiep nhan yeu cau. Don cho nhan vien kiem tra hang truoc khi hoan tien va nhap lai kho.";
        return true;
    }
    thongBao = "Khong tim thay don hang.";
    return false;
}

bool CoSoDuLieu::xacNhanTraHang(const std::string& maHD, const std::string& nguoiDuyet, std::string& thongBao) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& hd : danhSachHoaDon) {
        if (hd->layMaHoaDon() != maHD) continue;
        if (hd->layTrangThaiDon() != "CHO_KIEM_TRA_TRA_HANG" || hd->layDaHoanKhoTraHang() || hd->layTrangThaiThanhToan() != "DA_THANH_TOAN") {
            thongBao = "Don khong o trang thai cho phep duyet tra hang."; return false;
        }
        for (const auto& ct : hd->layDanhSachChiTiet()) {
            auto spIt = std::find_if(danhSachSanPham.begin(), danhSachSanPham.end(), [&](const auto& p) { return p->layMaSP() == ct.layMaSP(); });
            if (spIt == danhSachSanPham.end()) { thongBao = "Khong tim thay san pham trong don."; return false; }
            if ((*spIt)->laySoLuongTon() + (*spIt)->tinhLuongTonTieuHao(ct.laySoLuong()) > (*spIt)->layTonToiDa()) { thongBao = "Ton kho vuot muc toi da neu nhap lai hang."; return false; }
            auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(*spIt);
            if (hsd && hsd->daHetHan()) { thongBao = "Hang da het han, khong the nhap lai."; return false; }
        }
        std::vector<std::pair<std::shared_ptr<SanPham>, double>> tonCu;
        std::vector<std::pair<std::shared_ptr<SanPhamHanSuDung>, std::vector<SanPhamHanSuDung::LoHang>>> loCu;
        const size_t gdCu = danhSachGiaoDichKho.size(); const int demGdCu = demGiaoDichKho;
        for (const auto& ct : hd->layDanhSachChiTiet()) for (auto& sp : danhSachSanPham) if (sp->layMaSP() == ct.layMaSP()) {
            double qty = sp->tinhLuongTonTieuHao(ct.laySoLuong());
            tonCu.push_back({sp, sp->laySoLuongTon()});
            if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(sp)) loCu.push_back({hsd, hsd->layCacLoHang()});
            sp->capNhatSoLuongTon(qty);
            ghiNhanGiaoDichKho("TRA_HANG", maHD, nguoiDuyet, ct.layMaSP(), ct.layTenSP(), qty, "Hang da duoc nhan vien kiem tra va chap nhan nhap lai");
            break;
        }
        const std::string trangThaiCu = hd->layTrangThaiDon(), thanhToanCu = hd->layTrangThaiThanhToan();
        const bool daHoanKhoCu = hd->layDaHoanKhoTraHang();
        hd->datDaHoanKhoTraHang(true); hd->datTrangThaiDon("DA_TRA_HANG"); hd->datTrangThaiThanhToan("DA_HOAN_TIEN");
        if (!luuXuongDia()) {
            hd->datDaHoanKhoTraHang(daHoanKhoCu); hd->datTrangThaiDon(trangThaiCu); hd->datTrangThaiThanhToan(thanhToanCu);
            for (auto& x : tonCu) {
                if (auto hsd = std::dynamic_pointer_cast<SanPhamHanSuDung>(x.first)) hsd->capNhatSoLuongTon(x.second - hsd->laySoLuongTon());
                else x.first->datSoLuongTon(x.second);
            }
            for (auto& x : loCu) x.first->datCacLoHang(x.second);
            danhSachGiaoDichKho.resize(gdCu); demGiaoDichKho = demGdCu;
            return false;
        }
        kiemTraVaPhatCanhBaoTuDong(); thongBao = "Da duyet tra hang, hoan tien va cap nhat ton kho."; return true;
    }
    thongBao = "Khong tim thay don hang."; return false;
}

// 5. Kiem Ke
std::vector<PhieuKiemKe> CoSoDuLieu::layDanhSachKiemKe() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    return danhSachKiemKe;
}

bool CoSoDuLieu::themPhieuKiemKe(const PhieuKiemKe& kk) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    auto spIt = std::find_if(danhSachSanPham.begin(), danhSachSanPham.end(), [&](const auto& sp) { return sp->layMaSP() == kk.maSP; });
    if (spIt == danhSachSanPham.end() || kk.tonThucTe < 0 || kk.tonThucTe > (*spIt)->layTonToiDa()) return false;
    if ((*spIt)->layLoai() == LoaiSanPham::HAN_SU_DUNG && kk.tonThucTe > (*spIt)->laySoLuongTon()) return false;
    const double tonCu = (*spIt)->laySoLuongTon();
    auto hsdCu = std::dynamic_pointer_cast<SanPhamHanSuDung>(*spIt);
    const std::vector<SanPhamHanSuDung::LoHang> loCu = hsdCu ? hsdCu->layCacLoHang() : std::vector<SanPhamHanSuDung::LoHang>{};
    const size_t gdCu = danhSachGiaoDichKho.size(); const int demGdCu = demGiaoDichKho;
    danhSachKiemKe.push_back(kk);

    // Dieu chinh lai so luong ton thuc te
    for (auto& sp : danhSachSanPham) {
        if (sp->layMaSP() == kk.maSP) {
            sp->datSoLuongTon(kk.tonThucTe);
            ghiNhanGiaoDichKho("DIEU_CHINH_KIEM_KE", kk.maKiemKe, kk.nguoiKiemKe, kk.maSP,
                               kk.tenSP, kk.chenhLech, kk.lyDo);
            break;
        }
    }

    if (!luuXuongDia()) {
        danhSachKiemKe.pop_back();
        if (hsdCu) { hsdCu->capNhatSoLuongTon(tonCu - hsdCu->laySoLuongTon()); hsdCu->datCacLoHang(loCu); }
        else (*spIt)->datSoLuongTon(tonCu);
        danhSachGiaoDichKho.resize(gdCu); demGiaoDichKho = demGdCu;
        return false;
    }
    if (std::abs(kk.chenhLech) > 0.001) {
        std::string nd = "Chenh lech kiem ke SP " + kk.maSP + ": lech " + std::to_string(kk.chenhLech) + ". Ly do: " + kk.lyDo;
        themCanhBao("CHENH_LECH_KHO", kk.maSP, kk.tenSP, nd);
    }
    kiemTraVaPhatCanhBaoTuDong();
    return true;
}

std::vector<GiaoDichKho> CoSoDuLieu::layDanhSachGiaoDichKho() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    return danhSachGiaoDichKho;
}

std::vector<NhaCungCap> CoSoDuLieu::layDanhSachNhaCungCap() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    return danhSachNhaCungCap;
}

bool CoSoDuLieu::themNhaCungCap(NhaCungCap ncc) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    if (ncc.ten.empty()) return false;
    for (const auto& item : danhSachNhaCungCap) if (item.ten == ncc.ten) return false;
    std::ostringstream id; id << "NCC" << std::setw(3) << std::setfill('0') << ++demNhaCungCap;
    ncc.maNCC = id.str(); ncc.trangThai = "DANG_HOP_TAC";
    danhSachNhaCungCap.push_back(ncc); if (!luuXuongDia()) { danhSachNhaCungCap.pop_back(); return false; } return true;
}

bool CoSoDuLieu::capNhatNhaCungCap(const NhaCungCap& ncc) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (const auto& item : danhSachNhaCungCap)
        if (item.maNCC != ncc.maNCC && item.ten == ncc.ten) return false;
    for (auto& item : danhSachNhaCungCap) if (item.maNCC == ncc.maNCC) {
        if (ncc.ten.empty()) return false;
        const NhaCungCap cu = item;
        item.ten = ncc.ten; item.soDienThoai = ncc.soDienThoai;
        item.diaChi = ncc.diaChi; item.email = ncc.email;
        if (!luuXuongDia()) { item = cu; return false; }
        return true;
    }
    return false;
}

bool CoSoDuLieu::datTrangThaiNhaCungCap(const std::string& maNCC, const std::string& trangThai) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    if (trangThai != "DANG_HOP_TAC" && trangThai != "NGUNG_HOP_TAC") return false;
    for (auto& item : danhSachNhaCungCap) if (item.maNCC == maNCC) {
        const std::string cu = item.trangThai;
        item.trangThai = trangThai; if (!luuXuongDia()) { item.trangThai = cu; return false; } return true;
    }
    return false;
}

// 6. Canh Bao (Observer)
std::vector<ThongBaoCanhBao> CoSoDuLieu::layDanhSachCanhBao() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    return danhSachCanhBao;
}

void CoSoDuLieu::themCanhBao(const std::string& loai, const std::string& maSP, const std::string& tenSP, const std::string& noiDung) {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    ThongBaoCanhBao cb;
    cb.maCanhBao = sinhMaCanhBao();
    cb.thoiGian = layThoiGianHienTai();
    cb.loaiCanhBao = loai;
    cb.maSP = maSP;
    cb.tenSP = tenSP;
    cb.noiDung = noiDung;
    cb.daDoc = false;
    danhSachCanhBao.insert(danhSachCanhBao.begin(), cb);
    trungTamCanhBao.phatThongBao(cb);
}

void CoSoDuLieu::danhDauDaDocCanhBao() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& cb : danhSachCanhBao) {
        cb.daDoc = true;
    }
}

void CoSoDuLieu::kiemTraVaPhatCanhBaoTuDong() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    for (auto& sp : danhSachSanPham) {
        auto spHsd = sp->layLoai() == LoaiSanPham::HAN_SU_DUNG
            ? std::dynamic_pointer_cast<SanPhamHanSuDung>(sp)
            : nullptr;
        const bool sapHetHan = spHsd && spHsd->sapHetHan(7);

        // Xoa canh bao da het hieu luc de ton kho phuc hoi khong con hien canh bao cu.
        danhSachCanhBao.erase(std::remove_if(danhSachCanhBao.begin(), danhSachCanhBao.end(),
            [&](const ThongBaoCanhBao& cb) {
                if (cb.maSP != sp->layMaSP()) return false;
                if (cb.loaiCanhBao == "HET_HANG") return !sp->sapHetHang();
                if (cb.loaiCanhBao == "SAP_HET_HAN") return !sapHetHan;
                return false;
            }), danhSachCanhBao.end());

        // Canh bao het hang / ton duoi min
        if (sp->sapHetHang()) {
            bool daCo = false;
            for (const auto& cb : danhSachCanhBao) {
                if (cb.maSP == sp->layMaSP() && cb.loaiCanhBao == "HET_HANG") {
                    daCo = true;
                    break;
                }
            }
            if (!daCo) {
                std::string nd = "San pham '" + sp->layTenSP() + "' ton kho con " +
                                 std::to_string((int)sp->laySoLuongTon()) + " " + sp->layDonViTinh() +
                                 " (duoi muc min " + std::to_string((int)sp->layTonToiThieu()) + ").";
                themCanhBao("HET_HANG", sp->layMaSP(), sp->layTenSP(), nd);
            }
        }

        // Canh bao can date voi SanPhamHanSuDung
        if (spHsd) {
            if (sapHetHan) {
                bool daCo = false;
                for (const auto& cb : danhSachCanhBao) {
                    if (cb.maSP == sp->layMaSP() && cb.loaiCanhBao == "SAP_HET_HAN") {
                        daCo = true;
                        break;
                    }
                }
                if (!daCo) {
                    std::string nd = "San pham '" + sp->layTenSP() + "' con " +
                                     std::to_string(spHsd->tinhSoNgayConLai()) + " ngay la het han (HSD: " +
                                     spHsd->layHanSuDung() + ")! Can uu tien xuat truoc (FEFO).";
                    themCanhBao("SAP_HET_HAN", sp->layMaSP(), sp->layTenSP(), nd);
                }
            }
        }
    }
}

// 7. Thong ke Bao Cao
std::string CoSoDuLieu::taoBaoCaoTongHopJSON() {
    std::lock_guard<std::recursive_mutex> lock(mutexDuLieu);
    double tongDoanhThu = 0.0;
    int donThanhCong = 0;
    int donChoXacNhan = 0;
    int donDaHuy = 0;

    for (const auto& hd : danhSachHoaDon) {
        if (hd->layTrangThaiDon() == "DA_GIAO" && hd->layTrangThaiThanhToan() == "DA_THANH_TOAN") {
            tongDoanhThu += hd->layTongTienThanhToan();
            donThanhCong++;
        }
        if (hd->layTrangThaiDon() == "CHO_XAC_NHAN") {
            donChoXacNhan++;
        }
        if (hd->layTrangThaiDon() == "DA_HUY") {
            donDaHuy++;
        }
    }

    double tongGiaTriTon = 0.0;
    int soLuongSanPhamCanhBao = 0;
    for (const auto& sp : danhSachSanPham) {
        tongGiaTriTon += (sp->layGiaVonBinhQuan() * sp->laySoLuongTon());
        if (sp->sapHetHang()) {
            soLuongSanPhamCanhBao++;
        }
    }

    std::ostringstream ss;
    ss << "{"
       << "\"tongDoanhThu\":" << std::fixed << std::setprecision(0) << tongDoanhThu << ","
       << "\"tongDonHang\":" << danhSachHoaDon.size() << ","
       << "\"donThanhCong\":" << donThanhCong << ","
       << "\"donChoXacNhan\":" << donChoXacNhan << ","
       << "\"donDaHuy\":" << donDaHuy << ","
       << "\"tongSoSanPham\":" << danhSachSanPham.size() << ","
       << "\"tongGiaTriTon\":" << std::fixed << std::setprecision(0) << tongGiaTriTon << ","
       << "\"soLuongCanhBao\":" << soLuongSanPhamCanhBao << ","
       << "\"soLuongNhapKho\":" << danhSachPhieuNhap.size() << ","
       << "\"soLuongKiemKe\":" << danhSachKiemKe.size()
       << "}";
    return ss.str();
}
