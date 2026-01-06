#ifndef ZAP_DATATYPESTRING_H
#define ZAP_DATATYPESTRING_H
#include <string>

#include "DataType.h"
#include "DataTypeArray.h"
#include "DataTypeChar.h"

namespace ast {
    struct DataTypeString final : public DataTypeArray {
        explicit DataTypeString()
            : DataTypeArray(ast::GetCharType()) {
        }

        inline ~DataTypeString() override;

        TypeDataType Type() override;
    };

    inline DataTypeString::~DataTypeString() = default;

    inline TypeDataType DataTypeString::Type() {
        return ast::String;
    }

    inline std::shared_ptr<DataTypeString> GetStringType() {
        static auto instance = std::make_shared<DataTypeString>();
        return instance;
    }
}

#endif //ZAP_DATATYPESTRING_H