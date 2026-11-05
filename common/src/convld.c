#include "convld.h"


const _Float16 input [SIGNAL_SIZE] = {
    1.0f16,  2.0f16,  3.0f16,  4.0f16,
    5.0f16,  6.0f16,  7.0f16,  8.0f16,
    9.0f16, 10.0f16, 11.0f16, 12.0f16,
    13.0f16, 14.0f16, 15.0f16, 16.0f16
};

const _Float16 kernel [KERNEL_SIZE] = {
    0.5f16, 1.0f16, 0.5f16
};


// Scalar FP16 1D Convolution 
void conv1d_scalar (const _Float16  *in,
                    const _Float16  *kernel,
                    _Float16        *out)
{
    size_t out_idx;
    size_t kernel_idx;


    for (out_idx = 0; out_idx < OUT_SIZE; out_idx++)
    {
        out [out_idx] = 0.0f16;

        for (kernel_idx = 0; kernel_idx < KERNEL_SIZE; kernel_idx++)
        {
            out [out_idx] += in [out_idx + kernel_idx] * kernel [kernel_idx];
        }
    }
}

// Vectorized ARMv8.2 NEON FP16 1D Convolution
void conv1d_neon_fp16 (const _Float16  *in,
                       const _Float16  *kernel,
                       _Float16        *out)
{
    size_t      out_idx;
    size_t      kernel_idx;
    float16x8_t in_neon [KERNEL_SIZE];
    float16x8_t kernel_neon [KERNEL_SIZE];

 
    // Broadcast the kernel weights into 128-bit NEON vectors (each vector holds 8 x FP16 values)
    for (kernel_idx = 0; kernel_idx < KERNEL_SIZE; kernel_idx++)
    {
        kernel_neon [kernel_idx] = vdupq_n_f16 ((float16_t) kernel [kernel_idx]);
    }

    // Process 8 output elements at a time (128-bit vector / 16-bit FP = 8 lanes)
    for (out_idx = 0; out_idx < OUT_SIZE; out_idx += 8)
    {
        float16x8_t acc;


        // Load shifted input sequences (8 elements per load)
        for (kernel_idx = 0; kernel_idx < KERNEL_SIZE; kernel_idx++)
        {
            in_neon [kernel_idx] = vld1q_f16 ((const float16_t*) &in [out_idx + kernel_idx]);
        }

        // Multiply and Accumulate using native FP16 instructions
        acc = vmulq_f16 (in_neon [0], kernel_neon [0]);

        for (kernel_idx = 1; kernel_idx < KERNEL_SIZE; kernel_idx++)
        {
            acc = vfmaq_f16 (acc, in_neon [kernel_idx], kernel_neon [kernel_idx]);
        }

        // Store the 8 parallel results directly back to the output array
        vst1q_f16 ((float16_t*) &out [out_idx], acc);
    }
}
