#ifndef EVENT_TYPES_H
#define EVENT_TYPES_H

// Machine Check Exception (MCE) event structure
typedef struct {
    uint64_t timestamp;
    uint8_t cpu;
    uint64_t status;
    uint64_t addr;
    uint64_t misc;
} mce_event_t;

// Memory ECC Fault event structure
typedef struct {
    uint64_t timestamp;  // Event timestamp (ns)
    uint64_t page;       // Affected memory page (pfn)
    uint32_t error_type; // Correctable/Uncorrectable
    uint32_t pid;        // Process ID affected
} mem_event_t;

// Storage I/O event structure
typedef struct {
    uint64_t timestamp;       // Event timestamp (ns)
    uint64_t sector;          // Affected disk sector
    uint32_t error_code;      // Error code (e.g., EIO, ENOSPC)
    uint32_t rw_flag;         // Read (0) / Write (1)
    uint32_t device;          // Block device ID
} storage_event_t;

#endif // EVENT_TYPES_H
