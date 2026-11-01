#include "MayChuWeb.h"
#include "JsonUtil.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <filesystem>

// Ham helper trich xuat gia tri chuoi tu JSON don gian
static std::string layGiaTriChuoiJSON(const std::string& json, const std::string& key) {
    const std::string pattern = "\"" + key + "\"";
    const size_t keyPos = json.find(pattern);
    if (keyPos == std::string::npos) return "";

    const size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return "";

    size_t pos = json.find_first_not_of(" \t\r\n", colonPos + 1);
    if (pos == std::string::npos) return "";

    if (json[pos] == '"') {
        std::string value;
        ++pos;
        for (; pos < json.size(); ++pos) {
            char c = json[pos];
            if (c == '\\' && pos + 1 < json.size()) {
                ++pos;
                char nxt = json[pos];
                switch (nxt) {
                    case '"': value.push_back('"'); break;
                    case '\\': value.push_back('\\'); break;
                    case '/': value.push_back('/'); break;
                    case 'b': value.push_back('\b'); break;
                    case 'f': value.push_back('\f'); break;
                    case 'n': value.push_back('\n'); break;
                    case 'r': value.push_back('\r'); break;
                    case 't': value.push_back('\t'); break;
                    case 'u': {
                        if (pos + 4 < json.size()) {
                            char hex[5] = {json[pos + 1], json[pos + 2], json[pos + 3], json[pos + 4], '\0'};
                            int code = 0;
                            if (std::sscanf(hex, "%x", &code) == 1) {
                                value.push_back(static_cast<char>(code));
                                pos += 4;
                            } else {
                                value.push_back('u');
                            }
                        } else {
                            value.push_back('u');
                        }
                        break;
                    }
                    default: value.push_back(nxt); break;
                }
                continue;
            }
            if (c == '"') break;
            value.push_back(c);
        }
        return value;
    }

    const size_t endPos = json.find_first_of(",}\r\n\t ", pos);
    return (endPos == std::string::npos) ? json.substr(pos) : json.substr(pos, endPos - pos);
}

static double layGiaTriSoJSON(const std::string& json, const std::string& key, double macDinh = 0.0) {
    std::string val = layGiaTriChuoiJSON(json, key);
    if (val.empty()) return macDinh;
    try {
        return std::stod(val);
    } catch (...) {
        return macDinh;
    }
}

// URL Decode
static std::string giaiMaURL(const std::string& src) {
    std::string res;
    res.reserve(src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        if (src[i] == '%') {
            if (i + 2 < src.size() && std::isxdigit(static_cast<unsigned char>(src[i + 1])) && std::isxdigit(static_cast<unsigned char>(src[i + 2]))) {
                char hex[3] = {src[i + 1], src[i + 2], '\0'};
                int code = 0;
                std::sscanf(hex, "%x", &code);
                res.push_back(static_cast<char>(code));
                i += 2;
            } else {
                res.push_back('%');
            }
        } else if (src[i] == '+') {
            res.push_back(' ');
        } else {
            res.push_back(src[i]);
        }
    }
    return res;
}

MayChuWeb::MayChuWeb(int port, const std::string& frontendDir)
    : congMang(port), socketLangNghe(INVALID_SOCKET), dangChay(false), thuMucFrontend(frontendDir) {
    csdl = CoSoDuLieu::layInstance();
    authService = std::make_shared<AuthService>(csdl);
    quanLyBanHang = std::make_shared<QuanLyBanHang>(csdl);
}

MayChuWeb::~MayChuWeb() {
    dung();
}

std::string MayChuWeb::layMimeType(const std::string& duongDan) {
    if (duongDan.ends_with(".html") || duongDan.ends_with(".htm")) return "text/html; charset=utf-8";
    if (duongDan.ends_with(".css")) return "text/css; charset=utf-8";
    if (duongDan.ends_with(".js")) return "application/javascript; charset=utf-8";
    if (duongDan.ends_with(".json")) return "application/json; charset=utf-8";
    if (duongDan.ends_with(".png")) return "image/png";
    if (duongDan.ends_with(".jpg") || duongDan.ends_with(".jpeg")) return "image/jpeg";
    if (duongDan.ends_with(".svg")) return "image/svg+xml";
    if (duongDan.ends_with(".ico")) return "image/x-icon";
    return "text/plain; charset=utf-8";
}

YeuCauHTTP MayChuWeb::phanTichYeuCau(const std::string& raw) {
    YeuCauHTTP req;
    std::istringstream stream(raw);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        lineStream >> req.phuongThuc >> req.duongDan;

        size_t qPos = req.duongDan.find('?');
        if (qPos != std::string::npos) {
            req.chuoiTruyVan = req.duongDan.substr(qPos + 1);
            req.duongDan = req.duongDan.substr(0, qPos);

            // Phan tich query string
            std::istringstream qStream(req.chuoiTruyVan);
            std::string pair;
            while (std::getline(qStream, pair, '&')) {
                size_t eqPos = pair.find('=');
                if (eqPos != std::string::npos) {
                    req.thamSo[pair.substr(0, eqPos)] = giaiMaURL(pair.substr(eqPos + 1));
                }
            }
        }
    }

    // Doc cac headers
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        size_t cPos = line.find(':');
        if (cPos != std::string::npos) {
            std::string k = line.substr(0, cPos);
            std::string v = line.substr(cPos + 1);
            std::transform(k.begin(), k.end(), k.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (!v.empty() && v.back() == '\r') v.pop_back();
            while (!v.empty() && v.front() == ' ') v.erase(0, 1);
            req.tieuDe[k] = v;
        }
    }

    // Doc body
    std::string bodyContent;
    char ch;
    while (stream.get(ch)) {
        bodyContent += ch;
    }
    req.body = bodyContent;

    return req;
}

