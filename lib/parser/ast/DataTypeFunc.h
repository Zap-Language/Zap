#ifndef ZAP_DATATYPEFUNC_H
#define ZAP_DATATYPEFUNC_H
#include <memory>
#include <vector>

#include "DataType.h"
#include "utils.h"

namespace ast {
    struct DataTypeFunc : public DataType {
        ~DataTypeFunc() override = default;

        TypeDataType Type() override;

        std::string String() override;

        std::vector<std::shared_ptr<DataType>> params;
        std::shared_ptr<DataType> returnType;
    };

    inline TypeDataType DataTypeFunc::Type() {
        return TypeDataType::Func;
    }

    inline std::string DataTypeFunc::String() {
        std::string result = "func(";

        for (const auto& param : params) {
            result += trim(param->String()) + ", ";
        }

        if (!result.empty()) {
            result.pop_back();
            result.pop_back();
        }

        result += ") ";
        result += returnType->String();

        return result;
    }
}

#endif //ZAP_DATATYPEFUNC_H