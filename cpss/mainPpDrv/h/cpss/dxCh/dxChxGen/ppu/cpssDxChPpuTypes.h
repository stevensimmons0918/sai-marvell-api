/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChPpuTypes.h
*
* @brief CPSS DXCH ppu types declaration.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChPpuTypesh
#define __cpssDxChPpuTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>

/* PPU profile table max entries */
#define CPSS_DXCH_PPU_PROFILE_ENTRIES_MAX_CNS 16

/* SP_BUS default profile table max entries */
#define CPSS_DXCH_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS 4

/* SP_BUS length in bytes */
#define CPSS_DXCH_PPU_SP_BUS_BYTES_MAX_CNS 20

/* SP_BUS length in 4-byte words */
#define CPSS_DXCH_PPU_SP_BUS_WORDS_MAX_CNS 5

/* K_stgs max number */
#define CPSS_DXCH_PPU_KSTGS_MAX_CNS 3

/* K_stg key generation profile table max entries */
#define CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILES_MAX_CNS 16

/* K_stg key generation profile table entry max byte selections */
#define CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_BYTE_SELECT_MAX_CNS 7

/* K_stg tcam key max length */
#define CPSS_DXCH_PPU_KSTG_TCAM_KEY_LEN_CNS 7

/* K_stg tcam max entries */
#define CPSS_DXCH_PPU_KSTG_TCAM_ENTRIES_MAX_CNS 63

/* K_stg Reg Operation Tree (ROTs) max number */
#define CPSS_DXCH_PPU_KSTG_ROTS_MAX_CNS 5

/* K_stg Reg Operation Tree (ROTs) max number */
#define CPSS_DXCH_PPU_KSTG_ACTION_TABLE_ENTRIES_MAX_CNS 64

/* DAU Profile table max entries */
#define CPSS_DXCH_PPU_DAU_PROFILE_ENTRIES_MAX_CNS 16

/* DAU descriptor modification max bytes */
#define CPSS_DXCH_PPU_DAU_DESC_BYTE_SET_MAX_CNS 20

/* DAU protected window table max entries */
#define CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_MAX_CNS 5

/* PPU Error Profile table entry max fields */
#define CPSS_DXCH_PPU_ERROR_PROFILE_FIELDS_MAX_CNS 4

/* PPU Error Profile table max entries */
#define CPSS_DXCH_PPU_ERROR_PROFILES_MAX_CNS 2

/* PPU Interrupt max number */
#define CPSS_DXCH_PPU_INT_IDX_MAX_CNS 8

/* PPU Global Register ppu_enable field offset */
#define CPSS_DXCH_PPU_GLOBAL_REG_PPU_ENABLE_OFFSET 0

/* PPU Global Register ppu_enable field size */
#define CPSS_DXCH_PPU_GLOBAL_REG_PPU_ENABLE_SIZE 1

/* PPU Global Register max loopback field offset */
#define CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_OFFSET 1

/* PPU Global Register max loopback field size */
#define CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_SIZE 3

/* PPU max loopbacks */
#define CPSS_DXCH_PPU_MAX_LOOPBACK_NUM_CNS 3

/* PPU Global Register err profile max loopback field offset */
#define CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_MAX_LOOPBACK_OFFSET 4

/* PPU Global Register error profile max loopback field size */
#define CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_MAX_LOOPBACK_SIZE 2

/* PPU Global Register err profile SER field offset */
#define CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_SER_OFFSET 6

/* PPU Global Register error profile SER field size */
#define CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_SER_SIZE 2

/* PPU Global Register err profile offset out of range field offset */
#define CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_OFFSET_ORR_OFFSET 8

/* PPU Global Register error profile offset out of range field size */
#define CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_OFFSET_ORR_SIZE 2

/* PPU Global Register debug counter enable field offset*/
#define CPSS_DXCH_PPU_GLOBAL_DEBUG_COUNTER_ENABLE_OFFSET 10

/* PPU Global Register debug counter enable field size*/
#define CPSS_DXCH_PPU_GLOBAL_DEBUG_COUNTER_ENABLE_SIZE 1

