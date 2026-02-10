<div align="center">

# My Thread Safe Malloc

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

[**English**](README.md) | [**中文**](README_CN.md)

</div>

---

## Project Overview

This project implements a thread-safe dynamic memory allocator in C. It provides both **lock-based** and **non-lock-based (Thread-Local Storage)** versions of `malloc` and `free`, along with First-Fit and Best-Fit allocation strategies.

## Features

- **Allocation Strategies**: First-Fit (`ff_malloc`) and Best-Fit (`bf_malloc`).
- **Thread Safety**:
  - **Lock-based**: Uses a global mutex to protect the shared heap.
  - **Non-lock-based**: Utilizes Thread-Local Storage (TLS) and `sbrk` to minimize lock contention and improve performance in multi-threaded environments.
- **Efficiency**: Implements block splitting and coalescing to reduce fragmentation.
- **Reporting**: Tools to track total free size and the largest free data segment.

## API Reference

| Function | Description |
| --- | --- |
| `ts_malloc_lock(size_t size)` | Thread-safe allocation using a global lock. |
| `ts_free_lock(void *ptr)` | Thread-safe free using a global lock. |
| `ts_malloc_nolock(size_t size)` | Thread-safe allocation using Thread-Local Storage (no lock). |
| `ts_free_nolock(void *ptr)` | Thread-safe free using Thread-Local Storage (no lock). |
| `ff_malloc(size_t size)` | Standard First-Fit allocation. |
| `bf_malloc(size_t size)` | Standard Best-Fit allocation. |

## Compilation and Usage

To build the library:
```bash
make
```
This generates `libmymalloc.so`.

To run tests:
```bash
cd thread_tests
make
./thread_test
```