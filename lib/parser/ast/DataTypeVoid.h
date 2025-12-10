#ifndef ZAP_DATATYPEVOID_H
#define ZAP_DATATYPEVOID_H
#include <memory>
#include <string>

#include "DataType.h"

namespace ast {
    struct DataTypeVoid : public DataType {
        inline ~DataTypeVoid() override;

        TypeDataType Type() override;

        std::string String() override;
    };

    inline TypeDataType DataTypeVoid::Type() {
        return Void;
    }

    inline DataTypeVoid::~DataTypeVoid() = default;

    inline std::string DataTypeVoid::String() {
        return {};
    }

    const auto VOID = std::make_shared<DataTypeVoid>();
}

#endif //ZAP_DATATYPEVOID_H