/* PPU profile entry ppu enable field offset */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_ENABLE_OFFSET 0

/* PPU profile entry ppu enable field size */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_ENABLE_SIZE 1

/* PPU profile entry ppu state field offset */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_STATE_OFFSET 1

/* PPU profile entry ppu state field size */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_STATE_SIZE 8

/* PPU profile entry anchor type field offset */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_OFFSET 9

/* PPU profile entry anchor type field size */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_SIZE 3

/* PPU profile entry offset field offset */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_OFFSET_OFFSET 12

/* PPU profile entry offset field size */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_OFFSET_SIZE 7

/* PPU profile entry sp bus profile index field offset */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_SP_BUS_PROFILE_INDEX_OFFSET 19

/* PPU profile entry sp bus profile index field size */
#define CPSS_DXCH_PPU_PROFILE_ENTRY_SP_BUS_PROFILE_INDEX_SIZE 2

/* PPU action entry width in words */
#define CPSS_DXCH_PPU_ACTION_TABLE_ENTRY_WIDTH_IN_WORDS_CNS 16

/* PPU DAU profile entry width in words */
#define CPSS_DXCH_PPU_DAU_PROFILE_WIDTH_IN_WORDS_CNS 16

/* PPU DAU protectd window enable bit field offset */
#define CPSS_DXCH_PPU_DAU_PROT_WIN_ENABLE_OFFSET 0

/* PPU DAU protectd window enable bit field size */
#define CPSS_DXCH_PPU_DAU_PROT_WIN_ENABLE_SIZE 1

/* PPU DAU protectd window start offset bit field offset */
#define CPSS_DXCH_PPU_DAU_PROT_WIN_START_OFFSET_OFFSET 1

/* PPU DAU protectd window start offset bit field size */
#define CPSS_DXCH_PPU_DAU_PROT_WIN_START_OFFSET_SIZE 12

/* PPU DAU protectd window end offset bit field offset */
#define CPSS_DXCH_PPU_DAU_PROT_WIN_END_OFFSET_OFFSET 13

/* PPU DAU protectd window end offset bit field size */
#define CPSS_DXCH_PPU_DAU_PROT_WIN_END_OFFSET_SIZE 12

/* Tti2Pcl Descriptor size in bits */
#define CPSS_DXCH_PPU_DAU_DESCR_SIZE_IN_BITS 2266

/**
 * @struct CPSS_DXCH_PPU_GLOBAL_CONFIG_STC
 *
 * @brief PPU Global configuration data.
 */
typedef struct {
    /** MAX loopback allowed by the PPU. If number exceed this configurable number,
     * the logic will assert an INT and error handling will be according to the
     * associated error profile
     */
    GT_U32 maxLoopBack;

    /** Will point towards error profile associated with number of loopbacks
     * exceeds MAX value
     * 0x0 = NO_UPDATE; don't update descriptor
     * 0x1 = PROFILE_1; update desc according to error profile 1
     * 0x2 = PROFILE_2; update desc according to error profile 2
     */
    GT_U32 errProfMaxLoopBack;

    /** Will point towards error profile associated with SER error
     * 0x0 = NO_UPDATE; don't update descriptor
     * 0x1 = PROFILE_1; update desc according to error profile 1
     * 0x2 = PROFILE_2; update desc according to error profile 2
     */
    GT_U32 errProfSer;

    /** Will point towards error profile associated with PPU offset (due to SHIFT) - out of range
     * 0x0 = NO_UPDATE; don't update descriptor
     * 0x1 = PROFILE_1; update desc according to error profile 1
     * 0x2 = PROFILE_2; update desc according to error profile 2
     */
    GT_U32 errProfOffsetOor;

    /** enable or disable the debug counters
     * 0x0 = DISABLE
     * 0x1 = ENABLE
     */
    GT_BOOL debugCounterEnable;
} CPSS_DXCH_PPU_GLOBAL_CONFIG_STC;

/**
 * @struct CPSS_DXCH_PPU_PROFILE_ENTRY_STC
 *
 * @brief PPU profile table entry. It supports 16 profiles.
 */
