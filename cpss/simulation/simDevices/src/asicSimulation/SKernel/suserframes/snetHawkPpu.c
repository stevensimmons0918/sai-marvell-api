/*******************************************************************************
*              (c), Copyright 2019, Marvell International Ltd.                 *
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
* @file snetHawkPpu.c
*
* @brief Hawk Asic Simulation .
* Programmable Parser Unit processing for incoming frame.
* Source Code file.
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregHawk.h>
#include <asicSimulation/SKernel/cheetahCommon/sregFalcon.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetHawkPpu.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>

#define SNET_PCL_UDB_MAX_NUMBER_CNS 70

/* convert MSB offset to LSB offset */
#define SNET_SIP6_10_OFFSET_MSB2LSB(x) ((32 - x / 8 - 1) * 8 + ((x) % 8))

/* Descriptor fields array */
static SNET_SIP6_10_PPU_DAU_FIELDS_FORMAT_STC dauDescFields[] =
{
    /*(Is PTP = "True") and (Is PTP = "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_START_BANK_E*/
    {1, 0},
    /*(Is PTP = "True") and (Is PTP = "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_START_GROUP_E*/
    {2, 0},
    /*(Is PTP = "True") and (Is PTP = "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_INGRESS_CORE_ID_E*/
    {3, 0},
    /*(Is PTP = "True") and (Is PTP = "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_DESC_CRITICAL_ECC_E*/
    {6, 0},
    /*Is FCoE = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_D_ID_E*/
    {23, 0},
    /*(Is FCoE != "True") and (Is IPv4 != "True") and (Is ARP != "True") and (Is IPv6 != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_23_0_E*/
    {23, 0},
    /*Is ARP = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_DIP_E*/
    {31, 0},
    /*Is IPv4 = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_SIP_E*/
    {31, 0},
    /*Is IPv6 = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_DIP_E*/
    {127, 0},
    /*(Is IPv4 != "True") and (Is ARP != "True") and (Is IPv6 != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_31_24_E*/
    {31, 24},
    /*Is IPv4 = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DIP_E*/
    {63, 32},
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_63_32_E*/
    {63, 32},
    /*Is ARP = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_SIP_E*/
    {95, 64},
    /*(Is IPv6 != "True") and (Is ARP != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_95_64_E*/
    {95, 64},
    /*Is FCoE = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_S_ID_E*/
    {119, 96},
    /*(Is IPv6 != "True") and (Is FCoE != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_119_96_E*/
    {119, 96},
    /*Is IPv4 = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_FRAGMENTED_E*/
    {121, 120},
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_121_120_E*/
    {121, 120},
    /*Is IPv4 = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_OPTION_FIELD_E*/
    {122, 122},
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_122_122_E*/
    {122, 122},
    /*(Is IP = "False") and (Is FCoE = "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_E*/
    {123, 123},
    /*(Is IPv6 != "True") and ((Is IP != "False") or (Is FCoE != "False"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_123_123_E*/
    {123, 123},
    /*Is IPv4 = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DF_E*/
    {124, 124},
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_124_124_E*/
    {124, 124},
    /*Is FCoE = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_LEGAL_E*/
    {125, 125},
    /*(Is IPv6 != "True") and (Is FCoE != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_125_125_E*/
    {125, 125},
    /*Is IPv4 = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FRAGMENTED_E*/
    {126, 126},
    /*(Is IPv6 != "True") and (Is IPv4 != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_126_126_E*/
    {126, 126},
    /*Is IP = "False"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_FCOE_E*/
    {127, 127},
    /*(Is IPv6 != "True") and (Is IP != "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_127_127_E*/
    {127, 127},
    /*Is IPv6 = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_SIP_E*/
    {255, 128},
    /*(Is IPv6 != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_255_128_E*/
    {255, 128},
    /*Is ARP For Hash = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_DIP_FOR_HASH_E*/
    {287, 256},
    /*Is IPv4 For Hash = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_SIP_FOR_HASH_E*/
    {287, 256},
    /*(Is IPv4 For Hash != "True") and (Is ARP For Hash != "True") and (Is IPv6 For Hash != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_287_256_E*/
    {287, 256},
    /*Is IPv6 For Hash = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_DIP_FOR_HASH_E*/
    {383, 256},
    /*Is IPv4 For Hash = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DIP_FOR_HASH_E*/
    {319, 288},
    /*(Is IPv6 For Hash != "True") and (Is IPv4 For Hash != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_319_288_E*/
    {319, 288},
    /*Is ARP For Hash = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_SIP_FOR_HASH_E*/
    {351, 320},
    /*(Is IPv6 For Hash != "True") and (Is ARP For Hash != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_351_320_E*/
    {351, 320},
    /*(Is IPv4 For Hash = "False") and (Is IPv6 For Hash = "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_FOR_HASH_E*/
    {352, 352},
    /*(Is IPv6 For Hash != "True") and ((Is IPv4 For Hash != "False") or (Is IPv6 For Hash != "False"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_352_352_E*/
    {352, 352},
    /*(Is IPv6 For Hash != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_383_353_E*/
    {383, 353},
    /*Is IPv6 For Hash = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_SIP_FOR_HASH_E*/
    {511, 384},
    /*(Is IPv6 For Hash != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_511_384_E*/
    {511, 384},
    /*Tunnel Start = "TS"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_PTR_E*/
    {527, 512},
    /*Tunnel Start = "LL"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_PTR_E*/
    {529, 512},
    /*Tunnel Start = "TS"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_START_PASSENGER_TYPE_E*/
    {528, 528},
    /*(Tunnel Start != "LL")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_529_529_E*/
    {529, 529},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_VIRTUAL_ID_E*/
    {541, 530},
    /*Redirect Cmd = "2"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICY_LTT_INDEX_E*/
    {559, 542},
    /*(Redirect Cmd != "2")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_547_542_E*/
    {559, 542},
    /*(Use VIDX = "0") and (Trg Is Trunk = "TrunkID")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_TRUNK_ID_E*/
    {571, 560},
    /*(Use VIDX = "0") and (Trg Is Trunk = "ePort")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_EPORT_E*/
    {572, 560},
    /*Use VIDX = "1"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVIDX_E*/
    {575, 560},
    /*((Use VIDX != "0") or (Trg Is Trunk != "ePort")) and (Use VIDX != "1")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_560_560_E*/
    {572, 572},
    /*Use VIDX = "0"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_IS_TRUNK_E*/
    {573, 573},
    /*Use VIDX = "0"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_TRG_PHY_PORT_VALID_E*/
    {574, 574},
    /*(Use VIDX != "1")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_563_563_E*/
    {575, 575},
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1") and (Excluded Is Trunk = "1")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_TRUNK_ID_E*/
    {587, 576},
    /*(Packet Cmd != "From CPU") and (Orig Is Trunk = "1")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_TRUNK_ID_E*/
    {587, 576},
    /*((Packet Cmd = "From CPU") or (Orig Is Trunk != "1")) and ((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "1")) and ((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "0")) and ((Packet Cmd = "From CPU") or (Orig Is Trunk != "0"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_575_564_E*/
    {587, 576},
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1") and (Excluded Is Trunk = "0")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_EPORT_E*/
    {588, 576},
    /*(Packet Cmd != "From CPU") and (Orig Is Trunk = "0")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_EPORT_E*/
    {588, 576},
    /*((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "0")) and ((Packet Cmd = "From CPU") or (Orig Is Trunk != "0"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_576_576_E*/
    {588, 588},
    /*(Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E*/
    {600, 589},
    /*(Packet Cmd = "To CPU") or (Packet Cmd = "To Target Sniffer")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_EPORT_E*/
    {601, 589},
    /*Packet Cmd = "From CPU"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_CPU_TO_CPU_MIRROR_E*/
    {601, 601},
    /*((Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer")) and (Packet Cmd != "From CPU")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_589_589_E*/
    {601, 601},
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_IS_TRUNK_E*/
    {602, 602},
    /*((Packet Cmd != "From CPU") or (Use VIDX != "1")) and ((Use VIDX != "0") or (Is Trg Phy Port Valid != "True"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_590_590_E*/
    {602, 602},
    /*(Use VIDX = "0") and (Is Trg Phy Port Valid = "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_PHY_PORT_E*/
    {611, 602},
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1") and (Excluded Is Trunk = "0")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_DEV_IS_LOCAL_E*/
    {603, 603},
    /*((Use VIDX != "0") or (Is Trg Phy Port Valid != "True")) and ((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "0"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_591_591_E*/
    {603, 603},
    /*(Packet Cmd = "From CPU") and (Use VIDX = "1") and (Excluded Is Trunk = "0")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_IS_PHY_PORT_E*/
    {604, 604},
    /*((Use VIDX != "0") or (Is Trg Phy Port Valid != "True")) and ((Packet Cmd != "From CPU") or (Use VIDX != "1") or (Excluded Is Trunk != "0"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_592_592_E*/
    {604, 604},
    /*((Use VIDX != "0") or (Is Trg Phy Port Valid != "True"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_599_593_E*/
    {611, 605},
    /*Packet Cmd = "From CPU"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FROM_CPU_TC_E*/
    {614, 612},
    /*Packet Cmd != "From CPU"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_PROFILE_E*/
    {621, 612},
    /*Packet Cmd = "From CPU"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FROM_CPU_DP_E*/
    {616, 615},
    /*Packet Cmd = "From CPU"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_FILTER_EN_E*/
    {617, 617},
    /*(Packet Cmd = "From CPU")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_609_606_E*/
    {621, 618},
    /*(Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_PHY_IS_TRUNK_E*/
    {622, 622},
    /*(Packet Cmd = "To CPU") or (Packet Cmd = "To Target Sniffer")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_DEV_E*/
    {631, 622},
    /*((Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_619_611_E*/
    {631, 623},
    /*((Packet Cmd != "To CPU") and (Packet Cmd != "To Target Sniffer"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_629_620_E*/
    {641, 632},
    /*(Packet Cmd = "To CPU") or (Packet Cmd = "To Target Sniffer")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_PHY_PORT_E*/
    {641, 632},
    /*((Use VIDX != "0") or (Trg Is Trunk != "ePort"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_639_630_E*/
    {651, 642},
    /*(Use VIDX = "0") and (Trg Is Trunk = "ePort")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_DEV_E*/
    {651, 642},
    /*QCN Rx = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_RX_QCN_PRIO_E*/
    {654, 652},
    /*QCN Rx = "False"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TTL_E*/
    {659, 652},
    /*(QCN Rx != "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_647_643_E*/
    {659, 655},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_OFFSET_E*/
    {666, 660},
    /*Timestamp En = "Enable"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PTP_OFFSET_INDEX_E*/
    {673, 667},
    /*Timestamp En = "Disable"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_OFFSET_E*/
    {673, 667},
    /*Tunnel Terminated = "TT"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_HEADER_OFFSET_E*/
    {680, 674},
    /*(Tunnel Terminated != "TT")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_668_662_E*/
    {680, 674},
    /*Tunnel Terminated = "TT"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_L3_OFFSET_E*/
    {686, 681},
    /*(Tunnel Terminated != "TT")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_674_669_E*/
    {686, 681},
    /*(L2 Echo = "0") and (Is PTP = "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_DOMAIN_E*/
    {689, 687},
    /*((L2 Echo != "0") or (Is PTP != "True"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_677_675_E*/
    {689, 687},
    /*Packet Cmd = "To Target Sniffer"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_SNIFF_E*/
    {690, 690},
    /*Packet Cmd != "To Target Sniffer"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ANALYZER_INDEX_E*/
    {692, 690},
    /*(Packet Cmd = "To Target Sniffer")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_680_679_E*/
    {692, 691},
    /*OAM Processing En = "Enable"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PROFILE_E*/
    {693, 693},
    /*(OAM Processing En != "Enable") and ((OAM Processing En != "Disable") or (L2 Echo != "0") or (Is PTP != "True"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_681_681_E*/
    {693, 693},
    /*(OAM Processing En = "Disable") and (L2 Echo = "0") and (Is PTP = "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_TRIGGER_TYPE_E*/
    {694, 693},
    /*((OAM Processing En != "Disable") or (L2 Echo != "0") or (Is PTP != "True"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_682_682_E*/
    {694, 694},
    /*Packet Cmd != "From CPU"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_IS_TRUNK_E*/
    {695, 695},
    /*(Packet Cmd = "From CPU")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_683_683_E*/
    {695, 695},
    /*(Packet Cmd = "From CPU")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_684_684_E*/
    {696, 696},
    /*Packet Cmd != "From CPU"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ROUTED_E*/
    {696, 696},
    /*(Is IP = "True") and (Is IPv4 = "False") */
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_E*/
    {697, 697},
    /*((Is IP != "True") or (Is IPv4 != "False"))*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_685_685_E*/
    {697, 697},
    /*Is IP = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_E*/
    {698, 698},
    /*(Is IP != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_686_686_E*/
    {698, 698},
    /*Is IPv4 For Hash = "False"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_FOR_HASH_E*/
    {699, 699},
    /*(Is IPv4 For Hash != "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_687_687_E*/
    {699, 699},
    /*Is IPv6 = "True"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_HBH_EXT_E*/
    {700, 700},
    /*(Is IPv6 != "True")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_688_688_E*/
    {700, 700},
    /*Packet Cmd != "From CPU"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_MODE_E*/
    {701, 701},
    /*(Packet Cmd = "From CPU")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_689_689_E*/
    {701, 701},
    /*L2 Echo = "0"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_PTP_E*/
    {702, 702},
    /*(L2 Echo != "0")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_690_690_E*/
    {702, 702},
    /*Is FCoE = "False"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_LEGAL_E*/
    {703, 703},
    /*(Is FCoE != "False")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_691_691_E*/
    {703, 703},
    /*(Packet Cmd != "To CPU")*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_692_692_E*/
    {704, 704},
    /*Packet Cmd = "To CPU"*/
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_E*/
    {704, 704},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SR_END_NODE_E*/
    {705, 705},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SR_EH_OFFSET_E*/
    {709, 706},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_CPU_CODE_E*/
    {717, 710},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_KEEP_PREVIOUS_HASH_E*/
    {718, 718},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_IS_CUT_THROUGH_E*/
    {719, 719},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TABLES_READ_ERROR_E*/
    {720, 720},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_FOR_KEY_E*/
    {721, 721},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_FOR_KEY_E*/
    {722, 722},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_PRIO_TAGGED_E*/
    {723, 723},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_IS_LLC_NON_SNAP_E*/
    {724, 724},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TCP_UDP_DEST_PORT_E*/
    {740, 725},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TCP_UDP_SRC_PORT_E*/
    {756, 741},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TCP_UDP_PORT_IS_0_E*/
    {757, 757},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IP_E*/
    {758, 758},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPM_E*/
    {759, 759},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SOURCE_PORT_LIST_FOR_KEY_E*/
    {787, 760},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_INGRESS_UDB_PACKET_TYPE_E*/
    {791, 788},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_CHANNEL_TYPE_PROFILE_E*/
    {795, 792},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E*/
    {796, 796},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_UDB_VALID_SET_E*/
    {866, 797},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_UDB_SET_E*/
    {1426, 867},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_FLOW_LABEL_FOR_HASH_E*/
    {1446, 1427},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_BYTES_FOR_HASH_E*/
    {1478, 1447},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL2_FOR_HASH_E*/
    {1498, 1479},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL1_FOR_HASH_E*/
    {1518, 1499},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL0_FOR_HASH_E*/
    {1538, 1519},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_FOR_HASH_E*/
    {1586, 1539},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_FOR_HASH_E*/
    {1634, 1587},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_FOR_HASH_E*/
    {1635, 1635},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_HEADER_INFO_E*/
    {1637, 1636},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_SUB_TEMPLATE_E*/
    {1645, 1638},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_REDIRECT_CMD_E*/
    {1648, 1646},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_TOS_E*/
    {1656, 1649},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_TYPE_FOR_KEY_E*/
    {1659, 1657},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_USE_INTERFACE_BASED_INDEX_E*/
    {1660, 1660},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_DISABLE_IPCL0_FOR_ROUTED_PACKETS_E*/
    {1661, 1661},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPCL_PROFILE_INDEX_E*/
    {1674, 1662},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP2_USE_INDEX_FROM_DESC_E*/
    {1675, 1675},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP2_PCL_MODE_E*/
    {1677, 1676},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP1_USE_INDEX_FROM_DESC_E*/
    {1678, 1678},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP1_PCL_MODE_E*/
    {1680, 1679},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP0_USE_INDEX_FROM_DESC_E*/
    {1681, 1681},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP0_PCL_MODE_E*/
    {1683, 1682},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICY_EN_E*/
    {1684, 1684},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVLAN_PRECEDENCE_E*/
    {1685, 1685},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_ENCAPSULATION_E*/
    {1687, 1686},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_BYTES_E*/
    {1727, 1688},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_TRACK_EN_E*/
    {1728, 1728},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_L3_OFFSET_INVALID_E*/
    {1729, 1729},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_VALID_E*/
    {1730, 1730},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_SRC_TAG_E*/
    {1731, 1731},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_TO_ME_E*/
    {1732, 1732},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_METERING_EN_E*/
    {1733, 1733},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_FLOW_LABEL_E*/
    {1753, 1734},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_EH_E*/
    {1754, 1754},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_BC_E*/
    {1755, 1755},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_MLD_E*/
    {1756, 1756},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_LINK_LOCAL_E*/
    {1757, 1757},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_MPLS_E*/
    {1758, 1758},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ND_E*/
    {1759, 1759},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_HASH_MASK_INDEX_E*/
    {1763, 1760},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_CMD_E*/
    {1766, 1764},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_SBIT_E*/
    {1767, 1767},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_EXP_E*/
    {1770, 1768},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_LABEL_E*/
    {1790, 1771},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ETHER_TYPE_E*/
    {1806, 1791},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_ASSIGNED_SST_ID_E*/
    {1818, 1807},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYPASS_INGRESS_PIPE_E*/
    {1819, 1819},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYPASS_BRIDGE_E*/
    {1820, 1820},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SOLICITATION_MULTICAST_MESSAGE_E*/
    {1821, 1821},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_E*/
    {1869, 1822},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_E*/
    {1917, 1870},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_QCN_RX_E*/
    {1918, 1918},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_ARP_SA_MISMATCH_E*/
    {1919, 1919},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_HEADER_LENGTH_E*/
    {1924, 1920},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_LENGTH_E*/
    {1940, 1925},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICER_PTR_E*/
    {1954, 1941},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_BILLING_EN_E*/
    {1955, 1955},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_PROFILE_PRECEDENCE_E*/
    {1956, 1956},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_PRIO_TAGGED_E*/
    {1957, 1957},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PROCESSING_EN_E*/
    {1958, 1958},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_VALID_E*/
    {1959, 1959},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_PROTOCOL_E*/
    {1967, 1960},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SYN_WITH_DATA_E*/
    {1968, 1968},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_SYN_E*/
    {1969, 1969},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_REPLY_E*/
    {1970, 1970},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_PROTECTION_SWITCH_EN_E*/
    {1971, 1971},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_IS_PROTECTION_PATH_E*/
    {1972, 1972},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_CMD_E*/
    {1975, 1973},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_TYPE_E*/
    {1977, 1976},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_DEV_IS_OWN_E*/
    {1978, 1978},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_TRUNK_ID_E*/
    {1990, 1979},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MODIFY_UP_E*/
    {1991, 1991},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ECN_CAPABLE_E*/
    {1992, 1992},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_REP_LAST_E*/
    {1993, 1993},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_REP_E*/
    {1994, 1994},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_USE_VIDX_E*/
    {1995, 1995},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAIL_BOX_TO_NEIGHBOR_CPU_E*/
    {1996, 1996},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_FILTER_REGISTERED_E*/
    {1997, 1997},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SST_ID_E*/
    {2009, 1998},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_L3_OFFSET_E*/
    {2015, 2010},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_ECHO_E*/
    {2016, 2016},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_PACKET_TYPE_E*/
    {2019, 2017},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_HASH_E*/
    {2051, 2020},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_TAGGED_E*/
    {2052, 2052},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_IS_LLC_NON_SNAP_E*/
    {2053, 2053},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_COPY_RESERVED_E*/
    {2073, 2054},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_ID_E*/
    {2089, 2074},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MODIFY_DSCP_EXP_E*/
    {2090, 2090},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_DO_ROUTE_HA_E*/
    {2091, 2091},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_DROP_ON_SOURCE_E*/
    {2092, 2092},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_IS_LOOPED_E*/
    {2093, 2093},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_NESTED_VLAN_EN_E*/
    {2094, 2094},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_TRILL_E*/
    {2095, 2095},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_TAI_SELECT_E*/
    {2096, 2096},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_U_FIELD_E*/
    {2097, 2097},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_TAGGED_E*/
    {2100, 2098},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_E*/
    {2132, 2101},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_EN_E*/
    {2133, 2133},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_NUM_OF_TAG_BYTES_TO_POP_E*/
    {2135, 2134},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_TPID_INDEX_E*/
    {2138, 2136},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_TPID_INDEX_E*/
    {2141, 2139},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_LOCAL_DEV_SRC_TAGGED_E*/
    {2142, 2142},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TAG0_IS_OUTER_TAG_E*/
    {2143, 2143},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_SRC_TAGGED_E*/
    {2144, 2144},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_SRC_TAGGED_E*/
    {2145, 2145},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_OVERRIDE_EVLAN_WITH_ORIGVID_E*/
    {2146, 2146},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_VID_E*/
    {2158, 2147},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_UP1_E*/
    {2161, 2159},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_CFI1_E*/
    {2162, 2162},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_VID1_E*/
    {2174, 2163},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_UP0_E*/
    {2177, 2175},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_CFI0_E*/
    {2178, 2178},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVLAN_E*/
    {2191, 2179},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_DEV_E*/
    {2201, 2192},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_PHY_SRC_MC_FILTER_EN_E*/
    {2202, 2202},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_EPORT_E*/
    {2215, 2203},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_PORT_E*/
    {2225, 2216},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_UDB_PACKET_TYPE_E*/
    {2229, 2226},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_START_E*/
    {2230, 2230},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_TERMINATED_E*/
    {2231, 2231},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MARVELL_TAGGED_EXTENDED_E*/
    {2233, 2232},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MARVELL_TAGGED_E*/
    {2234, 2234},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_RECALC_CRC_E*/
    {2235, 2235},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYTE_COUNT_E*/
    {2249, 2236},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_MDB_E*/
    {2250, 2250},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_ECN_E*/
    {2252, 2251},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_SKIP_FDB_SA_LOOKUP_E*/
    {2253, 2253},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_FIRST_BUFFER_E*/
    {2264, 2254},
    /*SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPFIX_EN_E*/
    {2265, 2265}
};

