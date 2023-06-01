#pragma once
#include "../base/_.hpp"
#include <vector>

namespace jdc
{

    struct xJdcConfig {
        std::string InputDirectory;
        std::string OutputDirectory;
    };

    using xBitSet = std::vector<bool>;
    X_PRIVATE std::string ToString(const xBitSet & BitSet);

}

