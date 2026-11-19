# WMS - Hệ thống quản lý kho và bán hàng

Bài tập lớn môn Lập trình hướng đối tượng, xây dựng ứng dụng quản lý kho và bán hàng bằng **C++ chạy trên cửa sổ terminal**. Dữ liệu ứng dụng được lưu trong tệp JSON.

## Mục lục

- [Tổng quan chức năng](#tổng-quan-chức-năng)
- [Cấu trúc dự án](#cấu-trúc-dự-án)
- [Công nghệ và yêu cầu](#công-nghệ-và-yêu-cầu)
- [Biên dịch và chạy](#biên-dịch-và-chạy)
- [Tài khoản demo](#tài-khoản-demo)
- [Dữ liệu và cấu hình](#dữ-liệu-và-cấu-hình)
- [Menu terminal](#menu-terminal)
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
    ├── .gitignore                         # Quy tắc bỏ qua tệp build và dữ liệu cấu hình
    ├── README.md                          # Tài liệu dự án
    ├── build.bat                          # Biên dịch ứng dụng terminal
    ├── run.bat                            # Khởi chạy ứng dụng terminal
    ├── backend\                           # Mã nguồn C++ và file thực thi build ra
    │   ├── AuthService.cpp                # Xác thực và tài khoản
    │   ├── AuthService.h
    │   ├── CoSoDuLieu.cpp                 # Dữ liệu, nghiệp vụ lưu trữ JSON
    │   ├── CoSoDuLieu.h
    │   ├── Command.cpp                    # Lệnh nghiệp vụ theo Command
    │   ├── Command.h
    │   ├── EmailService.cpp               # Dịch vụ email mô phỏng
    │   ├── EmailService.h
    │   ├── HoaDon.cpp                     # Hóa đơn
    │   ├── HoaDon.h
    │   ├── JsonUtil.h                     # Tiện ích escape JSON
    │   ├── main.cpp                       # Điểm bắt đầu ứng dụng
    │   ├── NguoiDung.cpp                 # Người dùng và các vai trò
    │   ├── NguoiDung.h
    │   ├── OTPService.cpp                 # Tạo và xác minh OTP
    │   ├── OTPService.h
    │   ├── Observer.h                     # Cảnh báo theo mẫu Observer
    │   ├── PasswordUtil.h                 # Băm và kiểm tra mật khẩu
    │   ├── QuanLyBanHang.cpp              # Nghiệp vụ bán hàng và đơn hàng
    │   ├── QuanLyBanHang.h
    │   ├── QuanLyKho.cpp                  # Nghiệp vụ kho
    │   ├── QuanLyKho.h
    │   ├── SanPham.cpp                    # Cài đặt các loại sản phẩm
    │   ├── SanPham.h
    │   ├── Strategy.h                     # Chiến lược giảm giá và thuế
    │   └── wms.exe                        # Executable terminal được build.bat tạo
    ├── config\
    │   └── .env                            # Cấu hình ứng dụng
    └── database\
        └── wms.json                       # Dữ liệu người dùng, sản phẩm, đơn hàng và kho
```

Các file `.o` và `.exe` là sản phẩm build, không phải mã nguồn cần chỉnh sửa. Trong cấu hình build hiện tại, `build.bat` biên dịch trực tiếp các file `.cpp` thành `backend\wms.exe`; `run.bat` tự gọi `build.bat` trước khi khởi chạy. Các file build có thể bị xóa hoặc được tạo lại.

## Công nghệ và yêu cầu

- Windows.
- MinGW-w64 hoặc MSYS2 với `g++` có trong `PATH`.
- Thư viện hệ thống Windows `bcrypt`; script build đã liên kết thư viện này.
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

`run.bat` sẽ tự build nếu chưa có `backend\wms.exe`, sau đó mở ứng dụng trong cửa sổ terminal. Chọn tài khoản theo vai trò, chọn chức năng trong menu và chọn `0` để đăng xuất hoặc thoát.

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
- Sao lưu dữ liệu trước khi chỉnh sửa hoặc thay thế. Không chỉnh sửa JSON khi ứng dụng đang chạy.

## Menu terminal

Ứng dụng đăng nhập bằng tên tài khoản và mật khẩu, sau đó hiển thị menu theo vai trò:

- Quản trị viên: quản lý người dùng, kho, bán hàng và xem hóa đơn.
- Quản lý kho: xem kho, nhập kho, duyệt phiếu, xem giao dịch và cảnh báo.
- Nhân viên bán hàng: xem và xử lý đơn hàng.
- Khách hàng: xem sản phẩm, đặt hàng và xem hóa đơn của mình.

## Thiết kế hướng đối tượng

- **Kế thừa và đa hình:** `SanPham` có các kiểu `SanPhamThuong`, `SanPhamHanSuDung`, `SanPhamTheoCan`; `NguoiDung` có các vai trò quản trị, quản lý kho, bán hàng và khách hàng.
- **Strategy:** tách cách tính khuyến mãi và thuế thành các chiến lược có thể lựa chọn.
- **Observer:** phát cảnh báo liên quan đến tồn kho, hạn sử dụng và kiểm kê.
- **Command:** đóng gói thao tác hủy đơn và hỗ trợ hoàn tác lệnh gần nhất.
- **Singleton:** `CoSoDuLieu` cung cấp một điểm truy cập dùng chung tới dữ liệu trong ứng dụng.

## Phạm vi và giới hạn của đề tài

- OTP và chức năng gửi email được mô phỏng trong cửa sổ terminal; dự án chưa cấu hình dịch vụ email thật.
- Mã giảm VIP chỉ được chấp nhận khi tài khoản khách hàng có ít nhất 1.000 điểm.
- Backend hiện từ chối các lựa chọn thuế `VAT_5` và `MIEN_THUE`, vì dữ liệu chưa đủ để xác định điều kiện thuế cho từng mặt hàng.
- Yêu cầu trả hàng cần được duyệt; quy tắc hiện tại chỉ cho phép yêu cầu trong vòng 24 giờ sau khi giao đơn đã thanh toán.
- Ứng dụng được xây dựng phục vụ bài tập môn học; chưa hướng đến triển khai trong môi trường doanh nghiệp hoặc xử lý dữ liệu thực tế.