/* set bits in input stream */
static GT_VOID setBits
(
    IN  GT_U32   offset,
    IN  GT_U32   numBits,
    IN  GT_U32   val,
    OUT GT_U8    *data
)
{
    GT_U32 ii, byte, bitShift;
    GT_U8 mask = 0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        mask = ~(1 << bitShift);
        data[byte] |= (data[byte] & mask) | (((val >> ii) & 1) << bitShift);
    }
}

static GT_VOID setBitsLeFromDataPtr
(
    IN  GT_U32   offset,
    IN  GT_U32   numBits,
    IN  GT_U8    *inData,
    OUT GT_U8    *outData
)
{
    GT_U32 ii, byte;
    GT_U8 mask = 0;
    GT_U8 bitShift = 0;

    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii)%8;
        mask = ~(1 << bitShift);
        outData[byte] = (outData[byte] & mask) | ((GT_U8)(((inData[ii/8] >> (ii%8)) & 1) << bitShift));
    }
}

/* set bits in input stream
 * Ex: descPtr->mac_sa = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}
 * In output buffer, LSB = 0x01, MSB = 0x06 */
static GT_VOID setBitsBeFromDataPtr
(
    IN  GT_U32   offset,
    IN  GT_U32   numBits,
    IN  GT_U8    *inData,
    OUT GT_U8    *outData
)
{
    GT_U32 ii, byte, lastByte;
    GT_U8 mask = 0;
    GT_U8 bitShift = 0;

    lastByte = (numBits - 1) / 8;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (GT_U8)((offset + ii)%8);
        mask = ~(1 << bitShift);
        outData[byte] = (outData[byte] & mask) | (((inData[lastByte - ii/8] >> (ii%8)) & 1) << bitShift);
    }
}

/* set bits in input stream
 * Assumption: offset it multiple of 8.
 * This function sets bytes not bits */
static GT_VOID setIpv6BytesFromDataPtr
(
    IN  GT_U32   offset,
    IN  GT_U32   numBytes,
    IN  GT_U32   *inData,
    OUT GT_U8    *outData
)
{
    GT_U32 i;
    GT_U32 startByte = offset/8;

    for (i=0; i<numBytes; i++)
    {
        outData[startByte + (numBytes-i-1)] = (GT_U8)((inData[i/4] >> ((3 - (i%4))*8)) & 0xFF);
    }
}


/* set bits in descriptorinput stream */
static GT_VOID setBitsInDescriptor
(
    IN  GT_U32   offset,
    IN  GT_U32   numBits,
    IN  GT_U32   val,
    OUT GT_U8    *data,
    OUT GT_U8    *dataMask
)
{
    GT_U32 ii, byte, bitShift;
    GT_U8 mask = 0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        mask = ~(1 << bitShift);
        data[byte] = (data[byte] & mask) | (((val >> ii) & 1) << bitShift);
        dataMask[byte] = dataMask[byte] | (1 << bitShift);
    }
}

/* get bits from input stream */
static GT_U32 getBits
(
    IN  GT_U8    *data,
    IN  GT_U32   offset,
    IN  GT_U32   numBits
)
{
    GT_U32 ii, byte, bitShift, outData=0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        outData  |= ((data[byte] >> bitShift) & 1) << ii;
    }
    return outData;
}

/* get bits from hw descritor stream */
static GT_U32 getBitsForDescriptor
(
    IN  GT_U8    *data,
    IN  GT_U8    *dataMask,
    IN  GT_U32   offset,
    IN  GT_U32   numBits,
    OUT GT_BOOL  *isChanged
)
{
    GT_U32 ii, byte, bitShift, outData=0;

    *isChanged = GT_FALSE;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        if(((dataMask[byte] >> bitShift) & 1) == GT_TRUE)
        {
            *isChanged = GT_TRUE;
            outData  |= ((data[byte] >> bitShift) & 1) << ii;
        }
    }
    return outData;
}

/* get bits from hw descritor stream */
static GT_VOID getBitsLeToDataPtr
(
    IN  GT_U8    *data,
    IN  GT_U8    *dataMask,
    IN  GT_U32   offset,
    IN  GT_U32   numBits,
    OUT GT_BOOL  *isChanged,
    OUT GT_U8   *outDataPtr
)
{
    GT_U32 ii, byte;
    GT_U8 bitShift;

    *isChanged = GT_FALSE;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (GT_U8)((offset + ii)%8);
        if(((dataMask[byte] >> bitShift) & 1) == GT_TRUE)
        {
            *isChanged = GT_TRUE;
            outDataPtr[ii/8]  &= (GT_U8)(~(1 << (ii%8))); /* clear bit */
            outDataPtr[ii/8]  |= (GT_U8)(((data[byte] >> bitShift) & 1) << (ii%8));
        }
    }
}

/* get bits from hw descritor stream */
static GT_VOID getBitsBeToDataPtr
(
    IN  GT_U8    *data,
    IN  GT_U8    *dataMask,
    IN  GT_U32   offset,
    IN  GT_U32   numBits,
    OUT GT_BOOL  *isChanged,
    OUT GT_U8   *outDataPtr
)
{
    GT_U32 ii, byte, lastByte;
    GT_U8 bitShift;

    *isChanged = GT_FALSE;
    lastByte = (numBits -1) / 8;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (GT_U8)((offset + ii)%8);
        if(((dataMask[byte] >> bitShift) & 1) == GT_TRUE)
        {
            *isChanged = GT_TRUE;
            outDataPtr[lastByte - ii/8]  &= ~(1 << (ii%8)); /* clear bit */
            outDataPtr[lastByte - ii/8]  |= ((data[byte] >> bitShift) & 1) << (ii%8); /* set bit */
        }
    }
}

/* get bits from hw descritor stream
 * Assumption offset is multiple of 8
 */
static GT_VOID getIpv6BytesToDataPtr
(
    IN  GT_U8    *data,
    IN  GT_U32   offset,
    IN  GT_U32   numBytes,
    OUT GT_BOOL  *isChanged,
    OUT GT_U32   *outDataPtr
)
{
    GT_U32 i;
    GT_U32  startByte = offset/8;
    GT_U32  shift, mask;

    *isChanged = GT_FALSE;
    for(i = 0; i < numBytes; i++)
    {
        *isChanged = GT_TRUE;
        shift = (3-(i%4))*8;
        mask = (0xFF << shift);
        outDataPtr[i/4] = (outDataPtr[i/4] & ~(mask)) | ((data[startByte + (numBytes-i-1)] << shift) & mask);
    }
}


/* software to hardware descriptor field set */
#define SNET_SIP6_10_DESC_SW2HW_FIELD_SET(bus, field, param) \
    setBits(dauDescFields[field].startOffset, \
            dauDescFields[field].endOffset - dauDescFields[field].startOffset + 1, \
            (GT_U32)param, bus)

/* software to hardware descriptor field set to data pointer*/
#define SNET_SIP6_10_DESC_SW2HW_FIELD_PTR_SET(bus, field, param) \
    setBitsBeFromDataPtr(dauDescFields[field].startOffset, \
            dauDescFields[field].endOffset - dauDescFields[field].startOffset + 1, \
            param, bus)

/* software to hardware descriptor field set to data pointer*/
#define SNET_SIP6_10_DESC_SW2HW_FIELD_IPV6_SET(bus, field, param) \
    setIpv6BytesFromDataPtr(dauDescFields[field].startOffset, \
            (dauDescFields[field].endOffset - dauDescFields[field].startOffset + 1)/8, \
            param, bus)

/* software to hardware descriptor field set to data pointer*/
#define SNET_SIP6_10_DESC_SW2HW_FIELD_LE_PTR_SET(bus, field, param) \
    setBitsLeFromDataPtr(dauDescFields[field].startOffset, \
            dauDescFields[field].endOffset - dauDescFields[field].startOffset + 1, \
            param, bus)

/* hardware to software descriptor field set */
#define SNET_SIP6_10_DESC_HW2SW_FIELD_SET(bus, mask, field, param) \
    {\
        GT_U32 tmpValue;\
        GT_BOOL isChanged;\
        tmpValue = getBitsForDescriptor(bus, mask, dauDescFields[field].startOffset, \
            dauDescFields[field].endOffset - dauDescFields[field].startOffset + 1, &isChanged);\
        if(isChanged == GT_TRUE)\
        {\
            param = tmpValue;\
        }\
    }

#define MAX_DESC_PTR_PARAM_LENGTH_IN_BYTES_CNS 128

/* hardware to software descriptor field set to data pointer*/
#define SNET_SIP6_10_DESC_HW2SW_FIELD_PTR_SET(bus, mask, field, param) \
    {\
        GT_U8 tmpParam[MAX_DESC_PTR_PARAM_LENGTH_IN_BYTES_CNS];\
        GT_U32 paramLength = dauDescFields[field].endOffset - dauDescFields[field].startOffset + 1;\
        GT_U32 paramLengthInBytes = (paramLength%8 == 0) ? paramLength/8 : paramLength/8+1;\
        GT_BOOL isChanged;\
        memcpy(tmpParam, param, paramLengthInBytes);\
        getBitsBeToDataPtr(bus, mask, dauDescFields[field].startOffset, \
                paramLength, &isChanged, tmpParam);\
        if(isChanged == GT_TRUE)\
        {\
            memcpy(param, tmpParam, paramLengthInBytes);\
        }\
    }

            /* hardware to software descriptor field set to data pointer*/
#define SNET_SIP6_10_DESC_HW2SW_FIELD_IPV6_SET(bus, mask, field, param) \
    {\
        GT_U32 tmpParam[MAX_DESC_PTR_PARAM_LENGTH_IN_BYTES_CNS];\
        GT_U32 paramLength = dauDescFields[field].endOffset - dauDescFields[field].startOffset + 1;\
        GT_U32 paramLengthInBytes = (paramLength%8 == 0) ? paramLength/8 : paramLength/8+1;\
        GT_BOOL isChanged;\
        memcpy(tmpParam, param, paramLengthInBytes);\
        getIpv6BytesToDataPtr(bus, dauDescFields[field].startOffset, \
                paramLength/8, &isChanged, tmpParam);\
        if(isChanged == GT_TRUE)\
        {\
            memcpy(param, tmpParam, paramLengthInBytes);\
        }\
    }

/* hardware to software descriptor field set to data pointer*/
#define SNET_SIP6_10_DESC_HW2SW_FIELD_LE_PTR_SET(bus, mask, field, param) \
    {\
        GT_U8 tmpParam[MAX_DESC_PTR_PARAM_LENGTH_IN_BYTES_CNS];\
        GT_U32 paramLength = dauDescFields[field].endOffset - dauDescFields[field].startOffset + 1;\
        GT_U32 paramLengthInBytes = (paramLength%8 == 0) ? paramLength/8 : paramLength/8+1;\
        GT_BOOL isChanged;\
        memcpy(tmpParam, param, paramLengthInBytes);\
        getBitsLeToDataPtr(bus, mask, dauDescFields[field].startOffset, \
                paramLength, &isChanged, tmpParam);\
        if(isChanged == GT_TRUE)\
        {\
            memcpy(param, tmpParam, paramLengthInBytes);\
        }\
    }

/* TBD : Set the commented conditions appropriately */
#define COND_IS_IPV6_EQ_TRUE                    ((descrPtr->isIp == GT_TRUE) && \
                                                 (descrPtr->isIPv4 == GT_FALSE) && (descrPtr->isFcoe == GT_FALSE)) /* Is IPv6 = "True" */
#define COND_IS_IPV6_NOT_EQ_TRUE                0 /* Is IPv6 != "True" */
#define COND_IS_ARP_FOR_HASH_EQ_TRUE            0 /* Is ARP For Hash = "True" */
#define COND_IS_ARP_FOR_HASH_NOT_EQ_TRUE        0 /* Is ARP For Hash != "True" */
#define COND_IS_IPV4_FOR_HASH_EQ_TRUE           0 /* Is IPv4 For Hash = "True" */
#define COND_IS_IPV4_FOR_HASH_NOT_EQ_TRUE       0 /* Is IPv4 For Hash != "True" */
#define COND_IS_IPV4_FOR_HASH_EQ_FALSE          0 /* Is IPv4 For Hash = "False" */
#define COND_IS_IPV4_FOR_HASH_NOT_EQ_FALSE      0 /* Is IPv4 For Hash != "False" */
#define COND_IS_IPV6_FOR_HASH_EQ_TRUE           0 /* Is IPv6 For Hash = "True" */
#define COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE       0 /* Is IPv6 For Hash != "True" */
#define COND_IS_IPV6_FOR_HASH_EQ_FALSE          0 /* Is IPv6 For Hash = "False" */
#define COND_IS_IPV6_FOR_HASH_NOT_EQ_FALSE      0 /* Is IPv6 For Hash != "False" */
#define COND_TUNNEL_START_EQ_TS                 0 /* Tunnel Start = "TS" */
#define COND_TUNNEL_START_EQ_LL                 0 /* Tunnel Start = "LL" */
#define COND_TUNNEL_START_NOT_EQ_LL             0 /* Tunnel Start != "LL" */
#define COND_PACKET_CMD_EQ_FROM_CPU             0 /* Packet Cmd = "From CPU" */
#define COND_PACKET_CMD_NOT_EQ_FROM_CPU         0 /* Packet Cmd != "From CPU" */
#define COND_PACKET_CMD_EQ_TO_CPU               0 /* Packet Cmd = "To CPU" */
#define COND_PACKET_CMD_NOT_EQ_TO_CPU           0 /* Packet Cmd != "To CPU" */
#define COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER    0 /* Packet Cmd = "To Target Sniffer" */
#define COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER 0 /* Packet Cmd != "To Target Sniffer" */
#define COND_REDIRECT_CMD_EQ_2                  0 /* Redirect Cmd = "2" */
#define COND_REDIRECT_CMD_NOT_EQ_2              0 /* Redirect Cmd != "2" */
#define COND_US_VIDX_EQ_0                       0 /* Use VIDX = "0" */
#define COND_US_VIDX_NOT_EQ_0                   0 /* Use VIDX != "0" */
#define COND_US_VIDX_EQ_1                       0 /* Use VIDX = "1" */
#define COND_US_VIDX_NOT_EQ_1                   0 /* Use VIDX != "1" */
#define COND_TRG_IS_TRUNK_EQ_TRUNK_ID           0 /* Trg Is Trunk = "TrunkID" */
#define COND_TRG_IS_TRUNK_EQ_EPORT              0 /* Trg Is Trunk = "ePort" */
#define COND_TRG_IS_TRUNK_NOT_EQ_EPORT          0 /* Trg Is Trunk != "ePort" */
#define COND_EXCLUDED_IS_TRUNK_EQ_1             0 /* Excluded Is Trunk = "1" */
#define COND_EXCLUDED_IS_TRUNK_NOT_EQ_1         0 /* Excluded Is Trunk != "1" */
#define COND_EXCLUDED_IS_TRUNK_EQ_0             0 /* Excluded Is Trunk = "0" */
#define COND_EXCLUDED_IS_TRUNK_NOT_EQ_0         0 /* Excluded Is Trunk != "0" */
#define COND_ORIG_IS_TRUNK_EQ_1                 0 /* Orig Is Trunk = "1" */
#define COND_ORIG_IS_TRUNK_NOT_EQ_1             0 /* Orig Is Trunk != "1" */
#define COND_ORIG_IS_TRUNK_EQ_0                 0 /* Orig Is Trunk = "0" */
#define COND_ORIG_IS_TRUNK_NOT_EQ_0             0 /* Orig Is Trunk != "0" */
#define COND_IS_TRG_PHY_PORT_VALID_EQ_TRUE      0 /* Is Trg Phy Port Valid = "True" */
#define COND_IS_TRG_PHY_PORT_VALID_NOT_EQ_TRUE  0 /* Is Trg Phy Port Valid != "True" */
#define COND_QCN_RX_EQ_TRUE                     0 /* QCN Rx = "True" */
#define COND_QCN_RX_EQ_FALSE                    (descrPtr->qcnRx == GT_FALSE) /* QCN Rx = "False" */
#define COND_QCN_RX_NOT_EQ_FALSE                0 /* QCN Rx != "False" */
#define COND_TIME_STAMP_EN_EQ_ENABLE            0 /* Timestamp En = "Enable" */
#define COND_TIME_STAMP_EN_EQ_DISABLE           0 /* Timestamp En = "Disable" */
#define COND_TUNNEL_TERMINATED_EQ_TT            0 /* Tunnel Terminated = "TT" */
#define COND_TUNNEL_TERMINATED_NOT_EQ_TT        0 /* Tunnel Terminated != "TT" */
#define COND_L2_ECHO_EQ_0                       0 /* L2 Echo = "0" */
#define COND_L2_ECHO_NOT_EQ_0                   0 /* L2 Echo != "0" */
#define COND_OAM_PROCESSING_EN_EQ_ENABLE        0 /* OAM Processing En = "Enable" */
#define COND_OAM_PROCESSING_EN_NOT_EQ_ENABLE    0 /* OAM Processing En != "Enable" */
#define COND_OAM_PROCESSING_EN_EQ_DISABLE       0 /* OAM Processing En = "Disable" */
#define COND_OAM_PROCESSING_EN_NOT_EQ_DISABLE   0 /* OAM Processing En != "Disable" */