typedef struct {

    /** 0 - PPU is disabled
     * 1 - PPU is Enabled
     */
    GT_BOOL ppuEnable;

    /** Specifies the initial state of the PPU. The initial state of
     * the PPU is concatenated to the 6byte key in its first k_stg lookup
     */
    GT_U32   ppuState;

    /** points towards the SP_BUS default value (supporting 4 profiles) */
    GT_U32   spBusDefaultProfile;

    /** Allows to define a PPU initial offset relative to the
     * beginning of a specific layer. The following anchor types are supported:
     * 0x0 - L2 - Start of packet
     * 0x1 - L3 - Start of L3 header minus 2 bytes (allows easy extraction of EtherType)
     * 0x2 - Start of L4 header
     * 0x3 - Tunnel-L2 - The start of the L2 header data in the tunnel header
     * 0x4 - Tunnel-L3 - The beginning of the L3 header in the tunnel header minus 2 bytes
     * 0x5 - Tunnel-L4 - The beginning of the L4 header in the tunnel header
     */
    GT_U32   anchorType;

    /** Defines the position of the byte relative to the anchor (granularity of 2Bytes).*/
    GT_U32   offset;
} CPSS_DXCH_PPU_PROFILE_ENTRY_STC;

/**
 * @struct CPSS_DXCH_PPU_SP_BUS_STC
 *
 * @brief PPU SP Bus (Scrach Pad Bus) data of 20 bytes
 */
typedef struct {
    GT_U8   data[CPSS_DXCH_PPU_SP_BUS_BYTES_MAX_CNS];
} CPSS_DXCH_PPU_SP_BUS_STC;

/**
 * @struct CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC
 *
 * @brief   PPU Key generation table profile entry. There are 16 key gen profiles for each K_stg.
 *          7 byte key. Each byte could be chosen in byte resolution from the following bus
 *          {{3'b0,COUNT[4:0]},HD_IN[32bytes],DESC_Metadata[9Bytes],SP_BUS[20Byte],STATE[1Byte]}
 */
typedef struct {
    /* byte select from the input bus */
    GT_U8 byteSelOffset[CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_BYTE_SELECT_MAX_CNS];
} CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC;

/**
 * @struct  CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC
 *
 * @brief   K_stg TCAM entry. Each K_stg has a TCAM table with 64 rules.
 */
typedef struct {
    /** Key LSB 4 bytes */
    GT_U32   keyLsb;

    /** Key MSB 3 bytes */
    GT_U32   keyMsb;

    /** Mask LSB 4 bytes */
    GT_U32   maskLsb;

    /** Mask MSB 3 bytes */
    GT_U32   maskMsb;

    /** valid bit - 1 byte */
    GT_BOOL isValid;
} CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC;

/**
 * @struct  CPSS_DXCH_PPU_KSTG_ROT_ACTION_ENTRY_STC
 *
 * @brief Register Operation Tree (ROT) Entry data structure. There are 5 ROTS for each K_stg.
 */
