
#ifndef _PCI_DEF_H
#define _PCI_DEF_H

#include "nex_type.h"

#ifdef __cplusplus /*wdm.h*/
extern "C" {
#endif
/******************************************************************************
 *                         Memory manager Types                               *
 ******************************************************************************/
typedef enum _MEMORY_CACHING_TYPE_ORIG {
  MmFrameBufferCached = 2
} MEMORY_CACHING_TYPE_ORIG;

typedef enum _MEMORY_CACHING_TYPE {
  MmNonCached = FALSE,
  MmCached = TRUE,
  MmWriteCombined = MmFrameBufferCached,
  MmHardwareCoherentCached,
  MmNonCachedUnordered,
  MmUSWCCached,
  MmMaximumCacheType
} MEMORY_CACHING_TYPE;
/******************************************************************************
 *                            Configuration Manager Types                     *
 ******************************************************************************/
typedef enum _INTERFACE_TYPE {
  InterfaceTypeUndefined = -1,
  Internal,
  Isa,
  Eisa,
  MicroChannel,
  TurboChannel,
  PCIBus,
  VMEBus,
  NuBus,
  PCMCIABus,
  CBus,
  MPIBus,
  MPSABus,
  ProcessorInternal,
  InternalPowerBus,
  PNPISABus,
  PNPBus,
  Vmcs,
  MaximumInterfaceType
} INTERFACE_TYPE, *PINTERFACE_TYPE;

/******************************************************************************
 *                         I/O Manager Types                                  *
 ******************************************************************************/

#define PCI_COMMON_HEADER_LAYOUT                \
  USHORT VendorID;                              \
  USHORT DeviceID;                              \
  USHORT Command;                               \
  USHORT Status;                                \
  UCHAR RevisionID;                             \
  UCHAR ProgIf;                                 \
  UCHAR SubClass;                               \
  UCHAR BaseClass;                              \
  UCHAR CacheLineSize;                          \
  UCHAR LatencyTimer;                           \
  UCHAR HeaderType;                             \
  UCHAR BIST;                                   \
  union {                                       \
    struct /* _PCI_HEADER_TYPE_0 */ {                 \
      ULONG BaseAddresses[PCI_TYPE0_ADDRESSES]; \
      ULONG CIS;                                \
      USHORT SubVendorID;                       \
      USHORT SubSystemID;                       \
      ULONG ROMBaseAddress;                     \
      UCHAR CapabilitiesPtr;                    \
      UCHAR Reserved1[3];                       \
      ULONG Reserved2;                          \
      UCHAR InterruptLine;                      \
      UCHAR InterruptPin;                       \
      UCHAR MinimumGrant;                       \
      UCHAR MaximumLatency;                     \
    } type0;                                    \
    struct /* _PCI_HEADER_TYPE_1 */ {                 \
      ULONG BaseAddresses[PCI_TYPE1_ADDRESSES]; \
      UCHAR PrimaryBus;                         \
      UCHAR SecondaryBus;                       \
      UCHAR SubordinateBus;                     \
      UCHAR SecondaryLatency;                   \
      UCHAR IOBase;                             \
      UCHAR IOLimit;                            \
      USHORT SecondaryStatus;                   \
      USHORT MemoryBase;                        \
      USHORT MemoryLimit;                       \
      USHORT PrefetchBase;                      \
      USHORT PrefetchLimit;                     \
      ULONG PrefetchBaseUpper32;                \
      ULONG PrefetchLimitUpper32;               \
      USHORT IOBaseUpper16;                     \
      USHORT IOLimitUpper16;                    \
      UCHAR CapabilitiesPtr;                    \
      UCHAR Reserved1[3];                       \
      ULONG ROMBaseAddress;                     \
      UCHAR InterruptLine;                      \
      UCHAR InterruptPin;                       \
      USHORT BridgeControl;                     \
    } type1;                                    \
    struct /* _PCI_HEADER_TYPE_2 */ {                 \
      ULONG SocketRegistersBaseAddress;         \
      UCHAR CapabilitiesPtr;                    \
      UCHAR Reserved;                           \
      USHORT SecondaryStatus;                   \
      UCHAR PrimaryBus;                         \
      UCHAR SecondaryBus;                       \
      UCHAR SubordinateBus;                     \
      UCHAR SecondaryLatency;                   \
      struct {                                  \
        ULONG Base;                             \
        ULONG Limit;                            \
      } Range[PCI_TYPE2_ADDRESSES-1];           \
      UCHAR InterruptLine;                      \
      UCHAR InterruptPin;                       \
      USHORT BridgeControl;                     \
    } type2;                                    \
  } u;                                          \
  ULONG domain;/* PCI domain */

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _PCI_SLOT_NUMBER {
  union {
    struct {
      ULONG DeviceNumber:5;
      ULONG FunctionNumber:3;
      ULONG Reserved:24;
    } bits;
    ULONG AsULONG;
  } u;
} PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;
#ifdef __cplusplus
}
#endif

#define PCI_TYPE0_ADDRESSES               6
#define PCI_TYPE1_ADDRESSES               2
#define PCI_TYPE2_ADDRESSES               5

#ifdef __cplusplus
typedef struct _PCI_COMMON_CONFIG {
  PCI_COMMON_HEADER_LAYOUT
  UCHAR DeviceSpecific[192];
} PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;
#else
typedef struct _PCI_COMMON_CONFIG {
  __extension__ struct {
    PCI_COMMON_HEADER_LAYOUT
  };
  UCHAR DeviceSpecific[192];
} PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;
#endif

