# Heap Memory Allocator in C

This project implements a low-level dynamic memory allocator in C, simulating key behaviors of `malloc()` and `free()` using a fixed-size heap region backed by memory-mapped I/O. The allocator supports block splitting, coalescing, and precise alignment, following best-fit placement and embedded metadata techniques for managing allocation status.

---

## Features

- **Best-Fit Allocation Policy**: Finds the smallest suitable free block to reduce fragmentation.
- **8-Byte Alignment**: All payload pointers and block sizes are aligned to 8 bytes.
- **Splitting**: Large free blocks are split when excess space remains after allocation.
- **Coalescing**: Adjacent free blocks are automatically merged during deallocation.
- **Block Metadata Encoding**:
  - Size and status bits (allocated/free, previous block status) are packed into headers.
  - Free blocks also include a footer for coalescing support.
- **Custom Functions**:
  - `init_heap(size_t size)` — Initializes a memory-mapped heap.
  - `balloc(int size)` — Allocates a block of memory.
  - `bfree(void* ptr)` — Frees a previously allocated block.
  - `disp_heap()` — Debugging tool that prints current heap block layout.

---

## File Structure

```
.
├── p3Heap.c          # Main implementation file
├── p3Heap.h          # Provided header with interface (DO NOT MODIFY)
├── Makefile          # Builds shared library heaplib.so
├── tests/            # Provided test suite for allocation and freeing logic
│   ├── test101       # Single 8-byte allocation
│   ├── test102       # Multiple allocations with splitting
│   ├── test201       # Deallocation tests
│   └── ...           # More test cases covering alignment and coalescing
```

---

## Compilation & Usage

### 1. Build the shared library
From the project root:
```bash
make
```

This will compile `p3Heap.c` into a shared object: `heaplib.so`.

### 2. Compile & run tests
Navigate to the `tests/` directory and build the test binaries:
```bash
cd tests/
make
./test101   # Run a specific test
```

Each test validates allocator behavior (e.g., splitting, alignment, freeing). If there's no output, the test passed.

---

## Example (Manual Testing)

```c
#include "p3Heap.h"

int main() {
    init_heap(4096);                   // Initialize 4KB heap
    void* a = balloc(64);              // Allocate 64 bytes
    void* b = balloc(128);             // Allocate another block
    bfree(a);                          // Free the first block
    disp_heap();                       // Print current heap state
}
```

---

## Technologies Used

- **Language:** C
- **Concepts:** Pointer arithmetic, memory-mapped I/O (`mmap`), alignment, block metadata encoding, bit masking
- **Build Tools:** GCC, Make
- **Test Framework:** Provided black-box tests for allocator behavior

---

## Testing Breakdown

Tests are organized into difficulty tiers:

### Part A: Allocation (`balloc`)
- `test101` – Simple allocation
- `test102` – Multiple allocations + splitting
- `test103` – Varied allocation sizes
- `test121`– Pointer alignment check

### Part B: Deallocation (`bfree`)
- `test201`, `test202` – Freeing blocks, checking p-bit
- `test211` to `test214` – Best-fit reuse and memory reclamation

### Part C: Coalescing
- `test301`–`test303` – Immediate coalescing verification

---

## License & Academic Integrity

This implementation is based on course-assigned specifications. Redistribution or reuse outside educational purposes is not permitted.

---

## Contact

Feel free to reach out via [GitHub Issues](https://github.com/LoganByard) for questions or improvements.
