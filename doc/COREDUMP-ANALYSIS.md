# Coredump Analysis Guide

This document describes how to capture, download, and analyze coredumps from your Pixelix device using the REST API and ESP-IDF tools.

## Overview

When a crash occurs on the Pixelix device, the ESP32 firmware captures a complete system snapshot (coredump) to flash memory. This includes:

- Exception cause and program counter
- All CPU registers at the time of crash
- Stack memory of the crashed task
- Backtrace information
- Coredump version and application SHA256 hash

Two analysis methods are available:

| Method                      | Endpoint                           | Content       | Use Case                            |
| --------------------------- | ---------------------------------- | ------------- | ----------------------------------- |
| **On-device decode**        | `GET /api/v1/coredump?decode=true` | JSON summary  | Quick web UI inspection             |
| **Host-side full analysis** | `GET /api/v1/coredump/download`    | Raw ELF image | All tasks, detailed stacks, symbols |

## On-Device Decode (REST API)

Get a JSON summary of the crash directly from the device.

### Request

```bash
curl -u admin:password http://<device>/rest/api/v1/coredump?decode=true
```

### Response (200 OK)

```json
{
  "status": "ok",
  "data": {
    "crashedTask": "IDLE0",
    "crashedTaskTcb": "0x3fca4f20",
    "exceptionCause": "LoadProhibited",
    "exceptionCode": 28,
    "exceptionAddr": "0x00000000",
    "exceptionPc": "0x40084659",
    "coredumpVersion": 2,
    "appElfSha256": "9f2f6f7f1c74f58f00112233445566778899aabbccddeeff0011223344556677",
    "registers": {
      "A0": "0x80090c10",
      "A1": "0x3ffbf1bc",
      "A2": "0x00000000",
      ...
      "A15": "0x00000000",
      "EXCCAUSE": 28,
      "EXCVADDR": "0x00000000"
    },
    "epcx": {
      "EPC1": "0x40081234"
    },
    "backtrace": [
      "0x40084659",
      "0x4008abcd",
      "0x40091234"
    ],
    "backtraceDepth": 3,
    "backtraceCorrupted": false
  }
}
```

### Field Descriptions

- **crashedTask**: Name of the FreeRTOS task that was executing when the exception occurred
- **crashedTaskTcb**: Task Control Block address (useful for memory analysis)
- **exceptionCause**: Human-readable exception type (LoadProhibited, StoreProhibited, IntegerDivideByZero, etc.)
- **exceptionCode**: Numeric exception cause code (architecture-specific)
- **exceptionAddr**: Virtual address that triggered the exception (for memory access errors)
- **exceptionPc**: Program counter at the time of exception
- **coredumpVersion**: ELF coredump format version
- **appElfSha256**: SHA256 hash of the application firmware (for matching against build artifacts)
- **registers**: All Xtensa A-registers (A0–A15) at exception time, plus EXCCAUSE and EXCVADDR
- **epcx**: Exception PC registers at interrupt levels 1–7 (if available)
- **backtrace**: Array of return address pointers, ordered from most recent to oldest
- **backtraceDepth**: Number of valid backtrace entries
- **backtraceCorrupted**: True if backtrace integrity check failed (stack corruption suspected)

## Host-Side Full Analysis (ELF Download)

Download the raw coredump as an ELF image and use ESP-IDF tools for comprehensive analysis.

### Step 1: Download the Coredump

```bash
# Download raw coredump
curl -u admin:password -o coredump.elf http://<device>/rest/api/v1/coredump/download

# Verify file integrity (should be ELF format)
file coredump.elf
# Output: coredump.elf: ELF 32-bit LSB executable, Xtensa Instruction Set, version 1 (SYSV), ...
```

### Step 2: Analyze with ESP-IDF

Ensure your ESP-IDF environment is properly configured with the correct app binary.

```bash
# Set IDF_PATH if not already set
export IDF_PATH=<path-to-esp-idf>

# Run coredump analysis
idf.py coredump-info coredump.elf

# Expected output:
# Coredump version: 2
# Crashed task handle: 0x3fca4f20
# Crashed task name: IDLE0
# Exception cause: LoadProhibited (28)
# Exception vaddr: 0x00000000
# Exception PC: 0x40084659
# A0: 0x80090c10, A1: 0x3ffbf1bc, ...
# Backtrace: 0x40084659 0x4008abcd 0x40091234
```

### Step 3: Analyze with GDB (Optional)

For symbol-level debugging, use GDB with the application ELF:

