// include/event_types.h
#ifndef EVENT_TYPES_H
#define EVENT_TYPES_H

// #include "vmlinux.h"

// #define DEVICE_NAME_LEN 32

// Machine Check Exception (MCE) event structure
struct mce_event_t
{
    uint64_t timestamp;
    uint8_t cpu;
    uint64_t status;
    uint64_t addr;
    uint64_t misc;
};

// Memory ECC Fault event structure
struct mem_event_t {
    uint64_t timestamp;  // Event timestamp (ns)
    uint64_t page;       // Affected memory page (pfn)
    uint32_t error_type; // Correctable/Uncorrectable
    uint32_t pid;        // Process ID affected
};

// Storage I/O event structure
struct storage_event_t {
    uint64_t timestamp;       // Event timestamp (ns)
    uint64_t sector;          // Affected disk sector
    uint32_t error_code;      // Error code (e.g., EIO, ENOSPC)
    uint32_t rw_flag;         // Read (0) / Write (1)
    uint32_t device;          // Block device ID
};

#endif // EVENT_TYPES_H
