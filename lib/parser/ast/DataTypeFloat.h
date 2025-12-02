#ifndef ZAP_DATATYPEFLOAT_H
#define ZAP_DATATYPEFLOAT_H
#include <memory>
#include <string>

#include "DataType.h"

namespace ast {
    struct DataTypeFloat : public DataType {
        inline ~DataTypeFloat() override;

        TypeDataType Type() override;

        std::string String() override;
    };

    inline DataTypeFloat::~DataTypeFloat() = default;

    inline TypeDataType DataTypeFloat::Type() {
        return ast::Float;
    }

    inline std::string DataTypeFloat::String() {
        return "float ";
    }

    const auto FLOAT = std::make_shared<DataTypeFloat>();
}

#endif //ZAP_DATATYPEFLOAT_H