#ifndef ZAP_DATATYPEARRAY_H
#define ZAP_DATATYPEARRAY_H
#include <memory>
#include <string>
#include <utility>

#include "DataType.h"

namespace ast {
    struct DataTypeArray : public DataType {
        explicit DataTypeArray(std::shared_ptr<DataType> dataType);

        inline ~DataTypeArray() override;

        TypeDataType Type() override;

        std::string String() override;

        std::shared_ptr<ast::DataType> itemType;
    };

    inline DataTypeArray::DataTypeArray(std::shared_ptr<DataType> dataType) : itemType(std::move(dataType)) {
    }

    inline DataTypeArray::~DataTypeArray() = default;

    inline TypeDataType DataTypeArray::Type() {
        return ast::Array;
    }

    inline std::string DataTypeArray::String() {
        return "[]" + (itemType ? itemType->String() : "unknown");
    }
}

#endif //ZAP_DATATYPEARRAY_H