typedef struct {
    /** select source for action
     * 0x0 = HD (32 Bytes)
     * 0x1 = SP_BUS (20 Bytes)
     * 0x2 = UDB_METADATA (8 Bytes)
     * 0x3 = _4BYTE_SET_CMD
     */
    GT_U32 srcRegSel; /* Reg[1:0] */

    /** When srcRegSel == _4BYTE_SET_CMD */

        /** 4BYTE SET VALUE (part of 4 byte SET value) VALIDITY LOGIC #2.*/
        GT_U32  setCmd4Byte; /* Reg[33:2] */

    /** when srcRegSel != _4BYTE_SET_CMD */

        /** Offset inside 32Byte HEADER/20 bytes SP_BUS /UDB_Metadata (bit resolution)
         * VALIDITY LOGIC #0 or VALIDITY LOGIC #1. */
        GT_U32   srcOffset; /* Reg[9:2] */

        /** Number of valid bits of the field. MSBits exceeding this number will be zerod.
         * VALIDITY LOGIC #0 or VALIDITY LOGIC #1
         * It is valid when srcRegSel != _4BYTE_SET_CMD */
        GT_U32   srcNumValBits; /* Reg[14:10] */

        /** when srcRegSel != _4BYTE_SET_CMD and func != CMP_REG */

            /** Number of valid bits of the field. MSBits exceeding this number will be zerod.
             * VALIDITY LOGIC #0
             * 0x0 = LEFT; SHIFT LEFT DATA + PAD LSBits (MULT); ;
             * 0x1 = RIGHT; SHIFT RIGHT DATA + PAD MSBits (DIV); */
            GT_BOOL shiftLeftRightSel; /* Reg[15] */

            /** Number of SHIFTs (0 means no SHIFT) */
            GT_U32 shiftNum; /* Reg[18:16] */

            /** Number of bits to set for constant value (0-8).
             * The value of bits is determined according to R<x=0..3>_SET_BITS_VAL
             * The offset inside the full 32bit REG is determined according to R<x=0..3>_SET_BITS_OFFSET
             */
            GT_U32 setBitsNum; /* Reg[22:19] */

            /** 8 bits will be SET according to this value.
             * The location of the setting inside the full 32bits is determined according to R<x=0..3>_SET_BITS_OFFSET
             */
            GT_U32 setBitsVal; /* Reg[30:23] */

            /** OFFSET inside 32bits of REG to set the bits. The LSBit of R<x=0..3>_SET_BITS_VAL will
             * be written into R<x=0..3>_SET_BITS_OFFSET
             */
            GT_U32 setBitsOffset; /* Reg [35:31] */

            /** For 16b ADDER - CONST of 8 MSBits bits added to R<x=0..3> */
            GT_U16 addSubConst; /* Reg[51:36] */

            /** Constant select
             * VALIDITY LOGIC #0
             *   0x0 = ADD_16BIT_CONST
             *   0x1 = SUB_16BIT_CONST
             */
            GT_BOOL addSubConstSel; /* Reg[52] */


        /** when srcRegSel != _4BYTE_SET_CMD and func == CMP_REG */

            /** COMPARE VALUE. Used For compare FUNC when SECOND OPERAND == Current REG */
            GT_U32 cmpVal; /* Reg[22:15] */

            /** When compare result == FALSE, load R<x=0..3>_CMP_COMPARE_FALSE into
             * target, if CMP_FALSE_MASK is disabled
             */
            GT_U32 cmpFalseValLd; /* Reg[30:23] */

            /** When compare result is TRUE, REG will be loaded with R<x=0..3>_CMP_TRUE_VAL_LD
             * according to COMPARE_TRUE_MASK[3:0]. When condition is FALSE, REG will be loaded
             * with R<x=0..3>_CMP_FALSE_VAL_LD according to COMPARE_FALSE_MASK[3:0].
             * VALIDITY LOGIC #1,#2
             *  0x0 = equal,
             *  0x1 = greater than or equal,
             *  0x2 = less than or equal
             */
            GT_U32 cmpCond; /* Reg[32:31] */

            /** 2 bit granular WR MASK when COMPARE is operated (0xF means not to modify the target while 0x0 means to
             * modify full 8 bits of target)
            */
            GT_U32 cmpMask; /* Reg [43:36] */

            /** When compare==TRUE, load R<x=0..3>_CMP_COMPARE_VALID into Target */
            GT_U32 cmpTrueValLd; /* Reg[51:44] */

    /** Number of valid bits of the field. MSBits exceeding this number will be zerod.
     * VALIDITY LOGIC #0 or VALIDITY LOGIC #1 or VALIDITY LOGIC #2
     *   0x0 = NONE;
     *   0x1 = ADD_REG;
     *   0x2 = SUB_REG;
     *   0x3 = CMP_REG;
     */
    GT_U32 func; /* Reg[54:53] */

    /** values for use
     * 0x0 = intermediate val from Reg0
     * 0x1 = intermediate val from Reg1
     * 0x2 = intermediate val from Reg2
     * 0x3 = intermediate val from Reg3
     * 0x4 = intermediate val from Reg4
     * 0x5 = Use COMPARE_VAL[7:0] as the value for comparison
     *       (relevant for R<x=0..3>_FUNC == CMP_REG); ;
     */
    GT_U32 funcSecondOperand; /* Reg[57:55] */

    /** values for use
     * 0x0 = SP_BUS;
     * 0x1 = HD_NEXT_SHIFT; */
    GT_BOOL target; /* Reg[58] */

    /** Number of bytes loaded into SP_BUS (MAX 4 bytes) */
    GT_U32 ldSpBusNumBytes; /* Reg[61:59] */

    /** SP_BUS byte offset for loading */
    GT_U32 ldSpBusOffset; /* Reg[66:62] */

    /** When set to "1" - Assert an INT and INC Interrupt counter pointed by INT_IDX */
    GT_BOOL interrupt; /* Reg[67] */

    /** When R<x=0..4>_INT=="1", increment Interrupt counter INT_CNT_REG of index R<x=0..4>_INT_IDX */
    GT_U32 interruptIndex; /* Reg[70:68] */
} CPSS_DXCH_PPU_KSTG_ROT_ACTION_ENTRY_STC;