/* convert software descriptor to software descriptor */
static GT_VOID snetHawkPpuDauDescSwToHwConvert
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    OUT GT_U8                               *hwDescBusPtr
)
{
    GT_U32 dummyTbdParam = 0;

    if((descrPtr->isPtp == GT_TRUE) && (descrPtr->isPtp == GT_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_START_BANK_E,                        dummyTbdParam);
    }
    if((descrPtr->isPtp == GT_TRUE) && (descrPtr->isPtp == GT_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_START_GROUP_E,                       dummyTbdParam);
    }
    if((descrPtr->isPtp == GT_TRUE) && (descrPtr->isPtp == GT_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INGRESS_CORE_ID_E,                   descrPtr->srcCoreId);
    }
    if((descrPtr->isPtp == GT_TRUE) && (descrPtr->isPtp == GT_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_DESC_CRITICAL_ECC_E,                 dummyTbdParam);
    }
    if(descrPtr->isFcoe == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_D_ID_E,                         dummyTbdParam);
    }
    if((descrPtr->isFcoe != GT_TRUE) && (descrPtr->isIPv4 != GT_TRUE) && (descrPtr->arp != GT_TRUE) && (COND_IS_IPV6_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_23_0_E,                     dummyTbdParam);
    }
    if(descrPtr->arp == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_DIP_E,                           descrPtr->dip[0]);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_SIP_E,                          descrPtr->sip[0]);
    }
    if(COND_IS_IPV6_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_IPV6_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_DIP_E,                      descrPtr->dip);
    }
    if((descrPtr->isIPv4 != GT_TRUE) && (descrPtr->arp != GT_TRUE) && (COND_IS_IPV6_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_31_24_E,                    dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DIP_E,                          descrPtr->dip[0]);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_63_32_E,                    dummyTbdParam);
    }
    if(descrPtr->arp == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_SIP_E,                           descrPtr->sip[0]);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->arp != GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_95_64_E,                    dummyTbdParam);
    }
    if(descrPtr->isFcoe == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_S_ID_E,                         dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isFcoe != GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_119_96_E,                   dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_FRAGMENTED_E,                     dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_121_120_E,                  dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_OPTION_FIELD_E,                 dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_122_122_E,                  dummyTbdParam);
    }
    if((descrPtr->isIp == GT_FALSE) && (descrPtr->isFcoe == GT_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_E,                            descrPtr->arp);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && ((descrPtr->isIp != GT_FALSE) || (descrPtr->isFcoe != GT_FALSE)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_123_123_E,                  dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DF_E,                           descrPtr->ipv4DontFragmentBit);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_124_124_E,                  dummyTbdParam);
    }
    if(descrPtr->isFcoe == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_LEGAL_E,                        descrPtr->fcoeInfo.fcoeLegal);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isFcoe != GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_125_125_E,                  dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FRAGMENTED_E,                        dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_126_126_E,                  dummyTbdParam);
    }
    if(descrPtr->isIp == GT_FALSE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_FCOE_E,                           descrPtr->isFcoe);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIp != GT_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_127_127_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV6_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_IPV6_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_SIP_E,                  descrPtr->sip);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_255_128_E,                  dummyTbdParam);
    }
    if(COND_IS_ARP_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_DIP_FOR_HASH_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV4_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_SIP_FOR_HASH_E,                 dummyTbdParam);
    }
    if((COND_IS_IPV4_FOR_HASH_NOT_EQ_TRUE) && (COND_IS_ARP_FOR_HASH_NOT_EQ_TRUE) && (COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_287_256_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV6_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_DIP_FOR_HASH_E,                 dummyTbdParam);
    }
    if(COND_IS_IPV4_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DIP_FOR_HASH_E,                 dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE) && (COND_IS_IPV4_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_319_288_E,                  dummyTbdParam);
    }
    if(COND_IS_ARP_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_SIP_FOR_HASH_E,                  dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE) && (COND_IS_ARP_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_351_320_E,                  dummyTbdParam);
    }
    if((COND_IS_IPV4_FOR_HASH_EQ_FALSE) && (COND_IS_IPV6_FOR_HASH_EQ_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_FOR_HASH_E,                   dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE) && ((COND_IS_IPV4_FOR_HASH_NOT_EQ_FALSE) || (COND_IS_IPV6_FOR_HASH_NOT_EQ_FALSE)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_352_352_E,                  dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_383_353_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV6_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_SIP_FOR_HASH_E,                 dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_511_384_E,                  dummyTbdParam);
    }
    if(COND_TUNNEL_START_EQ_TS)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_PTR_E,                        descrPtr->tunnelPtr);
    }
    if(COND_TUNNEL_START_EQ_LL)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_PTR_E,                           descrPtr->arpPtr);
    }
    if(COND_TUNNEL_START_EQ_TS)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_START_PASSENGER_TYPE_E,       descrPtr->tunnelStartPassengerType);
    }
    if((COND_TUNNEL_START_NOT_EQ_LL))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_529_529_E,                  dummyTbdParam);
    }
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_VIRTUAL_ID_E,                        dummyTbdParam);
    if(COND_REDIRECT_CMD_EQ_2)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICY_LTT_INDEX_E,                  descrPtr->ttRouterLTT);
    }
    if((COND_REDIRECT_CMD_NOT_EQ_2))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_547_542_E,                  dummyTbdParam);
    }
    if((COND_US_VIDX_EQ_0) && (COND_TRG_IS_TRUNK_EQ_TRUNK_ID))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_TRUNK_ID_E,                      descrPtr->trgTrunkId);
    }
    if((COND_US_VIDX_EQ_0) && (COND_TRG_IS_TRUNK_EQ_EPORT))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_EPORT_E,                         descrPtr->trgEPort);
    }
    if(COND_US_VIDX_EQ_1)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVIDX_E,                             descrPtr->eVidx);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_TRG_IS_TRUNK_NOT_EQ_EPORT)) && (COND_US_VIDX_NOT_EQ_1))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_560_560_E,                  dummyTbdParam);
    }
    if(COND_US_VIDX_EQ_0)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_IS_TRUNK_E,                      descrPtr->origIsTrunk);
    }
    if(COND_US_VIDX_EQ_0)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_TRG_PHY_PORT_VALID_E,             dummyTbdParam);
    }
    if((COND_US_VIDX_NOT_EQ_1))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_563_563_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1) && (COND_EXCLUDED_IS_TRUNK_EQ_1))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_TRUNK_ID_E,                 descrPtr->excludedTrunk);
    }
    if((COND_PACKET_CMD_NOT_EQ_FROM_CPU) && (COND_ORIG_IS_TRUNK_EQ_1))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_TRUNK_ID_E,                 dummyTbdParam);
    }
    if(((COND_PACKET_CMD_EQ_FROM_CPU) || (COND_ORIG_IS_TRUNK_NOT_EQ_1)) && ((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_1)) && ((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_0)) && ((COND_PACKET_CMD_EQ_FROM_CPU) || (COND_ORIG_IS_TRUNK_NOT_EQ_0)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_575_564_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1) && (COND_EXCLUDED_IS_TRUNK_EQ_0))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_EPORT_E,                    descrPtr->excludedPort);
    }
    if((COND_PACKET_CMD_NOT_EQ_FROM_CPU) && (COND_ORIG_IS_TRUNK_EQ_0))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_EPORT_E,                    descrPtr->origSrcEPortOrTrnk);
    }
    if(((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_0)) && ((COND_PACKET_CMD_EQ_FROM_CPU) || (COND_ORIG_IS_TRUNK_NOT_EQ_0)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_576_576_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E,     descrPtr->origSrcEPortOrTrnk);
    }
    if((COND_PACKET_CMD_EQ_TO_CPU) || (COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_EPORT_E,                     dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CPU_TO_CPU_MIRROR_E,                 dummyTbdParam);
    }
    if(((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER)) && (COND_PACKET_CMD_NOT_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_589_589_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_IS_TRUNK_E,                 descrPtr->excludeIsTrunk);
    }
    if(((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1)) && ((COND_US_VIDX_NOT_EQ_0) || (COND_IS_TRG_PHY_PORT_VALID_NOT_EQ_TRUE)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_590_590_E,                  dummyTbdParam);
    }
    if((COND_US_VIDX_EQ_0) && (COND_IS_TRG_PHY_PORT_VALID_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_PHY_PORT_E,                      dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1) && (COND_EXCLUDED_IS_TRUNK_EQ_0))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_DEV_IS_LOCAL_E,             dummyTbdParam);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_IS_TRG_PHY_PORT_VALID_NOT_EQ_TRUE)) && ((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_0)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_591_591_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1) && (COND_EXCLUDED_IS_TRUNK_EQ_0))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_IS_PHY_PORT_E,              dummyTbdParam);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_IS_TRG_PHY_PORT_VALID_NOT_EQ_TRUE)) && ((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_0)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_592_592_E,                  dummyTbdParam);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_IS_TRG_PHY_PORT_VALID_NOT_EQ_TRUE)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_599_593_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FROM_CPU_TC_E,                       dummyTbdParam);
    }
    if(COND_PACKET_CMD_NOT_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_PROFILE_E,                       descrPtr->qos.qosProfile);
    }
    if(COND_PACKET_CMD_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FROM_CPU_DP_E,                       dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_FILTER_EN_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_609_606_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_PHY_IS_TRUNK_E,             dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_TO_CPU) || (COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_DEV_E,                       dummyTbdParam);
    }
    if(((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_619_611_E,                  dummyTbdParam);
    }
    if(((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_629_620_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_TO_CPU) || (COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_PHY_PORT_E,                  dummyTbdParam);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_TRG_IS_TRUNK_NOT_EQ_EPORT)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_639_630_E,                  dummyTbdParam);
    }
    if((COND_US_VIDX_EQ_0) && (COND_TRG_IS_TRUNK_EQ_EPORT))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_DEV_E,                           descrPtr->trgDev);
    }
    if(COND_QCN_RX_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_RX_QCN_PRIO_E,                  descrPtr->origRxQcnPrio);
    }
    if(COND_QCN_RX_EQ_FALSE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TTL_E,                               descrPtr->ttl);
    }
    if((COND_QCN_RX_NOT_EQ_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_647_643_E,                  dummyTbdParam);
    }
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_OFFSET_E,                         dummyTbdParam);
    if(COND_TIME_STAMP_EN_EQ_ENABLE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PTP_OFFSET_INDEX_E,              descrPtr->oamInfo.offsetIndex);
    }
    if(COND_TIME_STAMP_EN_EQ_DISABLE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_OFFSET_E,                        descrPtr->ptpOffset);
    }
    if(COND_TUNNEL_TERMINATED_EQ_TT)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_HEADER_OFFSET_E,               dummyTbdParam);
    }
    if((COND_TUNNEL_TERMINATED_NOT_EQ_TT))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_668_662_E,                  dummyTbdParam);
    }
    if(COND_TUNNEL_TERMINATED_EQ_TT)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_L3_OFFSET_E,                   dummyTbdParam);
    }
    if((COND_TUNNEL_TERMINATED_NOT_EQ_TT))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_674_669_E,                  dummyTbdParam);
    }
    if((COND_L2_ECHO_EQ_0) && (descrPtr->isPtp == GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_DOMAIN_E,                        descrPtr->ptpDomain);
    }
    if(((COND_L2_ECHO_NOT_EQ_0) || (descrPtr->isPtp != GT_TRUE)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_677_675_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_SNIFF_E,                          descrPtr->rxSniff);
    }
    if(COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ANALYZER_INDEX_E,                    descrPtr->analyzerIndex);
    }
    if((COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_680_679_E,                  dummyTbdParam);
    }
    if(COND_OAM_PROCESSING_EN_EQ_ENABLE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PROFILE_E,                       descrPtr->oamInfo.oamProfile);
    }
    if((COND_OAM_PROCESSING_EN_NOT_EQ_ENABLE) && ((COND_OAM_PROCESSING_EN_NOT_EQ_DISABLE) || (COND_L2_ECHO_NOT_EQ_0) || (descrPtr->isPtp != GT_TRUE)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_681_681_E,                  dummyTbdParam);
    }
    if((COND_OAM_PROCESSING_EN_EQ_DISABLE) && (COND_L2_ECHO_EQ_0) && (descrPtr->isPtp == GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_TRIGGER_TYPE_E,                  descrPtr->ptpTriggerType);
    }
    if(((COND_OAM_PROCESSING_EN_NOT_EQ_DISABLE) || (COND_L2_ECHO_NOT_EQ_0) || (descrPtr->isPtp != GT_TRUE)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_682_682_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_NOT_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_IS_TRUNK_E,                     descrPtr->origIsTrunk);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_683_683_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_684_684_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_NOT_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ROUTED_E,                            descrPtr->routed);
    }
    if((descrPtr->isIp == GT_TRUE) && (descrPtr->isIPv4 == GT_FALSE) )
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_E,                           dummyTbdParam);
    }
    if(((descrPtr->isIp != GT_TRUE) || (descrPtr->isIPv4 != GT_FALSE)))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_685_685_E,                  dummyTbdParam);
    }
    if(descrPtr->isIp == GT_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_E,                           descrPtr->isIPv4);
    }
    if((descrPtr->isIp != GT_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_686_686_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV4_FOR_HASH_EQ_FALSE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_FOR_HASH_E,                  dummyTbdParam);
    }
    if((COND_IS_IPV4_FOR_HASH_NOT_EQ_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_687_687_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV6_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_HBH_EXT_E,                      dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_688_688_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_NOT_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_MODE_E,                          dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_689_689_E,                  dummyTbdParam);
    }
    if(COND_L2_ECHO_EQ_0)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_PTP_E,                            descrPtr->isPtp);
    }
    if((COND_L2_ECHO_NOT_EQ_0))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_690_690_E,                  dummyTbdParam);
    }
    if(descrPtr->isFcoe == GT_FALSE)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_LEGAL_E,                          dummyTbdParam);
    }
    if((descrPtr->isFcoe != GT_FALSE))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_691_691_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_NOT_EQ_TO_CPU))
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_692_692_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_TO_CPU)
    {
        SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_E,                           descrPtr->srcTrg);
    }
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SR_END_NODE_E,                       dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SR_EH_OFFSET_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CPU_CODE_E,                          descrPtr->cpuCode);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_KEEP_PREVIOUS_HASH_E,                descrPtr->keepPreviousHash);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_IS_CUT_THROUGH_E,             descrPtr->cutThroughModeEnabled);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TABLES_READ_ERROR_E,                 descrPtr->tables_read_error);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_FOR_KEY_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_FOR_KEY_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_PRIO_TAGGED_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_IS_LLC_NON_SNAP_E,             dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TCP_UDP_DEST_PORT_E,                 dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TCP_UDP_SRC_PORT_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TCP_UDP_PORT_IS_0_E,             dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IP_E,                             descrPtr->isIp);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPM_E,                               descrPtr->ipm);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SOURCE_PORT_LIST_FOR_KEY_E,          dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INGRESS_UDB_PACKET_TYPE_E,           dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CHANNEL_TYPE_PROFILE_E,              descrPtr->channelTypeProfile);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E, descrPtr->channelTypeToOpcodeMappingEn);
    SNET_SIP6_10_DESC_SW2HW_FIELD_LE_PTR_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_UDB_VALID_SET_E,             descrPtr->ipclUdbValid);
    SNET_SIP6_10_DESC_SW2HW_FIELD_LE_PTR_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_UDB_SET_E,                   descrPtr->ipclUdbData);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_FLOW_LABEL_FOR_HASH_E,          dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_BYTES_FOR_HASH_E,                 dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL2_FOR_HASH_E,              descrPtr->label3);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL1_FOR_HASH_E,              descrPtr->label2);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL0_FOR_HASH_E,              descrPtr->label1);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_FOR_HASH_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_FOR_HASH_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_FOR_HASH_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_HEADER_INFO_E,                    dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_SUB_TEMPLATE_E,                 dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_REDIRECT_CMD_E,                      descrPtr->ttiRedirectCmd);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_TOS_E,                        dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_TYPE_FOR_KEY_E,               dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_USE_INTERFACE_BASED_INDEX_E,         dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_DISABLE_IPCL0_FOR_ROUTED_PACKETS_E,  dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPCL_PROFILE_INDEX_E,                descrPtr->ipclProfileIndex);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP2_USE_INDEX_FROM_DESC_E,       dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP2_PCL_MODE_E,                  descrPtr->pclLookUpMode[2]);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP1_USE_INDEX_FROM_DESC_E,       dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP1_PCL_MODE_E,                  descrPtr->pclLookUpMode[1]);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP0_USE_INDEX_FROM_DESC_E,       dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP0_PCL_MODE_E,                  descrPtr->pclLookUpMode[0]);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICY_EN_E,                         descrPtr->policyOnPortEn);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVLAN_PRECEDENCE_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_ENCAPSULATION_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_BYTES_E,                          dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_TRACK_EN_E,                     descrPtr->flowTrackEn);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L3_OFFSET_INVALID_E,                 descrPtr->l3NotValid);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_VALID_E,                          descrPtr->l2Valid);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_SRC_TAG_E,                     dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_TO_ME_E,                         descrPtr->mac2me);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_METERING_EN_E,                       dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_FLOW_LABEL_E,                   descrPtr->flowLabel);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_EH_E,                           dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_BC_E,                             dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_MLD_E,                       dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_LINK_LOCAL_E,                dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_MPLS_E,                           descrPtr->mpls);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ND_E,                             dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_HASH_MASK_INDEX_E,                   descrPtr->ttiHashMaskIndex);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_CMD_E,                          descrPtr->mplsCommand);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_SBIT_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_EXP_E,                    dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_LABEL_E,                  descrPtr->origInfoBeforeTunnelTermination.origMplsOuterLabel);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ETHER_TYPE_E,                        descrPtr->origInfoBeforeTunnelTermination.origEtherType);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_ASSIGNED_SST_ID_E,               descrPtr->pclAssignedSstId);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYPASS_INGRESS_PIPE_E,               descrPtr->bypassIngressPipe);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYPASS_BRIDGE_E,                     descrPtr->bypassBridge);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SOLICITATION_MULTICAST_MESSAGE_E,    descrPtr->solicitationMcastMsg);
    SNET_SIP6_10_DESC_SW2HW_FIELD_PTR_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_E,                            descrPtr->macDaPtr);
    SNET_SIP6_10_DESC_SW2HW_FIELD_PTR_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_E,                            descrPtr->macSaPtr);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_QCN_RX_E,                            descrPtr->qcnRx);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_ARP_SA_MISMATCH_E,            dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_HEADER_LENGTH_E,                 descrPtr->ipxHeaderLength);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_LENGTH_E,                        descrPtr->ipxLength);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICER_PTR_E,                       descrPtr->policerPtr);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_BILLING_EN_E,                        dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_PROFILE_PRECEDENCE_E,            descrPtr->qosProfilePrecedence);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_PRIO_TAGGED_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PROCESSING_EN_E,                 descrPtr->oamInfo.oamProcessEnable);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_VALID_E,                          descrPtr->l4Valid);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_PROTOCOL_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SYN_WITH_DATA_E,                     descrPtr->tcpSynWithData);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_SYN_E,                            descrPtr->tcpSyn);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_REPLY_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_PROTECTION_SWITCH_EN_E,           descrPtr->rxEnableProtectionSwitching);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_IS_PROTECTION_PATH_E,             descrPtr->rxIsProtectionPath);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_CMD_E,                        descrPtr->packetCmd);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_TYPE_E,                       descrPtr->macDaType);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_DEV_IS_OWN_E,                    descrPtr->srcDevIsOwn);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_TRUNK_ID_E,            descrPtr->localDevSrcTrunkId);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MODIFY_UP_E,                         descrPtr->modifyUp);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ECN_CAPABLE_E,                       descrPtr->ecnCapable);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_REP_LAST_E,                          dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_REP_E,                               dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_USE_VIDX_E,                          descrPtr->useVidx);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAIL_BOX_TO_NEIGHBOR_CPU_E,          descrPtr->mailBoxToNeighborCPU);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_FILTER_REGISTERED_E,          descrPtr->egressFilterRegistered);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SST_ID_E,                            descrPtr->sstId);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_L3_OFFSET_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_ECHO_E,                           descrPtr->VntL2Echo);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_PACKET_TYPE_E,                 descrPtr->innerPacketType);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_HASH_E,                       descrPtr->pktHash);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_TAGGED_E,                        descrPtr->trgTagged);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_IS_LLC_NON_SNAP_E,             dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_COPY_RESERVED_E,                     descrPtr->copyReserved);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_ID_E,                           descrPtr->flowId);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MODIFY_DSCP_EXP_E,                   descrPtr->modifyDscp);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_DO_ROUTE_HA_E,                       descrPtr->doRouterHa);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_DROP_ON_SOURCE_E,                    descrPtr->dropOnSource);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_IS_LOOPED_E,                  descrPtr->pktIsLooped);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_NESTED_VLAN_EN_E,                    descrPtr->nestedVlanAccessPort);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_TRILL_E,                          dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_TAI_SELECT_E,                    descrPtr->ptpTaiSelect);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_U_FIELD_E,                       descrPtr->ptpUField);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_TAGGED_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_E,                         dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_EN_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_NUM_OF_TAG_BYTES_TO_POP_E,           descrPtr->numOfBytesToPop);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_TPID_INDEX_E,                   descrPtr->tpidIndex[1]);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_TPID_INDEX_E,                   descrPtr->tpidIndex[0]);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_LOCAL_DEV_SRC_TAGGED_E,         descrPtr->tag1LocalDevSrcTagged);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TAG0_IS_OUTER_TAG_E,             dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_SRC_TAGGED_E,                   descrPtr->tagSrcTagged[1]);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_SRC_TAGGED_E,                   descrPtr->tagSrcTagged[0]);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OVERRIDE_EVLAN_WITH_ORIGVID_E,       descrPtr->overrideVid0WithOrigVid);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_VID_E,                          descrPtr->originalVid1);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_UP1_E,                               descrPtr->up1);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CFI1_E,                              descrPtr->cfidei1);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_VID1_E,                              descrPtr->vid1);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_UP0_E,                               descrPtr->up);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CFI0_E,                              descrPtr->cfidei);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVLAN_E,                             descrPtr->eVid);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_DEV_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PHY_SRC_MC_FILTER_EN_E,              dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_EPORT_E,               dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_PORT_E,                descrPtr->localDevSrcPort);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_UDB_PACKET_TYPE_E,            dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_START_E,                      descrPtr->tunnelStart);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_TERMINATED_E,                 descrPtr->tunnelTerminated);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MARVELL_TAGGED_EXTENDED_E,           descrPtr->marvellTaggedExtended);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MARVELL_TAGGED_E,                    descrPtr->marvellTagged);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RECALC_CRC_E,                        descrPtr->rxRecalcCrc);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYTE_COUNT_E,                        descrPtr->byteCount);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MDB_E,                               dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_ECN_E,                          dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SKIP_FDB_SA_LOOKUP_E,                descrPtr->skipFdbSaLookup);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FIRST_BUFFER_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_SW2HW_FIELD_SET( hwDescBusPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPFIX_EN_E,                          dummyTbdParam);
}

