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

// ARM event structure
typedef struct {
    uint64_t time;
    uint8_t cpu;
    uint64_t mpidr;                 // Multiprocessor Affinity Register
	uint64_t midr;                  // Main ID Register
	uint32_t running_state;
	uint32_t psci_state;            // Power State Coordination Interface
	uint8_t affinity;
} arm_event_t;

// MC (Memory Controller) event structure
typedef struct {
    uint64_t time;
    uint8_t cpu;
    unsigned int error_type;
    uint32_t __data_loc_msg;         // data location of error message
	uint32_t __data_loc_label;       // data location of error label
	uint16_t error_count;
	uint8_t mc_index;
	long int address;
	uint8_t grain_bits;
	long int syndrome;
	uint32_t __data_loc_driver_detail;
} mc_event_t;

// Non-Standard event structure
typedef struct {
    uint64_t time;
    uint8_t cpu;
    char sec_type[16];              // Section type
    char fru_id[16];
	uint32_t __data_loc_fru_text;   // Data location of FRU text
    char* fru_text;
	uint8_t sev;                    // Severity
	uint32_t len;
	uint32_t __data_loc_buf;
} ns_event_t;

// AER trace event
typedef struct {
    uint64_t time;
    uint8_t cpu;
    uint32_t __data_loc_dev_name;   // Data location of device name
    uint32_t status;
    uint8_t severity;
    char* dev_name;
} aer_event_t;

#endif // EVENT_TYPES_H
