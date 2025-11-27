#ifndef ZAP_DATATYPEINT_H
#define ZAP_DATATYPEINT_H
#include "DataType.h"
#include "lexer/Token.h"

namespace ast {
    struct DataTypeInt : public DataType {
        TypeDataType Type() override;

        std::string String() override;

        TypeDataType type;
    };

    inline TypeDataType DataTypeInt::Type() {
        return type;
    }

    inline std::string DataTypeInt::String() {
        return "int";
    }
}

#endif //ZAP_DATATYPEINT_H