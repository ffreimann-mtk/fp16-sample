import ctypes
import ctypes.util
import platform

# os.getauxval / os.AT_HWCAP do not exist in CPython, so call libc's
# getauxval(3) directly (Linux/AArch64 only)
AT_HWCAP      = 16
HWCAP_FPHP    = 1 << 9   # scalar FP16
HWCAP_ASIMDHP = 1 << 10  # SIMD FP16

if platform.system() == "Linux" and platform.machine() in ("aarch64", "arm64"):
    libc = ctypes.CDLL(ctypes.util.find_library("c"))
    libc.getauxval.restype  = ctypes.c_ulong
    libc.getauxval.argtypes = [ctypes.c_ulong]

    hwcap = libc.getauxval(AT_HWCAP)

    fphp    = bool (hwcap & HWCAP_FPHP)
    asimdhp = bool (hwcap & HWCAP_ASIMDHP)

    print (f"AT_HWCAP value: {hwcap}")

    print (f"Bitmask (hex): {hex(hwcap)}")
    print (f"  FPHP   : {fphp}")
    print (f"  ASIMDHP: {asimdhp}")
else:
    print (f"AT_HWCAP check needs Linux/AArch64 (this is {platform.system()}/{platform.machine()}).")
