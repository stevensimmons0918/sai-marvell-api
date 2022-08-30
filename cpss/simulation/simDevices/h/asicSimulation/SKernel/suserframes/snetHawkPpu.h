/*******************************************************************************
*              (c), Copyright 2019, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* snetHawkPpu.h
*
* DESCRIPTION:
*       This is a external API definition for SIP6 PPU
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __snetHawkPpuh
#define __snetHawkPpuh

#include <asicSimulation/SKernel/smain/smain.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* header segment length */
#define SNET_SIP6_10_PPU_HD_LEN_MAX_CNS 32
/* Total header size */
#define SNET_SIP6_10_PPU_TOTAL_HD_LEN_MAX_CNS 160
/* UDB start number */
#define SNET_SIP6_10_PPU_UDB_START_CNS 30
/* number of udbs */
#define SNET_SIP6_10_PPU_UDBS_MAX_CNS 8
/* UDB meta data length - 8 byte udbs + 1 byte valid */
#define SNET_SIP6_10_PPU_UDB_METADATA_LEN_MAX_CNS 9
/* UDB meta data anchor type */
#define SNET_SIP6_10_PPU_UDB_METADATA_ANCHOR_TYPE_CNS 6
/* number of ppu profiles */
#define SNET_SIP6_10_PPU_PROFILE_ENTRIES_MAX_CNS 16
/* number of SP_BUS profile entries */
#define SNET_SIP6_10_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS 4
/* SP_BUS size in bytes */
#define SNET_SIP6_10_PPU_SP_BUS_BYTES_MAX_CNS 20
/* SP_BUS size in 32byte workds */
#define SNET_SIP6_10_PPU_SP_BUS_WORDS_MAX_CNS 5
/* number of K_stgs */
#define SNET_SIP6_10_PPU_KSTGS_MAX_CNS 3
/* number of key generation profiles */
#define SNET_SIP6_10_PPU_KSTG_KEY_GEN_PROFILES_MAX_CNS 16
/* number of key generation profile byte selections */
#define SNET_SIP6_10_PPU_KSTG_KEY_GEN_PROFILE_BYTE_SELECT_MAX_CNS 7
/* key generation input bus length */
#define SNET_SIP6_10_PPU_KSTG_KEY_GEN_INPUT_BUS_LEN_CNS 64
/* key generation profile byte offset length */
#define SNET_SIP6_10_PPU_KSTG_KEY_GEN_PROFILE_BYTE_OFFSET_LEN_CNS 6
/* K_stg tcam key length */
#define SNET_SIP6_10_PPU_KSTG_TCAM_KEY_LEN_CNS 7
/* number of K_stg tcam entries */
#define SNET_SIP6_10_PPU_KSTG_TCAM_ENTRIES_MAX_CNS 64
/* number of ROTs in each KSTG */
#define SNET_SIP6_10_PPU_KSTG_ROTS_MAX_CNS 5
/* number of DAU profile entries */
#define SNET_SIP6_10_PPU_DAU_PROFILE_ENTRIES_MAX_CNS 16
/* number of DAU descriptor bytes */
#define SNET_SIP6_10_PPU_DAU_DESC_BYTE_SET_MAX_CNS 20
/* number of DAU protected windows */
#define SNET_SIP6_10_PPU_DAU_DATA_PROTECTED_WINDOW_MAX_CNS 5
/* DAU profile index mask */
#define SNET_SIP6_10_PPU_DAU_PROFILE_IDX_MASK 0xF
/* number ppu error profile fields */
#define SNET_SIP6_10_PPU_ERROR_PROFILE_FIELDS_MAX_CNS 4
/* number of error profiles */
#define SNET_SIP6_10_PPU_ERROR_PROFILES_MAX_CNS 2
/* number of ppu interrupts */
#define SNET_SIP6_10_PPU_INT_IDX_MAX_CNS 8
/* hardware descriptor max size in bytes */
#define SNET_SIP6_10_PPU_DAU_HW_DESC_LEN_IN_BYTES_CNS 300

/* structure definitions */

/* struct for interrupts */
typedef struct {
    GT_U32  intCause;   /* Interrupt Cause bit mask */
    GT_U32  intMask;    /* Interrupt mask */
    GT_U32  lastAddrViolation; /* capture last address violation */
    GT_U32   intCounter[SNET_SIP6_10_PPU_INT_IDX_MAX_CNS];  /* This REG is INC by HW, due to PPU action triggered the action fields
                                                         * R<y=0..4>_INT and the R<y=0..4>_INT_IDX==x
                                                         * comment: y=0..4 is the ROT index */
} SNET_SIP6_10_PPU_INT_STC;

/* struct for K_stg key generation profile */
typedef struct {
    GT_U8 byteSelOffset[SNET_SIP6_10_PPU_KSTG_KEY_GEN_PROFILE_BYTE_SELECT_MAX_CNS]; /* byte select offset */
} SNET_SIP6_10_PPU_KSTG_KEY_GEN_PROFILE_STC;

