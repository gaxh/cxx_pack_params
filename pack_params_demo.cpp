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



// unpack api end

int add(int x, int y, int z) {
    return x + y + z;
}

int main() {

    PackedParams params;

    params = PackParamsGeneric( 1, 111, 11111 );
    LOG("params size=%lu", params.size());

    params = PackParamsGeneric();
    LOG("params size=%lu", params.size());

    return 0;
}
