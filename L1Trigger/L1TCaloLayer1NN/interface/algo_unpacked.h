#ifndef ALGO_UNPACKED_H
#define ALGO_UNPACKED_H

#include "L1Trigger/L1TCaloLayer1NN/interface/ap_types/ap_int.h"
#include "L1Trigger/L1TCaloLayer1NN/interface/ap_types/ap_fixed.h"
#include "L1Trigger/L1TCaloLayer1NN/interface/ap_types/hls_stream.h"
#include "L1Trigger/L1TCaloLayer1NN/interface/defines.h"

void algo_unpacked(ap_uint<128> link_in[N_CH_IN], ap_uint<128> link_out[N_CH_OUT]);

#endif