/* struct for K_stg tcam entry */
typedef struct {
    GT_U8   key[SNET_SIP6_10_PPU_KSTG_TCAM_KEY_LEN_CNS];  /* key */
    GT_U8   mask[SNET_SIP6_10_PPU_KSTG_TCAM_KEY_LEN_CNS]; /* mask */
    GT_BOOL isValid;                                      /* valid */
} SNET_SIP6_10_PPU_KSTG_TCAM_ENTRY_STC;

/* struct for debug counters */
typedef struct {
    GT_U32 debugCounterIn;  /* counts incoming packets */
    GT_U32 debugCounterOut; /* counts outgoing packets */
    GT_U32 debugCounterKstg0; /* counts incoming packets to k_stage0 */
    GT_U32 debugCounterKstg1; /* counts incoming packets to k_stage1 */
    GT_U32 debugCounterKstg2; /* counts incoming packets to k_stage2 */
    GT_U32 debugCounterDau; /* counts incoming packets to DAU */
    GT_U32 debugCounterloopback; /* counts loopback packets */
    GT_U32 ppuDebug; /* Debug bus output. Controlled by Debug Bus Ctrl and Debug Bus Enable. */
} SNET_SIP6_10_PPU_DEBUG_STC;

/* struct for PPU error profile field */
typedef struct {
    GT_BOOL writeEnable;    /* Write Enable for specific byte */
    GT_U32  numBits;        /* Number of bits minus 1, per each of the bytes which can modify DESC */
    GT_U32  errorDataField; /* Data per each of the 4 modified bytes which will run-over the DESC */
    GT_U32  targetOffset; /* BIT offset location inside DESC */
} SNET_SIP6_10_PPU_ERROR_PROFILE_FIELD_STC;

/* struct for PPU error profile */
typedef struct {
    SNET_SIP6_10_PPU_ERROR_PROFILE_FIELD_STC errProfileField[SNET_SIP6_10_PPU_ERROR_PROFILE_FIELDS_MAX_CNS];
} SNET_SIP6_10_PPU_ERROR_PROFILE_STC;

/* struct for DAU protected window */
typedef struct {
    GT_BOOL protWinEnable; /* protected window enable */
    GT_U32 protWinStartOffset; /* protected window start offset */
    GT_U32 protWinEndOffset; /* protected window end offset */
} SNET_SIP6_10_PPU_DAU_DATA_PROTECTED_WINDOW_STC;