/* convert software descriptor to software descriptor */
static GT_VOID snetHawkPpuDauDescHwToSwConvert
(
    IN    SKERNEL_DEVICE_OBJECT               *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN    GT_U8                               *hwDescBusPtr,
    IN    GT_U8                               *hwDescMaskPtr
)
{
    GT_U32 dummyTbdParam = 0;

    /* set but unused param */
    (void) dummyTbdParam;

    if((descrPtr->isPtp == GT_TRUE) && (descrPtr->isPtp == GT_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_START_BANK_E,                        dummyTbdParam);
    }
    if((descrPtr->isPtp == GT_TRUE) && (descrPtr->isPtp == GT_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_START_GROUP_E,                       dummyTbdParam);
    }
    if((descrPtr->isPtp == GT_TRUE) && (descrPtr->isPtp == GT_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INGRESS_CORE_ID_E,                   descrPtr->srcCoreId);
    }
    if((descrPtr->isPtp == GT_TRUE) && (descrPtr->isPtp == GT_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_DESC_CRITICAL_ECC_E,                 dummyTbdParam);
    }
    if(descrPtr->isFcoe == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_D_ID_E,                         dummyTbdParam);
    }
    if((descrPtr->isFcoe != GT_TRUE) && (descrPtr->isIPv4 != GT_TRUE) && (descrPtr->arp != GT_TRUE) && (COND_IS_IPV6_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_23_0_E,                     dummyTbdParam);
    }
    if(descrPtr->arp == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_DIP_E,                           descrPtr->dip[0]);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_SIP_E,                          descrPtr->sip[0]);
    }
    if(COND_IS_IPV6_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_IPV6_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_DIP_E,                  descrPtr->dip);
    }
    if((descrPtr->isIPv4 != GT_TRUE) && (descrPtr->arp != GT_TRUE) && (COND_IS_IPV6_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_31_24_E,                    dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DIP_E,                          descrPtr->dip[0]);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_63_32_E,                    dummyTbdParam);
    }
    if(descrPtr->arp == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_SIP_E,                           descrPtr->sip[0]);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->arp != GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_95_64_E,                    dummyTbdParam);
    }
    if(descrPtr->isFcoe == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_S_ID_E,                         dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isFcoe != GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_119_96_E,                   dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_FRAGMENTED_E,                     dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_121_120_E,                  dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_OPTION_FIELD_E,                 dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_122_122_E,                  dummyTbdParam);
    }
    if((descrPtr->isIp == GT_FALSE) && (descrPtr->isFcoe == GT_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_E,                            descrPtr->arp);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && ((descrPtr->isIp != GT_FALSE) || (descrPtr->isFcoe != GT_FALSE)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_123_123_E,                  dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DF_E,                           descrPtr->ipv4DontFragmentBit);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_124_124_E,                  dummyTbdParam);
    }
    if(descrPtr->isFcoe == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FCOE_LEGAL_E,                        descrPtr->fcoeInfo.fcoeLegal);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isFcoe != GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_125_125_E,                  dummyTbdParam);
    }
    if(descrPtr->isIPv4 == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FRAGMENTED_E,                        dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIPv4 != GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_126_126_E,                  dummyTbdParam);
    }
    if(descrPtr->isIp == GT_FALSE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_FCOE_E,                           descrPtr->isFcoe);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE) && (descrPtr->isIp != GT_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_127_127_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV6_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_IPV6_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_SIP_E,                          descrPtr->sip);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_255_128_E,                  dummyTbdParam);
    }
    if(COND_IS_ARP_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_DIP_FOR_HASH_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV4_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_SIP_FOR_HASH_E,                 dummyTbdParam);
    }
    if((COND_IS_IPV4_FOR_HASH_NOT_EQ_TRUE) && (COND_IS_ARP_FOR_HASH_NOT_EQ_TRUE) && (COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_287_256_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV6_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_DIP_FOR_HASH_E,                 dummyTbdParam);
    }
    if(COND_IS_IPV4_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV4_DIP_FOR_HASH_E,                 dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE) && (COND_IS_IPV4_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_319_288_E,                  dummyTbdParam);
    }
    if(COND_IS_ARP_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_SIP_FOR_HASH_E,                  dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE) && (COND_IS_ARP_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_351_320_E,                  dummyTbdParam);
    }
    if((COND_IS_IPV4_FOR_HASH_EQ_FALSE) && (COND_IS_IPV6_FOR_HASH_EQ_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_FOR_HASH_E,                   dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE) && ((COND_IS_IPV4_FOR_HASH_NOT_EQ_FALSE) || (COND_IS_IPV6_FOR_HASH_NOT_EQ_FALSE)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_352_352_E,                  dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_383_353_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV6_FOR_HASH_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_SIP_FOR_HASH_E,                 dummyTbdParam);
    }
    if((COND_IS_IPV6_FOR_HASH_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_511_384_E,                  dummyTbdParam);
    }
    if(COND_TUNNEL_START_EQ_TS)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_PTR_E,                        descrPtr->tunnelPtr);
    }
    if(COND_TUNNEL_START_EQ_LL)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ARP_PTR_E,                           descrPtr->arpPtr);
    }
    if(COND_TUNNEL_START_EQ_TS)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_START_PASSENGER_TYPE_E,       descrPtr->tunnelStartPassengerType);
    }
    if((COND_TUNNEL_START_NOT_EQ_LL))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_529_529_E,                  dummyTbdParam);
    }
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_VIRTUAL_ID_E,                        dummyTbdParam);
    if(COND_REDIRECT_CMD_EQ_2)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICY_LTT_INDEX_E,                  descrPtr->ttRouterLTT);
    }
    if((COND_REDIRECT_CMD_NOT_EQ_2))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_547_542_E,                  dummyTbdParam);
    }
    if((COND_US_VIDX_EQ_0) && (COND_TRG_IS_TRUNK_EQ_TRUNK_ID))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_TRUNK_ID_E,                      descrPtr->trgTrunkId);
    }
    if((COND_US_VIDX_EQ_0) && (COND_TRG_IS_TRUNK_EQ_EPORT))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_EPORT_E,                         descrPtr->trgEPort);
    }
    if(COND_US_VIDX_EQ_1)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVIDX_E,                             descrPtr->eVidx);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_TRG_IS_TRUNK_NOT_EQ_EPORT)) && (COND_US_VIDX_NOT_EQ_1))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_560_560_E,                  dummyTbdParam);
    }
    if(COND_US_VIDX_EQ_0)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_IS_TRUNK_E,                      descrPtr->origIsTrunk);
    }
    if(COND_US_VIDX_EQ_0)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_TRG_PHY_PORT_VALID_E,             dummyTbdParam);
    }
    if((COND_US_VIDX_NOT_EQ_1))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_563_563_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1) && (COND_EXCLUDED_IS_TRUNK_EQ_1))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_TRUNK_ID_E,                 descrPtr->excludedTrunk);
    }
    if((COND_PACKET_CMD_NOT_EQ_FROM_CPU) && (COND_ORIG_IS_TRUNK_EQ_1))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_TRUNK_ID_E,                 dummyTbdParam);
    }
    if(((COND_PACKET_CMD_EQ_FROM_CPU) || (COND_ORIG_IS_TRUNK_NOT_EQ_1)) && ((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_1)) && ((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_0)) && ((COND_PACKET_CMD_EQ_FROM_CPU) || (COND_ORIG_IS_TRUNK_NOT_EQ_0)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_575_564_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1) && (COND_EXCLUDED_IS_TRUNK_EQ_0))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_EPORT_E,                    descrPtr->excludedPort);
    }
    if((COND_PACKET_CMD_NOT_EQ_FROM_CPU) && (COND_ORIG_IS_TRUNK_EQ_0))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_EPORT_E,                    descrPtr->origSrcEPortOrTrnk);
    }
    if(((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_0)) && ((COND_PACKET_CMD_EQ_FROM_CPU) || (COND_ORIG_IS_TRUNK_NOT_EQ_0)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_576_576_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E,     descrPtr->origSrcEPortOrTrnk);
    }
    if((COND_PACKET_CMD_EQ_TO_CPU) || (COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_EPORT_E,                     dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CPU_TO_CPU_MIRROR_E,                 dummyTbdParam);
    }
    if(((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER)) && (COND_PACKET_CMD_NOT_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_589_589_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_IS_TRUNK_E,                 descrPtr->excludeIsTrunk);
    }
    if(((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1)) && ((COND_US_VIDX_NOT_EQ_0) || (COND_IS_TRG_PHY_PORT_VALID_NOT_EQ_TRUE)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_590_590_E,                  dummyTbdParam);
    }
    if((COND_US_VIDX_EQ_0) && (COND_IS_TRG_PHY_PORT_VALID_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_PHY_PORT_E,                      dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1) && (COND_EXCLUDED_IS_TRUNK_EQ_0))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_DEV_IS_LOCAL_E,             dummyTbdParam);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_IS_TRG_PHY_PORT_VALID_NOT_EQ_TRUE)) && ((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_0)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_591_591_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU) && (COND_US_VIDX_EQ_1) && (COND_EXCLUDED_IS_TRUNK_EQ_0))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EXCLUDED_IS_PHY_PORT_E,              dummyTbdParam);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_IS_TRG_PHY_PORT_VALID_NOT_EQ_TRUE)) && ((COND_PACKET_CMD_NOT_EQ_FROM_CPU) || (COND_US_VIDX_NOT_EQ_1) || (COND_EXCLUDED_IS_TRUNK_NOT_EQ_0)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_592_592_E,                  dummyTbdParam);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_IS_TRG_PHY_PORT_VALID_NOT_EQ_TRUE)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_599_593_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FROM_CPU_TC_E,                       dummyTbdParam);
    }
    if(COND_PACKET_CMD_NOT_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_PROFILE_E,                       descrPtr->qos.qosProfile);
    }
    if(COND_PACKET_CMD_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FROM_CPU_DP_E,                       dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_FILTER_EN_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_609_606_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_PHY_IS_TRUNK_E,             dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_TO_CPU) || (COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_DEV_E,                       dummyTbdParam);
    }
    if(((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_619_611_E,                  dummyTbdParam);
    }
    if(((COND_PACKET_CMD_NOT_EQ_TO_CPU) && (COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_629_620_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_TO_CPU) || (COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_PHY_PORT_E,                  dummyTbdParam);
    }
    if(((COND_US_VIDX_NOT_EQ_0) || (COND_TRG_IS_TRUNK_NOT_EQ_EPORT)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_639_630_E,                  dummyTbdParam);
    }
    if((COND_US_VIDX_EQ_0) && (COND_TRG_IS_TRUNK_EQ_EPORT))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_DEV_E,                           descrPtr->trgDev);
    }
    if(COND_QCN_RX_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_RX_QCN_PRIO_E,                  descrPtr->origRxQcnPrio);
    }
    if(COND_QCN_RX_EQ_FALSE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TTL_E,                               descrPtr->ttl);
    }
    if((COND_QCN_RX_NOT_EQ_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_647_643_E,                  dummyTbdParam);
    }
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_OFFSET_E,                         dummyTbdParam);
    if(COND_TIME_STAMP_EN_EQ_ENABLE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PTP_OFFSET_INDEX_E,              descrPtr->oamInfo.offsetIndex);
    }
    if(COND_TIME_STAMP_EN_EQ_DISABLE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_OFFSET_E,                        descrPtr->ptpOffset);
    }
    if(COND_TUNNEL_TERMINATED_EQ_TT)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_HEADER_OFFSET_E,               dummyTbdParam);
    }
    if((COND_TUNNEL_TERMINATED_NOT_EQ_TT))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_668_662_E,                  dummyTbdParam);
    }
    if(COND_TUNNEL_TERMINATED_EQ_TT)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_L3_OFFSET_E,                   dummyTbdParam);
    }
    if((COND_TUNNEL_TERMINATED_NOT_EQ_TT))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_674_669_E,                  dummyTbdParam);
    }
    if((COND_L2_ECHO_EQ_0) && (descrPtr->isPtp == GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_DOMAIN_E,                        descrPtr->ptpDomain);
    }
    if(((COND_L2_ECHO_NOT_EQ_0) || (descrPtr->isPtp != GT_TRUE)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_677_675_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_SNIFF_E,                          descrPtr->rxSniff);
    }
    if(COND_PACKET_CMD_NOT_EQ_TO_TARGET_SNIFFER)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ANALYZER_INDEX_E,                    descrPtr->analyzerIndex);
    }
    if((COND_PACKET_CMD_EQ_TO_TARGET_SNIFFER))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_680_679_E,                  dummyTbdParam);
    }
    if(COND_OAM_PROCESSING_EN_EQ_ENABLE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PROFILE_E,                       descrPtr->oamInfo.oamProfile);
    }
    if((COND_OAM_PROCESSING_EN_NOT_EQ_ENABLE) && ((COND_OAM_PROCESSING_EN_NOT_EQ_DISABLE) || (COND_L2_ECHO_NOT_EQ_0) || (descrPtr->isPtp != GT_TRUE)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_681_681_E,                  dummyTbdParam);
    }
    if((COND_OAM_PROCESSING_EN_EQ_DISABLE) && (COND_L2_ECHO_EQ_0) && (descrPtr->isPtp == GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_TRIGGER_TYPE_E,                  dummyTbdParam);
    }
    if(((COND_OAM_PROCESSING_EN_NOT_EQ_DISABLE) || (COND_L2_ECHO_NOT_EQ_0) || (descrPtr->isPtp != GT_TRUE)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_682_682_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_NOT_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_IS_TRUNK_E,                     descrPtr->origIsTrunk);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_683_683_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_684_684_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_NOT_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ROUTED_E,                            descrPtr->routed);
    }
    if((descrPtr->isIp == GT_TRUE) && (descrPtr->isIPv4 == GT_FALSE) )
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_E,                           dummyTbdParam);
    }
    if(((descrPtr->isIp != GT_TRUE) || (descrPtr->isIPv4 != GT_FALSE)))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_685_685_E,                  dummyTbdParam);
    }
    if(descrPtr->isIp == GT_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_E,                           descrPtr->isIPv4);
    }
    if((descrPtr->isIp != GT_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_686_686_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV4_FOR_HASH_EQ_FALSE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_FOR_HASH_E,                  dummyTbdParam);
    }
    if((COND_IS_IPV4_FOR_HASH_NOT_EQ_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_687_687_E,                  dummyTbdParam);
    }
    if(COND_IS_IPV6_EQ_TRUE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_HBH_EXT_E,                      dummyTbdParam);
    }
    if((COND_IS_IPV6_NOT_EQ_TRUE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_688_688_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_NOT_EQ_FROM_CPU)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_MODE_E,                          dummyTbdParam);
    }
    if((COND_PACKET_CMD_EQ_FROM_CPU))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_689_689_E,                  dummyTbdParam);
    }
    if(COND_L2_ECHO_EQ_0)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_PTP_E,                            descrPtr->isPtp);
    }
    if((COND_L2_ECHO_NOT_EQ_0))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_690_690_E,                  dummyTbdParam);
    }
    if(descrPtr->isFcoe == GT_FALSE)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_LEGAL_E,                          dummyTbdParam);
    }
    if((descrPtr->isFcoe != GT_FALSE))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_691_691_E,                  dummyTbdParam);
    }
    if((COND_PACKET_CMD_NOT_EQ_TO_CPU))
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RESERVED_692_692_E,                  dummyTbdParam);
    }
    if(COND_PACKET_CMD_EQ_TO_CPU)
    {
        SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TRG_E,                           descrPtr->srcTrg);
    }
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SR_END_NODE_E,                       dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SR_EH_OFFSET_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CPU_CODE_E,                          descrPtr->cpuCode);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_KEEP_PREVIOUS_HASH_E,                descrPtr->keepPreviousHash);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_IS_CUT_THROUGH_E,             descrPtr->cutThroughModeEnabled);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TABLES_READ_ERROR_E,                 descrPtr->tables_read_error);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_FOR_KEY_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_FOR_KEY_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_PRIO_TAGGED_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_IS_LLC_NON_SNAP_E,             dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TCP_UDP_DEST_PORT_E,                 dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TCP_UDP_SRC_PORT_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TCP_UDP_PORT_IS_0_E,             dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IP_E,                             descrPtr->isIp);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPM_E,                               descrPtr->ipm);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SOURCE_PORT_LIST_FOR_KEY_E,          dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INGRESS_UDB_PACKET_TYPE_E,           dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CHANNEL_TYPE_PROFILE_E,              descrPtr->channelTypeProfile);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E, descrPtr->channelTypeToOpcodeMappingEn);
    SNET_SIP6_10_DESC_HW2SW_FIELD_LE_PTR_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_UDB_VALID_SET_E,             descrPtr->ipclUdbValid);
    SNET_SIP6_10_DESC_HW2SW_FIELD_LE_PTR_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_UDB_SET_E,                   descrPtr->ipclUdbData);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_FLOW_LABEL_FOR_HASH_E,          dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_BYTES_FOR_HASH_E,                 dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL2_FOR_HASH_E,              descrPtr->label3);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL1_FOR_HASH_E,              descrPtr->label2);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_LABEL0_FOR_HASH_E,              descrPtr->label1);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_FOR_HASH_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_FOR_HASH_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV4_FOR_HASH_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IP_HEADER_INFO_E,                    dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_SUB_TEMPLATE_E,                 dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_REDIRECT_CMD_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_TOS_E,                        dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_TYPE_FOR_KEY_E,               dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_USE_INTERFACE_BASED_INDEX_E,         dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_DISABLE_IPCL0_FOR_ROUTED_PACKETS_E,  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPCL_PROFILE_INDEX_E,                descrPtr->ipclProfileIndex);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP2_USE_INDEX_FROM_DESC_E,       dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP2_PCL_MODE_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP1_USE_INDEX_FROM_DESC_E,       dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP1_PCL_MODE_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP0_USE_INDEX_FROM_DESC_E,       dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOOKUP0_PCL_MODE_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICY_EN_E,                         descrPtr->policyOnPortEn);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVLAN_PRECEDENCE_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_ENCAPSULATION_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_BYTES_E,                          dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_TRACK_EN_E,                     descrPtr->flowTrackEn);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L3_OFFSET_INVALID_E,                 descrPtr->l3NotValid);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_VALID_E,                          descrPtr->l2Valid);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_SRC_TAG_E,                     dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_TO_ME_E,                         descrPtr->mac2me);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_METERING_EN_E,                       dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_FLOW_LABEL_E,                   descrPtr->flowLabel);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPV6_EH_E,                           dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_BC_E,                             dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_MLD_E,                       dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_IPV6_LINK_LOCAL_E,                dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_MPLS_E,                           descrPtr->mpls);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ND_E,                             dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_HASH_MASK_INDEX_E,                   descrPtr->ttiHashMaskIndex);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_CMD_E,                          dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_SBIT_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_EXP_E,                    dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MPLS_OUTER_LABEL_E,                  descrPtr->origInfoBeforeTunnelTermination.origMplsOuterLabel);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ETHER_TYPE_E,                        dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PCL_ASSIGNED_SST_ID_E,               descrPtr->pclAssignedSstId);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYPASS_INGRESS_PIPE_E,               descrPtr->bypassIngressPipe);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYPASS_BRIDGE_E,                     descrPtr->bypassBridge);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SOLICITATION_MULTICAST_MESSAGE_E,    descrPtr->solicitationMcastMsg);
    SNET_SIP6_10_DESC_HW2SW_FIELD_PTR_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_E,                            descrPtr->macDaPtr);
    SNET_SIP6_10_DESC_HW2SW_FIELD_PTR_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_E,                            descrPtr->macSaPtr);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_QCN_RX_E,                            descrPtr->qcnRx);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_SA_ARP_SA_MISMATCH_E,            dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_HEADER_LENGTH_E,                 descrPtr->ipxHeaderLength);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_LENGTH_E,                        descrPtr->ipxLength);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_POLICER_PTR_E,                       descrPtr->policerPtr);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_BILLING_EN_E,                        dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_QOS_PROFILE_PRECEDENCE_E,            dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_PRIO_TAGGED_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OAM_PROCESSING_EN_E,                 descrPtr->oamInfo.oamProcessEnable);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L4_VALID_E,                          descrPtr->l4Valid);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPX_PROTOCOL_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SYN_WITH_DATA_E,                     descrPtr->tcpSynWithData);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_SYN_E,                            descrPtr->tcpSyn);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_ARP_REPLY_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_PROTECTION_SWITCH_EN_E,           descrPtr->rxEnableProtectionSwitching);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RX_IS_PROTECTION_PATH_E,             descrPtr->rxIsProtectionPath);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_CMD_E,                        dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAC_DA_TYPE_E,                       dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_DEV_IS_OWN_E,                    descrPtr->srcDevIsOwn);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_TRUNK_ID_E,            descrPtr->localDevSrcTrunkId);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MODIFY_UP_E,                         descrPtr->modifyUp);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ECN_CAPABLE_E,                       descrPtr->ecnCapable);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_REP_LAST_E,                          dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_REP_E,                               dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_USE_VIDX_E,                          descrPtr->useVidx);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MAIL_BOX_TO_NEIGHBOR_CPU_E,          descrPtr->mailBoxToNeighborCPU);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_FILTER_REGISTERED_E,          descrPtr->egressFilterRegistered);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SST_ID_E,                            descrPtr->sstId);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_L3_OFFSET_E,                   dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_L2_ECHO_E,                           descrPtr->VntL2Echo);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_INNER_PACKET_TYPE_E,                 dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_HASH_E,                       descrPtr->pktHash);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TRG_TAGGED_E,                        descrPtr->trgTagged);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OUTER_IS_LLC_NON_SNAP_E,             dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_COPY_RESERVED_E,                     descrPtr->copyReserved);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FLOW_ID_E,                           descrPtr->flowId);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MODIFY_DSCP_EXP_E,                   descrPtr->modifyDscp);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_DO_ROUTE_HA_E,                       descrPtr->doRouterHa);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_DROP_ON_SOURCE_E,                    descrPtr->dropOnSource);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PACKET_IS_LOOPED_E,                  descrPtr->pktIsLooped);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_NESTED_VLAN_EN_E,                    descrPtr->nestedVlanAccessPort);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IS_TRILL_E,                          dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_TAI_SELECT_E,                    descrPtr->ptpTaiSelect);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PTP_U_FIELD_E,                       descrPtr->ptpUField);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_TAGGED_E,                  dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_E,                         dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TIMESTAMP_EN_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_NUM_OF_TAG_BYTES_TO_POP_E,           descrPtr->numOfBytesToPop);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_TPID_INDEX_E,                   descrPtr->tpidIndex[1]);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_TPID_INDEX_E,                   descrPtr->tpidIndex[0]);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_LOCAL_DEV_SRC_TAGGED_E,         descrPtr->tag1LocalDevSrcTagged);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SRC_TAG0_IS_OUTER_TAG_E,             dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG1_SRC_TAGGED_E,                   descrPtr->tagSrcTagged[1]);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TAG0_SRC_TAGGED_E,                   descrPtr->tagSrcTagged[0]);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_OVERRIDE_EVLAN_WITH_ORIGVID_E,       descrPtr->overrideVid0WithOrigVid);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_VID_E,                          descrPtr->originalVid1);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_UP1_E,                               descrPtr->up1);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CFI1_E,                              descrPtr->cfidei1);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_VID1_E,                              descrPtr->vid1);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_UP0_E,                               descrPtr->up);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_CFI0_E,                              descrPtr->cfidei);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EVLAN_E,                             descrPtr->eVid);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_SRC_DEV_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_PHY_SRC_MC_FILTER_EN_E,              dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_EPORT_E,               dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_LOCAL_DEV_SRC_PORT_E,                descrPtr->localDevSrcPort);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_EGRESS_UDB_PACKET_TYPE_E,            dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_START_E,                      descrPtr->tunnelStart);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_TUNNEL_TERMINATED_E,                 descrPtr->tunnelTerminated);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MARVELL_TAGGED_EXTENDED_E,           dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MARVELL_TAGGED_E,                    descrPtr->marvellTagged);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_RECALC_CRC_E,                        dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_BYTE_COUNT_E,                        descrPtr->byteCount);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_MDB_E,                               dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_ORIG_ECN_E,                          dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_SKIP_FDB_SA_LOOKUP_E,                descrPtr->skipFdbSaLookup);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_FIRST_BUFFER_E,                      dummyTbdParam);
    SNET_SIP6_10_DESC_HW2SW_FIELD_SET( hwDescBusPtr, hwDescMaskPtr, SNET_SIP6_10_PPU_DAU_DESC_FIELD_IPFIX_EN_E,                          dummyTbdParam);
}