PhanHoiHTTP MayChuWeb::dinhTuyenYeuCau(const YeuCauHTTP& req) {
    if (req.phuongThuc == "OPTIONS") {
        return PhanHoiHTTP(200, "text/plain", "OK");
    }

    // 1. Dinh tuyen cac API
    if (req.duongDan == "/api/dang-nhap" && req.phuongThuc == "POST") return xuLyDangNhap(req);
    if (req.duongDan == "/api/dang-ky" && req.phuongThuc == "POST") return xuLyDangKy(req);
    if (req.duongDan == "/api/xac-thuc-otp" && req.phuongThuc == "POST") return xuLyXacThucOTP(req);
    if (req.duongDan == "/api/lay-otp-hien-tai" && req.phuongThuc == "GET") {
        const char* demoOtp = std::getenv("WMS_ENABLE_DEMO_OTP");
        if (!demoOtp || std::string(demoOtp) != "1")
            return PhanHoiHTTP(404, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Endpoint OTP demo da tat.\"}");
        return xuLyLayOTPHienTai(req);
    }
    if (req.duongDan == "/api/quen-mat-khau" && req.phuongThuc == "POST") return xuLyQuenMatKhau(req);
    if (req.duongDan == "/api/dat-lai-mat-khau" && req.phuongThuc == "POST") return xuLyDatLaiMatKhau(req);

    // Kiem tra quyen tai backend; cac kiem tra giao dien khong thay the duoc buoc nay.
    const bool apiCongKhai = req.duongDan == "/api/san-pham" && req.phuongThuc == "GET";
    if (req.duongDan.rfind("/api/", 0) == 0 && !apiCongKhai) {
        auto user = layNguoiDungTuYeuCau(req);
        if (!user) return PhanHoiHTTP(401, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Vui long dang nhap lai.\"}");

        bool duocPhep = false;
        const auto& path = req.duongDan;
        if (path.rfind("/api/nguoi-dung", 0) == 0 || path == "/api/bao-cao") {
            duocPhep = user->coQuyenQuanTri();
        } else if (path == "/api/san-pham/them" || path == "/api/san-pham/sua" || path == "/api/san-pham/xoa") {
            duocPhep = user->coQuyenQuanTri();
        } else if (path.rfind("/api/san-pham/", 0) == 0) {
            duocPhep = user->coQuyenQuanTri() || user->coQuyenKho() || user->coQuyenBanHang();
        } else if (path.rfind("/api/kho/", 0) == 0) {
            duocPhep = user->coQuyenQuanTri() || user->coQuyenKho();
        } else if (path.rfind("/api/nha-cung-cap", 0) == 0) {
            duocPhep = user->coQuyenQuanTri() || user->coQuyenKho();
        } else if (path == "/api/hoa-don") {
            duocPhep = user->coQuyenQuanTri() || user->coQuyenBanHang() || user->coQuyenMuaHang();
        } else if (path == "/api/don-hang/dat-hang" || path == "/api/don-hang/tra-hang") {
            duocPhep = user->coQuyenQuanTri() || user->coQuyenMuaHang();
        } else if (path == "/api/don-hang/duyet-tra-hang") {
            duocPhep = user->coQuyenQuanTri() || user->coQuyenBanHang() || user->coQuyenKho();
        } else if (path.rfind("/api/don-hang/", 0) == 0) {
            duocPhep = user->coQuyenQuanTri() || user->coQuyenBanHang();
        }
        if (!duocPhep) return PhanHoiHTTP(403, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Tai khoan khong co quyen thuc hien thao tac nay.\"}");
    }

    if (req.duongDan == "/api/nguoi-dung" && req.phuongThuc == "GET") return xuLyLayDanhSachNguoiDung(req);
    if (req.duongDan == "/api/nguoi-dung/them" && req.phuongThuc == "POST") return xuLyThemNguoiDung(req);
    if (req.duongDan == "/api/nguoi-dung/sua" && req.phuongThuc == "POST") return xuLySuaNguoiDung(req);
    if (req.duongDan == "/api/nguoi-dung/xoa" && req.phuongThuc == "POST") return xuLyXoaNguoiDung(req);
    if (req.duongDan == "/api/nguoi-dung/trang-thai" && req.phuongThuc == "POST") return xuLyTrangThaiNguoiDung(req);

    if (req.duongDan == "/api/san-pham" && req.phuongThuc == "GET") return xuLyLayDanhSachSanPham(req);
    if (req.duongDan == "/api/san-pham/them" && req.phuongThuc == "POST") return xuLyThemSanPham(req);
    if (req.duongDan == "/api/san-pham/sua" && req.phuongThuc == "POST") return xuLySuaSanPham(req);
    if (req.duongDan == "/api/san-pham/xoa" && req.phuongThuc == "POST") return xuLyXoaSanPham(req);

    if (req.duongDan == "/api/nha-cung-cap" && req.phuongThuc == "GET") return xuLyNhaCungCap(req);
    if (req.duongDan == "/api/nha-cung-cap" && req.phuongThuc == "POST") return xuLyNhaCungCap(req);
    if (req.duongDan == "/api/nha-cung-cap/trang-thai" && req.phuongThuc == "POST") return xuLyTrangThaiNhaCungCap(req);

    if (req.duongDan == "/api/kho/nhap" && req.phuongThuc == "GET") return xuLyLayPhieuNhap(req);
    if (req.duongDan == "/api/kho/nhap" && req.phuongThuc == "POST") return xuLyThemPhieuNhap(req);
    if (req.duongDan == "/api/kho/nhap/duyet" && req.phuongThuc == "POST") return xuLyDuyetPhieuNhap(req);

    if (req.duongDan == "/api/kho/kiem-ke" && req.phuongThuc == "GET") return xuLyLayKiemKe(req);
    if (req.duongDan == "/api/kho/kiem-ke" && req.phuongThuc == "POST") return xuLyThemKiemKe(req);
    if (req.duongDan == "/api/kho/giao-dich" && req.phuongThuc == "GET") return xuLyLayGiaoDichKho(req);

    if (req.duongDan == "/api/kho/canh-bao" && req.phuongThuc == "GET") return xuLyLayCanhBao(req);
    if (req.duongDan == "/api/kho/canh-bao/da-doc" && req.phuongThuc == "POST") return xuLyDocTatCaCanhBao(req);

    if (req.duongDan == "/api/hoa-don" && req.phuongThuc == "GET") return xuLyLayHoaDon(req);
    if (req.duongDan == "/api/don-hang/dat-hang" && req.phuongThuc == "POST") return xuLyDatHang(req);
    if (req.duongDan == "/api/don-hang/xac-nhan" && req.phuongThuc == "POST") return xuLyXacNhanDon(req);
    if (req.duongDan == "/api/don-hang/giao-hang" && req.phuongThuc == "POST") return xuLyGiaoHang(req);
    if (req.duongDan == "/api/don-hang/tra-hang" && req.phuongThuc == "POST") return xuLyTraHang(req);
    if (req.duongDan == "/api/don-hang/duyet-tra-hang" && req.phuongThuc == "POST") return xuLyDuyetTraHang(req);
    if (req.duongDan == "/api/don-hang/thanh-toan" && req.phuongThuc == "POST") return xuLyThanhToan(req);
    if (req.duongDan == "/api/don-hang/huy" && req.phuongThuc == "POST") return xuLyHuyDon(req);
    if (req.duongDan == "/api/don-hang/hoan-tac" && req.phuongThuc == "POST") return xuLyHoanTacHuy(req);

    if (req.duongDan == "/api/bao-cao" && req.phuongThuc == "GET") return xuLyBaoCao(req);

    // 2. Phuc vu Static Files
    std::string duongDanFile = req.duongDan;
    if (duongDanFile == "/" || duongDanFile.empty()) {
        duongDanFile = "/index.html";
    }

    namespace fs = std::filesystem;
    std::string relativePath = duongDanFile;
    while (!relativePath.empty() && (relativePath.front() == '/' || relativePath.front() == '\\')) relativePath.erase(relativePath.begin());
    fs::path fullPath;
    std::ifstream file;
    const fs::path roots[] = {fs::absolute(thuMucFrontend), fs::absolute(fs::path("..") / thuMucFrontend)};
    for (const auto& rootInput : roots) {
        std::error_code ec;
        const fs::path root = fs::weakly_canonical(rootInput, ec);
        if (ec) continue;
        const fs::path candidate = fs::weakly_canonical(root / relativePath, ec);
        if (ec) continue;
        const fs::path relative = candidate.lexically_relative(root);
        if (relative.empty() || relative.is_absolute() || *relative.begin() == "..") continue;
        if (!fs::is_regular_file(candidate, ec) || ec) continue;
        file.clear();
        file.open(candidate, std::ios::binary);
        if (file.is_open()) { fullPath = candidate; break; }
    }
    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        return PhanHoiHTTP(200, layMimeType(duongDanFile), ss.str());
    }

    return PhanHoiHTTP(404, "text/html; charset=utf-8", "<h1>404 Not Found - WMS Web Server</h1>");
}