/* struct for Reg Operation Tree (ROT) action entry */
typedef struct {
    /* select source for action
     * 0x0 = HD (32 Bytes)
     * 0x1 = SP_BUS (20 Bytes)
     * 0x2 = UDB_METADATA (8 Bytes)
     * 0x3 = _4BYTE_SET_CMD
     */
    GT_U32 srcRegSel; /* Reg[1:0] */

    /* When srcRegSel == _4BYTE_SET_CMD */

        /* 4BYTE SET VALUE (part of 4 byte SET value) VALIDITY LOGIC #2.*/
        GT_U32  setCmd4Byte; /* Reg[33:2] */

    /* when srcRegSel != _4BYTE_SET_CMD */

        /* Offset inside 32Byte HEADER/20 bytes SP_BUS/UDB_Metadata (bit resolution) */
        GT_U32  srcOffset; /* Reg[9:2] */

        /* Number of valid bits of the field. MSBits exceeding this number will be zerod. */
        GT_U32  srcNumValBits; /* Reg[14:10] */

        /* when srcRegSel != _4BYTE_SET_CMD and func != CMP_REG */

            /* Number of valid bits of the field. MSBits exceeding this number will be zerod.
             * 0x0 = LEFT; SHIFT LEFT DATA + PAD LSBits (MULT); ;
             * 0x1 = RIGHT; SHIFT RIGHT DATA + PAD MSBits (DIV);
             */
            GT_BOOL shiftLeftRightSel; /* Reg[15] */

            /* Number of SHIFTs (0 means no SHIFT) */
            GT_U32  shiftNum; /* Reg[18:16] */

            /* Number of bits to set for constant value (0-8).
             * The value of bits is determined according to R<x=0..3>_SET_BITS_VAL
             * The offset inside the full 32bit REG is determined according to R<x=0..3>_SET_BITS_OFFSET
             */
            GT_U32  setBitsNum; /* Reg[22:19] */

            /* 8 bits will be SET according to this value.
             * Location of the setting inside the full 32bits is determined according to R<x=0..3>_SET_BITS_OFFSET
             */
            GT_U32  setBitsVal; /* Reg[30:23] */

            /* OFFSET inside 32bits of REG to set the bits. The LSBit of R<x=0..3>_SET_BITS_VAL will
             * be written into R<x=0..3>_SET_BITS_OFFSET
             */
            GT_U32  setBitsOffset; /* Reg [35:31] */

            /* For 16b ADDER - CONST of 8 MSBits bits added to R<x=0..3> */
            GT_U32  addSubConst; /* Reg[51:36] */

            /* Constant select
             *   0x0 = ADD_16BIT_CONST;
             *   0x1 = SUB_16BIT_CONST;
             */
            GT_BOOL addSubConstSel; /* Reg[52] */

        /* when srcRegSel != _4BYTE_SET_CMD and func == CMP_REG */

            /* COMPARE VALUE. Used For compare FUNC when SECOND OPERAND == Current REG
             * 0x0 = LEFT; SHIFT LEFT DATA + PAD LSBits (MULT)
             * 0x1 = RIGHT; SHIFT RIGHT DATA + PAD MSBits (DIV)
             */
            GT_U32  cmpVal; /* Reg[22:15] */

            /* When compare result == FALSE, load R<x=0..3>_CMP_COMPARE_FALSE into
             * target, if CMP_FALSE_MASK is disabled
             */
            GT_U32  cmpFalseValLd; /* Reg[30:23] */

            /* When compare result is TRUE, REG will be loaded with R<x=0..3>_CMP_TRUE_VAL_LD
             * according to COMPARE_TRUE_MASK[3:0]. When condition is FALSE, REG will be loaded
             * with R<x=0..3>_CMP_FALSE_VAL_LD according to COMPARE_FALSE_MASK[3:0].
             *  0x0 = Equal
             *  0x1 = Greater than or equal
             *  0x2 = Less than or equal
             */
            GT_U32  cmpCond; /* Reg[32:31] */

            /* 2 bit granular WR MASK when COMPARE is operated (0xF means not to modify the target while 0x0 means to
             * modify full 8 bits of target)
            */
            GT_U32  cmpMask; /* Reg [43:36] */

            /* When compare==TRUE, load R<x=0..3>_CMP_COMPARE_VALID into Target */
            GT_U32  cmpTrueValLd; /* Reg[51:44] */


    /* Number of valid bits of the field. MSBits exceeding this number will be zerod.
     *   0x0 = NONE;
     *   0x1 = ADD_REG;
     *   0x2 = SUB_REG;
     *   0x3 = CMP_REG;
     */
    GT_U32  func; /* Reg[54:53] */

    /* values for use
     * 0x0 = R0; intermediate val from Reg0;
     * 0x1 = R1; intermediate val from Reg1;
     * 0x2 = R2; intermediate val from Reg2;
     * 0x3 = R3; intermediate val from Reg3;
     * 0x4 = R4; intermediate val from Reg4;
     * 0x5 = USE_CMP_VALL; Use COMPARE_VAL[7:0] as the value for comparison
     *       (relevant for R<x=0..3>_FUNC == CMP_REG); ;
     */
    GT_U32  funcSecondOperand; /* Reg[57:55] */

    /* values for use
     * 0x0 = SP_BUS;
     * 0x1 = HD_NEXT_SHIFT; */
    GT_BOOL target; /* Reg[58] */

    /* Number of bytes loaded into SP_BUS (MAX 4 bytes) */
    GT_U32  ldSpBusNumBytes; /* Reg[61:59] */

    /* SP_BUS byte offset for loading */
    GT_U32  ldSpBusOffset; /* Reg[66:62] */

    /* When set to "1" - Assert an INT and INC Interrupt counter pointed by INT_IDX */
    GT_BOOL interrupt; /* Reg[67] */

    /* When R<x=0..4>_INT=="1", increment Interrupt counter INT_CNT_REG of index R<x=0..4>_INT_IDX */
    GT_U32  interruptIndex; /* Reg[70:68] */
} SNET_SIP6_10_PPU_KSTG_ROT_ACTION_ENTRY_STC;

/* struct for ppu action table entry */
typedef struct {
    /* REG Operation Tree Action Entries */
    SNET_SIP6_10_PPU_KSTG_ROT_ACTION_ENTRY_STC rotActionEntry[SNET_SIP6_10_PPU_KSTG_ROTS_MAX_CNS];

    /* Value of Next state of k_stg */
    GT_U32  setNextState;

    /* SHIFT in 160B header (2B granular). Notice that SHIFT loaded by REG has higher priority than
     * the one set by CONSTANT */
    GT_U32  setConstNextShift;

    /* loop-back select
     *  0x0 = NO_LOOPBACK; No Loopback (inputs received from previous stage);
     *  0x1 = K_STG_LOOPBACK; k_stg in Loopback mode, meaning:
     * 1. Inputs for the k_stg will be loaded from result of previous cycle from the same k_stg
     * (connection of k_stg outputs from previous cycle, to the inputs of current cycle).
     * 2. k_in_rdy will be de-asserted in order to back pressure the previous k_stg
     * Comment - the assumption is that ordering between packets should maintain only when the
     * packets have the exact same PPU number of stages.
     */
    GT_BOOL setLoopBack;

    /* counter set select
     * 0x0 = DONOT_SET; DONOT_SET; Do not SET counter; ;
     * 0x1 = SET; SET; Set counter to the value defined under COUNTER_SET_VAL; */
    GT_BOOL counterSet;

    /* Set value to 5 bits counter */
    GT_U32  counterSetVal;

    /* counter operation:
     * 0x0 = NONE;
     * 0x1 = INC;
     * 0x2 = DEC;
     */
    GT_U32  counterOper;
} SNET_SIP6_10_PPU_KSTG_ACTION_ENTRY_STC;

