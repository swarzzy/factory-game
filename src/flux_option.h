#pragma once

enum OptionStatus: u8 {
    None = 0, Some
};

template<typename T>
struct Option
{
    T _some;
    OptionStatus status;

    inline bool IsSome() {
        return this->status == OptionStatus::Some ? true : false;
    }

    inline bool IsNone() {
        return !this->IsSome();
    }

    explicit operator bool() {
        return this->IsSome();
    }

    inline T Unwrap() {
        if (this->status == ::Some) {
            return this->_some;
        } else {
            panic("[Option] Empty Option was unwrapped");
            return {};
        }
    }

    static inline Option<T> Some(T value) {
        Option<T> result;
        result._some = value;
        result.status = ::Some;
        return result;
    }

    static inline Option<T> None() {
        Option<T> result;
        result.status = ::None;
        return result;
    }
};


struct Empty {};

enum ResultStatus : unsigned char {
    Ok, Error
};

template <typename OkType, typename ErrorType>
union Result
{
    OkType ok;
    ErrorType error;
    ResultStatus status;
    inline OkType Unwrap() {
        if (this->status == Ok) {
            return ok;
        }
        panic("[Result] Called Unwrap() on a Result that contains eror"; );
    }
};

template<typename OkType, typename ErrorType>
inline Result<OkType, ErrorType> ResultOk(OkType ok) {
    Result<OkType, ErrorType> result;
    result.ok = ok;
    result.status = Ok;
    return result;
}

template<typename OkType, typename ErrorType>
inline Result<OkType, ErrorType> ResultError(ErrorType err) {
    Result<OkType, ErrorType> result;
    result.error = err;
    result.status = Error;
    return result;
}

template<typename OkType, typename ErrorType>
inline Result<Empty, ErrorType> ResultOk() {
    return ResultOk<Empty, ErrorType>(Empty{});
}

template<typename OkType>
inline Result<OkType, Empty> ResultOk(OkType result) {
    return ResultOk<OkType, Empty>(result);
}

template<typename ErrorType>
inline Result<Empty, ErrorType> ResultError(ErrorType result) {
    return ResultOk<Empty, ErrorType>(result);
}

template<typename OkType>
inline Result<OkType, Empty> ResultError() {
    return ResultError<OkType, Empty>();
}

template<typename OkType, typename ErrorType>
inline Result<OkType, Empty> ResultError() {
    return ResultError<OkType, Empty>(Empty{});
}
