#ifndef ZAP_DATATYPE_H
#define ZAP_DATATYPE_H


namespace ast {
    enum TypeDataType {
        Int,
        Float,
        Char,
        String,
        Bool,
        Array,
        Func,
    };

    struct DataType {
        virtual ~DataType();

        virtual TypeDataType Type() = 0;

        virtual std::string String() = 0;
    };

    inline DataType::~DataType() = default;
}

#endif //ZAP_DATATYPE_H