/**
 * @struct CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC
 *
 * @brief   PPU Action Table Entry. There are 3 action tables, one each for a K_stg.
 */
typedef struct {
    /** REG Operation Tree Action Entries */
    CPSS_DXCH_PPU_KSTG_ROT_ACTION_ENTRY_STC rotActionEntry[CPSS_DXCH_PPU_KSTG_ROTS_MAX_CNS];

    /** Value of Next state of k_stg */
    GT_U32 setNextState;

    /** SHIFT in 160B header (2B granular). Notice that SHIFT loaded by REG has higher priority than
     * the one set by CONSTANT */
    GT_U32 setConstNextShift;

    /** loop-back select
     *  0x0 = NO_LOOPBACK;
     *  0x1 = K_STG_LOOPBACK; k_stg in Loopback mode, meaning:
     *        1. Inputs for the k_stg will be loaded from result of previous cycle from the same k_stg
     *           (connection of k_stg outputs from previous cycle, to the inputs of current cycle).
     *        2. k_in_rdy will be de-asserted in order to back pressure the previous k_stg
     *           Comment - the assumption is that ordering between packets should maintain only when the
     *           packets have the exact same PPU number of stages.
     */
    GT_BOOL setLoopBack;

    /** counter set select
     * 0x0 = DONOT_SET; DONOT_SET; Do not SET counter; ;
     * 0x1 = SET; SET; Set counter to the value defined under COUNTER_SET_VAL; */
    GT_BOOL counterSet;

    /** Set value to 5 bits counter */
    GT_U32 counterSetVal;

    /** counter operation:
     * 0x0 = NONE;
     * 0x1 = INC;
     * 0x2 = DEC;
     */
    GT_U32 counterOper;
} CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC;

/**
* @struct CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_BIT_SEL_PER_BYTE_STC
*
* @brief PPU DAU (Descriptor Alteration Unit) profile table entry enables modification of partial
*        byte/field per each of the 20 bytes.
*/
typedef struct {
    /** Enable write to SP bus from current SP interface */
    GT_BOOL spByteWriteEnable;

    /** Number of bits MINUS 1 from SP_BUS<x> which modify the DESC.
     * The MSBits above this value, will keep the DESC pre-modified value.
     */
    GT_U32   spByteNumBits;

    /** Bit offset inside SP_BUS taken as byte data_in[7:0] for
     * the specific write interface.
     */
    GT_U32   spByteSrcOffset;

    /** BIT offset location inside descriptor */
    GT_U16  spByteTargetOffset;
} CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_BIT_SEL_PER_BYTE_STC;

/**
* @struct CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC
*
* @brief PPU DAU (Descriptor Alteration Unit) profile table entry. It contain information for writing of the
*        20 SP_BUS bytes which could be divided to 20 difference descriptor fields.
*/
typedef struct {
    /* 20 descriptor fields */
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_BIT_SEL_PER_BYTE_STC setDescBits[CPSS_DXCH_PPU_DAU_DESC_BYTE_SET_MAX_CNS];
} CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC;