std::shared_ptr<NguoiDung> MayChuWeb::layNguoiDungTuYeuCau(const YeuCauHTTP& req) {
    auto auth = req.tieuDe.find("authorization");
    const std::string prefix = "Bearer ";
    if (auth == req.tieuDe.end() || auth->second.rfind(prefix, 0) != 0) return nullptr;
    return authService->timNguoiDungTheoToken(auth->second.substr(prefix.size()));
}

void MayChuWeb::xuLyKetNoi(SOCKET clientSocket) {
    constexpr size_t maxRequestSize = 1024 * 1024;
    std::string rawRequest;
    size_t requiredSize = 0;
    char buffer[8192];

    while (rawRequest.size() < maxRequestSize) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) break;
        rawRequest.append(buffer, static_cast<size_t>(bytesReceived));

        if (requiredSize == 0) {
            size_t headerEnd = rawRequest.find("\r\n\r\n");
            size_t separatorSize = 4;
            if (headerEnd == std::string::npos) {
                headerEnd = rawRequest.find("\n\n");
                separatorSize = 2;
            }
            if (headerEnd != std::string::npos) {
                requiredSize = headerEnd + separatorSize;
                std::string headers = rawRequest.substr(0, headerEnd);
                std::transform(headers.begin(), headers.end(), headers.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                const std::string contentLengthKey = "content-length:";
                const size_t contentLengthPos = headers.find(contentLengthKey);
                if (contentLengthPos != std::string::npos) {
                    const size_t valueStart = contentLengthPos + contentLengthKey.size();
                    const size_t valueEnd = headers.find('\n', valueStart);
                    const std::string value = headers.substr(valueStart, valueEnd - valueStart);
                    try {
                        requiredSize += static_cast<size_t>(std::stoull(value));
                    } catch (...) {
                        requiredSize = maxRequestSize + 1;
                    }
                }
            }
        }
        if (requiredSize != 0 && rawRequest.size() >= requiredSize) break;
    }

    if (!rawRequest.empty() && rawRequest.size() <= maxRequestSize) {
        YeuCauHTTP req = phanTichYeuCau(rawRequest);
        PhanHoiHTTP resp = dinhTuyenYeuCau(req);

        std::ostringstream headerStream;
        headerStream << "HTTP/1.1 " << resp.maTrangThai << " " << (resp.maTrangThai == 200 ? "OK" : "Error") << "\r\n";
        headerStream << "Content-Type: " << resp.loaiNoiDung << "\r\n";
        headerStream << "Content-Length: " << resp.noiDung.size() << "\r\n";
        headerStream << "Access-Control-Allow-Origin: *\r\n";
        headerStream << "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n";
        headerStream << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
        headerStream << "Connection: close\r\n\r\n";

        std::string headers = headerStream.str();
        send(clientSocket, headers.c_str(), (int)headers.size(), 0);
        send(clientSocket, resp.noiDung.c_str(), (int)resp.noiDung.size(), 0);
    }
    closesocket(clientSocket);
}

bool MayChuWeb::khoiDong() {
    WSADATA wsaData;
    int wsResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsResult != 0) {
        std::cerr << "Loi khoi tao Winsock: " << wsResult << std::endl;
        return false;
    }

    socketLangNghe = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketLangNghe == INVALID_SOCKET) {
        std::cerr << "Loi tao socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    BOOL optval = TRUE;
    setsockopt(socketLangNghe, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));

    sockaddr_in diaChi;
    diaChi.sin_family = AF_INET;
    const char* demoOtp = std::getenv("WMS_ENABLE_DEMO_OTP");
    // Khi bat endpoint OTP demo, chi bind loopback de khong the truy cap tu Internet/LAN.
    diaChi.sin_addr.s_addr = (demoOtp && std::string(demoOtp) == "1") ? htonl(INADDR_LOOPBACK) : INADDR_ANY;
    diaChi.sin_port = htons(congMang);

    if (bind(socketLangNghe, (sockaddr*)&diaChi, sizeof(diaChi)) == SOCKET_ERROR) {
        std::cerr << "Loi bind port " << congMang << ": " << WSAGetLastError() << std::endl;
        closesocket(socketLangNghe);
        WSACleanup();
        return false;
    }

    if (listen(socketLangNghe, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Loi listen: " << WSAGetLastError() << std::endl;
        closesocket(socketLangNghe);
        WSACleanup();
        return false;
    }

    dangChay = true;
    std::cout << "==========================================================" << std::endl;
    std::cout << " MAY CHU WEB WMS DANG CHAY TAI: http://localhost:" << congMang << std::endl;
    std::cout << " Mau giao dien: Xanh nhat & Trang (Light Blue & White)" << std::endl;
    std::cout << " Mo trinh duyet va truy cap de bat dau su dung." << std::endl;
    std::cout << "==========================================================" << std::endl;

    while (dangChay) {
        SOCKET clientSocket = accept(socketLangNghe, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            if (!dangChay) break;
            continue;
        }
        workerThreads.emplace_back([this, clientSocket]() {
            this->xuLyKetNoi(clientSocket);
        });
    }

    for (auto& worker : workerThreads) if (worker.joinable()) worker.join();
    workerThreads.clear();
    WSACleanup();

    return true;
}

