/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

/*** Fore more details refer to VESA BIOS Extension Standard v.3.0
 *** or visit www.vesa.org
 ***/

#ifndef _VESA_VBE_H_
#define _VESA_VBE_H_

/* Protected Mode Info Block */
struct PMInfoBlock {
    char   Signature[4];   /* PM Info Block Signature (should be 'PMID') */
    uint16 EntryPoint;     /* Offset of PM entry point within BIOS       */
    uint16 PMInitialize;   /* Offset of PM initialization entry point    */
    uint16 BIOSDataSel;    /* Selector to BIOS data area emulation block */
    uint16 A0000Sel;       /* Selector to access A0000h physical mem     */
    uint16 B0000Sel;       /* Selector to access B0000h physical mem     */
    uint16 B8000Sel;       /* Selector to access B8000h physical mem     */
    uint16 CodeSegSel;     /* Selector to access code segment as data    */
    uint8  InProtectMode;  /* Set to 1 when in protected mode            */
    uint8  Checksum;       /* Checksum byte for structure                */
} _PACKED;

/* VBE Info Block */
struct VbeInfoBlock {
    char   VbeSignature[4];    /* VBE signature (should be 'VESA' or 'VBE2')   */
    uint16 VbeVersion;         /* VBE Version                                  */
    uint32 OemStringPtr;       /* VbeFarPtr to OEM String                      */
    uint32 Capabilities;       /* Capabilities of graphics controller          */
    uint32 VideoModePtr;       /* VbeFarPtr to VideoModeList                   */
    uint16 TotalMemory;        /* Number of 64Kb memory blocks                 */
    /* VBE 2.0+ fields */
    uint16 OemSoftwareRev;     /* VBE implementation Software revision         */
    uint32 OemVendorNamePtr;   /* VbeFarPtr to Vendor Name String              */
    uint32 OemProductNamePtr;  /* VbeFarPtr to Product Name String             */
    uint32 OemProductRevPtr;   /* VbeFarPtr to Product Revision String         */
    uint8  Reserved[222];      /* Reserved for VBE implementation scratch area */
    uint8  OemData[256];       /* Data Area for OEM Strings                    */
} _PACKED;

/* Mode Info Block */
struct ModeInfoBlock {
    /* Mandatory information for all VBE revisions */
    uint16 ModeAttributes;        /* Mode attributes                                     */
    uint8 WinAAttributes;         /* Window A attributes                                 */
    uint8 WinBAttributes;         /* Window B attributes                                 */
    uint16 WinGranularity;        /* Window granularity                                  */
    uint16 WinSize;               /* Window size                                         */
    uint16 WinASegment;           /* Window A start segment                              */
    uint16 WinBSegment;           /* Window B start segment                              */
    uint32 WinFuncPtr;            /* Real mode pointer to window function                */
    uint16 BytesPerScanLine;      /* Bytes per scan line                                 */
    /* Mandatory information for VBE 1.2 and above */
    uint16 XResolution;           /* Horizontal resolution in pixels or characters       */
    uint16 YResolution;           /* Vertical resolution in pixels or characters         */
    uint8 XCharSize;              /* Character cell width in pixels                      */
    uint8 YCharSize;              /* Character cell height in pixels                     */
    uint8 NumberOfPlanes;         /* Number of memory planes                             */
    uint8 BitsPerPixel;           /* Bits per pixel                                      */
    uint8 NumberOfBanks;          /* Number of banks                                     */
    uint8 MemoryModel;            /* Memory model type                                   */
    uint8 BankSize;               /* Bank size in Kb                                     */
    uint8 NumberOfImagePages;     /* Number of images                                    */
    uint8 RsvdForPageFunc;        /* Reserved for page function                          */
    /* Direct Color Fields (required for direct/6 and YUV/7 memory models) */
    uint8 RedMaskSize;            /* Size of direct color red mask in bits               */
    uint8 RedFieldPosition;       /* Bit position of lsb of red mask                     */
    uint8 GreenMaskSize;          /* Size of direct color green mask in bits             */
    uint8 GreenFieldPosition;     /* Bit position of lsb of green mask                   */
    uint8 BlueMaskSize;           /* Size of direct color blue mask in bits              */
    uint8 BlueFieldPosition;      /* Bit position of lsb of blue mask                    */
    uint8 RsvdMaskSize;           /* Size of direct color reserved mask in bits          */
    uint8 RsvdFieldPosition;      /* Bit position of lsb of reserved mask                */
    uint8 DirectColorModeInfo;    /* Direct color mode attributes                        */
    /* Mandatory information for VBE 2.0 and above */
    uint32 PhysBasePtr;           /* Physical address for flat memory frame buffer       */
    uint32 OffScreenMemOffset;    /* Reserved                                            */
    uint16 OffScreenMemSize;      /* Reserved                                            */
    /* Mandatory information for VBE 3.0 and above */
    uint16 LinBytesPerScanLine;   /* Bytes per scan line for linear modes                */
    uint8 BnkNumberOfImagePages;  /* Number of images for banked modes                   */
    uint8 LinNumberOfImagePages;  /* Number of images for linear modes                   */
    uint8 LinRedMaskSize;         /* Size of direct color red mask (linear modes)        */
    uint8 LinRedFieldPosition;    /* Bit position of lsb of red mask (linear modes)      */
    uint8 LinGreenMaskSize;       /* Size of direct color green mask (linear modes)      */
    uint8 LinGreenFieldPosition;  /* Bit position of lsb of green mask (linear modes)    */
    uint8 LinBlueMaskSize;        /* Size of direct color blue mask (linear modes)       */
    uint8 LinBlueFieldPosition;   /* Bit position of lsb of blue mask (linear modes)     */
    uint8 LinRsvdMaskSize;        /* Size of direct color reserved mask (linear modes)   */
    uint8 LinRsvdFieldPosition;   /* Bit position of lsb of reserved mask (linear modes) */
    uint32 MaxPixelClock;         /* Maximum pixel clock (in Hz) for graphics mode       */
    uint8 Reserved[189];          /* Remainder of ModeInfoBlock                          */
} _PACKED;

/* Type definitions */
typedef struct PMInfoBlock    PMInfoBlock_t;
typedef struct VbeInfoBlock   VbeInfoBlock_t;
typedef struct ModeInfoBlock  ModeInfoBlock_t;

#endif
