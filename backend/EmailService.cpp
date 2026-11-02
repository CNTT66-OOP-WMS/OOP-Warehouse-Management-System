#include "EmailService.h"
#include <iostream>

bool EmailService::guiEmail(const std::string& denEmail, const std::string& tieuDe,
                            const std::string& noiDung) {
    std::cout << "========================================\n"
              << "[EMAIL DEMO] OTP danh cho: " << denEmail << '\n'
              << "TIEU DE: " << tieuDe << '\n'
              << "NOI DUNG: " << noiDung << '\n'
              << "========================================\n";
    return true;
}
