#pragma once

#define PROTON_SHM_NAME "facetracknoir-proton-shm"
#define PROTON_MTX_NAME "facetracknoir-proton-mtx"

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

struct ProtonSHM {
    double data[6];
    int gameid, gameid2;
    unsigned char table[8];
    bool stop;
};