#define PCI_COMMON_HDR_LENGTH (FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceSpecific))

#define PCI_EXTENDED_CONFIG_LENGTH               0x1000

#define PCI_MAX_DEVICES        32
#define PCI_MAX_FUNCTION       8
#define PCI_MAX_BRIDGE_NUMBER  0xFF
#define PCI_INVALID_VENDORID   0xFFFF

/* PCI_COMMON_CONFIG.HeaderType */
#define PCI_MULTIFUNCTION                 0x80
#define PCI_DEVICE_TYPE                   0x00
#define PCI_BRIDGE_TYPE                   0x01
#define PCI_CARDBUS_BRIDGE_TYPE           0x02

#define PCI_CONFIGURATION_TYPE(PciData) \
  (((PPCI_COMMON_CONFIG) (PciData))->HeaderType & ~PCI_MULTIFUNCTION)

#define PCI_MULTIFUNCTION_DEVICE(PciData) \
  ((((PPCI_COMMON_CONFIG) (PciData))->HeaderType & PCI_MULTIFUNCTION) != 0)

/* PCI_COMMON_CONFIG.Command */
#define PCI_ENABLE_IO_SPACE               0x0001
#define PCI_ENABLE_MEMORY_SPACE           0x0002
#define PCI_ENABLE_BUS_MASTER             0x0004
#define PCI_ENABLE_SPECIAL_CYCLES         0x0008
#define PCI_ENABLE_WRITE_AND_INVALIDATE   0x0010
#define PCI_ENABLE_VGA_COMPATIBLE_PALETTE 0x0020
#define PCI_ENABLE_PARITY                 0x0040
#define PCI_ENABLE_WAIT_CYCLE             0x0080
#define PCI_ENABLE_SERR                   0x0100
#define PCI_ENABLE_FAST_BACK_TO_BACK      0x0200
#define PCI_DISABLE_LEVEL_INTERRUPT       0x0400

/* PCI_COMMON_CONFIG.Status */
#define PCI_STATUS_INTERRUPT_PENDING      0x0008
#define PCI_STATUS_CAPABILITIES_LIST      0x0010
#define PCI_STATUS_66MHZ_CAPABLE          0x0020
#define PCI_STATUS_UDF_SUPPORTED          0x0040
#define PCI_STATUS_FAST_BACK_TO_BACK      0x0080
#define PCI_STATUS_DATA_PARITY_DETECTED   0x0100
#define PCI_STATUS_DEVSEL                 0x0600
#define PCI_STATUS_SIGNALED_TARGET_ABORT  0x0800
#define PCI_STATUS_RECEIVED_TARGET_ABORT  0x1000
#define PCI_STATUS_RECEIVED_MASTER_ABORT  0x2000
#define PCI_STATUS_SIGNALED_SYSTEM_ERROR  0x4000
#define PCI_STATUS_DETECTED_PARITY_ERROR  0x8000

/* IO_STACK_LOCATION.Parameters.ReadWriteControl.WhichSpace */

#define PCI_WHICHSPACE_CONFIG             0x0
#define PCI_WHICHSPACE_ROM                0x52696350 /* 'PciR' */

#define PCI_CAPABILITY_ID_POWER_MANAGEMENT  0x01
#define PCI_CAPABILITY_ID_AGP               0x02
#define PCI_CAPABILITY_ID_VPD               0x03
#define PCI_CAPABILITY_ID_SLOT_ID           0x04
#define PCI_CAPABILITY_ID_MSI               0x05
#define PCI_CAPABILITY_ID_CPCI_HOTSWAP      0x06
#define PCI_CAPABILITY_ID_PCIX              0x07
#define PCI_CAPABILITY_ID_HYPERTRANSPORT    0x08
#define PCI_CAPABILITY_ID_VENDOR_SPECIFIC   0x09
#define PCI_CAPABILITY_ID_DEBUG_PORT        0x0A
#define PCI_CAPABILITY_ID_CPCI_RES_CTRL     0x0B
#define PCI_CAPABILITY_ID_SHPC              0x0C
#define PCI_CAPABILITY_ID_P2P_SSID          0x0D
#define PCI_CAPABILITY_ID_AGP_TARGET        0x0E
#define PCI_CAPABILITY_ID_SECURE            0x0F
#define PCI_CAPABILITY_ID_PCI_EXPRESS       0x10
#define PCI_CAPABILITY_ID_MSIX              0x11

/******************************************************************************
 *                       Memory manager Functions                             *
 ******************************************************************************/
#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field) ((ULONG)&(((type *)0)->field))
#endif

#ifndef FIELD_SIZE
#define FIELD_SIZE(type, field) (sizeof(((type *)0)->field))
#endif

#ifdef __cplusplus
}
#endif/*wdm.h*/


#ifdef __cplusplus/*ntddk.h*/
extern "C" {
#endif

typedef enum _BUS_DATA_TYPE {
  ConfigurationSpaceUndefined = -1,
  Cmos,
  EisaConfiguration,
  Pos,
  CbusConfiguration,
  PCIConfiguration,
  VMEConfiguration,
  NuBusConfiguration,
  PCMCIAConfiguration,
  MPIConfiguration,
  MPSAConfiguration,
  PNPISAConfiguration,
  SgiInternalConfiguration,
  MaximumBusDataType
} BUS_DATA_TYPE, *PBUS_DATA_TYPE;

#ifdef __cplusplus
}
#endif/*ntddk.h*/

#endif