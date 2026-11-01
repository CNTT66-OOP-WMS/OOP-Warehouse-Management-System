# WMS - Hệ thống quản lý kho và bán hàng

Bài tập lớn môn Lập trình hướng đối tượng, xây dựng với **C++**. Backend là HTTP server chạy trên Windows bằng Winsock; frontend sử dụng HTML, CSS và JavaScript thuần. Dữ liệu ứng dụng được lưu trong tệp JSON.

## Mục lục

- [Tổng quan chức năng](#tổng-quan-chức-năng)
- [Cấu trúc dự án](#cấu-trúc-dự-án)
- [Công nghệ và yêu cầu](#công-nghệ-và-yêu-cầu)
- [Biên dịch và chạy](#biên-dịch-và-chạy)
- [Tài khoản demo](#tài-khoản-demo)
- [Dữ liệu và cấu hình](#dữ-liệu-và-cấu-hình)
- [Các trang giao diện](#các-trang-giao-diện)
- [Các nhóm API](#các-nhóm-api)
- [Thiết kế hướng đối tượng](#thiết-kế-hướng-đối-tượng)
- [Phạm vi và giới hạn của đề tài](#phạm-vi-và-giới-hạn-của-đề-tài)

## Tổng quan chức năng

- Quản lý người dùng, trạng thái tài khoản và quyền theo vai trò.
- Đăng nhập, đăng ký, xác minh OTP và luồng quên/đặt lại mật khẩu.
- Quản lý danh mục sản phẩm, giá, tồn kho, vị trí lưu trữ và ngưỡng tồn.
- Quản lý nhà cung cấp, phiếu nhập, kiểm kê và lịch sử giao dịch kho.
- Hỗ trợ sản phẩm thường, sản phẩm có hạn sử dụng và sản phẩm bán theo cân.
- Theo dõi cảnh báo tồn thấp, hàng sắp hết hạn và chênh lệch kiểm kê.
- Tạo và xử lý đơn hàng, giao hàng, thanh toán, hủy đơn và yêu cầu trả hàng.
- Tổng hợp dữ liệu báo cáo kho và bán hàng.

## Cấu trúc dự án

```text
D:\BTL_OOP\
├── nghiệp vụ btl oop.docx                 # Tài liệu yêu cầu nghiệp vụ
└── WMS\
    ├── .gitignore                         # Tệp cấu hình Git (hiện chưa có quy tắc)
    ├── README.md                          # Tài liệu dự án
    ├── build.bat                          # Biên dịch backend
    ├── run.bat                            # Khởi chạy backend và mở trình duyệt
    ├── backend\                           # Backend C++ và các tệp thực thi
    │   ├── AuthService.cpp                # Xác thực và tài khoản
    │   ├── AuthService.h
    │   ├── AuthService.o                  # Tệp đối tượng sinh khi biên dịch
    │   ├── CoSoDuLieu.cpp                 # Dữ liệu, nghiệp vụ lưu trữ JSON
    │   ├── CoSoDuLieu.h
    │   ├── CoSoDuLieu.o
    │   ├── Command.cpp                    # Lệnh nghiệp vụ theo Command
    │   ├── Command.h
    │   ├── EmailService.cpp               # Dịch vụ email mô phỏng
    │   ├── EmailService.h
    │   ├── EmailService.o
    │   ├── HoaDon.cpp                     # Hóa đơn
    │   ├── HoaDon.h
    │   ├── HoaDon.o
    │   ├── JsonUtil.h                     # Tiện ích escape JSON
    │   ├── main.cpp                       # Điểm bắt đầu ứng dụng
    │   ├── main.o
    │   ├── MayChuWeb.cpp                  # HTTP server, API và phục vụ frontend
    │   ├── MayChuWeb.h
    │   ├── MayChuWeb.o
    │   ├── NguoiDung.cpp                 # Người dùng và các vai trò
    │   ├── NguoiDung.h
    │   ├── NguoiDung.o
    │   ├── OTPService.cpp                 # Tạo và xác minh OTP
    │   ├── OTPService.h
    │   ├── OTPService.o
    │   ├── Observer.h                     # Cảnh báo theo mẫu Observer
    │   ├── PasswordUtil.h                 # Băm và kiểm tra mật khẩu
    │   ├── QuanLyBanHang.cpp              # Nghiệp vụ bán hàng và đơn hàng
    │   ├── QuanLyBanHang.h
    │   ├── QuanLyBanHang.o
    │   ├── QuanLyKho.cpp                  # Nghiệp vụ kho
    │   ├── QuanLyKho.h
    │   ├── QuanLyKho.o
    │   ├── SanPham.cpp                    # Cài đặt các loại sản phẩm
    │   ├── SanPham.h
    │   ├── SanPham.o
    │   ├── Strategy.h                     # Chiến lược giảm giá và thuế
    │   ├── wms_server.exe                 # Executable được build.bat tạo và run.bat sử dụng
    │   └── wms_server_updated.exe         # Executable build có sẵn khác
    ├── config\
    │   └── .env                           # Cấu hình môi trường, bao gồm cổng server
    ├── database\
    │   └── wms.json                       # Dữ liệu người dùng, sản phẩm, đơn hàng và kho
    └── frontend\                          # Giao diện web
        ├── banhang.html                   # Bán hàng
        ├── baocao.html                    # Báo cáo
        ├── forgot-password.html           # Quên mật khẩu
        ├── hoadon.html                    # Hóa đơn và đơn hàng
        ├── index.html                     # Trang chủ/cửa hàng
        ├── kho.html                       # Quản lý kho
        ├── login.html                     # Đăng nhập
        ├── register.html                  # Đăng ký
        ├── reset-password.html            # Đặt lại mật khẩu
        ├── sanpham.html                   # Quản lý sản phẩm
        ├── verify-otp.html                # Xác minh OTP
        ├── css\
        │   └── style.css                  # Kiểu dáng giao diện
        └── js\
            ├── app.js                     # Giao diện nghiệp vụ và gọi API
            └── auth.js                    # Giao diện xác thực
```

Các tệp `.o` và `.exe` là sản phẩm build, không phải mã nguồn cần chỉnh sửa. `build.bat` tạo/cập nhật `backend\wms_server.exe`; `run.bat` sử dụng tệp này. Các tệp build có thể thay đổi hoặc được tạo lại.

## Công nghệ và yêu cầu

- Windows (backend sử dụng Winsock).
- MinGW-w64 hoặc MSYS2 với `g++` có trong `PATH`.
- Thư viện hệ thống Windows `ws2_32` và `bcrypt`; script build đã liên kết các thư viện này.
- PowerShell hoặc Command Prompt để chạy các tệp `.bat`.

Kiểm tra trình biên dịch trong PowerShell:

```powershell
g++ --version
```

## Biên dịch và chạy

Mở PowerShell tại thư mục gốc của dự án (thư mục chứa `build.bat` và `run.bat`), sau đó chạy:

```powershell
.\build.bat
.\run.bat
```

`run.bat` sẽ tự build nếu chưa có `backend\wms_server.exe`, mở trình duyệt tại `http://localhost:8080/login.html` và chạy server trong cửa sổ terminal. Giữ cửa sổ này mở khi sử dụng ứng dụng; nhấn `Ctrl+C` để dừng server.

Server mặc định dùng cổng `8080`. Cổng được đọc từ `config\.env`; nếu thay cổng, hãy truy cập `http://localhost:<PORT>/login.html` theo cổng mới. `run.bat` hiện mở URL mặc định `8080`, vì vậy khi đổi cổng cần nhập URL mới thủ công.

## Tài khoản demo

Các tài khoản sau được tạo khi ứng dụng khởi tạo dữ liệu mẫu (ví dụ khi chưa có tệp dữ liệu):

| Vai trò | Tên đăng nhập | Mật khẩu |
| --- | --- | --- |
| Quản trị viên | `admin` | `admin123` |
| Quản lý kho | `thukho` | `kho123` |
| Nhân viên bán hàng | `banhang` | `banhang123` |
| Khách hàng | `khachhang` | `khach123` |

Nếu `database\wms.json` đã có dữ liệu, ứng dụng sử dụng dữ liệu trong tệp đó; tài khoản demo có thể đã bị thay đổi hoặc xóa. Thông tin đăng nhập trên chỉ dành cho bài tập và không được dùng trong môi trường thật.

## Dữ liệu và cấu hình

- Dữ liệu được lưu tại `database\wms.json`.
- Khi chưa có tệp dữ liệu, ứng dụng tạo dữ liệu mẫu ban đầu.
- Sao lưu dữ liệu trước khi chỉnh sửa hoặc thay thế. Không chỉnh sửa JSON khi server đang chạy.
- Cấu hình server nằm trong `config\.env`; khóa bí mật nếu có không nên đưa vào tài liệu, mã nguồn công khai hoặc chia sẻ ra ngoài.
- Nếu cổng mặc định đang được chương trình khác sử dụng, giải phóng cổng đó hoặc cấu hình cổng khác trong `.env`.

## Các trang giao diện

| Trang | Mục đích |
| --- | --- |
| `index.html` | Trang chủ/cửa hàng và danh sách sản phẩm công khai |
| `login.html`, `register.html` | Đăng nhập và đăng ký |
| `verify-otp.html` | Xác minh OTP |
| `forgot-password.html`, `reset-password.html` | Khôi phục và đặt lại mật khẩu |
| `sanpham.html` | Quản lý sản phẩm |
| `kho.html` | Phiếu nhập, kiểm kê, nhà cung cấp, giao dịch và cảnh báo kho |
| `banhang.html` | Nghiệp vụ bán hàng |
| `hoadon.html` | Theo dõi hóa đơn/đơn hàng |
| `baocao.html` | Báo cáo |
| `css/style.css` | Kiểu dáng dùng chung |
| `js/app.js`, `js/auth.js` | Logic giao diện, gọi API và xác thực |

## Các nhóm API

Backend cung cấp API JSON. Ngoại trừ các luồng xác thực và danh sách sản phẩm công khai, API nghiệp vụ yêu cầu đăng nhập và kiểm tra quyền theo vai trò.

| Nhóm | Phương thức và endpoint | Chức năng |
| --- | --- | --- |
| Xác thực | `POST /api/dang-nhap`, `POST /api/dang-ky`, `POST /api/xac-thuc-otp`, `GET /api/lay-otp-hien-tai`, `POST /api/quen-mat-khau`, `POST /api/dat-lai-mat-khau` | Đăng nhập, đăng ký, xác minh OTP và khôi phục mật khẩu |
| Người dùng | `GET /api/nguoi-dung`; `POST /api/nguoi-dung/them`, `/sua`, `/xoa`, `/trang-thai` | Danh sách và quản lý tài khoản |
| Sản phẩm | `GET /api/san-pham`; `POST /api/san-pham/them`, `/sua`, `/xoa` | Xem công khai và quản lý sản phẩm |
| Nhà cung cấp | `GET`, `POST /api/nha-cung-cap`; `POST /api/nha-cung-cap/trang-thai` | Danh sách, thêm và cập nhật trạng thái nhà cung cấp |
| Nhập kho | `GET`, `POST /api/kho/nhap`; `POST /api/kho/nhap/duyet` | Lập và duyệt phiếu nhập |
| Kiểm kê và giao dịch | `GET`, `POST /api/kho/kiem-ke`; `GET /api/kho/giao-dich` | Ghi nhận kiểm kê và tra cứu giao dịch kho |
| Cảnh báo kho | `GET /api/kho/canh-bao`; `POST /api/kho/canh-bao/da-doc` | Xem và đánh dấu đã đọc cảnh báo |
| Đơn hàng | `POST /api/don-hang/dat-hang`, `/xac-nhan`, `/giao-hang`, `/tra-hang`, `/duyet-tra-hang`, `/thanh-toan`, `/huy`, `/hoan-tac` | Đặt hàng, xử lý giao/nhận, thanh toán, hủy và duyệt trả hàng |
| Hóa đơn | `GET /api/hoa-don` | Tra cứu hóa đơn theo quyền người dùng |
| Báo cáo | `GET /api/bao-cao` | Báo cáo tổng hợp |

## Thiết kế hướng đối tượng

- **Kế thừa và đa hình:** `SanPham` có các kiểu `SanPhamThuong`, `SanPhamHanSuDung`, `SanPhamTheoCan`; `NguoiDung` có các vai trò quản trị, quản lý kho, bán hàng và khách hàng.
- **Strategy:** tách cách tính khuyến mãi và thuế thành các chiến lược có thể lựa chọn.
- **Observer:** phát cảnh báo liên quan đến tồn kho, hạn sử dụng và kiểm kê.
- **Command:** đóng gói thao tác hủy đơn và hỗ trợ hoàn tác lệnh gần nhất.
- **Singleton:** `CoSoDuLieu` cung cấp một điểm truy cập dùng chung tới dữ liệu trong ứng dụng.

## Phạm vi và giới hạn của đề tài

- OTP và gửi email được mô phỏng; dự án chưa cấu hình dịch vụ email thật.
- `GET /api/lay-otp-hien-tai` chỉ hoạt động khi đặt `WMS_ENABLE_DEMO_OTP=1`; khi bật chế độ này, server chỉ bind vào loopback. Không bật biến này khi triển khai ra Internet.
- Mã giảm VIP chỉ được chấp nhận khi tài khoản khách hàng có ít nhất 1.000 điểm.
- Backend hiện từ chối các lựa chọn thuế `VAT_5` và `MIEN_THUE`, vì dữ liệu chưa đủ để xác định điều kiện thuế cho từng mặt hàng.
- Yêu cầu trả hàng cần được duyệt; quy tắc hiện tại chỉ cho phép yêu cầu trong vòng 24 giờ sau khi giao đơn đã thanh toán.
- Ứng dụng được xây dựng phục vụ bài tập môn học; chưa hướng đến triển khai trong môi trường doanh nghiệp hoặc xử lý dữ liệu thực tế.
