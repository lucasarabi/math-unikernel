#ifndef KERNEL_LOGS_MU_H
#define KERNEL_LOGS_MU_H

#define LIMINE_SUCCESS_LOG              "READY: Limine handshake successful.\n"
#define GDT_INITIALIZED                 "READY: GDT has been initialized and loaded.\n"
#define IDT_INITIALIZED                 "READY: IDT has been initialized and loaded.\n"
#define PMM_INITIALIZED                 "READY: PMM has been initialized.\n"
#define VMM_INITIALIZED                 "READY: VMM has been initialized and loaded.\n"
#define SIMD_ENABLED                    "READY: AVX/SSE enabled.\n"
#define DISPLAY_INITIALIZED             "READY: Display has been initialized.\n"
#define SERIAL_DRIVER_INITIALIZED       "READY: Serial drivers have been initialized.\n"
#define NETWORK_CONTROLLER_FOUND        "READY: Network Controller found on PCI bus.\n"

#define LIMINE_FAILURE_LOG              "ERROR: Limine handshake failed.\n"
#define GDT_FAILURE                     "ERROR: GDT initialization failed.\n"
#define IDT_FAILURE                     "ERROR: IDT initialization failed.\n"
#define PMM_FAILURE                     "ERROR: PMM initialization failed.\n"
#define VMM_FAILURE                     "ERROR: VMM initialization failed.\n"
#define SIMD_FAILURE                    "ERROR: AVX/SSE not supported by CPU.\n"
#define SERIAL_DRIVER_FAILURE           "ERROR: Serial driver initialization failed.\n"
#define NETWORK_CONTROLLER_MISSING      "ERROR: Network Controller not found on PCI bus.\n"

#define STATE_POLLING                   "STATE: Polling\n"
#define STATE_EXECUTING                 "STATE: Executing\n"
#define STATE_EXTRACTING                "STATE: Extracting\n"

#define KERNEL_FINISH                   "Finished kernel execution. Exiting.\n"

#endif