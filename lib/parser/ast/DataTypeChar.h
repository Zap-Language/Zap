#ifndef ZAP_DATATYPECHAR_H
#define ZAP_DATATYPECHAR_H
#include <memory>
#include <string>

#include "DataType.h"

namespace ast {
    struct DataTypeChar : public DataType {
        inline ~DataTypeChar() override;

        TypeDataType Type() override;

        std::string String() override;
    };

    inline DataTypeChar::~DataTypeChar() = default;

    inline TypeDataType DataTypeChar::Type() {
        return ast::Char;
    }

    inline std::string DataTypeChar::String() {
        return "char ";
    }

    const auto CHAR = std::make_shared<DataTypeChar>();
}

#endif //ZAP_DATATYPECHAR_H