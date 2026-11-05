#include <stdio.h>
#include <stdlib.h>
#include <arm_neon.h>

#include <zephyr/kernel.h>

#include "convld.h"


#define FP16_THREAD_STACK_SIZE    2048


static struct k_thread fp16_thread;

static K_THREAD_STACK_DEFINE (fp16_stack, FP16_THREAD_STACK_SIZE);


void fp16_task () {
    size_t   out_idx;
    _Float16 out_scalar [SIGNAL_SIZE];      // Only out_scalar [OUT_SIZE] will be valid.
    _Float16 out_neon   [SIGNAL_SIZE];      // Only out_neon [OUT_SIZE] will be valid.


    // Clear output arrays
    for (out_idx = 0; out_idx < OUT_SIZE; out_idx++)
    {
        out_scalar [out_idx] = 0.0f16;
        out_neon   [out_idx] = 0.0f16;
    }

    // Run both implementations
    conv1d_scalar    (input, kernel, out_scalar);
    conv1d_neon_fp16 (input, kernel, out_neon);

    // Verify and Print Results
    printf("Index | Scalar FP16 | NEON FP16\n");
    printf("-------------------------------\n");
    for (out_idx = 0; out_idx < OUT_SIZE; out_idx++)
    {
        printf(" [%02zu] |   %5.2f    |   %5.2f\n", out_idx, (double) out_scalar [out_idx], (double) out_neon [out_idx]);
    }
}

int main (void)
{
    printf ("FP16-TEST ...\n");

	k_thread_create (&fp16_thread, fp16_stack, FP16_THREAD_STACK_SIZE, fp16_task, NULL, NULL, NULL, K_PRIO_COOP (7), 0, K_NO_WAIT);

	return (0);
}
