@echo off
setlocal
chcp 65001 > nul
echo ========================================================
echo   DANG BIEN DICH HE THONG QUAN LY KHO VA BAN HANG (WMS)
echo   Ngon ngu: C++20 ^| Trinh bien dich: g++ (MinGW/MSYS2)
echo ========================================================

pushd "%~dp0backend"
if errorlevel 1 (
    echo [!] Khong tim thay thu muc backend.
    exit /b 1
)
echo [1/3] Dang bien dich cac module OOP C++...
g++ -std=c++20 -Wall -Wextra -O2 main.cpp CoSoDuLieu.cpp SanPham.cpp HoaDon.cpp QuanLyKho.cpp QuanLyBanHang.cpp NguoiDung.cpp AuthService.cpp EmailService.cpp OTPService.cpp MayChuWeb.cpp -lws2_32 -lbcrypt -o wms_server.exe

if %ERRORLEVEL% EQU 0 (
    echo [2/3] Bien dich thanh cong file wms_server.exe!
    popd
    echo [3/3] San sang khoi chay! Hay chay run.bat de mo he thong.
    echo ========================================================
    exit /b 0
) else (
    echo [!] Co loi xay ra trong qua trinh bien dich.
    popd
    exit /b 1
)

