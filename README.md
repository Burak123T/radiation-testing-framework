# **COTS Radiation Testing Framework**

## **TO-DO**

- [x] Add configuration using file (Config is currently not being utilized besides activating different handlers)
- [x] Structured logging and offloading to other device using UART and CAN (should be extensible to other user-defined interfaces)
- [ ] Find universal memory failure tracepoint for the memory monitoring
- [ ] Add more storage and FS related failure tracepoints
- [ ] Add tests for triggering errors
- [x] Provide API for tracking user-space applications / benchmarks (work in progress - only one example function is traces, needs to be expanded with more interesting examples, such as latency/throughput calculation)

## **Overview**

This project provides a **COTS (Commercial Off-The-Shelf) Radiation Testing Framework** for **Linux-based processors**. It leverages **eBPF** to monitor and log **CPU, memory, and storage faults** in a **radiation environment**. The framework is designed to be modular, extensible, and efficient, ensuring minimal impact on system performance.

## **Features**

- **CPU Monitoring**: Detects Machine Check Exceptions (MCEs) using eBPF tracepoints.
- **Memory Monitoring**: Captures ECC faults and memory failures.
- **Storage Monitoring**: Tracks I/O errors, filesystem corruption, and storage faults.
- **Efficient eBPF-based event handling** with **userspace processing**.
- **Modular Architecture**: Easy to extend for additional monitoring requirements.
- **Test Suite**: Provides automated test cases for each event type.

---

## **Project Structure**

```
📂 radiation-testing-framework
│── 📂 blazesym                    # External library for address symbolization and related tasks
│── 📂 bpftool                     # A command-line tool to inspect and manage BPF objects
│── 📂 framework
│   │── 📂 include                 # header files
│   │   ├── event_types.h
│   │   ├── event_handler.h
│   │── 📂 src
│   │   │── 📂 bpf
│   │   │   ├── cpu_monitor.bpf.c      # eBPF program for CPU/MCE monitoring
│   │   │   ├── mem_monitor.bpf.c      # eBPF program for Memory ECC monitoring
│   │   │   ├── storage_monitor.bpf.c  # eBPF program for Storage errors
│   │   │── 📂 user
│   │   │   ├── main.c                  # Main orchestrator for all event collection
│   │   │   ├── cpu_event_handler.c     # Handles CPU/MCE events + setup
│   │   │   ├── mem_event_handler.c     # Handles Memory ECC events + setup
│   │   │   ├── storage_event_handler.c # Handles Storage errors + setup
│   ├── CMakeLists.txt
│── 📂 libbpf                      # External library containing a BPF loader
│── 📂 tools                       # Misc. tools and CMake helpers
│── 📂 vmlinux.h                   # Header file containing all type definitions of a kernel
```

---

## **Installation & Setup**

### **:one: Install Required Dependencies**

Ensure that `clang`, `bpftool`, and `gcc` are installed:

```sh
sudo apt update
sudo $ apt install clang libelf1 libelf-dev zlib1g-dev
```

### **:two: Clone the Repository**

```sh
git clone --recurse-submodules https://github.com/Resource-Aware-Data-systems-RAD/radiation-testing-framework
cd radiation-testing-framework
```

### **:three: Build the Project**

```sh
sudo ./configure
```

### **:four: Run the Monitoring System**

```sh
sudo ./build/radiation_testing_framework -v -c framework/configs/all.ini
```

### **Individual Tests (To be uploaded)**

```sh
sudo ./test_cpu_events
sudo ./test_mem_events
sudo ./test_storage_events
```

---

## **Development & Contribution**

### **Adding New eBPF Programs**

1. Place new `.bpf.c` files in `framework/src/bpf/`
2. Add new event handlers in `framework/src/user/`.
3. Add type definitions to `framework/include/event_handler.h`
4. Make the handler available in `framework/src/user/main.c`
5. Run the installation and setup steps again.

### **Debugging eBPF Programs**

- Use `bpftool prog show` to inspect loaded eBPF programs.
- Use `dmesg` or `trace_pipe` to view kernel logs:
  
  ```sh
  cat /sys/kernel/debug/tracing/trace_pipe
  ```

<!-- ---

## **License**

This project is licensed under the **MIT License**.

---

## **Acknowledgments**

Special thanks to the **Linux eBPF community** and the **libbpf** maintainers for their open-source contributions.

---

## **Contact**

For issues, suggestions, or contributions, please open an **issue** or submit a **pull request** on GitHub.

🚀 **Happy Monitoring!** 🚀 -->