void MayChuWeb::dung() {
    dangChay = false;
    if (socketLangNghe != INVALID_SOCKET) {
        closesocket(socketLangNghe);
        socketLangNghe = INVALID_SOCKET;
    }
}

// -------------------------------------------------------------
// CAC HAM XU LY REST API (API Handlers)
// -------------------------------------------------------------

PhanHoiHTTP MayChuWeb::xuLyDangNhap(const YeuCauHTTP& req) {
    std::string user = layGiaTriChuoiJSON(req.body, "tenDangNhap");
    std::string pass = layGiaTriChuoiJSON(req.body, "matKhau");

    auto kq = authService->dangNhap(user, pass);
    std::ostringstream ss;
    if (kq.thanhCong && kq.nguoiDung) {
        ss << "{\"thanhCong\":true,"
           << "\"thongBao\":\"" << kq.thongBao << "\","
           << "\"token\":\"" << kq.token << "\","
           << "\"nguoiDung\":" << kq.nguoiDung->xuatThongTinJSON()
           << "}";
        return PhanHoiHTTP(200, "application/json", ss.str());
    } else {
        ss << "{\"thanhCong\":false,\"thongBao\":\"" << kq.thongBao << "\"}";
        return PhanHoiHTTP(400, "application/json", ss.str());
    }
}