/* Set interrupt */
static GT_VOID snetHawkPpuInterruptSet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                      intOffset
)
{
    GT_U32 interruptMaskField;

    /* Get interrupt mask for input offset*/
    smemRegFldGet(devObjPtr, SIP_6_10_REG(devObjPtr, PPU.generalRegs.ppuInterruptMask),
            intOffset, 1, &interruptMaskField);

    if(interruptMaskField)
    {
        /* Set an interrupt */
        smemRegFldSet(devObjPtr, SIP_6_10_REG(devObjPtr, PPU.generalRegs.ppuInterruptCause),
                intOffset, 1, 1);

        /* Set sum of all interrupts to 1 */
        smemRegFldSet(devObjPtr, SIP_6_10_REG(devObjPtr, PPU.generalRegs.ppuInterruptCause),
                0, 1, 1);
    }
}

/* read counter original value and increment it */
static GT_VOID snetHawkPpuDebugCounterIncrement
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                      address
)
{
    GT_U32 regVal;
    GT_U32 debugCounterEnable;

    /* check whether debug counter enable is set */
    smemRegFldGet(devObjPtr, SMEM_SIP6_10_PPU_GLOBAL_CONFIG_REG(devObjPtr), 10, 1, &debugCounterEnable);

    if(debugCounterEnable)
    {
        /* read original counter value */
        smemRegGet(devObjPtr, address, &regVal);

        regVal ++;

        /* set register value */
        smemRegSet(devObjPtr, address, regVal);
    }
}

/* Get PPU Global configuration */
static GT_VOID snetHawkPpuGlobalConfigGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    OUT SNET_SIP6_10_PPU_CONFIG_STC *ppuGlobalCfgPtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuGlobalConfigGet);
    GT_U32 regValue;

    /* read ppu global config register data */
    smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_GLOBAL_CONFIG_REG(devObjPtr), &regValue);

    /* populate global config structure */
    ppuGlobalCfgPtr->ppuEnable          = (GT_BOOL) (regValue & 0x1);
    ppuGlobalCfgPtr->maxLoopBack        = (regValue >> 1) & 0x7;
    ppuGlobalCfgPtr->errProfMaxLoopBack = (regValue >> 4) & 0x3;
    ppuGlobalCfgPtr->errProfSer         = (regValue >> 6) & 0x3;
    ppuGlobalCfgPtr->errProfOffsetOor   = (regValue >> 8) & 0x3;
    ppuGlobalCfgPtr->debugCounterEnable = (GT_BOOL) ((regValue >> 10) & 0x1);

    if(simLogIsOpenFlag)
    {
        __LOG_PARAM(ppuGlobalCfgPtr->ppuEnable         );
        __LOG_PARAM(ppuGlobalCfgPtr->maxLoopBack       );
        __LOG_PARAM(ppuGlobalCfgPtr->errProfMaxLoopBack);
        __LOG_PARAM(ppuGlobalCfgPtr->errProfSer        );
        __LOG_PARAM(ppuGlobalCfgPtr->errProfOffsetOor  );
        __LOG_PARAM(ppuGlobalCfgPtr->debugCounterEnable);
    }
}

/* Get PPU profile */
static GT_VOID snetHawkPpuProfileGet
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  GT_U32                           ppuProfileIdx,
    OUT SNET_SIP6_10_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuGlobalConfigGet);

    GT_U32 regValue;

    /* read ppu profile register data*/
    smemRegGet(devObjPtr, SMEM_SIP6_10_TTI_PPU_PROFILE_REG(devObjPtr, ppuProfileIdx), &regValue);

    /* populate ppu profile data structure */
    ppuProfilePtr->ppuEnable           = (GT_BOOL) (regValue & 0x1);
    ppuProfilePtr->ppuState            = (regValue >> 1 ) & 0xFF;
    ppuProfilePtr->anchorType          = (regValue >> 9 ) & 0x7 ;
    ppuProfilePtr->offset              = (regValue >> 12) & 0x7F;
    ppuProfilePtr->spBusDefaultProfile = (regValue >> 19) & 0x3 ;

    if(simLogIsOpenFlag)
    {
        __LOG_PARAM(ppuProfilePtr->ppuEnable          );
        __LOG_PARAM(ppuProfilePtr->ppuState           );
        __LOG_PARAM(ppuProfilePtr->anchorType         );
        __LOG_PARAM(ppuProfilePtr->offset             );
        __LOG_PARAM(ppuProfilePtr->spBusDefaultProfile);
    }
}

/* copy U32 data to U8 array */
static GT_VOID copyU32toU8array
(
    IN  GT_U32 regValue,
    IN  GT_U32 numBytes,
    OUT GT_U8  *data
)
{
    GT_U32 ii;

    if(numBytes > 4)
    {
        skernelFatalError("numBytes %d is out of range. Max=4\n", numBytes);
    }

    for(ii = 0; ii < numBytes; ii++)
    {
        data[ii] = (GT_U8) ((regValue >>  ii*8) & 0xFF);
    }
}


/* Get PPU SP_BUS profile */
static GT_VOID snetHawkPpuSpBusProfileGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                       profileIdx,
    OUT SNET_SIP6_10_PPU_SP_BUS_STC    *spBusProfilePtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuSpBusProfileGet);
    GT_U32 regValue;
    GT_U32 ii;

    __LOG(("SP_BUS Profile Words[0 1 2 3 4] = ["));
    for(ii = 0; ii < SNET_SIP6_10_PPU_SP_BUS_WORDS_MAX_CNS; ii++)
    {
        /* read SP_BUS profile data */
        smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_SP_BUS_DEFAULT_PROFILE_WORD_REG(devObjPtr, profileIdx, ii), &regValue);

        __LOG(("0x%08X ", regValue));
        /* populate SP_BUS profile structure */
        copyU32toU8array(regValue, sizeof(regValue), &spBusProfilePtr->data[ii*4]);
    }
    __LOG(("]\n"));
}

/* Get UDB Metadata */
static GT_VOID snetHawkPpuUdbMetadataGet
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN  SNET_SIP6_10_PPU_ERROR_INFO_STC   *errInfoPtr,
    OUT SNET_SIP6_10_PPU_UDB_METADATA_STC *udbMetadataPtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuUdbMetadataGet);

    GT_STATUS rc;
    GT_U32    tableEntryAddress = 0;
    GT_U32    *memPtr;
    GT_U32    ii;
    GT_U32    regValue, anchorType, offset;

    ASSERT_PTR(udbMetadataPtr);

    /* calclulate table entry address */
    tableEntryAddress = SMEM_XCAT_POLICY_UDB_CONF_MEM_REG(devObjPtr, descrPtr->tti_pcktType_sip5);

    memPtr = smemMemGet(devObjPtr,tableEntryAddress);

    udbMetadataPtr->udbValidMask = 0;
    __LOG(("UDB Metadata: [ "));
    for(ii = 0; ii < SNET_SIP6_10_PPU_UDBS_MAX_CNS; ii++)
    {
        /* get valid bit */
        regValue = 0;
        regValue = snetFieldValueGet(memPtr, (SNET_SIP6_10_PPU_UDB_START_CNS + ii) * 12 + 0, 1);
        udbMetadataPtr->udbValidMask |= (regValue & 0x1) << ii;

        if(regValue)
        {
            /* check whether the anchor type is metadata */
            anchorType = snetFieldValueGet(memPtr, (SNET_SIP6_10_PPU_UDB_START_CNS + ii) * 12 + 1, 3);
            if((anchorType & 0x7) != SNET_SIP6_10_PPU_UDB_METADATA_ANCHOR_TYPE_CNS)
            {
                __LOG(("UDB anchor type 0x%X is not Metadata\n", anchorType));

                /* set Oor Error */
                snetHawkPpuInterruptSet(devObjPtr, 8);
                errInfoPtr->isOorErr = GT_TRUE;
                break;
            }

            /* get anchor offset */
            offset = snetFieldValueGet(memPtr, (SNET_SIP6_10_PPU_UDB_START_CNS + ii) * 12 + 4, 8);

            /* extract the needed 8 bits from the 'metadata' fields */
            /* udbMetadataPtr->data[ii] = regValue; */
            rc = snetXCatPclUserDefinedByteGet(devObjPtr, descrPtr, anchorType, offset,
                                               SNET_UDB_CLIENT_IPCL_E, &udbMetadataPtr->data[ii]);
            if(rc != GT_OK)
            {
                __LOG(("UDB Get failed. Anchor=%d, offset=%d\n", anchorType, offset));
                /* set Oor Error */
                snetHawkPpuInterruptSet(devObjPtr, 8);
                errInfoPtr->isOorErr = GT_TRUE;
            }
            __LOG(("0x%02X ", udbMetadataPtr->data[ii]));
        }
    }
    __LOG(("] Valid Mask: 0x%X\n", udbMetadataPtr->udbValidMask))
}

/* Get header shift offset */
static GT_VOID snetHawkPpuHeaderShiftGet
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN  SNET_SIP6_10_PPU_PROFILE_ENTRY_STC  *ppuProfilePtr,
    IN  SNET_SIP6_10_PPU_ERROR_INFO_STC     *errInfoPtr,
    OUT GT_U32                              *kstgHdShift
)
{
    DECLARE_FUNC_NAME(snetHawkPpuHeaderShiftGet);

    GT_U32 hdShift = 0;
    GT_U8  *startFramePtr = descrPtr->frameBuf->actualDataPtr;
    if(startFramePtr == NULL)
    {
        __LOG(("Invalid start frame Ptr: NULL\n"));

        /* set Oor Error */
        snetHawkPpuInterruptSet(devObjPtr, 8);
        errInfoPtr->isOorErr = GT_TRUE;
        return;
    }

    switch(ppuProfilePtr->anchorType)
    {
        case SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L2_E:
            if(descrPtr->startFramePtr < startFramePtr)
            {
                __LOG(("Invalid L2 start offset pointer : 0x%X\n", descrPtr->startFramePtr));

                /* set Oor Error */
                snetHawkPpuInterruptSet(devObjPtr, 8);
                errInfoPtr->isOorErr = GT_TRUE;
                break;
            }
            hdShift = descrPtr->startFramePtr - startFramePtr;
            break;
        case SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L3_E:
            if(descrPtr->l3StartOffsetPtr < startFramePtr)
            {
                __LOG(("Invalid L3 start offset pointer : 0x%X\n", descrPtr->l3StartOffsetPtr));

                /* set Oor Error */
                snetHawkPpuInterruptSet(devObjPtr, 8);
                errInfoPtr->isOorErr = GT_TRUE;
                break;
            }
            hdShift = descrPtr->l3StartOffsetPtr - startFramePtr - 2;
            break;
        case SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L4_E:
            if(descrPtr->l23HeaderSize == 0)
            {
                __LOG(("Invalid L2 + L3 header size: 0x%X\n", descrPtr->l23HeaderSize));

                /* set Oor Error */
                snetHawkPpuInterruptSet(devObjPtr, 8);
                errInfoPtr->isOorErr = GT_TRUE;
                break;
            }
            hdShift = descrPtr->l23HeaderSize;
            break;
        case SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L2_E:
            if(descrPtr->tunnelTerminated == GT_FALSE || descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr < startFramePtr)
            {
                __LOG(("Invalid Original L2 start offset pointer before tunnel termination : 0x%X\n",
                                    descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr));

                /* set Oor Error */
                snetHawkPpuInterruptSet(devObjPtr, 8);
                errInfoPtr->isOorErr = GT_TRUE;
                break;
            }
            hdShift = descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr - startFramePtr;
            break;
        case SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L3_E:
            if(descrPtr->tunnelTerminated == GT_FALSE || descrPtr->origInfoBeforeTunnelTermination.originalL3Ptr < startFramePtr)
            {
                __LOG(("Invalid Original L3 start offset pointer before tunnel termination : 0x%X\n",
                                    descrPtr->origInfoBeforeTunnelTermination.originalL3Ptr));

                /* set Oor Error */
                snetHawkPpuInterruptSet(devObjPtr, 8);
                errInfoPtr->isOorErr = GT_TRUE;
                break;
            }
            hdShift = descrPtr->origInfoBeforeTunnelTermination.originalL3Ptr - startFramePtr - 2;
            break;
        case SNET_SIP6_10_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L4_E:
            if(descrPtr->tunnelTerminated == GT_FALSE || descrPtr->origInfoBeforeTunnelTermination.originalL23HeaderSize == 0)
            {
                __LOG(("Invalid Original L2 + L3 header size before tunnel termination : 0x%X\n",
                                    descrPtr->origInfoBeforeTunnelTermination.originalL23HeaderSize));

                /* set Oor Error */
                snetHawkPpuInterruptSet(devObjPtr, 8);
                errInfoPtr->isOorErr = GT_TRUE;
                break;
            }
            hdShift = descrPtr->origInfoBeforeTunnelTermination.originalL23HeaderSize;
            break;
        default:
            hdShift = 0;
            __LOG(("Invalid Anchor type : %d\n", ppuProfilePtr->anchorType));

            /* set Oor Error */
            snetHawkPpuInterruptSet(devObjPtr, 8);
            errInfoPtr->isOorErr = GT_TRUE;
            break;
    }

    if(!errInfoPtr->isOorErr)
    {
        hdShift += (ppuProfilePtr->offset) * 2; /* 2 byte granularity */

        if(hdShift >= descrPtr->frameBuf->actualDataSize)
        {
            __LOG(("header shift: 0x%X is out of range\n", hdShift));

            /* set Oor Error */
            snetHawkPpuInterruptSet(devObjPtr, 8);
            errInfoPtr->isOorErr = GT_TRUE;
        }
    }

    *kstgHdShift = hdShift;
}

