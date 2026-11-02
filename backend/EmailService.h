#ifndef EMAILSERVICE_H
#define EMAILSERVICE_H

#include <string>

class EmailService {
public:
    // Demo mode: prints the generated OTP to the server console; no email is sent.
    static bool guiEmail(const std::string& denEmail, const std::string& tieuDe,
                         const std::string& noiDung);
};

#endif // EMAILSERVICE_H
