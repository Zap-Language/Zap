#ifndef ZAP_DATATYPESTRUCT_H
#define ZAP_DATATYPESTRUCT_H

#include <string>
#include <memory>
#include "DataType.h"

namespace ast {
    struct StructStatement;

    struct DataTypeStruct : public DataType {
        explicit DataTypeStruct(std::string name);
        ~DataTypeStruct() override = default;

        TypeDataType Type() override;
        std::string String() override;

        std::string structName;
        std::weak_ptr<StructStatement> definition;
    };

    inline DataTypeStruct::DataTypeStruct(std::string name)
        : structName(std::move(name)) {}

    inline TypeDataType DataTypeStruct::Type() {
        return TypeDataType::Struct;
    }

    inline std::string DataTypeStruct::String() {
        return "struct " + structName;
    }
}

#endif // ZAP_DATATYPESTRUCT_H
