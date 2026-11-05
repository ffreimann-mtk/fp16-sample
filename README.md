# fp16-sample

Test programs for checking **ARMv8.2-A half-precision (FP16)** arithmetic on the Cortex-A55 cores of MediaTek Genio boards (Genio 510 Pumpkin, Genio 510 EVK, Genio 700 EVK).

The same test is built two ways:

| Target | Runs on | Output |
|--------|---------|--------|
| `linux/` | Yocto RITY (Scarthgap) Linux userspace | `linux/build/yocto_armv8/fp16-test` |
| `zephyr/` | Zephyr RTOS in a Jailhouse non-root cell | `zephyr.bin` |

Both run a small **1-D convolution** twice: once with **scalar FP16** (`FEAT_FP16` / `FPHP`) and once with **NEON FP16** (`ASIMDHP`). They print the two results side by side. If the results match, the core can execute both kinds of FP16 instruction correctly in that environment.

---

## Directory layout

```
fp16-sample/
├── common/
│   ├── include/convld.h      # test sizes, data, and function prototypes
│   └── src/convld.c          # input/kernel data + scalar and NEON FP16 conv1d
├── linux/
│   ├── Makefile              # top-level: builds yocto_armv8
│   ├── Makefile.yocto_armv8  # cross-compile using the Yocto SDK
│   └── src/main.c            # Linux test driver
├── zephyr/
│   ├── CMakeLists.txt
│   ├── prj.conf              # FPU + FPU sharing, ARMv8-A non-secure
│   ├── boards/*.overlay      # UART1 @ 115200 console for each board
│   └── src/main.c            # Zephyr test driver (runs the test in a thread)
├── make/Common.mak           # shared make variables (PROJ_NAME, YOCTO_SDK_PATH)
├── hwcap.py                  # runtime check of the FPHP / ASIMDHP HWCAP bits on Linux
└── jailhouse_<board>_zephyr_fp16.sh   # starts the Zephyr cell under Jailhouse
```

---

## What is tested

`common/src/convld.c` uses a fixed 16-element input signal `1.0 … 16.0` and a 3-tap kernel `{0.5, 1.0, 0.5}`. Both stored as `_Float16`. The output has `OUT_SIZE = 16 − 3 + 1 = 14` elements.

### `conv1d_scalar()` — scalar FP16
A simple nested loop over `_Float16` values. Built with `-march=armv8.2-a+fp16` (Linux) or `-mcpu=cortex-a55` (Zephyr), so the compiler emits native scalar half-precision instructions (`fmul h`, `fadd h`, `fmadd h`, …) instead of converting to single precision.

### `conv1d_neon_fp16()` — NEON FP16
Uses the ARMv8.2 FP16 vector intrinsics from `<arm_neon.h>` on `float16x8_t` (8 lanes × 16 bits = 128 bits):

| Intrinsic | Instruction | Purpose |
|-----------|-------------|---------|
| `vdupq_n_f16` | `DUP Vd.8H` | broadcast each kernel weight into a vector |
| `vld1q_f16` | `LD1 {Vt.8H}` | load 8 shifted input samples |
| `vmulq_f16` | `FMUL Vd.8H` | first kernel tap |
| `vfmaq_f16` | `FMLA Vd.8H` | fused multiply-accumulate for the other taps |
| `vst1q_f16` | `ST1 {Vt.8H}` | store 8 results |

It computes 8 outputs per iteration.

### Expected output
For this data, `out[i] = 0.5·(i+1) + 1.0·(i+2) + 0.5·(i+3) = 2i + 4`. Every value fits exactly in FP16, so the two columns should match exactly:

```
Index | Scalar FP16 | NEON FP16
-------------------------------
 [00] |    4.00    |    4.00
 [01] |    6.00    |    6.00
 [02] |    8.00    |    8.00
 ...
 [12] |   28.00    |   28.00
 [13] |   30.00    |   30.00
```

If the core or the environment does not support the instructions (for example, the FPU is not enabled in the Zephyr cell), the program usually stops with an **undefined-instruction / FP-access trap**. It does not just print wrong numbers.

---

## Checking CPU support on Linux (`hwcap.py`)

Copy the script to the target and run it:

```sh
python3 hwcap.py
```

It reads `AT_HWCAP` through libc `getauxval(3)` and reports:

- `FPHP` (bit 9): scalar half-precision support
- `ASIMDHP` (bit 10): Advanced SIMD half-precision support

On Cortex-A55 both should be `True`. You can check the same thing with `grep -o 'fphp\|asimdhp' /proc/cpuinfo | sort -u`.

---

## Building and running: Linux

### Prerequisites
A Yocto SDK installed for the RITY image (aarch64-poky-linux toolchain).

