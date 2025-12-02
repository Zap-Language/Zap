#ifndef ZAP_DATATYPEBOOL_H
#define ZAP_DATATYPEBOOL_H
#include <memory>
#include <string>

#include "DataType.h"

namespace ast {
    struct DataTypeBool : public DataType {
        inline ~DataTypeBool() override;

        TypeDataType Type() override;

        std::string String() override;
    };

    inline DataTypeBool::~DataTypeBool() = default;

    inline TypeDataType DataTypeBool::Type() {
        return ast::Bool;
    }

    inline std::string DataTypeBool::String() {
        return "bool ";
    }

    const auto BOOL = std::make_shared<DataTypeBool>();
}

#endif //ZAP_DATATYPEBOOL_H