/* Get packet header segment */
static GT_VOID snetHawkPpuHeaderGet
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  GT_U32                          kstgHdShift,
    IN  SNET_SIP6_10_PPU_ERROR_INFO_STC *errInfoPtr,
    OUT SNET_SIP6_10_PPU_HEADER_STC     *headerPtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuHeaderGet);

    GT_U8   *headerStartPtr;
    GT_U32  ii;
    GT_U8   maxHeaderLen = SNET_SIP6_10_PPU_HD_LEN_MAX_CNS;

    __LOG_PARAM(kstgHdShift);

    if(kstgHdShift > descrPtr->frameBuf->actualDataSize)
    {
        __LOG(("Header shift (%d) exceeds frame size (%d)\n",
                kstgHdShift, descrPtr->frameBuf->actualDataSize));

        /* set Oor Error */
        snetHawkPpuInterruptSet(devObjPtr, 8);
        errInfoPtr->isOorErr = GT_TRUE;
        return;
    }

    /* copy 32 byte header segment to PPU header data structure */
    headerStartPtr = descrPtr->frameBuf->actualDataPtr + kstgHdShift;
    if(descrPtr->frameBuf->actualDataSize < kstgHdShift + SNET_SIP6_10_PPU_HD_LEN_MAX_CNS)
    {
        maxHeaderLen = SNET_SIP6_10_PPU_HD_LEN_MAX_CNS - (kstgHdShift
                + SNET_SIP6_10_PPU_HD_LEN_MAX_CNS - descrPtr->frameBuf->actualDataSize);
    }

    for(ii = 0; ii < maxHeaderLen; ii++)
    {
        headerPtr->data[SNET_SIP6_10_PPU_HD_LEN_MAX_CNS - ii -1] = headerStartPtr[ii];
    }

    __LOG(("Header Segment:\n"));
    for(ii = 0; ii < SNET_SIP6_10_PPU_HD_LEN_MAX_CNS; ii++)
    {
        __LOG(("%02X ", headerPtr->data[ii]));
        if((ii + 1) % 16 == 0)
        {
            __LOG(("\n"));
        }
    }
}

/* Get K_stg key generation profile */
static GT_VOID snetHawkPpuKstgKeyGenProfileGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                       kstgNum,
    IN  GT_U32                       profileIdx,
    OUT SNET_SIP6_10_PPU_KSTG_KEY_GEN_PROFILE_STC *keyGenProfilePtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuKstgKeyGenProfileGet);
    GT_U32 regValue;
    GT_U32 ii;

    __LOG(("KSTG key Gen Profile Bytes [0 1 ... 6] = ["));
    for(ii = 0; ii < SNET_SIP6_10_PPU_KSTG_KEY_GEN_PROFILE_BYTE_SELECT_MAX_CNS; ii++)
    {
        /* read key generation profile data */
        smemRegGet(devObjPtr,
            SMEM_SIP6_10_PPU_KSTG_KEY_GEN_PROFILE_BYTE_REG(devObjPtr, kstgNum, profileIdx, ii),
            &regValue);

        /* populate key generation profile structure */
        keyGenProfilePtr->byteSelOffset[ii] = (GT_U8) (regValue & 0xFF);

        __LOG(("%02X ", keyGenProfilePtr->byteSelOffset[ii] & 0x3F));
    }
    __LOG(("]\n"));
}

/* Get Tcam Entry */
static GT_VOID snetHawkPpuKstgTcamEntryGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                       kstgNum,
    IN  GT_U32                       index,
    OUT SNET_SIP6_10_PPU_KSTG_TCAM_ENTRY_STC *kstgTcamEntryPtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuKstgTcamEntryGet);
    GT_U32 regValue;

    /* key least significant 4 bytes */
    smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_LSB_REG(devObjPtr, kstgNum, index), &regValue);
    copyU32toU8array(regValue, sizeof(regValue), &kstgTcamEntryPtr->key[0]);

    /* key most significant 3 bytes */
    smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_MSB_REG(devObjPtr, kstgNum, index), &regValue);
    copyU32toU8array(regValue, SNET_SIP6_10_PPU_KSTG_TCAM_KEY_LEN_CNS-sizeof(regValue), &kstgTcamEntryPtr->key[4]);

    __LOG(("K_stg tcam key at index: %d is hex(7bytes) [0 1 ... 6] = [%02X %02X %02X %02X %02X %02X %02X]\n", index,
            kstgTcamEntryPtr->key[0], kstgTcamEntryPtr->key[1], kstgTcamEntryPtr->key[2], kstgTcamEntryPtr->key[3],
            kstgTcamEntryPtr->key[4], kstgTcamEntryPtr->key[5], kstgTcamEntryPtr->key[6]));

    /* mask least significant 4 bytes */
    smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_MASK_LSB_REG(devObjPtr, kstgNum, index), &regValue);
    copyU32toU8array(regValue, sizeof(regValue), &kstgTcamEntryPtr->mask[0]);

    /* mask most significant 3 bytes */
    smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_MASK_MSB_REG(devObjPtr, kstgNum, index), &regValue);
    copyU32toU8array(regValue, SNET_SIP6_10_PPU_KSTG_TCAM_KEY_LEN_CNS-sizeof(regValue), &kstgTcamEntryPtr->mask[4]);

    __LOG(("K_stg tcam mask at index: %d is hex(7bytes) [0 1 ... 6] = [%02X %02X %02X %02X %02X %02X %02X]\n", index,
            kstgTcamEntryPtr->mask[0], kstgTcamEntryPtr->mask[1], kstgTcamEntryPtr->mask[2], kstgTcamEntryPtr->mask[3],
            kstgTcamEntryPtr->mask[4], kstgTcamEntryPtr->mask[5], kstgTcamEntryPtr->mask[6]));

    /* mask least significant 4 bytes */
    smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_KSTG_TCAM_KEY_VALID_REG(devObjPtr, kstgNum, index), &regValue);
    kstgTcamEntryPtr->isValid = (GT_BOOL) (regValue & 0x1);

    __LOG_PARAM(kstgTcamEntryPtr->isValid);
}

/* print the ppuActionEntry in log */
static GT_VOID snetHawkPpuRotActionEntryPrintLog
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  SNET_SIP6_10_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuRotActionEntryPrintLog);
    GT_U32 offset;
    GT_U32 ii;

    /* populate ppu action entry fields */
    __LOG_PARAM(ppuActionEntryPtr->setNextState     );
    __LOG_PARAM(ppuActionEntryPtr->setConstNextShift);
    __LOG_PARAM(ppuActionEntryPtr->setLoopBack      );
    __LOG_PARAM(ppuActionEntryPtr->counterSet       );
    __LOG_PARAM(ppuActionEntryPtr->counterSetVal    );
    __LOG_PARAM(ppuActionEntryPtr->counterOper      );

    offset = 0;
    /* populate all 5 ROTs data */
    for(ii = 0; ii < SNET_SIP6_10_PPU_KSTG_ROTS_MAX_CNS; ii++)
    {
        __LOG_PARAM(ii);
        __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].srcRegSel);
        __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].func     );
        if(ppuActionEntryPtr->rotActionEntry[ii].srcRegSel == SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_4BYTE_SET_CMD_E)
        {
            __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].setCmd4Byte);
        }
        else
        {
            __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].srcOffset    );
            __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].srcNumValBits);
            if(ppuActionEntryPtr->rotActionEntry[ii].func == SNET_SIP6_10_PPU_ROT_FUNC_CMP_REG_E)
            {
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].cmpVal       );
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].cmpFalseValLd);
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].cmpCond      );
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].cmpMask      );
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].cmpTrueValLd );
            }
            else
            {
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].shiftLeftRightSel);
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].shiftNum         );
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].setBitsNum       );
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].setBitsVal       );
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].srcOffset        );
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].addSubConst      );
                __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].addSubConstSel   );
            }
        }

        __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].funcSecondOperand);
        __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].target           );
        __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].ldSpBusNumBytes  );
        __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].ldSpBusOffset    );
        __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].interrupt        );
        __LOG_PARAM(ppuActionEntryPtr->rotActionEntry[ii].interruptIndex   );

        offset += 71;
    }

}

/* Get ROT action entry */
static GT_VOID snetHawkPpuKstgRotActionEntryGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                       kstgNum,
    IN  GT_U32                       ppuActionEntryIndex,
    OUT SNET_SIP6_10_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
)
{
    GT_U32 tableEntryAddress = 0;
    GT_U32 offset;
    GT_U32 ii;
    GT_U32 *memPtr;

    /* calculate table entry address */
    if(kstgNum == 0)
    {
        tableEntryAddress = SMEM_SIP6_10_PPU_ACTION_TBL_0_MEM(devObjPtr, ppuActionEntryIndex);
    }
    else if(kstgNum == 1)
    {
        tableEntryAddress = SMEM_SIP6_10_PPU_ACTION_TBL_1_MEM(devObjPtr, ppuActionEntryIndex);
    }
    else if(kstgNum == 2)
    {
        tableEntryAddress = SMEM_SIP6_10_PPU_ACTION_TBL_2_MEM(devObjPtr, ppuActionEntryIndex);
    }
    else
    {
        skernelFatalError("K_stg number : %d, out of range\n", kstgNum);
    }

    /* get table entry start memory pointer */
    memPtr = smemMemGet(devObjPtr,tableEntryAddress);

    /* populate ppu action entry fields */
    ppuActionEntryPtr->setNextState      = snetFieldValueGet(memPtr, 355, 8);
    ppuActionEntryPtr->setConstNextShift = snetFieldValueGet(memPtr, 363, 7);
    ppuActionEntryPtr->setLoopBack       = snetFieldValueGet(memPtr, 370, 1);
    ppuActionEntryPtr->counterSet        = snetFieldValueGet(memPtr, 371, 1);
    ppuActionEntryPtr->counterSetVal     = snetFieldValueGet(memPtr, 372, 5);
    ppuActionEntryPtr->counterOper       = snetFieldValueGet(memPtr, 377, 2);

    offset = 0;
    /* populate all 5 ROTs data */
    for(ii = 0; ii < SNET_SIP6_10_PPU_KSTG_ROTS_MAX_CNS; ii++)
    {
        ppuActionEntryPtr->rotActionEntry[ii].srcRegSel = snetFieldValueGet(memPtr, offset + 0, 2);
        ppuActionEntryPtr->rotActionEntry[ii].func      = snetFieldValueGet(memPtr, offset + 53, 2);
        if(ppuActionEntryPtr->rotActionEntry[ii].srcRegSel == SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_4BYTE_SET_CMD_E)
        {
            ppuActionEntryPtr->rotActionEntry[ii].setCmd4Byte = snetFieldValueGet(memPtr, offset + 2, 32);
        }
        else
        {
            ppuActionEntryPtr->rotActionEntry[ii].srcOffset     = snetFieldValueGet(memPtr, offset + 2, 8);
            ppuActionEntryPtr->rotActionEntry[ii].srcNumValBits = snetFieldValueGet(memPtr, offset + 10, 5);
            if(ppuActionEntryPtr->rotActionEntry[ii].func == SNET_SIP6_10_PPU_ROT_FUNC_CMP_REG_E)
            {
                ppuActionEntryPtr->rotActionEntry[ii].cmpVal        = snetFieldValueGet(memPtr, offset + 15, 8);
                ppuActionEntryPtr->rotActionEntry[ii].cmpFalseValLd = snetFieldValueGet(memPtr, offset + 23, 8);
                ppuActionEntryPtr->rotActionEntry[ii].cmpCond       = snetFieldValueGet(memPtr, offset + 31, 2);
                ppuActionEntryPtr->rotActionEntry[ii].cmpMask       = snetFieldValueGet(memPtr, offset + 36, 8);
                ppuActionEntryPtr->rotActionEntry[ii].cmpTrueValLd  = snetFieldValueGet(memPtr, offset + 44, 8);
            }
            else
            {
                ppuActionEntryPtr->rotActionEntry[ii].shiftLeftRightSel = snetFieldValueGet(memPtr, offset + 15, 1);
                ppuActionEntryPtr->rotActionEntry[ii].shiftNum          = snetFieldValueGet(memPtr, offset + 16, 3);
                ppuActionEntryPtr->rotActionEntry[ii].setBitsNum        = snetFieldValueGet(memPtr, offset + 19, 4);
                ppuActionEntryPtr->rotActionEntry[ii].setBitsVal        = snetFieldValueGet(memPtr, offset + 23, 8);
                ppuActionEntryPtr->rotActionEntry[ii].setBitsOffset     = snetFieldValueGet(memPtr, offset + 31, 5);
                ppuActionEntryPtr->rotActionEntry[ii].addSubConst       = snetFieldValueGet(memPtr, offset + 36, 16);
                ppuActionEntryPtr->rotActionEntry[ii].addSubConstSel    = snetFieldValueGet(memPtr, offset + 52, 1);
            }
        }

        ppuActionEntryPtr->rotActionEntry[ii].funcSecondOperand = snetFieldValueGet(memPtr, offset + 55, 3);
        ppuActionEntryPtr->rotActionEntry[ii].target            = snetFieldValueGet(memPtr, offset + 58, 1);
        ppuActionEntryPtr->rotActionEntry[ii].ldSpBusNumBytes   = snetFieldValueGet(memPtr, offset + 59, 3);
        ppuActionEntryPtr->rotActionEntry[ii].ldSpBusOffset     = snetFieldValueGet(memPtr, offset + 62, 5);
        ppuActionEntryPtr->rotActionEntry[ii].interrupt         = snetFieldValueGet(memPtr, offset + 67, 1);
        ppuActionEntryPtr->rotActionEntry[ii].interruptIndex    = snetFieldValueGet(memPtr, offset + 68, 3);

        offset += 71;
    }

    if(simLogIsOpenFlag)
    {
        snetHawkPpuRotActionEntryPrintLog(devObjPtr, ppuActionEntryPtr);
    }
}

/* Get DAU profile */
static GT_VOID snetHawkPpuDauProfileGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                       dauProfileIdx,
    OUT SNET_SIP6_10_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuDauProfileGet);
    GT_U32 tableEntryAddress = 0;
    GT_U32 offset;
    GT_U32 ii;
    GT_U32  *memPtr;

    /* calculate dau table entry address */
    tableEntryAddress = SMEM_SIP6_10_PPU_DAU_PROFILE_TBL_MEM(devObjPtr, dauProfileIdx);

    /* get table entry start memory pointer */
    memPtr = smemMemGet(devObjPtr,tableEntryAddress);

    /* populate dau profile entry data into structure */
    offset = 0;
    for(ii=0; ii<SNET_SIP6_10_PPU_DAU_DESC_BYTE_SET_MAX_CNS; ii++)
    {
        dauProfilePtr->setDescBits[ii].spByteWriteEnable  = snetFieldValueGet(memPtr, offset + 0, 1);
        dauProfilePtr->setDescBits[ii].spByteNumBits      = snetFieldValueGet(memPtr, offset + 1, 3);
        dauProfilePtr->setDescBits[ii].spByteSrcOffset    = snetFieldValueGet(memPtr, offset + 4, 8);
        dauProfilePtr->setDescBits[ii].spByteTargetOffset = snetFieldValueGet(memPtr, offset + 12, 12);

        __LOG_PARAM(ii);
        __LOG_PARAM(dauProfilePtr->setDescBits[ii].spByteWriteEnable );
        __LOG_PARAM(dauProfilePtr->setDescBits[ii].spByteNumBits     );
        __LOG_PARAM(dauProfilePtr->setDescBits[ii].spByteSrcOffset   );
        __LOG_PARAM(dauProfilePtr->setDescBits[ii].spByteTargetOffset);

        offset += 24;
    }
}

/* set bits in a 32byte word for ROT data */
static GT_VOID setBitsForRotData
(
    IN GT_U32 numBits,
    IN GT_U32 value,
    IN GT_U32 offset,
    INOUT GT_U32 *data
)
{
    GT_U32 dataMask = 0, ii;

    for(ii = 0; ii < numBits; ii++)
    {
        dataMask |= 1 << ii;
    }
    dataMask <<= offset;

    *data &= ~dataMask;
    *data |= ((value << offset) & dataMask);
}

/* calculate compliment for two bit granular */
static GT_U8 twoBitGranularComplement(GT_U8 mask)
{
    GT_U8 outMask = 0, ii;

    for(ii = 0; ii < 8; ii++)
    {
        outMask |= (((~mask >> ii/2) & 1) << ii);
    }
    return outMask;
}

/**
* @internal snetHawkPpuKstgKeyGenAndLookup function
* @endinternal
*
* @brief    Generate K_stg key and compare with available 64 keys and
*           gives match index
*
* @param[in]    devObjPtr       - pointer to device object.
* @param[in]    headerPtr       - pointer to 32 byte header segment
* @param[in]    udbMetadataPtr  - pointer to 9 byte UDB Metadta
* @param[in,out] spBusPtr       - pointer to 20 byte SP Bus
* @param[in]     kstgNum        - K_stg number
* @param[in]     count          - kstg counter
* @param[in,out] hdShift        - header shift offset with 2 byte granularity
* @param[in,out] kstgState      - K_stg current state
* @param[out]    matchIndex     - pointer to TCAM match index
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetHawkPpuKstgKeyGenAndLookup
(
    IN      SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN      SNET_SIP6_10_PPU_HEADER_STC       *headerPtr,
    IN      SNET_SIP6_10_PPU_UDB_METADATA_STC *udbMetadataPtr,
    INOUT   SNET_SIP6_10_PPU_SP_BUS_STC       *spBusPtr,
    IN      GT_U32                            kstgNum,
    IN      GT_U32                            count,
    IN      GT_U32                            hdShift,
    IN      GT_U32                            kstgState,
    IN      SNET_SIP6_10_PPU_ERROR_INFO_STC   *errInfoPtr,
    OUT     GT_U32                            *matchIndex
)
{
    DECLARE_FUNC_NAME(snetHawkPpuKstgKeyGenAndLookup);
    GT_U8   key_gen_input_bus[SNET_SIP6_10_PPU_KSTG_KEY_GEN_INPUT_BUS_LEN_CNS];
    GT_U32  inputBusOffset = 0;
    GT_U32  keyGenByteOffset;
    GT_U32  keyGenProfileIdx;
    GT_U8   key[SNET_SIP6_10_PPU_KSTG_TCAM_KEY_LEN_CNS];
    GT_U32  ii, jj;
    GT_U32  matchLen = 0;
    SNET_SIP6_10_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    SNET_SIP6_10_PPU_KSTG_TCAM_ENTRY_STC kstgTcamEntry;

    __LOG(("Build input bus: count(1B) + Header(32B) + UDB Meatadata(9B) + SP_BUS(20B) + KeyState(1B)\n"));

    inputBusOffset = 0;

    /* copy ppu_state 1 byte */
    key_gen_input_bus[inputBusOffset] = kstgState;
    inputBusOffset ++;

    /* copy SP_BUS 20 bytes to input bus */
    memcpy(&key_gen_input_bus[inputBusOffset], spBusPtr->data, SNET_SIP6_10_PPU_SP_BUS_BYTES_MAX_CNS);
    inputBusOffset += SNET_SIP6_10_PPU_SP_BUS_BYTES_MAX_CNS;

    /* copy UDB Metadata of 9 bytes to input bus */
    memcpy(&key_gen_input_bus[inputBusOffset], udbMetadataPtr, SNET_SIP6_10_PPU_UDB_METADATA_LEN_MAX_CNS);
    inputBusOffset += SNET_SIP6_10_PPU_UDB_METADATA_LEN_MAX_CNS;

        /* copy header of 32bytes to input bus */
    memcpy(&key_gen_input_bus[inputBusOffset],headerPtr->data,SNET_SIP6_10_PPU_HD_LEN_MAX_CNS);
    inputBusOffset += SNET_SIP6_10_PPU_HD_LEN_MAX_CNS;

    /* copy {3'b0,COUNT[4:0]} to input bus */
    key_gen_input_bus[inputBusOffset] = count & 0x1f;
    inputBusOffset++;


    /* Get key gen profile */
    keyGenProfileIdx = (kstgState >> 4) & 0xf;

    __LOG_PARAM(keyGenProfileIdx);
    snetHawkPpuKstgKeyGenProfileGet(devObjPtr, kstgNum, keyGenProfileIdx, &keyGenProfile);

    /* Generate 7 byte key */
    for(ii = 0; ii < SNET_SIP6_10_PPU_KSTG_TCAM_KEY_LEN_CNS; ii++)
    {
        keyGenByteOffset = keyGenProfile.byteSelOffset[ii];
        if(keyGenByteOffset >= SNET_SIP6_10_PPU_KSTG_KEY_GEN_INPUT_BUS_LEN_CNS)
        {
            __LOG(("Key byte offset out of range\n"));
            errInfoPtr->isSerErr = GT_TRUE;
        }
        key[ii] = key_gen_input_bus[keyGenByteOffset];
    }

    __LOG(("Generated Key(7bytes) [0 1 ... 6] = [0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X]\n",
                key[0], key[1], key[2], key[3], key[4], key[5], key[6]));

    /* Compare key against all entries in tcam and find a match*/
    for(ii = 0; ii < SNET_SIP6_10_PPU_KSTG_TCAM_ENTRIES_MAX_CNS; ii++)
    {
        snetHawkPpuKstgTcamEntryGet(devObjPtr, kstgNum, ii, &kstgTcamEntry);
        if(!kstgTcamEntry.isValid)
        {
            continue;
        }

        matchLen = 0;
        for(jj = 0; jj < SNET_SIP6_10_PPU_KSTG_TCAM_KEY_LEN_CNS; jj++)
        {
            if((key[jj] & ~kstgTcamEntry.mask[jj]) == (kstgTcamEntry.key[jj] & ~kstgTcamEntry.mask[jj]))
                matchLen++;
        }

        if(matchLen == SNET_SIP6_10_PPU_KSTG_TCAM_KEY_LEN_CNS)
        {
            *matchIndex = ii;
            break;
        }
    }

    if(ii == SNET_SIP6_10_PPU_KSTG_TCAM_ENTRIES_MAX_CNS)
    {
        __LOG(("K_stg tcam match not found\n"));
    }
    else
    {
        __LOG(("K_stg tcam match found at index=%d\n", *matchIndex));
    }
}

