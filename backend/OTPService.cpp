#include "OTPService.h"

std::map<std::string, ThongTinOTP> OTPService::danhSachOTP;
std::mutex OTPService::mutexOTP;

