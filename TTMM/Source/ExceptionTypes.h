#ifndef EXCEPTION_TYPES_H
#define EXCEPTION_TYPES_H

#include <string>

namespace ttmm {

    struct GeneralException : public std::exception {
        GeneralException(std::string const& message) : exception(), message(message) {}

        const char* what() const override final { return message.c_str(); }
    private:
        std::string message;
    };

}

#endif