```bash
# Start GDB with the app binary
xtensa-esp32-elf-gdb -ex "target remote |idf.py coredump-debug" \
  build/esp32/firmware.elf

# In GDB:
(gdb) bt
(gdb) info registers
(gdb) x/16x $a1         # Inspect stack memory
(gdb) disassemble 0x40084659  # Disassemble crash location
```

## Error Responses

### 404: No Coredump Found

```bash
curl -u admin:password http://<device>/rest/api/v1/coredump?decode=true

# Response (404):
{
  "status": "error",
  "error": {
    "msg": "No coredump data found"
  }
}
```

**Cause**: No crash has been recorded, or the coredump partition has been cleared.

### 404: Coredump Partition Not Found

```json
{
  "status": "error",
  "error": {
    "msg": "Coredump partition not found"
  }
}
```

**Cause**: The device's partition table does not include a coredump partition. Ensure the firmware is built with:

```ini
CONFIG_ESP_COREDUMP_ENABLE=y
CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH=y
CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF=y
CONFIG_ESP_COREDUMP_CHECKSUM_CRC32=y
```

## Clearing Coredumps

After analysis, clear the coredump to free partition space for future crashes:

```bash
curl -X DELETE -u admin:password http://<device>/rest/api/v1/coredump

# Response (200):
{
  "status": "ok",
  "data": {}
}
```

## Troubleshooting

### "Failed to validate coredump data" (500)

**Symptom**: Coredump partition exists but returns validation error.

**Cause**: Coredump data is corrupted (checksum mismatch).

**Fix**: Clear the coredump and trigger a new crash or investigate the device logs.

### Backtrace Corruption Warning

**Symptom**: `"backtraceCorrupted": true` in JSON response.

**Cause**: Stack memory was overwritten or corrupted, making the backtrace unreliable.

**Recommendation**:

- Check for stack overflows in the crashed task
- Review memory allocation patterns
- Look for buffer overflows or use-after-free bugs

### Symbol Resolution Fails in GDB

**Symptom**: `(gdb) bt` shows only addresses, not function names.

**Cause**: The app binary doesn't match the firmware that crashed (SHA256 mismatch).

**Fix**:

1. Verify the firmware version matches the device
2. Rebuild the firmware from the same source
3. Cross-check the `appElfSha256` from coredump with your build artifacts:

    ```bash
    # Extract SHA256 from your build
    xtensa-esp32-elf-readelf -p .elf_sha256_digest build/esp32/firmware.elf
   ```

## Common Exception Causes (Xtensa ESP32)

| Code | Cause                 | Typical Reason                                     |
| ---- | --------------------- | -------------------------------------------------- |
| 0    | IllegalInstruction    | Invalid opcode executed                            |
| 2    | InstructionFetchError | Attempt to fetch instruction from unmapped memory  |
| 3    | LoadStoreError        | Generic memory access error                        |
| 6    | IntegerDivideByZero   | Division by zero                                   |
| 9    | LoadStoreAlignment    | Unaligned memory access                            |
| 28   | LoadProhibited        | Read from non-readable memory (null pointer, etc.) |
| 29   | StoreProhibited       | Write to non-writable memory (ROM, etc.)           |

For a complete list, refer to the Xtensa ISA documentation or the coredump decoder source in [src/General/CoredumpDecoder.cpp](src/General/CoredumpDecoder.cpp).

## Workflow Example: Debugging a Null Pointer Dereference

**Scenario**: Device crashes with `LoadProhibited` (code 28) at address 0x00000000.

**Steps**:

1. **Fetch coredump summary**:

   ```bash
   curl -u admin:password http://<device>/rest/api/v1/coredump?decode=true | jq '.data.registers'
   ```

2. **Identify the faulting task**:

   ```json
   "crashedTask": "MyPlugin",
   "exceptionPc": "0x40084659"
   ```

3. **Find source location**:

   ```bash
   xtensa-esp32-elf-addr2line -e build/esp32/firmware.elf 0x40084659
   # Output: /path/to/project/lib/MyPlugin/src/MyPlugin.cpp:123
   ```

4. **Examine the code at that location** and look for unchecked pointer dereferences.

5. **Download and analyze the full coredump if needed**:

   ```bash
   curl -o coredump.elf http://<device>/rest/api/v1/coredump/download
   idf.py coredump-info coredump.elf
   ```

6. **Clear the coredump after analysis**:

   ```bash
   curl -X DELETE http://<device>/rest/api/v1/coredump
   ```

## References

- [ESP-IDF Coredump Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/coredump.html)
- [Xtensa ISA Reference](https://www.tensilica.com/products/software-tools/xtensa-software-development-toolkit)
- [REST API: /coredump endpoints](./OpenAPI/OpenAPI.yaml) (see paths → /coredump)