PhanHoiHTTP MayChuWeb::xuLyDangKy(const YeuCauHTTP& req) {
    std::string user = layGiaTriChuoiJSON(req.body, "tenDangNhap");
    std::string pass = layGiaTriChuoiJSON(req.body, "matKhau");
    std::string ten = layGiaTriChuoiJSON(req.body, "hoTen");
    std::string email = layGiaTriChuoiJSON(req.body, "email");
    std::string sdt = layGiaTriChuoiJSON(req.body, "soDienThoai");

    auto kq = authService->dangKy(user, pass, ten, email, sdt);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (kq.thanhCong ? "true" : "false")
       << ",\"thongBao\":\"" << kq.thongBao << "\"}";
    return PhanHoiHTTP(kq.thanhCong ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyXacThucOTP(const YeuCauHTTP& req) {
    std::string email = layGiaTriChuoiJSON(req.body, "email");
    std::string otp = layGiaTriChuoiJSON(req.body, "otp");
    std::string tb;

    bool ok = authService->xacThucTaiKhoanOTP(email, otp, tb);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << tb << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyLayOTPHienTai(const YeuCauHTTP& req) {
    auto it = req.thamSo.find("email");
    std::string email = (it != req.thamSo.end()) ? it->second : "";
    std::string otp = OTPService::layOTPHienTai(email);
    std::ostringstream ss;
    ss << "{\"thanhCong\":true,\"email\":\"" << email << "\",\"otp\":\"" << otp << "\"}";
    return PhanHoiHTTP(200, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyQuenMatKhau(const YeuCauHTTP& req) {
    std::string email = layGiaTriChuoiJSON(req.body, "email");
    std::string tb;

    bool ok = authService->yeuCauQuenMatKhau(email, tb);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << tb << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyDatLaiMatKhau(const YeuCauHTTP& req) {
    std::string email = layGiaTriChuoiJSON(req.body, "email");
    std::string otp = layGiaTriChuoiJSON(req.body, "otp");
    std::string passMoi = layGiaTriChuoiJSON(req.body, "matKhauMoi");
    std::string tb;

    bool ok = authService->datLaiMatKhau(email, otp, passMoi, tb);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << tb << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyLayDanhSachNguoiDung(const YeuCauHTTP& /*req*/) {
    auto list = csdl->layDanhSachNguoiDung();
    std::ostringstream ss;
    ss << "{\"thanhCong\":true,\"danhSach\":[";
    for (size_t i = 0; i < list.size(); ++i) {
        ss << list[i]->xuatThongTinJSON();
        if (i + 1 < list.size()) ss << ",";
    }
    ss << "]}";
    return PhanHoiHTTP(200, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyThemNguoiDung(const YeuCauHTTP& req) {
    std::string user = layGiaTriChuoiJSON(req.body, "tenDangNhap");
    std::string pass = layGiaTriChuoiJSON(req.body, "matKhau");
    std::string ten = layGiaTriChuoiJSON(req.body, "hoTen");
    std::string email = layGiaTriChuoiJSON(req.body, "email");
    std::string sdt = layGiaTriChuoiJSON(req.body, "soDienThoai");
    std::string vaiTro = layGiaTriChuoiJSON(req.body, "vaiTro");

    std::string maND = csdl->sinhMaNguoiDung();
    std::shared_ptr<NguoiDung> nd = nullptr;

    if (vaiTro == "ADMIN") {
        nd = std::make_shared<QuanTriVien>(maND, user, pass, ten, email, sdt);
    } else if (vaiTro == "QUAN_LY_KHO") {
        nd = std::make_shared<NguoiQuanLyKho>(maND, user, pass, ten, email, sdt);
    } else if (vaiTro == "BAN_HANG") {
        nd = std::make_shared<NhanVienBanHang>(maND, user, pass, ten, email, sdt);
    } else {
        nd = std::make_shared<KhachHang>(maND, user, pass, ten, email, sdt, 0.0);
    }

    bool ok = csdl->themNguoiDung(nd);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Them nguoi dung thanh cong!" : "Ten dang nhap hoac email da ton tai!") << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLySuaNguoiDung(const YeuCauHTTP& req) {
    std::string ma = layGiaTriChuoiJSON(req.body, "maNguoiDung");
    std::string ten = layGiaTriChuoiJSON(req.body, "hoTen");
    std::string email = layGiaTriChuoiJSON(req.body, "email");
    std::string sdt = layGiaTriChuoiJSON(req.body, "soDienThoai");
    std::string pass = layGiaTriChuoiJSON(req.body, "matKhau");

    bool ok = csdl->capNhatNguoiDung(ma, ten, email, sdt, pass);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Cap nhat thanh cong!" : "Khong tim thay nguoi dung!") << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyXoaNguoiDung(const YeuCauHTTP& req) {
    std::string ma = layGiaTriChuoiJSON(req.body, "maNguoiDung");
    bool ok = csdl->xoaNguoiDung(ma);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Xoa tai khoan thanh cong!" : "Khong the xoa tai khoan!") << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyTrangThaiNguoiDung(const YeuCauHTTP& req) {
    const bool ok = csdl->datTrangThaiNguoiDung(layGiaTriChuoiJSON(req.body, "maNguoiDung"),
        layGiaTriChuoiJSON(req.body, "trangThai"));
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", std::string("{\"thanhCong\":") +
        (ok ? "true" : "false") + ",\"thongBao\":\"" + (ok ? "Da cap nhat trang thai tai khoan." : "Khong cap nhat duoc tai khoan.") + "\"}");
}

PhanHoiHTTP MayChuWeb::xuLyLayDanhSachSanPham(const YeuCauHTTP& /*req*/) {
    auto list = csdl->layDanhSachSanPham();
    std::ostringstream ss;
    ss << "{\"thanhCong\":true,\"danhSach\":[";
    for (size_t i = 0; i < list.size(); ++i) {
        ss << list[i]->xuatThongTinJSON();
        if (i + 1 < list.size()) ss << ",";
    }
    ss << "]}";
    return PhanHoiHTTP(200, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyThemSanPham(const YeuCauHTTP& req) {
    std::string ma = layGiaTriChuoiJSON(req.body, "maSP");
    std::string ten = layGiaTriChuoiJSON(req.body, "tenSP");
    std::string loai = layGiaTriChuoiJSON(req.body, "loaiSP");
    std::string dvt = layGiaTriChuoiJSON(req.body, "donViTinh");
    double gNhap = layGiaTriSoJSON(req.body, "giaNhap", 0);
    double gBan = layGiaTriSoJSON(req.body, "giaBan", 0);
    double slTon = layGiaTriSoJSON(req.body, "soLuongTon", 0);
    double minTon = layGiaTriSoJSON(req.body, "tonToiThieu", 5);
    double maxTon = layGiaTriSoJSON(req.body, "tonToiDa", 500);
    std::string viTri = layGiaTriChuoiJSON(req.body, "viTriKho");
    std::string moTa = layGiaTriChuoiJSON(req.body, "moTa");
    std::string nsx = layGiaTriChuoiJSON(req.body, "ngaySanXuat");
    std::string hsd = layGiaTriChuoiJSON(req.body, "hanSuDung");
    double trongLuong = layGiaTriSoJSON(req.body, "trongLuong", 1.0);
    double haoHut = layGiaTriSoJSON(req.body, "heSoHaoHut", 0.0);

    if (ten.empty() || gNhap < 0 || gBan <= 0 || slTon < 0 || minTon < 0 || maxTon < minTon) {
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Du lieu san pham khong hop le: gia ban phai lon hon 0, ton va gia nhap khong am.\"}");
    }
    if (slTon > maxTon) return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Ton ban dau vuot ton toi da.\"}");
    if (loai == "HAN_SU_DUNG" && (!ngayISOHopLe(nsx) || !ngayISOHopLe(hsd) || hsd < nsx || nsx > ngayHienTaiISO() || hsd < ngayHienTaiISO()))
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Ngay san xuat va han su dung phai dung YYYY-MM-DD, han su dung khong truoc ngay san xuat.\"}");
    if (loai == "THEO_CAN" && (trongLuong <= 0 || haoHut < 0 || haoHut > 1))
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Trong luong phai lon hon 0 va he so hao hut nam trong 0..1.\"}");

    if (ma.empty()) {
        ma = "SP" + std::to_string(csdl->layDanhSachSanPham().size() + 1);
    }

    std::shared_ptr<SanPham> sp = nullptr;
    if (loai == "HAN_SU_DUNG") {
        sp = std::make_shared<SanPhamHanSuDung>(ma, ten, dvt, gNhap, gBan, slTon, nsx, hsd, minTon, maxTon, viTri, moTa);
    } else if (loai == "THEO_CAN") {
        sp = std::make_shared<SanPhamTheoCan>(ma, ten, dvt, gNhap, gBan, slTon, trongLuong, haoHut, minTon, maxTon, viTri, moTa);
    } else {
        sp = std::make_shared<SanPhamThuong>(ma, ten, dvt, gNhap, gBan, slTon, minTon, maxTon, viTri, moTa);
    }

    bool ok = csdl->themSanPham(sp);
    std::string thongBao = ok ? "Them san pham thanh cong!" : csdl->layLoiLuuGanNhat();
    if (!ok && thongBao.empty()) thongBao = "Ma san pham da ton tai!";
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << jsonEscape(thongBao) << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLySuaSanPham(const YeuCauHTTP& req) {
    std::string ma = layGiaTriChuoiJSON(req.body, "maSP");
    std::string ten = layGiaTriChuoiJSON(req.body, "tenSP");
    std::string dvt = layGiaTriChuoiJSON(req.body, "donViTinh");
    double gNhap = layGiaTriSoJSON(req.body, "giaNhap");
    double gBan = layGiaTriSoJSON(req.body, "giaBan");
    double minTon = layGiaTriSoJSON(req.body, "tonToiThieu", 5);
    double maxTon = layGiaTriSoJSON(req.body, "tonToiDa", 500);
    std::string viTri = layGiaTriChuoiJSON(req.body, "viTriKho");
    std::string moTa = layGiaTriChuoiJSON(req.body, "moTa");
    std::string nsx = layGiaTriChuoiJSON(req.body, "ngaySanXuat");
    std::string hsd = layGiaTriChuoiJSON(req.body, "hanSuDung");

    if (ten.empty() || gNhap < 0 || gBan <= 0 || minTon < 0 || maxTon < minTon) {
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Du lieu san pham khong hop le.\"}");
    }

    bool ok = csdl->capNhatSanPham(ma, ten, dvt, gNhap, gBan, minTon, maxTon, viTri, moTa, nsx, hsd);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Cap nhat san pham thanh cong!" : "Khong tim thay san pham!") << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyXoaSanPham(const YeuCauHTTP& req) {
    std::string ma = layGiaTriChuoiJSON(req.body, "maSP");
    bool ok = csdl->xoaSanPham(ma);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Xoa san pham thanh cong!" : "Khong the xoa san pham!") << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyNhaCungCap(const YeuCauHTTP& req) {
    if (req.phuongThuc == "GET") {
        const auto list = csdl->layDanhSachNhaCungCap();
        std::ostringstream ss; ss << "{\"thanhCong\":true,\"danhSach\":[";
        for (size_t i = 0; i < list.size(); ++i) { ss << list[i].xuatJSON(); if (i + 1 < list.size()) ss << ","; }
        ss << "]}";
        return PhanHoiHTTP(200, "application/json", ss.str());
    }
    NhaCungCap ncc{layGiaTriChuoiJSON(req.body, "maNCC"), layGiaTriChuoiJSON(req.body, "ten"),
        layGiaTriChuoiJSON(req.body, "soDienThoai"), layGiaTriChuoiJSON(req.body, "diaChi"),
        layGiaTriChuoiJSON(req.body, "email"), "DANG_HOP_TAC"};
    const bool ok = ncc.maNCC.empty() ? csdl->themNhaCungCap(ncc) : csdl->capNhatNhaCungCap(ncc);
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", std::string("{\"thanhCong\":") +
        (ok ? "true" : "false") + ",\"thongBao\":\"" + (ok ? "Da luu nha cung cap." : "Khong luu duoc nha cung cap.") + "\"}");
}

PhanHoiHTTP MayChuWeb::xuLyTrangThaiNhaCungCap(const YeuCauHTTP& req) {
    const bool ok = csdl->datTrangThaiNhaCungCap(layGiaTriChuoiJSON(req.body, "maNCC"),
        layGiaTriChuoiJSON(req.body, "trangThai"));
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", std::string("{\"thanhCong\":") +
        (ok ? "true" : "false") + ",\"thongBao\":\"" + (ok ? "Da cap nhat trang thai." : "Khong cap nhat duoc trang thai.") + "\"}");
}

PhanHoiHTTP MayChuWeb::xuLyLayPhieuNhap(const YeuCauHTTP& /*req*/) {
    auto list = csdl->layDanhSachPhieuNhap();
    std::ostringstream ss;
    ss << "{\"thanhCong\":true,\"danhSach\":[";
    for (size_t i = 0; i < list.size(); ++i) {
        ss << list[i].xuatJSON();
        if (i + 1 < list.size()) ss << ",";
    }
    ss << "]}";
    return PhanHoiHTTP(200, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyThemPhieuNhap(const YeuCauHTTP& req) {
    const std::string maNCC = layGiaTriChuoiJSON(req.body, "nhaCungCap");
    std::string ncc;
    for (const auto& item : csdl->layDanhSachNhaCungCap()) {
        if (item.maNCC == maNCC && item.trangThai == "DANG_HOP_TAC") { ncc = item.ten; break; }
    }
    if (ncc.empty()) return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Nha cung cap khong ton tai hoac da ngung hop tac.\"}");
    auto nguoiDungLap = layNguoiDungTuYeuCau(req);
    std::string nguoiLap = nguoiDungLap ? nguoiDungLap->layHoTen() : "";
    std::string ghiChu = layGiaTriChuoiJSON(req.body, "ghiChu");
    std::string maSP = layGiaTriChuoiJSON(req.body, "maSP");
    double soLuong = layGiaTriSoJSON(req.body, "soLuong", 1.0);
    double donGiaNhap = layGiaTriSoJSON(req.body, "donGiaNhap", 0.0);
    std::string hsd = layGiaTriChuoiJSON(req.body, "hanSuDung");

    auto sp = csdl->timSanPhamTheoMa(maSP);
    if (soLuong <= 0 || donGiaNhap < 0) {
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"So luong phai lon hon 0 va don gia khong duoc am.\"}");
    }
    if (!sp) {
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Khong tim thay san pham can nhap!\"}");
    }
    if (sp->laySoLuongTon() + soLuong > sp->layTonToiDa())
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"So luong nhap vuot ton toi da cho phep.\"}");
    if (sp->layLoai() == LoaiSanPham::HAN_SU_DUNG && (!ngayISOHopLe(hsd) || hsd < ngayHienTaiISO()))
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Hang co han su dung can HSD hop le theo YYYY-MM-DD.\"}");

    std::string maPhieu = csdl->sinhMaPhieuNhap();
    
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream timeSS;
    timeSS << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    PhieuNhapKho pn(maPhieu, timeSS.str(), ncc, nguoiLap, ghiChu);
    ChiTietNhapKho ct;
    ct.maChiTiet = "CTN_" + maPhieu;
    ct.maPhieu = maPhieu;
    ct.maSP = maSP;
    ct.tenSP = sp->layTenSP();
    ct.soLuong = soLuong;
    ct.donGiaNhap = (donGiaNhap > 0) ? donGiaNhap : sp->layGiaNhap();
    ct.thanhTien = ct.soLuong * ct.donGiaNhap;
    ct.hanSuDung = hsd;
    pn.themChiTiet(ct);
    pn.datTrangThai("CHO_DUYET");

    bool ok = csdl->themPhieuNhap(pn);
    std::string thongBao = ok ? "Da gui phieu nhap cho quan ly duyet. Ton kho se cap nhat sau khi duoc phe duyet." : csdl->layLoiLuuGanNhat();
    if (!ok && thongBao.empty()) thongBao = "Khong the lap phieu nhap; kiem tra ton toi da va thong tin phieu.";
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << jsonEscape(thongBao) << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyDuyetPhieuNhap(const YeuCauHTTP& req) {
    auto authHeader = req.tieuDe.find("authorization");
    const std::string prefix = "Bearer ";
    if (authHeader == req.tieuDe.end() || authHeader->second.rfind(prefix, 0) != 0) {
        return PhanHoiHTTP(401, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Vui long dang nhap bang tai khoan quan tri.\"}");
    }
    auto nguoiDung = authService->timNguoiDungTheoToken(authHeader->second.substr(prefix.size()));
    if (!nguoiDung || !(nguoiDung->coQuyenQuanTri() || nguoiDung->coQuyenKho())) {
        return PhanHoiHTTP(403, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Chi quan tri vien hoac quan ly kho duoc phe duyet phieu nhap.\"}");
    }

    const std::string maPhieu = layGiaTriChuoiJSON(req.body, "maPhieu");
    const bool ok = csdl->duyetPhieuNhap(maPhieu);
    std::string thongBao = ok
        ? "Da duyet phieu nhap va cap nhat ton kho."
        : csdl->layLoiLuuGanNhat();
    if (!ok && thongBao.empty()) thongBao = "Khong tim thay phieu cho duyet, phieu da xu ly, hoac ton se vuot gioi han.";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json",
        std::string("{\"thanhCong\":") + (ok ? "true" : "false") + ",\"thongBao\":\"" + thongBao + "\"}");
}

PhanHoiHTTP MayChuWeb::xuLyLayKiemKe(const YeuCauHTTP& /*req*/) {
    auto list = csdl->layDanhSachKiemKe();
    std::ostringstream ss;
    ss << "{\"thanhCong\":true,\"danhSach\":[";
    for (size_t i = 0; i < list.size(); ++i) {
        ss << list[i].xuatJSON();
        if (i + 1 < list.size()) ss << ",";
    }
    ss << "]}";
    return PhanHoiHTTP(200, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyLayGiaoDichKho(const YeuCauHTTP& /*req*/) {
    const auto list = csdl->layDanhSachGiaoDichKho();
    std::ostringstream ss;
    ss << "{\"thanhCong\":true,\"danhSach\":[";
    for (size_t i = 0; i < list.size(); ++i) {
        ss << list[i].xuatJSON();
        if (i + 1 < list.size()) ss << ",";
    }
    ss << "]}";
    return PhanHoiHTTP(200, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyThemKiemKe(const YeuCauHTTP& req) {
    std::string maSP = layGiaTriChuoiJSON(req.body, "maSP");
    double tonThucTe = layGiaTriSoJSON(req.body, "tonThucTe");
    auto nguoiKiem = layNguoiDungTuYeuCau(req);
    std::string nguoiKiemKe = nguoiKiem ? nguoiKiem->layHoTen() : "";
    std::string lyDo = layGiaTriChuoiJSON(req.body, "lyDo");
    std::string cachXuLy = layGiaTriChuoiJSON(req.body, "cachXuLy");

    auto sp = csdl->timSanPhamTheoMa(maSP);
    if (!sp) {
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Khong tim thay san pham!\"}");
    }
    if (tonThucTe < 0 || lyDo.empty()) {
        return PhanHoiHTTP(400, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Ton thuc te khong duoc am va can ghi ro ly do kiem ke.\"}");
    }

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream timeSS;
    timeSS << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    PhieuKiemKe kk;
    kk.maKiemKe = csdl->sinhMaKiemKe();
    kk.ngayKiemKe = timeSS.str();
    kk.nguoiKiemKe = nguoiKiemKe;
    kk.maSP = maSP;
    kk.tenSP = sp->layTenSP();
    kk.tonSoSach = sp->laySoLuongTon();
    kk.tonThucTe = tonThucTe;
    kk.chenhLech = kk.tonThucTe - kk.tonSoSach;
    kk.lyDo = lyDo;
    kk.cachXuLy = cachXuLy;

    bool ok = csdl->themPhieuKiemKe(kk);
    std::string thongBao = ok ? "Lap phieu kiem ke thanh cong. Da can bang ton kho." : csdl->layLoiLuuGanNhat();
    if (!ok && thongBao.empty()) thongBao = "Ton kiem ke khong hop le hoac vuot muc ton toi da.";
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << jsonEscape(thongBao) << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyLayCanhBao(const YeuCauHTTP& /*req*/) {
    csdl->kiemTraVaPhatCanhBaoTuDong();
    auto list = csdl->layDanhSachCanhBao();
    std::ostringstream ss;
    ss << "{\"thanhCong\":true,\"danhSach\":[";
    for (size_t i = 0; i < list.size(); ++i) {
        ss << list[i].xuatJSON();
        if (i + 1 < list.size()) ss << ",";
    }
    ss << "]}";
    return PhanHoiHTTP(200, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyDocTatCaCanhBao(const YeuCauHTTP& /*req*/) {
    csdl->danhDauDaDocCanhBao();
    return PhanHoiHTTP(200, "application/json", "{\"thanhCong\":true}");
}

PhanHoiHTTP MayChuWeb::xuLyLayHoaDon(const YeuCauHTTP& req) {
    auto authHeader = req.tieuDe.find("authorization");
    const std::string prefix = "Bearer ";
    if (authHeader == req.tieuDe.end() || authHeader->second.rfind(prefix, 0) != 0) {
        return PhanHoiHTTP(401, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Vui long dang nhap de xem don hang.\"}");
    }
    auto nguoiDung = authService->timNguoiDungTheoToken(authHeader->second.substr(prefix.size()));
    if (!nguoiDung || (!nguoiDung->coQuyenQuanTri() && !nguoiDung->coQuyenBanHang() && !nguoiDung->coQuyenMuaHang())) {
        return PhanHoiHTTP(403, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Tai khoan khong co quyen xem don hang.\"}");
    }

    auto list = csdl->layDanhSachHoaDon();
    std::ostringstream ss;
    ss << "{\"thanhCong\":true,\"danhSach\":[";
    bool first = true;
    for (const auto& hoaDon : list) {
        if (nguoiDung->layVaiTro() == VaiTro::KHACH_HANG && hoaDon->layMaKhachHang() != nguoiDung->layMaNguoiDung()) continue;
        if (!first) ss << ",";
        ss << hoaDon->xuatJSON();
        first = false;
    }
    ss << "]}";
    return PhanHoiHTTP(200, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyDatHang(const YeuCauHTTP& req) {
    std::string maKH = layGiaTriChuoiJSON(req.body, "maKhachHang");
    std::string tenKH = layGiaTriChuoiJSON(req.body, "tenKhachHang");
    std::string sdt = layGiaTriChuoiJSON(req.body, "soDienThoai");
    std::string diaChi = layGiaTriChuoiJSON(req.body, "diaChiGiaoHang");
    std::string loaiGG = layGiaTriChuoiJSON(req.body, "loaiGiamGia");
    std::string loaiThue = layGiaTriChuoiJSON(req.body, "loaiThue");
    std::string ptTT = layGiaTriChuoiJSON(req.body, "phuongThucThanhToan");
    std::string ghiChu = layGiaTriChuoiJSON(req.body, "ghiChu");

    auto nguoiDat = layNguoiDungTuYeuCau(req);
    if (nguoiDat && nguoiDat->coQuyenMuaHang()) {
        maKH = nguoiDat->layMaNguoiDung();
        tenKH = nguoiDat->layHoTen();
        sdt = nguoiDat->laySoDienThoai();
    }

    // Trich xuat danh sach mon hang: duoi dang maSP:soLuong, phan tach bang dau phay
    // Vi du: "danhSachMon": "SP01:2,SP03:1"
    std::string danhSachStr = layGiaTriChuoiJSON(req.body, "danhSachMon");
    std::vector<MonHangDatMua> dsMon;

    std::istringstream ssDs(danhSachStr);
    std::string item;
    while (std::getline(ssDs, item, ',')) {
        size_t cPos = item.find(':');
        if (cPos != std::string::npos) {
            MonHangDatMua mon;
            mon.maSP = item.substr(0, cPos);
            try {
                mon.soLuong = std::stod(item.substr(cPos + 1));
            } catch (...) {
                mon.soLuong = 1.0;
            }
            dsMon.push_back(mon);
        }
    }

    std::string maHDMoi, thongBaoLoi;
    bool ok = quanLyBanHang->taoDonHangMoi(maKH, tenKH, sdt, diaChi, dsMon, loaiGG, loaiThue, ptTT, ghiChu, maHDMoi, thongBaoLoi);

    std::ostringstream ss;
    if (ok) {
        ss << "{\"thanhCong\":true,\"maHoaDon\":\"" << maHDMoi << "\",\"thongBao\":\"Dat hang thanh cong! Don hang dang cho xac nhan.\"}";
        return PhanHoiHTTP(200, "application/json", ss.str());
    } else {
        ss << "{\"thanhCong\":false,\"thongBao\":\"" << thongBaoLoi << "\"}";
        return PhanHoiHTTP(400, "application/json", ss.str());
    }
}

PhanHoiHTTP MayChuWeb::xuLyXacNhanDon(const YeuCauHTTP& req) {
    std::string maHD = layGiaTriChuoiJSON(req.body, "maHoaDon");
    auto nhanVien = layNguoiDungTuYeuCau(req);
    std::string nguoiXuLy = nhanVien ? nhanVien->layHoTen() : "";
    std::string thongBaoLoi;

    bool ok = quanLyBanHang->xacNhanDonHang(maHD, nguoiXuLy, thongBaoLoi);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Xac nhan don hang thanh cong!" : thongBaoLoi) << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyGiaoHang(const YeuCauHTTP& req) {
    std::string maHD = layGiaTriChuoiJSON(req.body, "maHoaDon");
    std::string ttMoi = layGiaTriChuoiJSON(req.body, "trangThaiMoi");
    std::string thongBaoLoi;

    bool ok = quanLyBanHang->capNhatGiaoHang(maHD, ttMoi, thongBaoLoi);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Cap nhat trang thai don hang thanh cong!" : thongBaoLoi) << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyTraHang(const YeuCauHTTP& req) {
    auto auth = req.tieuDe.find("authorization");
    const std::string prefix = "Bearer ";
    if (auth == req.tieuDe.end() || auth->second.rfind(prefix, 0) != 0)
        return PhanHoiHTTP(401, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Vui long dang nhap.\"}");
    auto user = authService->timNguoiDungTheoToken(auth->second.substr(prefix.size()));
    if (!user || !user->coQuyenMuaHang())
        return PhanHoiHTTP(403, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Chi khach hang moi duoc tra don.\"}");
    std::string maHD = layGiaTriChuoiJSON(req.body, "maHoaDon"), tb;
    bool ok = csdl->yeuCauTraHang(maHD, user->layMaNguoiDung(), tb);
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", std::string("{\"thanhCong\":") + (ok ? "true" : "false") + ",\"thongBao\":\"" + tb + "\"}");
}

PhanHoiHTTP MayChuWeb::xuLyDuyetTraHang(const YeuCauHTTP& req) {
    auto auth = req.tieuDe.find("authorization");
    const std::string prefix = "Bearer ";
    if (auth == req.tieuDe.end() || auth->second.rfind(prefix, 0) != 0)
        return PhanHoiHTTP(401, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Vui long dang nhap.\"}");
    auto user = authService->timNguoiDungTheoToken(auth->second.substr(prefix.size()));
    if (!user || (!user->coQuyenBanHang() && !user->coQuyenKho() && !user->coQuyenQuanTri()))
        return PhanHoiHTTP(403, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Nhan vien kho, ban hang hoac quan tri vien moi duoc duyet tra hang sau khi kiem tra thuc te.\"}");
    std::string message;
    bool ok = csdl->xacNhanTraHang(layGiaTriChuoiJSON(req.body, "maHoaDon"), user->layHoTen(), message);
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", std::string("{\"thanhCong\":") + (ok ? "true" : "false") + ",\"thongBao\":\"" + message + "\"}");
}

PhanHoiHTTP MayChuWeb::xuLyThanhToan(const YeuCauHTTP& req) {
    auto auth = req.tieuDe.find("authorization");
    const std::string prefix = "Bearer ";
    if (auth == req.tieuDe.end() || auth->second.rfind(prefix, 0) != 0)
        return PhanHoiHTTP(401, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Vui long dang nhap.\"}");
    auto user = authService->timNguoiDungTheoToken(auth->second.substr(prefix.size()));
    if (!user || (!user->coQuyenBanHang() && !user->coQuyenQuanTri()))
        return PhanHoiHTTP(403, "application/json", "{\"thanhCong\":false,\"thongBao\":\"Chi nhan vien ban hang hoac quan tri vien duoc xac nhan thanh toan.\"}");
    std::string maHD = layGiaTriChuoiJSON(req.body, "maHoaDon");
    std::string thongBaoLoi;

    bool ok = quanLyBanHang->xacNhanThanhToan(maHD, thongBaoLoi);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Xac nhan da thanh toan don hang!" : thongBaoLoi) << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyHuyDon(const YeuCauHTTP& req) {
    std::string maHD = layGiaTriChuoiJSON(req.body, "maHoaDon");
    std::string thongBaoLoi;

    // Ap dung Command Pattern
    bool ok = quanLyBanHang->huyDonHang(maHD, thongBaoLoi);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Huy don hang thanh cong (Command Pattern da hoan ton kho an toan)!" : thongBaoLoi) << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyHoanTacHuy(const YeuCauHTTP& /*req*/) {
    std::string thongBaoLoi;
    // Undo Command Pattern
    bool ok = quanLyBanHang->hoanTacHuyDon(thongBaoLoi);
    std::ostringstream ss;
    ss << "{\"thanhCong\":" << (ok ? "true" : "false")
       << ",\"thongBao\":\"" << (ok ? "Hoan tac thanh cong (Command Undo)!" : thongBaoLoi) << "\"}";
    return PhanHoiHTTP(ok ? 200 : 400, "application/json", ss.str());
}

PhanHoiHTTP MayChuWeb::xuLyBaoCao(const YeuCauHTTP& /*req*/) {
    std::string jsonBaoCao = csdl->taoBaoCaoTongHopJSON();
    return PhanHoiHTTP(200, "application/json", jsonBaoCao);
}