### Build
`make/Common.mak` provides a default `YOCTO_SDK_PATH`. Override it to point at your SDK:

```sh
cd linux
make YOCTO_SDK_PATH=/path/to/RITY/SDK/<soc>/<version>
```

When the SDK path is invalid, the build prints a red "Skip building" message and produces no binary.

Output: `linux/build/yocto_armv8/fp16-test`

Main compiler flags (from `Makefile.yocto_armv8`): `-march=armv8.2-a+fp16 -O2 --sysroot=<SDK>/sysroots/armv8a-poky-linux`.

### Run on target
```sh
scp linux/build/yocto_armv8/fp16-test root@<board>:/root/
ssh root@<board> /root/fp16-test
```

To check that native FP16 instructions were emitted:
```sh
aarch64-poky-linux-objdump -d fp16-test | grep -E 'fmla\s+v[0-9]+\.8h|fmadd\s+h|fmul\s+h'
```

---

## Building and running: Zephyr under Jailhouse

### Configuration
`prj.conf`:

| Option | Why |
|--------|-----|
| `CONFIG_FPU=y` | enable FP/SIMD in the cell (otherwise FP16 instructions trap) |
| `CONFIG_FPU_SHARING=y` | save/restore FP context across threads; the test runs in its own thread |
| `CONFIG_ARMV8_A_NS=y` | the cell runs in Non-secure EL1 under Jailhouse |
| `CONFIG_HEAP_MEM_POOL_SIZE=131072` | heap for the libc `printf` |

The board overlays enable `uart1` at **115200 baud** as the cell console, plus `gpio0_31`.

### Build
Board targets match the overlay file names in `zephyr/boards/`:

| Board | `west build -b` |
|-------|-----------------|
| Genio 510 Pumpkin | `mt8370_genio_510_pumpkin/mt8188/a55` |
| Genio 510 EVK | `mt8370_genio_510_evk/mt8188/a55` |
| Genio 700 EVK | `mt8390_genio_700_evk/mt8188/a55` |

```sh
west build -p always -b mt8370_genio_510_pumpkin/mt8188/a55 \
    /hdb/MediaTek/Pumpkin/RITY-Scarthgap_v26.0_Jailhouse/fp16-sample/zephyr
```

Output: `build/zephyr/zephyr.bin`

### Run on target
Copy the image to `/root/zephyr.bin` on the board, then run the script for that board from the Linux root cell:

```sh
scp build/zephyr/zephyr.bin root@<board>:/root/zephyr.bin
scp jailhouse_genio-510-pumpkin_zephyr_fp16.sh root@<board>:/root/
ssh root@<board> sh /root/jailhouse_genio-510-pumpkin_zephyr_fp16.sh
```

The script:
1. `modprobe jailhouse`
2. `jailhouse enable /usr/share/jailhouse/cells/<board>.cell`: enables the hypervisor with the root-cell config
3. `jailhouse cell create /usr/share/jailhouse/cells/<board>-zephyr.cell`: creates the Zephyr cell
4. `jailhouse cell load zephyr /root/zephyr.bin -a 0x8000`: loads the image at cell address `0x8000`
5. `jailhouse cell start zephyr`

Connect a terminal to the cell's UART1 (115200 8N1). You should see `FP16-TEST ...` followed by the result table.

To stop:
```sh
jailhouse cell shutdown zephyr
jailhouse cell destroy zephyr
jailhouse disable
```

---

## Known issues / notes

- **NEON loop reads past the input.** `OUT_SIZE` (14) is not a multiple of 8. The second NEON iteration (`out_idx = 8`) loads `input[8..17]`, which is 2 elements beyond `input[16]`, and stores `out[8..15]`. The output buffers have `SIGNAL_SIZE` (16) elements, so the store stays in bounds and only the first 14 results are printed. The extra reads are still out of bounds. Fix by padding `input` to 18 elements or handling the tail with the scalar loop.
- `zephyr/src/main.c` defines `SIGNAL_SIZE` / `KERNEL_SIZE` / `OUT_SIZE` again, though `convld.h` already provides them. The values match, so it builds, but the copies can drift apart.
- `linux/Makefile.yocto_armv8` `clean` deletes `$(PROJ_NAME)_$(NAME)` (`fp16-test_yocto_armv8`), but the binary is called `fp16-test`, so `make clean` leaves it in place.
- `printf("%02ld", out_idx)` passes a `size_t`; `%02zu` is the portable format.
- The default `YOCTO_SDK_PATH` in `make/Common.mak` points at a v25.1 SDK; override it for v26.0.
- The `kernel/include` / `arch_page_phys_get()` block in `zephyr/CMakeLists.txt` comes from an ivshmem-based project. This sample does not need it.
- `zephyr/README.rst` and `zephyr/sample.yaml` are empty placeholders.