/* struct for DAU profile entry descriptor bit select */
typedef struct {
    GT_BOOL spByteWriteEnable;  /* Enable write to SP bus from current SP interface */
    GT_U32  spByteNumBits;      /* Number of bits MINUS 1 from SP_BUS<x> which modify the DESC.
                                 * The MSBits above this value, will keep the DESC pre-modified value. */
    GT_U32  spByteSrcOffset;    /* Bit offset inside SP_BUS taken as byte data_in[7:0] for
                                 * the specific write interface. */
    GT_U32  spByteTargetOffset; /* BIT offset location inside DESC */
} SNET_SIP6_10_PPU_DAU_PROFILE_ENTRY_DESC_BITS_SET_STC;

/* struct for DAU profile entry */
typedef struct {
    SNET_SIP6_10_PPU_DAU_PROFILE_ENTRY_DESC_BITS_SET_STC setDescBits[SNET_SIP6_10_PPU_DAU_DESC_BYTE_SET_MAX_CNS];
} SNET_SIP6_10_PPU_DAU_PROFILE_ENTRY_STC;

/* struct for SP_BUS data */
typedef struct {
    GT_U8   data[SNET_SIP6_10_PPU_SP_BUS_BYTES_MAX_CNS]; /* SP_BUS data */
} SNET_SIP6_10_PPU_SP_BUS_STC;

/* struct for UDB metadata */
typedef struct {
    GT_U8 data[SNET_SIP6_10_PPU_UDBS_MAX_CNS]; /* UDBs */
    GT_U8 udbValidMask;                        /* valid mask */
} SNET_SIP6_10_PPU_UDB_METADATA_STC;

/* struct for header segment */
typedef struct {
    GT_U8   data[SNET_SIP6_10_PPU_HD_LEN_MAX_CNS]; /* header data */
} SNET_SIP6_10_PPU_HEADER_STC;

/* struct for PPU profile entry */
typedef struct {
    GT_BOOL ppuEnable;  /* 0 - PPU is disabled
                         * 1 - PPU is Enabled */
    GT_U32  ppuState;   /* Specifies the initial state of the PPU. The initial state of
                         * the PPU is concatenated to the 6byte key in its first k_stg lookup */
    GT_U32  spBusDefaultProfile; /* points towards the SP_BUS default value (supporting 4 profiles) */
    GT_U32  anchorType; /* Allows to define a PPU initial offset relative to the
                         * beginning of a specific layer. The following anchor types are supported:
                         * 0x0 - L2 - Start of packet
                         * 0x1 - L3 - Start of L3 header minus 2 bytes (allows easy extraction of EtherType)
                         * 0x2 - Start of L4 header
                         * 0x3 - Tunnel-L2 - The start of the L2 header data in the tunnel header
                         * 0x4 - Tunnel-L3 - The beginning of the L3 header in the tunnel header minus 2 bytes
                         * 0x5 - Tunnel-L4 - The beginning of the L4 header in the tunnel header */
    GT_U32  offset;     /* Defines the position of the byte relative to the anchor (granularity of 2Bytes).*/
} SNET_SIP6_10_PPU_PROFILE_ENTRY_STC;

/* struct for PPU global configuration */
typedef struct {
    GT_BOOL ppuEnable;  /* Enable to the PPU unit
                         * If this bit is 0, no data is received by the TTI unit.
                         * Note: This bit should not be toggled under traffic.
                         * For debug purposes.
                         *  0x0 = Disable; Disable
                         *  0x1 = Enable; Enable */

    GT_U32  maxLoopBack; /* MAX loopback allowed by the PPU. If number exceed this configurable number,
                           * the logic will assert an INT and error handling will be according to the
                           * associated error profile */
    GT_U32  errProfMaxLoopBack;   /* Will point towards error profile associated with number of loopbacks
                                 * exceeds MAX value
                                 * 0x0 = NO_UPDATE; NO_UPDATE; don't update descriptor
                                 * 0x1 = PROFILE_1; PROFILE_1; update desc according to error profile 1
                                 * 0x2 = PROFILE_2; PROFILE_2; update desc according to error profile 2 */
    GT_U32  errProfSer;           /* Will point towards error profile associated with SER error
                                 * 0x0 = NO_UPDATE; NO_UPDATE; don't update descriptor
                                 * 0x1 = PROFILE_1; PROFILE_1; update desc according to error profile 1
                                 * 0x2 = PROFILE_2; PROFILE_2; update desc according to error profile 2 */

     GT_U32 errProfOffsetOor;    /* Will point towards error profile associated with PPU offset (due to SHIFT) - out of range
                                 * 0x0 = NO_UPDATE; NO_UPDATE; don't update descriptor
                                 * 0x1 = PROFILE_1; PROFILE_1; update desc according to error profile 1
                                 * 0x2 = PROFILE_2; PROFILE_2; update desc according to error profile 2 */
    GT_BOOL debugCounterEnable; /* enable the debug counters
                                 * 0x0 = DISABLE; counters are disabled
                                 * 0x1 = ENABLE; counters are enabled */
} SNET_SIP6_10_PPU_CONFIG_STC;

