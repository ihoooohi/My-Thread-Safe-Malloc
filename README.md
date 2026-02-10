# My-Thread-Safe-Malloc

[English](#english) | [中文](#中文)

---

## English

### Project Overview
This project implements a thread-safe dynamic memory allocator in C. It provides both **lock-based** and **non-lock-based (Thread-Local Storage)** versions of `malloc` and `free`, along with First-Fit and Best-Fit allocation strategies.

### Features
- **Allocation Strategies**: First-Fit (`ff_malloc`) and Best-Fit (`bf_malloc`).
- **Thread Safety**:
  - **Lock-based**: Uses a global mutex to protect the shared heap.
  - **Non-lock-based**: Utilizes Thread-Local Storage (TLS) and `sbrk` to minimize lock contention and improve performance in multi-threaded environments.
- **Efficiency**: Implements block splitting and coalescing to reduce fragmentation.
- **Reporting**: Tools to track total free size and the largest free data segment.

### API Reference
| Function | Description |
| --- | --- |
| `ts_malloc_lock(size_t size)` | Thread-safe allocation using a global lock. |
| `ts_free_lock(void *ptr)` | Thread-safe free using a global lock. |
| `ts_malloc_nolock(size_t size)` | Thread-safe allocation using Thread-Local Storage (no lock). |
| `ts_free_nolock(void *ptr)` | Thread-safe free using Thread-Local Storage (no lock). |
| `ff_malloc(size_t size)` | Standard First-Fit allocation. |
| `bf_malloc(size_t size)` | Standard Best-Fit allocation. |

### Compilation and Usage
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

---

## 中文

### 项目概述
本项目使用 C 语言实现了一个线程安全的动态内存分配器。它提供了**基于锁**和**非基于锁（线程局部存储 TLS）**两个版本的 `malloc` 和 `free`，并实现了“首次适应 (First-Fit)”和“最佳适应 (Best-Fit)”分配策略。

### 主要特性
- **分配策略**：支持首次适应 (`ff_malloc`) 和最佳适应 (`bf_malloc`)。
- **线程安全**：
  - **锁版本 (Lock-based)**：使用全局互斥锁保护共享堆空间。
  - **非锁版本 (Non-locking)**：利用线程局部存储 (TLS) 和 `sbrk` 减少锁竞争，显著提升多线程环境下的性能。
- **高效管理**：实现内存块拆分 (Splitting) 和合并 (Coalescing)，有效减少内存碎片。
- **监控报告**：提供查询总空闲大小及最大空闲段大小的接口。

### API 参考
| 函数 | 描述 |
| --- | --- |
| `ts_malloc_lock(size_t size)` | 基于全局锁的线程安全分配。 |
| `ts_free_lock(void *ptr)` | 基于全局锁的线程安全释放。 |
| `ts_malloc_nolock(size_t size)` | 基于 TLS 的线程安全分配（无锁）。 |
| `ts_free_nolock(void *ptr)` | 基于 TLS 的线程安全释放（无锁）。 |
| `ff_malloc(size_t size)` | 标准首次适应分配。 |
| `bf_malloc(size_t size)` | 标准最佳适应分配。 |

### 编译与使用
编译动态库：
```bash
make
```
该命令将生成 `libmymalloc.so`。

运行测试：
```bash
cd thread_tests
make
./thread_test
```