/**
* @internal snetHawkPpuKstgCalcUnit function
* @endinternal
*
* @brief    calculation unit parses input data and write to SP bus / next State
*
* @param[in]    devObjPtr       - pointer to device object.
* @param[in]    headerPtr       - pointer to 32 byte header segment
* @param[in]    udbMetadataPtr  - pointer to 9 byte UDB Metadta
* @param[in,out] spBusPtr       - pointer to 20 byte SP Bus
* @param[in,out] kstgNum        - K_stg number
* @param[in]     matchIndex     - match index. It is index to PPU action table.
* @param[in,out] count          - kstg counter
* @param[in,out] hdShift        - header shift offset with 2 byte granularity
* @param[in,out] nextState      - pointer to K_stg next state
* @param[out]    nextHdShift    - pointer to next header shift offset with 2 byte granularity.
* @param[out]    loopback       - pointer to loopback enable
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetHawkPpuKstgCalcUnit
(
    IN      SKERNEL_DEVICE_OBJECT              *devObjPtr,
    IN      SNET_SIP6_10_PPU_HEADER_STC        *headerPtr,
    IN      SNET_SIP6_10_PPU_UDB_METADATA_STC  *udbMetadataPtr,
    INOUT   SNET_SIP6_10_PPU_SP_BUS_STC        *spBusPtr,
    IN      GT_U32                             kstgNum,
    IN      GT_U32                             matchIndex,
    INOUT   GT_U32                             *countPtr,
    INOUT   GT_U32                             *nextHdShiftPtr,
    INOUT   GT_U32                             *nextStatePtr,
    OUT     GT_BOOL                            *loopbackPtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuKstgCalcUnit);
    GT_U32  rot;
    GT_U32  srcRegSel;
    GT_U32  srcOffset, srcNumValBits;
    GT_U32  rotFunc;
    GT_U32  rotData = 0;
    GT_BOOL shiftLeftRightSel;
    GT_U32  shiftNum;
    GT_U32  setBitsNum, setBitsVal, setBitsOffset;
    GT_U32  addSubConst;
    GT_BOOL addSubConstSel;
    GT_U32  setCmd4Byte;
    GT_U32  rotRegData[SNET_SIP6_10_PPU_KSTG_ROTS_MAX_CNS];
    GT_U32  firstOperand = 0;
    GT_U32  secondOperand = 0;
    GT_U32  cmpVal, cmpFalseValLd, cmpCond, cmpMask, cmpTrueValLd, cmpTrueMask, cmpFalseMask;
    GT_U32  funcSecondOperand;
    GT_U32  target;
    GT_U32  ldSpBusNumBytes, ldSpBusOffset;
    GT_U32  hdShift = 0;
    GT_BOOL cmpResult = 0;
    SNET_SIP6_10_PPU_KSTG_ACTION_ENTRY_STC ppuActionEntry;
    GT_U32  intIdx = 0;
    GT_U32  intCountVal = 0;

    __LOG(("Calculation Unit in K_stg %d\n", kstgNum));

    /* fetch ROT action entry */
    snetHawkPpuKstgRotActionEntryGet(devObjPtr, kstgNum, matchIndex, &ppuActionEntry);

    hdShift = ppuActionEntry.setConstNextShift * 2; /* 2 byte granularity */

    /* Stage 1: start ROT operation for all 5 ROTS */
    for(rot = 0; rot < SNET_SIP6_10_PPU_KSTG_ROTS_MAX_CNS; rot++)
    {
        __LOG_PARAM(rot);
        /* source reg selection */
        srcRegSel = ppuActionEntry.rotActionEntry[rot].srcRegSel;

        if(srcRegSel == SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_4BYTE_SET_CMD_E)
        {
            /* get 4 byte command */
            setCmd4Byte = ppuActionEntry.rotActionEntry[rot].setCmd4Byte;

            rotData = setCmd4Byte;
            __LOG(("ROT%d: Set 4 byte data 0x%X, rotData=%X\n", rot, setCmd4Byte, rotData));
        }
        else
        {
            srcOffset = ppuActionEntry.rotActionEntry[rot].srcOffset;
            srcNumValBits = ppuActionEntry.rotActionEntry[rot].srcNumValBits + 1;

            /* Choose input bus - Header/SP_BUS/UDB meatadata */
            if(srcRegSel == SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_HD_E)
            {
                rotData = getBits(headerPtr->data, srcOffset, srcNumValBits);
                __LOG(("ROT%d: Get bits from header: offset=%d, numBits=%d, rotData=%X\n",
                        rot, srcOffset, srcNumValBits, rotData));
            }
            else if(srcRegSel == SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_SP_BUS_E)
            {
                rotData = getBits(spBusPtr->data, srcOffset, srcNumValBits);
                __LOG(("ROT%d: Get bits from SP bus: offset=%d, numBits=%d, rotData=%X\n",
                        rot, srcOffset, srcNumValBits, rotData));
            }
            else if(srcRegSel == SNET_SIP6_10_PPU_ROT_SRC_REG_SEL_UDB_METADATA_E)
            {
                rotData = getBits(udbMetadataPtr->data, srcOffset, srcNumValBits);
                __LOG(("ROT%d: Get bits from udb metadata: offset=%d, numBits=%d, rotData=%X\n",
                                        rot, srcOffset, srcNumValBits, rotData));
            }

            rotFunc = ppuActionEntry.rotActionEntry[rot].func;

            if (rotFunc != SNET_SIP6_10_PPU_ROT_FUNC_CMP_REG_E)
            {
                /* Shift logic for multiplication or division */
                shiftLeftRightSel = ppuActionEntry.rotActionEntry[rot].shiftLeftRightSel;
                shiftNum = ppuActionEntry.rotActionEntry[rot].shiftNum;
                if(shiftLeftRightSel == (GT_BOOL) SNET_SIP6_10_PPU_ROT_SHIFT_LEFT_E)
                {
                    rotData <<= shiftNum;
                    __LOG(("ROT%d: Left Shift by %d bits, rotData=%X\n", rot, shiftNum, rotData));
                }
                else
                {
                    rotData >>= shiftNum;
                    __LOG(("ROT%d: Right Shift by %d bits, rotData=%X\n", rot, shiftNum, rotData));
                }

                /* set bits logic 8b */
                setBitsNum    = ppuActionEntry.rotActionEntry[rot].setBitsNum;
                setBitsVal    = ppuActionEntry.rotActionEntry[rot].setBitsVal;
                setBitsOffset = ppuActionEntry.rotActionEntry[rot].setBitsOffset;

                setBitsForRotData(setBitsNum, setBitsVal, setBitsOffset, &rotData);
                __LOG(("ROT%d: set rotData bits, offset=%d, numBits=%d, value=0x%X, rotData=%X\n",
                        rot, setBitsOffset, setBitsNum, setBitsVal, rotData));

                /* add or subtract a constant 16b */
                addSubConst = ppuActionEntry.rotActionEntry[rot].addSubConst;
                addSubConstSel = ppuActionEntry.rotActionEntry[rot].addSubConstSel;

                if(addSubConstSel == (GT_BOOL) SNET_SIP6_10_PPU_ROT_ADD_SEL_E)
                {
                    rotData += addSubConst;
                    __LOG(("ROT%d: add constant value 0x%X, rotData=%X\n", rot, addSubConst, rotData));
                }
                else
                {
                    rotData -= addSubConst;
                    __LOG(("ROT%d: subtract constant value 0x%X, rotData=%X\n", rot, addSubConst, rotData));
                }
            }
            else
            {
                ;
                /* No change in rotData */
            }
        }

        rotRegData[rot] = rotData;
        __LOG(("ROT%d: rotData=0x%X\n", rot, rotData));
    }

    /* Stage 2: Perform ROT operation using intermediate regData from all other ROTs*/
    for(rot = 0; rot < SNET_SIP6_10_PPU_KSTG_ROTS_MAX_CNS; rot++)
    {
        srcRegSel = ppuActionEntry.rotActionEntry[rot].srcRegSel;
        rotFunc = ppuActionEntry.rotActionEntry[rot].func;

        funcSecondOperand = ppuActionEntry.rotActionEntry[rot].funcSecondOperand;
        target = ppuActionEntry.rotActionEntry[rot].target;
        ldSpBusNumBytes = ppuActionEntry.rotActionEntry[rot].ldSpBusNumBytes;
        ldSpBusOffset = ppuActionEntry.rotActionEntry[rot].ldSpBusOffset;

        /* get first and second operands */
        firstOperand = rotRegData[rot];
        if(funcSecondOperand < SNET_SIP6_10_PPU_KSTG_ROTS_MAX_CNS)
        {
            secondOperand = rotRegData[funcSecondOperand];
        }

        if(rotFunc == SNET_SIP6_10_PPU_ROT_FUNC_CMP_REG_E)
        {
            /* perform compare operation */

            cmpVal        = ppuActionEntry.rotActionEntry[rot].cmpVal;
            cmpMask       = ppuActionEntry.rotActionEntry[rot].cmpMask;
            cmpCond       = ppuActionEntry.rotActionEntry[rot].cmpCond;
            cmpFalseValLd = ppuActionEntry.rotActionEntry[rot].cmpFalseValLd;
            cmpTrueValLd  = ppuActionEntry.rotActionEntry[rot].cmpTrueValLd;

            if(funcSecondOperand == rot || !(funcSecondOperand < SNET_SIP6_10_PPU_KSTG_ROTS_MAX_CNS))
            {
                secondOperand = cmpVal;
            }

            /* compare first and second operaands based on compare condition */
            if(cmpCond == SNET_SIP6_10_PPU_ROT_CMP_COND_EQUAL_E)
            {
                cmpResult = (firstOperand == secondOperand);
                __LOG(("ROT%d: check if equal firstOperand1=0x%X and secondOperand=0x%X, cmpResult=%X\n",
                                        rot, firstOperand, secondOperand, cmpResult));
            }
            else if(cmpCond == SNET_SIP6_10_PPU_ROT_CMP_COND_GREATER_THAN_OR_EQUAL_E)
            {
                cmpResult = (firstOperand >= secondOperand);
                __LOG(("ROT%d: check if greater or equal firstOperand1=0x%X and secondOperand=0x%X, cmpResult=%X\n",
                                        rot, firstOperand, secondOperand, cmpResult));
            }
            else if(cmpCond == SNET_SIP6_10_PPU_ROT_CMP_COND_LESS_THAN_OR_EQUAL_E)
            {
                cmpResult = (firstOperand <= secondOperand);
                __LOG(("ROT%d: check if less or equal firstOperand1=0x%X and secondOperand=0x%X, cmpResult=%X\n",
                                        rot, firstOperand, secondOperand, cmpResult));
            }

            cmpTrueMask = twoBitGranularComplement(cmpMask & 0xF);
            cmpFalseMask = twoBitGranularComplement((cmpMask >> 4) & 0xF);

            __LOG_PARAM(cmpResult);
            /* set rotData based on compare result */
            if (cmpResult == GT_TRUE)
            {
                rotData = (rotData & ~cmpTrueMask) | (cmpTrueValLd & cmpTrueMask);
                __LOG(("ROT%d: Load value 0x%X with mask 0x%X, rotData=%X\n", rot, cmpTrueValLd, cmpTrueMask, rotData));
            }
            else
            {
                rotData = (rotData & ~cmpFalseMask) | (cmpFalseValLd & cmpFalseMask);
                __LOG(("ROT%d: Load value 0x%X with mask 0x%X, rotData=%X\n", rot, cmpFalseValLd, cmpFalseMask, rotData));
            }
        }
        else if(rotFunc == SNET_SIP6_10_PPU_ROT_FUNC_ADD_REG_E)
        {
            /* perform addition operation */
            rotData = firstOperand + secondOperand;
            __LOG(("ROT%d: add operands, firstOperand1=0x%X and secondOperand=0x%X, rotData=%X\n",
                                rot, firstOperand, secondOperand, rotData));
        }
        else if(rotFunc == SNET_SIP6_10_PPU_ROT_FUNC_SUB_REG_E)
        {
            /* perform subtraction operation */
            rotData = firstOperand - secondOperand;
            __LOG(("ROT%d: subtract operands, firstOperand1=0x%X and secondOperand=0x%X, rotData=%X\n",
                    rot, firstOperand, secondOperand, rotData));
        }
        else
        {
            /* perform no operation */
            rotData = firstOperand;
            __LOG(("ROT%d: No operation, rotData=%X\n", rot, rotData));
        }

        __LOG(("ROT%d: rotData=0x%X\n", rot, rotData));

        if(target == SNET_SIP6_10_PPU_ROT_TARGET_SEL_SP_BUS_E)
        {
            if(rotFunc == SNET_SIP6_10_PPU_ROT_FUNC_CMP_REG_E && ldSpBusNumBytes > 1)
            {
                /* Max write allowed is one byte */
                ldSpBusNumBytes = 1;
                __LOG(("ROT%d: Overwrite ldSpBusNumBytes to %d\n", rot, ldSpBusNumBytes));
            }
            copyU32toU8array(rotData, ldSpBusNumBytes, &spBusPtr->data[ldSpBusOffset]);
        }
        else
        {
            /* set next HD shift value */
            hdShift = rotData & 0x7f;
            __LOG(("ROT%d: HdShift=0x%X\n", rot, hdShift));
        }

        /* set interrupt register */
        if(ppuActionEntry.rotActionEntry[rot].interrupt == GT_TRUE)
        {
            intIdx = ppuActionEntry.rotActionEntry[rot].interruptIndex & 0x7;

            /* read original counter value */
            smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_INT_COUNTER_REG(devObjPtr, intIdx), &intCountVal);

            intCountVal ++;

            /* set incremented counter value */
            smemRegSet(devObjPtr, SMEM_SIP6_10_PPU_INT_COUNTER_REG(devObjPtr, intIdx), intCountVal);

            /* set interrupt cause register */
            snetHawkPpuInterruptSet(devObjPtr, 11 + intIdx);
        }
    }

    /* set next state */
    *nextStatePtr = ppuActionEntry.setNextState;
    __LOG_PARAM(*nextStatePtr);

    *nextHdShiftPtr += 2*hdShift;
    __LOG_PARAM(*nextHdShiftPtr);

    /* set counter */
    if(ppuActionEntry.counterSet == GT_TRUE)
    {
        *countPtr = ppuActionEntry.counterSetVal & 0x1f;
    }
    else
    {
        if(ppuActionEntry.counterOper == SNET_SIP6_10_PPU_PPU_ACT_COUNTER_OPER_INC_E)
        {
            (*countPtr) ++;
        }
        else if(ppuActionEntry.counterOper == SNET_SIP6_10_PPU_PPU_ACT_COUNTER_OPER_DEC_E)
        {
            (*countPtr) --;
        }
    }
    __LOG_PARAM(*countPtr);

    /* set loopback */
    *loopbackPtr = ppuActionEntry.setLoopBack;
    __LOG_PARAM(*loopbackPtr);

}

static GT_STATUS snetHawkPpuDauProtectedWindowCheck
(
        IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
        IN  GT_U32                          offset,
        IN  GT_U32                          numBytes
)
{
    DECLARE_FUNC_NAME(snetHawkPpuDauProtectedWindowCheck);

    GT_U32 regValue;
    GT_U32 ii;
    SNET_SIP6_10_PPU_DAU_DATA_PROTECTED_WINDOW_STC protWin;

    for(ii = 0; ii < SNET_SIP6_10_PPU_DAU_DATA_PROTECTED_WINDOW_MAX_CNS; ii++)
    {
        smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_DAU_PROTECTED_WINDOW_REG(devObjPtr, ii), &regValue);
        protWin.protWinEnable      = (GT_BOOL)(regValue & 0x1);
        protWin.protWinStartOffset = (regValue >> 1 ) & 0xFFF;
        protWin.protWinEndOffset   = (regValue >> 13) & 0xFFF;

        if((protWin.protWinEnable == GT_TRUE) && (offset >= protWin.protWinStartOffset)
            && (offset <= protWin.protWinEndOffset))
        {
            __LOG(("Error: offset %d inside protected window. starOffset: %d, endOffset: %d\n",
                    offset, protWin.protWinStartOffset, protWin.protWinEndOffset));
            return GT_FAIL;
        }

        if((protWin.protWinEnable == GT_TRUE) && (offset + numBytes - 1 >= protWin.protWinStartOffset)
            && (offset + numBytes - 1 <= protWin.protWinEndOffset))
        {
            __LOG(("Error: offset + numbytes - 1 = %d inside protected window. starOffset: %d, endOffset: %d\n",
                    offset + numBytes - 1, protWin.protWinStartOffset, protWin.protWinEndOffset));
            return GT_FAIL;
        }
    }
    return GT_OK;
}

/**
* @internal snetHawkPpuKstg function
* @endinternal
*
* @brief   PPU K_STG parses generates key and extracts required fields from
*          header/Udb Metadata/Other fields and writes to SP bus.
*
* @param[in]    devObjPtr       - pointer to device object.
* @param[in]    kstgNum         - k_stg number
* @param[in]    headerPtr       - pointer to 32 byte header segment
* @param[in]    udbMetadataPtr  - pointer to 9 byte UDB Metadta
* @param[in,out] spBusPtr       - pointer to 20 byte SP Bus
* @param[in,out] count          - kstg counter
* @param[in,out] hdShift        - header shift offset with 2 byte granularity
* @param[in,out] nextState      - pointer to K_stg next state
* @param[out]    loopbackEn     - pointer to loopback enable/disable.
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetHawkPpuKstg
(
    IN      SKERNEL_DEVICE_OBJECT              *devObjPtr,
    IN      GT_U32                             kstgNum,
    IN      SNET_SIP6_10_PPU_HEADER_STC        *headerPtr,
    IN      SNET_SIP6_10_PPU_UDB_METADATA_STC  *udbMetadataPtr,
    INOUT   SNET_SIP6_10_PPU_SP_BUS_STC        *spBusPtr,
    IN      SNET_SIP6_10_PPU_ERROR_INFO_STC    *errInfoPtr,
    INOUT   GT_U32                             *count,
    INOUT   GT_U32                             *hdShift,
    INOUT   GT_U32                             *nextState,
    OUT     GT_BOOL                            *loopbackEn
)
{
    DECLARE_FUNC_NAME(snetHawkPpuKstg);
    GT_U32   matchIndex = 0;

    if(*nextState == 0)
    {
        __LOG(("This K_stage %d is disabled\n", kstgNum));
        return;
    }

    if(errInfoPtr->isSerErr | errInfoPtr->isLoopbackErr | errInfoPtr->isOorErr)
    {
        __LOG(("Error flag is set. Skip further processing.\n"));
        return;
    }

    /* increment debug counter for incoming packet to K_stg */
    snetHawkPpuDebugCounterIncrement(devObjPtr,
            SIP_6_10_REG(devObjPtr, PPU.debugRegs.ppuDebugCounterKstg0) + 4*kstgNum);

    /* key generation and tcam lookup */
    snetHawkPpuKstgKeyGenAndLookup(devObjPtr, headerPtr, udbMetadataPtr, spBusPtr,
            kstgNum, *count, *hdShift, *nextState, errInfoPtr, &matchIndex);

    if(matchIndex >= SNET_SIP6_10_PPU_KSTG_TCAM_ENTRIES_MAX_CNS)
    {
        __LOG(("Match index out of range %d (must be < %d\n",
            matchIndex, SNET_SIP6_10_PPU_KSTG_TCAM_ENTRIES_MAX_CNS));
        errInfoPtr->isSerErr = GT_TRUE;
        return;
    }

    /* calculation unit */
    snetHawkPpuKstgCalcUnit(devObjPtr, headerPtr, udbMetadataPtr, spBusPtr, kstgNum,
            matchIndex, count, hdShift, nextState, loopbackEn);
}

/**
* @internal snetHawkPpuErrorCheck function
* @endinternal
*
* @brief  Desctriptor alteration unit. It modifies fields in the descriptor.
*
* @param[in]    devObjPtr               - pointer to device object.
* @param[out]   isErrPtr                - pointer to isErr
* @param[out]   errProfPtr              - pointer to error profile index
*/
static GT_VOID snetHawkPpuErrorCheck
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SNET_SIP6_10_PPU_ERROR_INFO_STC *errInfoPtr,
    OUT GT_BOOL                         *isErrPtr,
    OUT GT_U32                          *errProfPtr
)
{
    GT_U32 globalConfig;

    smemRegGet(devObjPtr, SMEM_SIP6_10_PPU_GLOBAL_CONFIG_REG(devObjPtr), &globalConfig);

    if(errInfoPtr->isSerErr == GT_TRUE)
    {
        /* SER error */
        *isErrPtr   = 1;
        *errProfPtr = (globalConfig >> 6) & 0x3;
    }
    else if(errInfoPtr->isOorErr == GT_TRUE)
    {
        /* Out of range error */
        *isErrPtr   = 1;
        *errProfPtr = (globalConfig >> 8) & 0x3;
    }
    else if(errInfoPtr->isLoopbackErr == GT_TRUE)
    {
        /* loopback error */
        *isErrPtr   = 1;
        *errProfPtr = (globalConfig >> 4) & 0x3;
    }
    else
    {
        *isErrPtr   = 0;
        *errProfPtr = 0;
    }

}

/**
* @internal snetHawkPpuDau function
* @endinternal
*
* @brief  Desctriptor alteration unit. It modifies fields in the descriptor.
*
* @param[in]     devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
* @param[in]     spBus                    - pointer to scratch pad bus
* @param[in]     dauProfile               - pointer to DAU profile.
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetHawkPpuDau
(
    IN      SKERNEL_DEVICE_OBJECT                  *devObjPtr,
    INOUT   SKERNEL_FRAME_CHEETAH_DESCR_STC        *descrPtr,
    IN      SNET_SIP6_10_PPU_SP_BUS_STC            *spBusPtr,
    IN      SNET_SIP6_10_PPU_DAU_PROFILE_ENTRY_STC *dauProfile,
    IN      SNET_SIP6_10_PPU_ERROR_INFO_STC        *errInfoPtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuDau);

    GT_STATUS rc;
    GT_U32    ii;
    GT_U32    val;
    GT_U32    spByteSrcOffset, spByteNumBits;
    GT_U32    spByteTargetOffset;
    GT_U8     hwDescBus[SNET_SIP6_10_PPU_DAU_HW_DESC_LEN_IN_BYTES_CNS]; /* hardware descriptor bus */
    GT_U8     hwDescMask[SNET_SIP6_10_PPU_DAU_HW_DESC_LEN_IN_BYTES_CNS]; /* hardware descriptor bus */
    GT_BOOL   isError = GT_FALSE;
    GT_U32    errProfIdx;
    GT_U32    errProfWrEn;
    GT_U32    errProfNumBits;
    GT_U32    errProfData;
    GT_U32    errProfOffset;

    /* increment debug counter for incoming packet to DAU unit */
    snetHawkPpuDebugCounterIncrement(devObjPtr, SIP_6_10_REG(devObjPtr, PPU.debugRegs.ppuDebugCounterDau));

    memset(hwDescBus, 0, sizeof(hwDescBus));
    memset(hwDescMask, 0, sizeof(hwDescMask));

    /* convert software descriptor to hardware descriptor */
    snetHawkPpuDauDescSwToHwConvert(devObjPtr, descrPtr, hwDescBus);

    snetHawkPpuErrorCheck(devObjPtr, errInfoPtr, &isError, &errProfIdx);

    if(isError == GT_FALSE)
    {
        /* Modify hardware descriptor */
        for(ii=0; ii<SNET_SIP6_10_PPU_DAU_DESC_BYTE_SET_MAX_CNS; ii++)
        {
            if(!dauProfile->setDescBits[ii].spByteWriteEnable)
            {
                continue;
            }

            spByteSrcOffset    = dauProfile->setDescBits[ii].spByteSrcOffset;
            spByteNumBits      = dauProfile->setDescBits[ii].spByteNumBits + 1;
            spByteTargetOffset = dauProfile->setDescBits[ii].spByteTargetOffset;

            /* get required bits from SP_BUS */
            val = getBits(spBusPtr->data, spByteSrcOffset, spByteNumBits);

            rc = snetHawkPpuDauProtectedWindowCheck(devObjPtr, spByteTargetOffset, spByteNumBits);
            if(rc != GT_OK)
            {
                __LOG(("Descriptor alteration bits inside protected window. offset: %d, numBits: %d\n",
                        spByteTargetOffset, spByteNumBits));

                /* set interrupt */
                snetHawkPpuInterruptSet(devObjPtr, 10);
            }
            else
            {
                __LOG(("Set descriptor bits: offset=%d, numBits=%d, value=0x%X", spByteTargetOffset, spByteNumBits, val));
                /* modified the selected bits in descriptor */
                setBitsInDescriptor(spByteTargetOffset, spByteNumBits, val, hwDescBus, hwDescMask);
            }
        }
    }
    else if(errProfIdx > 0)
    {
        /* error profile 0 is saved. Index for error profile 1 and 2 is 0 and 1 respectively */
        errProfIdx -= 1;

        /* Get error profile and apply it to descriptor */
        for(ii = 0; ii < SNET_SIP6_10_PPU_ERROR_PROFILE_FIELDS_MAX_CNS; ii++)
        {
            smemRegFldGet(devObjPtr, SMEM_SIP6_10_PPU_ERROR_PROFILE_REG(devObjPtr, errProfIdx, ii),
                          0, 1, &errProfWrEn);
            smemRegFldGet(devObjPtr, SMEM_SIP6_10_PPU_ERROR_PROFILE_REG(devObjPtr, errProfIdx, ii),
                          1, 3, &errProfNumBits); /* numBits - 1 */
            smemRegFldGet(devObjPtr, SMEM_SIP6_10_PPU_ERROR_PROFILE_REG(devObjPtr, errProfIdx, ii),
                          4, 8, &errProfData);
            smemRegFldGet(devObjPtr, SMEM_SIP6_10_PPU_ERROR_PROFILE_REG(devObjPtr, errProfIdx, ii),
                          12, 12, &errProfOffset);

            __LOG_PARAM(ii);
            __LOG_PARAM(errProfWrEn);
            __LOG_PARAM(errProfNumBits);
            __LOG_PARAM(errProfData);
            __LOG_PARAM(errProfOffset);

            if(!errProfWrEn)
            {
                continue;
            }

            errProfNumBits ++;

            rc = snetHawkPpuDauProtectedWindowCheck(devObjPtr, errProfOffset, errProfNumBits);
            if(rc != GT_OK)
            {
                __LOG(("Descriptor alteration bits inside protected window. offset: %d, numBits: %d\n",
                        errProfOffset, errProfNumBits));

                /* set interrupt */
                snetHawkPpuInterruptSet(devObjPtr, 10);
            }
            else
            {
                setBitsInDescriptor(errProfOffset, errProfNumBits, errProfData, hwDescBus, hwDescMask);
            }
        }
    }

    /* convert hardware descriptor to software descriptor */
    snetHawkPpuDauDescHwToSwConvert(devObjPtr, descrPtr, hwDescBus, hwDescMask);

}

static GT_VOID snetHawkPpuPreparePclmetadata
(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpuPreparePclmetadata);

    CHT_PCL_EXTRA_PACKET_INFO_STC    pclExtraData;
    CHT_PCL_EXTRA_PACKET_INFO_STC    origPclExtraData;
    SKERNEL_FRAME_CHEETAH_DESCR_STC  *origDescrPtr;/* pointer to the original descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC  *passengerDescrPtr;/* pointer to the transit descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC  *fixedFieldsTypeDescrPtr;/* pointer to the descriptor for fixed fields */

    origDescrPtr =  descrPtr;

    /* prepare for transit issues */
    descrPtr = snetXcatIpclTransitPacketPrepare(devObjPtr,descrPtr);

    passengerDescrPtr       = descrPtr->ingressTunnelInfo.passengerDescrPtr;
    fixedFieldsTypeDescrPtr = descrPtr->ingressTunnelInfo.fixedFieldsTypeDescrPtr;

    /* assign packet type for PCL usage
       must be done only after descrPtr may hold different values then origDescrPtr !!! */
    pcktTypeAssign(devObjPtr, passengerDescrPtr,PACKET_TYPE_USED_FOR_CLIENT_IPCL_E);

    descrPtr->ingressTunnelInfo.origDescrPtr->pcl_pcktType_sip5 = descrPtr->pcl_pcktType_sip5;
    descrPtr->ingressTunnelInfo.origDescrPtr->ingressTunnelInfo.passengerDescrPtr->pcl_pcktType_sip5 = descrPtr->pcl_pcktType_sip5;
    descrPtr->ingressTunnelInfo.origDescrPtr->ingressTunnelInfo.fixedFieldsTypeDescrPtr->pcl_pcktType_sip5 = descrPtr->pcl_pcktType_sip5;
    descrPtr->ingressTunnelInfo.origDescrPtr->ingressTunnelInfo.udbDescrPtr->pcl_pcktType_sip5 = descrPtr->pcl_pcktType_sip5;

    /* Get extra data from packet that will be used in PCL engine */
    __LOG(("Get extra data from packet that will be used in PCL engine"));
    snetChtPclSrvParseExtraData(devObjPtr, fixedFieldsTypeDescrPtr, &pclExtraData);
    fixedFieldsTypeDescrPtr->pclExtraDataPtr = &pclExtraData;

    if (origDescrPtr != fixedFieldsTypeDescrPtr)
    {
        /* Get extra data from original packet that will be used in PCL engine */
        snetChtPclSrvParseExtraData(devObjPtr, origDescrPtr, &origPclExtraData);
        origDescrPtr->pclExtraDataPtr = &origPclExtraData;
    }
}

/**
* @internal snetHawkPpu function
* @endinternal
*
* @brief   PPU Engine processing for outgoing frame on Hawk
*         asic simulation.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to frame data buffer Id
*/
GT_VOID snetHawkPpu
(
    IN     SKERNEL_DEVICE_OBJECT               *devObjPtr,
    INOUT  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr
)
{
    DECLARE_FUNC_NAME(snetHawkPpu);

    SNET_SIP6_10_PPU_CONFIG_STC            ppuGlobalCfg;
    SNET_SIP6_10_PPU_PROFILE_ENTRY_STC     ppuProfile;
    SNET_SIP6_10_PPU_SP_BUS_STC            spBusProfile;
    SNET_SIP6_10_PPU_UDB_METADATA_STC      udbMetadata;
    SNET_SIP6_10_PPU_HEADER_STC            header;
    SNET_SIP6_10_PPU_DAU_PROFILE_ENTRY_STC dauProfile;
    SNET_SIP6_10_PPU_ERROR_INFO_STC        errInfo;

    GT_U32  kstgHdShift = 0;
    GT_U32  kstgNum;
    GT_BOOL kstgLoopback  = GT_FALSE;
    GT_U32  kstgNextState = 0;
    GT_U32  dauProfileIdx = 0;
    GT_U32  kstgCounter = 0;
    GT_U32  maxLoopback;
    GT_U32  loopbackCount = 0;

    __LOG(("PPU engine processing.\n"));

    /* Get ppu global configuration */
    snetHawkPpuGlobalConfigGet(devObjPtr, &ppuGlobalCfg);
    __LOG_PARAM(ppuGlobalCfg.ppuEnable);
    if(!ppuGlobalCfg.ppuEnable)
    {
        __LOG(("PPU is disabled for this device\n"));
        return;
    }

    /* prepare pcl metadata */
    snetHawkPpuPreparePclmetadata(devObjPtr, descrPtr);


    /* increment debug counter for incoming packet */
    snetHawkPpuDebugCounterIncrement(devObjPtr, SIP_6_10_REG(devObjPtr, PPU.debugRegs.ppuDebugCounterIn));

    __LOG_PARAM(descrPtr->ppuProfileIdx);
    snetHawkPpuProfileGet(devObjPtr, descrPtr->ppuProfileIdx, &ppuProfile);
    if(!ppuProfile.ppuEnable)
    {
        /* Increment debug counters to align with GM */
        for(kstgNum = 0; kstgNum < SNET_SIP6_10_PPU_KSTGS_MAX_CNS; kstgNum++)
        {
            /* increment debug counter for incoming packet to K_stg */
            snetHawkPpuDebugCounterIncrement(devObjPtr,
                SIP_6_10_REG(devObjPtr, PPU.debugRegs.ppuDebugCounterKstg0) + 4*kstgNum);
        }

        /* increment debug counter for incoming packet to DAU unit */
        snetHawkPpuDebugCounterIncrement(devObjPtr, SIP_6_10_REG(devObjPtr, PPU.debugRegs.ppuDebugCounterDau));

        /* increment debug counter for out going packet */
        snetHawkPpuDebugCounterIncrement(devObjPtr, SIP_6_10_REG(devObjPtr, PPU.debugRegs.ppuDebugCounterOut));

        __LOG(("PPU is disabled for this profile:%d\n", descrPtr->ppuProfileIdx));
        return;
    }

    /* initialize memory */
    memset(&errInfo, 0, sizeof(SNET_SIP6_10_PPU_ERROR_INFO_STC));

    /* Get spBusProfile */
    snetHawkPpuSpBusProfileGet(devObjPtr, ppuProfile.spBusDefaultProfile, &spBusProfile);

    /* Get UDB MetaData */
    snetHawkPpuUdbMetadataGet(devObjPtr, descrPtr, &errInfo, &udbMetadata);

    /* Get header shift offset for first KSTG */
    snetHawkPpuHeaderShiftGet(devObjPtr, descrPtr, &ppuProfile, &errInfo, &kstgHdShift);

    kstgNextState = ppuProfile.ppuState;
    /* KSTG Processing */
    /* TBD kstgNextState = ppuProfile.ppuState;*/
    do
    {
        if(errInfo.isSerErr | errInfo.isLoopbackErr | errInfo.isOorErr)
        {
            __LOG(("Error flag is set. Skip further processing.\n"));
            break;
        }

        for(kstgNum = 0; kstgNum < SNET_SIP6_10_PPU_KSTGS_MAX_CNS; kstgNum++)
        {
            __LOG(("PPU K_stg %d Processing.\n", kstgNum));

            /* Get 32 byte Header starting from kstgHdShift offset */
            snetHawkPpuHeaderGet(devObjPtr, descrPtr, kstgHdShift, &errInfo, &header);

            /* Run KSTG */
            snetHawkPpuKstg(devObjPtr, kstgNum, &header, &udbMetadata, &spBusProfile, &errInfo, &kstgCounter,
                    &kstgHdShift, &kstgNextState, &kstgLoopback);
        }

        if(kstgLoopback == GT_TRUE)
        {
            loopbackCount ++;
            smemRegFldGet(devObjPtr, SMEM_SIP6_10_PPU_GLOBAL_CONFIG_REG(devObjPtr), 1, 3, &maxLoopback);

            /* set loopback error */
            if(loopbackCount > maxLoopback)
            {
                __LOG(("Loopback error detected: observed=%d, max=%d\n", loopbackCount, maxLoopback))
                snetHawkPpuInterruptSet(devObjPtr, 9);
                errInfo.isLoopbackErr = GT_TRUE;
            }

            /* increment debug counter for packet loopback */
            snetHawkPpuDebugCounterIncrement(devObjPtr, SIP_6_10_REG(devObjPtr, PPU.debugRegs.ppuDebugCounterLoopback));
        }
    } while(kstgLoopback == GT_TRUE);

    /* Build IPCL UDB data */
    snetHawkBuildIpclUdbData(devObjPtr, descrPtr);

    /* Get DAU profile */
    dauProfileIdx = kstgNextState & SNET_SIP6_10_PPU_DAU_PROFILE_IDX_MASK;
    __LOG_PARAM(dauProfileIdx);

    snetHawkPpuDauProfileGet(devObjPtr, dauProfileIdx, &dauProfile);

    __LOG(("PPU DAU Processing.\n"));
    /* Update descripter */
    snetHawkPpuDau(devObjPtr, descrPtr, &spBusProfile, &dauProfile, &errInfo);
    __LOG(("PPU Processing complete\n"));

    /* increment debug counter for out going packet */
    snetHawkPpuDebugCounterIncrement(devObjPtr, SIP_6_10_REG(devObjPtr, PPU.debugRegs.ppuDebugCounterOut));
}

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
    IN    SKERNEL_DEVICE_OBJECT            *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr
)
{
    DECLARE_FUNC_NAME (snetHawkBuildIpclUdbData);

    GT_STATUS rc = GT_OK;
    GT_U32    i = 0;

    __LOG(("Building IPCL UDB Data\n"));

    for (i=0; i<SNET_PCL_UDB_MAX_NUMBER_CNS; i++)
    {
        rc = snetLionPclUdbKeyValueGet(devObjPtr, descrPtr, SMAIN_DIRECTION_INGRESS_E, i,
                                  &descrPtr->ipclUdbData[i]);
        if (rc != GT_OK)
        {
            __LOG(("UDB data get failed: Udb index - %d, rc - %d\n", i, rc));
            return;
        }

        if (((descrPtr->ipclUdbValid[i/8] >> (i%8)) & 0x1) == 1)
        {
            __LOG(("Udb Idx = %d, Udb Valid = 1, Udb Value = 0x%02X\n", i, descrPtr->ipclUdbData[i]));
        }
    }

    descrPtr->ipclUdbDataReady = 1;

    return;
}