/* structure for DAU fields */
typedef struct {
    GT_U32 endOffset;       /* End offset   */
    GT_U32 startOffset;     /* Start offset */
}SNET_SIP6_10_PPU_DAU_FIELDS_FORMAT_STC;


/* struct for error information */
typedef struct {

    GT_BOOL isSerErr;      /* Is SER error */
    GT_BOOL isLoopbackErr; /* Is Loopback error */
    GT_BOOL isOorErr;      /* Is Oor Error */
} SNET_SIP6_10_PPU_ERROR_INFO_STC;

/* enums */

/* enumerator for ROT source reg selection */
typedef enum {
    SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_HD_E,
    SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_SP_BUS_E,
    SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_UDB_METADATA_E,
    SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_4BYTE_SET_CMD_E
} SNET_SIP6_10_PPU_ROT_SRC_REG_SEL;

/* enumerator for ROT shift left/right */
typedef enum {
    SNET_SIP6_10_PPU_ROT_SHIFT_LEFT_E,
    SNET_SIP6_10_PPU_ROT_SHIFT_RIGHT_E
} SNET_SIP6_10_PPU_ROT_SHIFT_ENT;

/* enumerator for ROT add/sub selection */
typedef enum {
    SNET_SIP6_10_PPU_ROT_ADD_SEL_E,
    SNET_SIP6_10_PPU_ROT_SUB_SEL_E
} SNET_SIP6_10_PPU_ROT_ADD_SUB_SEL_ENT;

/* enumerator for ROT target write selection */
typedef enum {
    SNET_SIP6_10_PPU_ROT_TARGET_SEL_SP_BUS_E,
    SNET_SIP6_10_PPU_ROT_TARGET_SEL_HD_SHIFT_E
} SNET_SIP6_10_PPU_ROT_TARGET_SEL_ENT;

/* enumerator for ROT compare condition */
typedef enum {
    SNET_SIP6_10_PPU_ROT_CMP_COND_EQUAL_E,
    SNET_SIP6_10_PPU_ROT_CMP_COND_GREATER_THAN_OR_EQUAL_E,
    SNET_SIP6_10_PPU_ROT_CMP_COND_LESS_THAN_OR_EQUAL_E,
} SNET_SIP6_10_PPU_ROT_CMP_COND_ENT;

/* enumerator for ROT function operation */
typedef enum {
    SNET_SIP6_10_PPU_ROT_FUNC_NONE_E,
    SNET_SIP6_10_PPU_ROT_FUNC_ADD_REG_E,
    SNET_SIP6_10_PPU_ROT_FUNC_SUB_REG_E,
    SNET_SIP6_10_PPU_ROT_FUNC_CMP_REG_E
} SNET_SIP6_10_PPU_ROT_FUNC_ENT;

/* enumerator for PPU action counter operation */
typedef enum {
    SNET_SIP6_10_PPU_PPU_ACT_COUNTER_OPER_NONE_E,
    SNET_SIP6_10_PPU_PPU_ACT_COUNTER_OPER_INC_E,
    SNET_SIP6_10_PPU_PPU_ACT_COUNTER_OPER_DEC_E
} SNET_SIP6_10_PPU_PPU_ACT_COUNTER_OPER_ENT;

/* enumerator for PPU profile entry anchor type */
typedef enum {
    SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L2_E,
    SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L3_E,
    SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L4_E,
    SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L2_E,
    SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L3_E,
    SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L4_E,
} SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_ENT;

