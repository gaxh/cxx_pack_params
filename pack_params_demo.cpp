#include "anyobj/anyobj.h"

#include <vector>
#include <string>
#include <functional>

#include <stdio.h>

#define LOG(fmt, ...) printf("[%s:%d:%s] " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

using Any = AnyObj<sizeof(void *)>;

using PackedParams = std::vector<Any>;
using CallResult = Any;

// pack api begin

template<typename FirstArg, typename ... Args>
void PackParamsGenericRecursive(PackedParams &out, FirstArg && first, Args && ... args) {
    out.emplace_back( std::forward<FirstArg>(first) );

    PackParamsGenericRecursive(out, std::forward<Args>(args) ...);
}

inline
void PackParamsGenericRecursive(PackedParams &out) {
    // do nothing
}

template<typename ... Args>
PackedParams PackParamsGeneric(Args && ... args) {
    PackedParams out;
    out.reserve(sizeof ... (args));

    PackParamsGenericRecursive(out, std::forward<Args>(args) ...);

    return out;
}

// pack api end

// unpack api begin

template<typename RetType, typename ... Args>
struct UnpackCallerGenericRecursive;

template<typename RetType, typename FirstParam, typename ... EtcParams>
struct UnpackCallerGenericRecursive<RetType, FirstParam, EtcParams ...> {
    template<typename F>
    static RetType Call(F &&f, PackedParams &params, size_t index) {
        assert(index < params.size());
        FirstParam &first = params[index].Reference<FirstParam>();

        auto subf = [&](EtcParams ... etc_params) -> RetType {
            return f(first, etc_params ...);
        };

        return UnpackCallerGenericRecursive<RetType, EtcParams ...>::Call(subf, params, index + 1);
    }
};

template<typename RetType>
struct UnpackCallerGenericRecursive<RetType> {
    template<typename F>
    static RetType Call(F &&f, PackedParams &params, size_t index) {
        return f();
    }
};

template<typename F, typename RetType, typename ... Params>
struct UnpackCallerFunction {
    template<typename F1>
    UnpackCallerFunction(F1 &&f) : f(std::forward<F1>(f)) {}

    CallResult operator() (PackedParams &params) {
        return UnpackCallerGenericRecursive<RetType, Params ...>::Call(f, params, 0);
    }

    F f;
};

template<typename RetType, typename ... Params>
struct CreateUnpackCallerGeneric {
    template<typename F>
    static UnpackCallerFunction<F, RetType, Params ...> Create(F && f) {
        return UnpackCallerFunction<F, RetType, Params ...>( std::forward<F>(f) );
    }
};

template<typename F, typename ... Params>
struct UnpackCallerFunction<F, void, Params ...> {
    template<typename F1>
    UnpackCallerFunction(F1 &&f) : f(std::forward<F1>(f)) {}

    CallResult operator() (PackedParams &params) {
        UnpackCallerGenericRecursive<void, Params ...>::Call(f, params, 0);
        return CallResult();
    }

    F f;
};

template<typename ... Params>
struct CreateUnpackCallerGeneric<void, Params ...> {
    template<typename F>
    static UnpackCallerFunction<F, void, Params ...> Create(F && f) {
        return UnpackCallerFunction<F, void, Params ...>( std::forward<F>(f) );
    }
};

// unpack api end

int add(int x, int y, int z) {
    return x + y + z;
}

void vdd(int x, int y) {
    LOG("vdd: x=%d, y=%d", x, y);
}

int main() {

    PackedParams params;

    params = PackParamsGeneric( 1, 111, 11111 );
    LOG("params size=%lu", params.size());

    auto f = CreateUnpackCallerGeneric<int, int, int, int>::Create(add);
    CallResult r1 = f(params);
    LOG("result=%d", r1.Reference<int>());

    auto g = CreateUnpackCallerGeneric<void, int, int>::Create(vdd);
    CallResult r2 = g(params);

    return 0;
}
