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
    };

    struct DataType {
        virtual ~DataType() = 0;

        virtual TypeDataType Type() = 0;

        virtual std::string String() = 0;
    };
}

#endif //ZAP_DATATYPE_H