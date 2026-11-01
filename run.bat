@echo off
chcp 65001 > nul
echo ========================================================
echo   KHOI DONG HE THONG QUAN LY KHO & BAN HANG (WMS)
echo   Giao dien Web: Xanh nhat & Trang (Light Blue & White)
echo ========================================================

if not exist "backend\wms_server.exe" (
    echo [!] Chua tim thay file thuc thi backend\wms_server.exe.
    echo Dang tu dong bien dich...
    call build.bat
    if errorlevel 1 (
        echo [!] Khong the bien dich backend. Hay kiem tra cai dat g++/MinGW.
        exit /b 1
    )
)

echo Dang mo trang dang nhap tai http://localhost:8080/login.html...
start http://localhost:8080/login.html

echo May chu C++ dang chay tren cong 8080.
echo Nhan Ctrl + C de dung may chu.
echo ========================================================
cd backend
wms_server.exe
cd ..
pause

