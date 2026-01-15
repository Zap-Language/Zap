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

    inline std::shared_ptr<DataTypeChar> GetCharType() {
        static auto instance = std::make_shared<DataTypeChar>();
        return instance;
    }
}

#endif //ZAP_DATATYPECHAR_H