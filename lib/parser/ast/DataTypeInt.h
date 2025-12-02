#ifndef ZAP_DATATYPEINT_H
#define ZAP_DATATYPEINT_H
#include "DataType.h"
#include "lexer/Token.h"

namespace ast {
    struct DataTypeInt : public DataType {
        TypeDataType Type() override;

        std::string String() override;
    };

    inline TypeDataType DataTypeInt::Type() {
        return ast::Int;
    }

    inline std::string DataTypeInt::String() {
        return "int ";
    }

    const auto INT = std::make_shared<DataTypeInt>();
}

#endif //ZAP_DATATYPEINT_H