/**
 * @struct CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC
 *
 * @brief   upto 5 dau protected windows can be defined. PPU will not alter bit fields in the
 *          protected windows
 */
typedef struct {
     /** protected window enable
      * 0x0 = Disable
      * 0x1 = Enable
      */
    GT_BOOL protWinEnable;

    /** protected window start offset */
    GT_U32 protWinStartOffset;

    /** protected window end offset */
    GT_U32 protWinEndOffset;
} CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC;

/**
 * @struct CPSS_DXCH_PPU_ERROR_PROFILE_FIELD_STC
 *
 * @brief PPU Error profile field
 */
typedef struct {
    /** Write Enable for specific byte */
    GT_BOOL writeEnable;

    /** Number of bits minus 1, per each of the bytes which can modify DESC */
    GT_U32   numBits;

    /** Data per each of the 4 modified bytes which will run-over the DESC */
    GT_U32   errorDataField;

    /** BIT offset location inside DESC */
    GT_U32   targetOffset;
} CPSS_DXCH_PPU_ERROR_PROFILE_FIELD_STC;

/**
 * @struct CPSS_DXCH_PPU_ERROR_PROFILE_STC
 *
 * @brief PPU suports several error cases. The error cases are the following
 *          - Loopback exceeds a configurable number (3 bit counter is enough)
 *          - SER error
 *          - Offset/Shift out of range
 *        Each of the reasons for errors will be associated with an error profile
 *        (0 is a saved index, plus additional 2 configurable profiles)
 */
typedef struct {
    CPSS_DXCH_PPU_ERROR_PROFILE_FIELD_STC errProfileField[CPSS_DXCH_PPU_ERROR_PROFILE_FIELDS_MAX_CNS];
} CPSS_DXCH_PPU_ERROR_PROFILE_STC;

/**
 * @struct CPSS_DXCH_PPU_DEBUG_COUNTERS_STC
 *
 * @brief PPU Debug counters
 */
typedef struct {
    /* incoming packet count for ppu unit */
    GT_U32 counterIn;

    /* outgoing packet count from ppu unit */
    GT_U32 counterOut;

    /* incoming packet count for K_stg 0 */
    GT_U32 counterKstg0;

    /* incoming packet count for K_stg 1 */
    GT_U32 counterKstg1;

    /* incoming packet count for K_stg 2 */
    GT_U32 counterKstg2;

    /* incoming packet count for Dau unit */
    GT_U32 counterDau;

    /* loopback counter */
    GT_U32 counterLoopback;
} CPSS_DXCH_PPU_DEBUG_COUNTERS_STC;

/**
 * @enum CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_ENT
 *
 * @brief Enumerator for PPU profile entry anchor type
 */
typedef enum {
    /** 0x0 - L2 - Start of packet */
    CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L2_E,

    /** 0x1 - L3 - Start of L3 header minus 2 bytes (allows easy extraction of EtherType) */
    CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L3_E,

    /** 0x2 - Start of L4 header */
    CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L4_E,

    /** 0x3 - Tunnel-L2 - The start of the L2 header data in the tunnel header */
    CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L2_E,

    /** 0x4 - Tunnel-L3 - The beginning of the L3 header in the tunnel header minus 2 bytes */
    CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L3_E,

    /** 0x5 - Tunnel-L4 - The beginning of the L4 header in the tunnel header */
    CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L4_E
} CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_ENT;

/**
 * @enum CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_SEL_ENT
 *
 * @brief Enumerator for PPU ROT action second operand selection
 */
typedef enum {
    /** 0x0 = intermediate val from Reg0 */
    CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG0_E,

    /** 0x1 = intermediate val from Reg1 */
    CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG1_E,

    /** 0x2 = intermediate val from Reg2 */
    CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG2_E,

    /** 0x3 = intermediate val from Reg3 */
    CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG3_E,

    /** 0x4 = intermediate val from Reg4 */
    CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG4_E,

    /** 0x5 = use compare value for operation */
    CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_COMPARE_E,
} CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_SEL_ENT;

/**
 * @enum    CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_ENT
 *
 * @brief   Enumerator for PPU ROT source input bus/command selection.
 */
