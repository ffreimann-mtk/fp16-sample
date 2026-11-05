// Scalar FP16 1D Convolution 
#include <arm_neon.h>
#include <stdlib.h>


#define SIGNAL_SIZE   16
#define KERNEL_SIZE    3
#define OUT_SIZE      (SIGNAL_SIZE - KERNEL_SIZE + 1)


// Add some padding to the input array to ensure that the convolution
// can be performed without running out of bounds.
extern const _Float16 input [SIGNAL_SIZE + KERNEL_SIZE - 1];
extern const _Float16 kernel [KERNEL_SIZE];


extern void conv1d_scalar (const _Float16  *in,
                           const _Float16  *kernel,
                           _Float16        *out);
// Vectorized ARMv8.2 NEON FP16 1D Convolution
extern void conv1d_neon_fp16 (const _Float16  *in,
                              const _Float16  *kernel,
                             _Float16        *out);