/* enumerator for hardware descriptor fields */
typedef enum {
    /*(Is PTP = "True") and (Is PTP = "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_START_BANK_E,
    /*(Is PTP = "True") and (Is PTP = "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_START_GROUP_E,
    /*(Is PTP = "True") and (Is PTP = "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_INGRESS_CORE_ID_E,
    /*(Is PTP = "True") and (Is PTP = "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_DESC_CRITICAL_ECC_E,
    /*Is FCoE = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_D_ID_E,
    /*(Is FCoE != "True") and (Is IPv4 != "True") and (Is ARP != "True") and (Is IPv6 != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_23_0_E,
    /*Is ARP = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_DIP_E,
    /*Is IPv4 = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_SIP_E,
    /*Is IPv6 = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_DIP_E,
    /*(Is IPv4 != "True") and (Is ARP != "True") and (Is IPv6 != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_31_24_E,
    /*Is IPv4 = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DIP_E,
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_63_32_E,
    /*Is ARP = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_SIP_E,
    /*(Is IPv6 != "True") and (Is ARP != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_95_64_E,
    /*Is FCoE = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_S_ID_E,
    /*(Is IPv6 != "True") and (Is FCoE != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_119_96_E,
    /*Is IPv4 = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_FRAGMENTED_E,
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_121_120_E,
    /*Is IPv4 = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_OPTION_FIELD_E,
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_122_122_E,
    /*(Is IP = "False") and (Is FCoE = "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_E,
    /*(Is IPv6 != "True") and ((Is IP != "False") or (Is FCoE != "False"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_123_123_E,
    /*Is IPv4 = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DF_E,
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_124_124_E,
    /*Is FCoE = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_LEGAL_E,
    /*(Is IPv6 != "True") and (Is FCoE != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_125_125_E,
    /*Is IPv4 = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FRAGMENTED_E,
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_126_126_E,
    /*Is IP = "False"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_FCOE_E,
    /*(Is IPv6 != "True") and (Is IP != "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_127_127_E,
    /*Is IPv6 = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_SIP_E,
    /*(Is IPv6 != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_255_128_E,
    /*Is ARP For Hash = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_DIP_FOR_HASH_E,
    /*Is IPv4 For Hash = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_SIP_FOR_HASH_E,
    /*(Is IPv4 For Hash != "True") and (Is ARP For Hash != "True") and (Is IPv6 For Hash != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_287_256_E,
    /*Is IPv6 For Hash = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_DIP_FOR_HASH_E,
    /*Is IPv4 For Hash = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DIP_FOR_HASH_E,
    /*(Is IPv6 For Hash != "True") and (Is IPv4 For Hash != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_319_288_E,
    /*Is ARP For Hash = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_SIP_FOR_HASH_E,
    /*(Is IPv6 For Hash != "True") and (Is ARP For Hash != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_351_320_E,
    /*(Is IPv4 For Hash = "False") and (Is IPv6 For Hash = "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_FOR_HASH_E,
    /*(Is IPv6 For Hash != "True") and ((Is IPv4 For Hash != "False") or (Is IPv6 For Hash != "False"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_352_352_E,
    /*(Is IPv6 For Hash != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_383_353_E,
    /*Is IPv6 For Hash = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_SIP_FOR_HASH_E,
    /*(Is IPv6 For Hash != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_511_384_E,
    /*Tunnel Start = "TS"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_PTR_E,
    /*Tunnel Start = "LL"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_PTR_E,
    /*Tunnel Start = "TS"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_START_PASSENGER_TYPE_E,
    /*(Tunnel Start != "LL")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_529_529_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_VIRTUAL_ID_E,
    /*Redirect Cmd = "2"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICY_LTT_INDEX_E,
    /*(Redirect Cmd != "2")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_547_542_E,
    /*(Use VIDX = "0") and (Trg Is Trunk = "TrunkID")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_TRUNK_ID_E,
    /*(Use VIDX = "0") and (Trg Is Trunk = "ePort")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_EPORT_E,
    /*Use VIDX = "1"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVIDX_E,
    /*((Use VIDX != "0") or (Trg Is Trunk != "ePort")) and (Use VIDX != "1")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_560_560_E,
    /*Use VIDX = "0"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_IS_TRUNK_E,
    /*Use VIDX = "0"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_TRG_PHY_PORT_VALID_E,
    /*(Use VIDX != "1")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_563_563_E,
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1") and (Excluded Is Trunk = "1")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_TRUNK_ID_E,
    /*(Packet Cmd != "From CPU") and (Orig Is Trunk = "1")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_TRUNK_ID_E,
    /*((Packet Cmd = "From CPU") or (Orig Is Trunk != "1")) and ((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "1")) and ((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "0")) and ((Packet Cmd = "From CPU") or (Orig Is Trunk != "0"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_575_564_E,
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1") and (Excluded Is Trunk = "0")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_EPORT_E,
    /*(Packet Cmd != "From CPU") and (Orig Is Trunk = "0")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_EPORT_E,
    /*((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "0")) and ((Packet Cmd = "From CPU") or (Orig Is Trunk != "0"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_576_576_E,
    /*(Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E,
    /*(Packet Cmd = "To CPU") or (Packet Cmd = "To Target Sniffer")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_EPORT_E,
    /*Packet Cmd = "From CPU"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_CPU_TO_CPU_MIRROR_E,
    /*((Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer")) and (Packet Cmd != "From CPU")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_589_589_E,
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_IS_TRUNK_E,
    /*((Packet Cmd != "From CPU") or (Use VIDX != "1")) and ((Use VIDX != "0") or (Is Trg Phy Port Valid != "True"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_590_590_E,
    /*(Use VIDX = "0") and (Is Trg Phy Port Valid = "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_PHY_PORT_E,
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1") and (Excluded Is Trunk = "0")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_DEV_IS_LOCAL_E,
    /*((Use VIDX != "0") or (Is Trg Phy Port Valid != "True")) and ((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "0"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_591_591_E,
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1") and (Excluded Is Trunk = "0")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_IS_PHY_PORT_E,
    /*((Use VIDX != "0") or (Is Trg Phy Port Valid != "True")) and ((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "0"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_592_592_E,
    /*((Use VIDX != "0") or (Is Trg Phy Port Valid != "True"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_599_593_E,
    /*Packet Cmd = "From CPU"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FROM_CPU_TC_E,
    /*Packet Cmd != "From CPU"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_PROFILE_E,
    /*Packet Cmd = "From CPU"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FROM_CPU_DP_E,
    /*Packet Cmd = "From CPU"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_FILTER_EN_E,
    /*(Packet Cmd = "From CPU")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_609_606_E,
    /*(Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_PHY_IS_TRUNK_E,
    /*(Packet Cmd = "To CPU") or (Packet Cmd = "To Target Sniffer")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_DEV_E,
    /*((Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_619_611_E,
    /*((Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_629_620_E,
    /*(Packet Cmd = "To CPU") or (Packet Cmd = "To Target Sniffer")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_PHY_PORT_E,
    /*((Use VIDX != "0") or (Trg Is Trunk != "ePort"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_639_630_E,
    /*(Use VIDX = "0") and (Trg Is Trunk = "ePort")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_DEV_E,
    /*QCN Rx = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_RX_QCN_PRIO_E,
    /*QCN Rx = "False"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TTL_E,
    /*(QCN Rx != "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_647_643_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_OFFSET_E,
    /*Timestamp En = "Enable"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PTP_OFFSET_INDEX_E,
    /*Timestamp En = "Disable"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_OFFSET_E,
    /*Tunnel Terminated = "TT"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_HEADER_OFFSET_E,
    /*(Tunnel Terminated != "TT")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_668_662_E,
    /*Tunnel Terminated = "TT"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_L3_OFFSET_E,
    /*(Tunnel Terminated != "TT")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_674_669_E,
    /*(L2 Echo = "0") and (Is PTP = "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_DOMAIN_E,
    /*((L2 Echo != "0") or (Is PTP != "True"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_677_675_E,
    /*Packet Cmd = "To Target Sniffer"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_SNIFF_E,
    /*Packet Cmd != "To Target Sniffer"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ANALYZER_INDEX_E,
    /*(Packet Cmd = "To Target Sniffer")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_680_679_E,
    /*OAM Processing En = "Enable"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PROFILE_E,
    /*(OAM Processing En != "Enable") and ((OAM Processing En != "Disable") or (L2 Echo != "0") or (Is PTP != "True"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_681_681_E,
    /*(OAM Processing En = "Disable") and (L2 Echo = "0") and (Is PTP = "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_TRIGGER_TYPE_E,
    /*((OAM Processing En != "Disable") or (L2 Echo != "0") or (Is PTP != "True"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_682_682_E,
    /*Packet Cmd != "From CPU"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_IS_TRUNK_E,
    /*(Packet Cmd = "From CPU")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_683_683_E,
    /*(Packet Cmd = "From CPU")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_684_684_E,
    /*Packet Cmd != "From CPU"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ROUTED_E,
    /*(Is IP = "True") and (Is IPv4 = "False") */
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_E,
    /*((Is IP != "True") or (Is IPv4 != "False"))*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_685_685_E,
    /*Is IP = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_E,
    /*(Is IP != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_686_686_E,
    /*Is IPv4 For Hash = "False"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_FOR_HASH_E,
    /*(Is IPv4 For Hash != "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_687_687_E,
    /*Is IPv6 = "True"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_HBH_EXT_E,
    /*(Is IPv6 != "True")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_688_688_E,
    /*Packet Cmd != "From CPU"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_MODE_E,
    /*(Packet Cmd = "From CPU")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_689_689_E,
    /*L2 Echo = "0"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_PTP_E,
    /*(L2 Echo != "0")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_690_690_E,
    /*Is FCoE = "False"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_LEGAL_E,
    /*(Is FCoE != "False")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_691_691_E,
    /*(Packet Cmd != "To CPU")*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_692_692_E,
    /*Packet Cmd = "To CPU"*/
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SR_END_NODE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SR_EH_OFFSET_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_CPU_CODE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_KEEP_PREVIOUS_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_IS_CUT_THROUGH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TABLES_READ_ERROR_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_FOR_KEY_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_FOR_KEY_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_PRIO_TAGGED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_IS_LLC_NON_SNAP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TCP_UDP_DEST_PORT_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TCP_UDP_SRC_PORT_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TCP_UDP_PORT_IS_0_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPM_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SOURCE_PORT_LIST_FOR_KEY_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_INGRESS_UDB_PACKET_TYPE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_CHANNEL_TYPE_PROFILE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_UDB_VALID_SET_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_UDB_SET_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_FLOW_LABEL_FOR_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_BYTES_FOR_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL2_FOR_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL1_FOR_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL0_FOR_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_FOR_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_FOR_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_FOR_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_HEADER_INFO_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_SUB_TEMPLATE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_REDIRECT_CMD_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_TOS_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_TYPE_FOR_KEY_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_USE_INTERFACE_BASED_INDEX_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_DISABLE_IPCL0_FOR_ROUTED_PACKETS_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPCL_PROFILE_INDEX_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP2_USE_INDEX_FROM_DESC_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP2_PCL_MODE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP1_USE_INDEX_FROM_DESC_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP1_PCL_MODE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP0_USE_INDEX_FROM_DESC_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP0_PCL_MODE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICY_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVLAN_PRECEDENCE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_ENCAPSULATION_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_BYTES_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_TRACK_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_L3_OFFSET_INVALID_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_VALID_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_SRC_TAG_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_TO_ME_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_METERING_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_FLOW_LABEL_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_EH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_BC_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_MLD_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_LINK_LOCAL_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_MPLS_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ND_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_HASH_MASK_INDEX_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_CMD_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_SBIT_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_EXP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_LABEL_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ETHER_TYPE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_ASSIGNED_SST_ID_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYPASS_INGRESS_PIPE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYPASS_BRIDGE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SOLICITATION_MULTICAST_MESSAGE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_QCN_RX_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_ARP_SA_MISMATCH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_HEADER_LENGTH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_LENGTH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICER_PTR_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_BILLING_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_PROFILE_PRECEDENCE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_PRIO_TAGGED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PROCESSING_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_VALID_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_PROTOCOL_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SYN_WITH_DATA_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_SYN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_REPLY_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_PROTECTION_SWITCH_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_IS_PROTECTION_PATH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_CMD_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_TYPE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_DEV_IS_OWN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_TRUNK_ID_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MODIFY_UP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ECN_CAPABLE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_REP_LAST_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_REP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_USE_VIDX_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAIL_BOX_TO_NEIGHBOR_CPU_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_FILTER_REGISTERED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SST_ID_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_L3_OFFSET_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_ECHO_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_PACKET_TYPE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_HASH_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_TAGGED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_IS_LLC_NON_SNAP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_COPY_RESERVED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_ID_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MODIFY_DSCP_EXP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_DO_ROUTE_HA_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_DROP_ON_SOURCE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_IS_LOOPED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_NESTED_VLAN_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_TRILL_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_TAI_SELECT_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_U_FIELD_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_TAGGED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_NUM_OF_TAG_BYTES_TO_POP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_TPID_INDEX_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_TPID_INDEX_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_LOCAL_DEV_SRC_TAGGED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TAG0_IS_OUTER_TAG_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_SRC_TAGGED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_SRC_TAGGED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_OVERRIDE_EVLAN_WITH_ORIGVID_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_VID_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_UP1_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_CFI1_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_VID1_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_UP0_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_CFI0_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVLAN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_DEV_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_PHY_SRC_MC_FILTER_EN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_EPORT_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_PORT_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_UDB_PACKET_TYPE_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_START_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_TERMINATED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MARVELL_TAGGED_EXTENDED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MARVELL_TAGGED_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_RECALC_CRC_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYTE_COUNT_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_MDB_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_ECN_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_SKIP_FDB_SA_LOOKUP_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_FIRST_BUFFER_E,
    SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPFIX_EN_E
} SNET_SIP6_10_PPU_DAU_DESC_FIELDS_ENT;

/* function declarations */

/**
* @internal snetHawkPpu function
* @endinternal
*
* @brief   PPU Engine processing for outgoing frame on Hawk
*         asic simulation.
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to frame data buffer Id
*/
GT_VOID snetHawkPpu
(
    IN      SKERNEL_DEVICE_OBJECT               *devObjPtr,
    INOUT   SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr
);

/**
 * @internal snetHawkBuildIpclUdbData function
 * @endinternal
 *
 * @brief Build IPCL UDB data and store in descriptor
 *
 * @param[in] devObjPtr                - pointer to device object.
 * @param[in,out] descrPtr             - pointer to frame data buffer Id
 */
GT_VOID snetHawkBuildIpclUdbData
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__snetHawkPpuh*/