typedef enum {
    /** source reg select is Header */
    CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E,

    /** source reg select is SP_BUS */
    CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E,

    /** source reg select is UDB_Metadata */
    CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_UDB_METADATA_E,

    /** source reg select is 4 byte command */
    CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_4BYTE_SET_CMD_E
} CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_ENT;

/**
 * @enum CPSS_DXCH_PPU_KSTG_ROT_SHIFT_ENT
 *
 * @brief Enumerator for PPU ROT shift selection
 */
typedef enum {
    /** shift left */
    CPSS_DXCH_PPU_KSTG_ROT_SHIFT_LEFT_E,

    /** shift right */
    CPSS_DXCH_PPU_KSTG_ROT_SHIFT_RIGHT_E
} CPSS_DXCH_PPU_KSTG_ROT_SHIFT_ENT;

/**
 * @enum    CPSS_DXCH_PPU_KSTG_ROT_ADD_SUB_SEL_ENT
 *
 * @brief   Enumerator for PPU ROT addition or subtraction selection
 */
typedef enum {
    /** operation is addition */
    CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E,

    /** operation is substraction */
    CPSS_DXCH_PPU_KSTG_ROT_SUB_SEL_E
} CPSS_DXCH_PPU_KSTG_ROT_ADD_SUB_SEL_ENT;

/**
 * @enum    CPSS_DXCH_PPU_ROT_TARGET_SEL_ENT
 *
 * @brief   Enumerator for PPU ROT target selection
 */
typedef enum {
    /** target is SP_BUS */
    CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E,

    /** target is Header shift */
    CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_HD_SHIFT_E
} CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_ENT;

/**
 * @enum CPSS_DXCH_PPU_KSTG_ROT_CMP_COND_ENT
 *
 * @brief   Enumerator for PPU ROT compare condition
 */
typedef enum {
    /** compare condition is equal */
    CPSS_DXCH_PPU_KSTG_ROT_CMP_COND_EQUAL_E,

    /** compare condition is greater than or equal */
    CPSS_DXCH_PPU_KSTG_ROT_CMP_COND_GREATER_THAN_OR_EQUAL_E,

    /** compare condition is less than or equal */
    CPSS_DXCH_PPU_KSTG_ROT_CMP_COND_LESS_THAN_OR_EQUAL_E,
} CPSS_DXCH_PPU_KSTG_ROT_CMP_COND_ENT;

/**
 * @enum CPSS_DXCH_PPU_KSTG_ROT_FUNC_ENT
 *
 * @brief   Enumerator for PPU ROT function
 */
typedef enum {
    /** ROT function is none */
    CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E,

    /** ROT function is add register */
    CPSS_DXCH_PPU_KSTG_ROT_FUNC_ADD_REG_E,

    /** ROT function is subtract register */
    CPSS_DXCH_PPU_KSTG_ROT_FUNC_SUB_REG_E,

    /** ROT function is compare register */
    CPSS_DXCH_PPU_KSTG_ROT_FUNC_CMP_REG_E
} CPSS_DXCH_PPU_KSTG_ROT_FUNC_ENT;

/**
 * @enum    CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_ENT
 *
 * @brief   Enumerator for PPU Action counter.
 */
typedef enum {
    /** do not change ppu counter */
    CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_NONE_E,

    /** increment ppu counter */
    CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_INC_E,

    /** decrement ppu counter */
    CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_DEC_E
} CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_ENT;

/**
 * @enum CPSS_DXCH_PPU_ERROR_PROFILE_SEL_ENT
 *
 * @brief Enumerator for PPU error profile selection
 */
typedef enum {
    /** error profile is none */
    CPSS_DXCH_PPU_ERROR_PROFILE_SEL_NONE_E,

    /** error profile is profile 1 */
    CPSS_DXCH_PPU_ERROR_PROFILE_SEL_1_E,

    /** error profile is profile 2 */
    CPSS_DXCH_PPU_ERROR_PROFILE_SEL_2_E
} CPSS_DXCH_PPU_ERROR_PROFILE_SEL_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPpuTypesh */
