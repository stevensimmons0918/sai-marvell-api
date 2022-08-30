#ifdef _VISUALC
    #pragma warning(disable: 4214) /* nonstandard extension used : bit field types other than int */
#endif


#include "ppa_fw_defs.h"
#include "ppa_fw_base_types.h"
#include "ppa_accelerator_commands.h"
#include "ppa_threads_fw_defs.h"

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/smem/smemPipe.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SLog/simLog.h>

#define PIPE_APPLY_CHANGES(addr,structCast)     pipeMemApply_##structCast(addr)

static void applyAllChanges(void);

static SKERNEL_DEVICE_OBJECT * devObjPtr = NULL;/* needed for __LOG() and __LOG_PARAM() */
/* The descriptor memory format is expected to be in 'BIG ENDIAN' words order */
/* ... just the words ... not the bytes/bits */
static GT_U32   desc_copyWordsFromMem_needToSwap = 1;
static void desc_copyWordsFromMem(
    /*IN*/GT_U32    *origMemPtr,
    /*IN*/GT_U32    numOfWords,
    /*OUT*/GT_U32   *trgMemPtr
)
{
    GT_U32  ii,iiOrig;

    for(ii = 0 ; ii < numOfWords; ii++)
    {
        iiOrig = desc_copyWordsFromMem_needToSwap ? numOfWords-(ii+1) : ii;

        trgMemPtr[ii] = origMemPtr[iiOrig];
    }

    return;
}
/* The configuration memory format is expected to be in 'LITTLE ENDIAN' words order */
/* ... just the words ... not the bytes/bits */
static GT_U32   config_copyWordsFromMem_needToSwap = 0;
static void config_copyWordsFromMem(
    /*IN*/GT_U32    *origMemPtr,
    /*IN*/GT_U32    numOfWords,
    /*OUT*/GT_U32   *trgMemPtr
)
{
    GT_U32  ii,iiOrig;

    for(ii = 0 ; ii < numOfWords; ii++)
    {
        iiOrig = config_copyWordsFromMem_needToSwap ? numOfWords-(ii+1) : ii;

        trgMemPtr[ii] = origMemPtr[iiOrig];
    }

    return;
}


/* The header memory format is expected to be in 'BIG ENDIAN' bytes order */
static void copyBytesFromHeader(
    /*IN*/GT_BOOL   fromAddrToU32,/* GT_TRUE ->copy from address to u32MemPtr  */
                                  /* GT_FALDE->copy from u32MemPtr  to address */
    /*IN or OUT */GT_U32   address,
    /*IN or OUT */GT_U32   *u32MemPtr,
    /*IN*/GT_U32    numOfBytes
)
{
    GT_U32  ii;
    GT_U8   byte0,byte1,byte2,byte3;
    GT_U32  lastWordNumBytes = (numOfBytes & 3);
    GT_U32  numBytes;

    if(lastWordNumBytes == 0)
    {
        lastWordNumBytes = 4;
    }

    for(ii = 0 ; ii < numOfBytes; ii+=4)
    {
        if(fromAddrToU32 == GT_TRUE)
        {
            byte0 = PPA_FW_SP_BYTE_READ(address + ii + 0);
            byte1 = PPA_FW_SP_BYTE_READ(address + ii + 1);
            byte2 = PPA_FW_SP_BYTE_READ(address + ii + 2);
            byte3 = PPA_FW_SP_BYTE_READ(address + ii + 3);

            u32MemPtr[ii/4] = byte0 << 24 | byte1 << 16 |
                              byte2 <<  8 | byte3 <<  0 ;
        }
        else
        {
            byte0 = (u32MemPtr[ii/4] >> 24) & 0xFF;
            byte1 = (u32MemPtr[ii/4] >> 16) & 0xFF;
            byte2 = (u32MemPtr[ii/4] >>  8) & 0xFF;
            byte3 = (u32MemPtr[ii/4] >>  0) & 0xFF;

            if((ii + 4) >= numOfBytes)
            {
                /* last word iteration */
                numBytes = lastWordNumBytes;
            }
            else
            {
                numBytes = 4;
            }

            PPA_FW_SP_BYTE_WRITE(address + ii + 0, byte0);
            if(numBytes > 1)
            {
                PPA_FW_SP_BYTE_WRITE(address + ii + 1, byte1);
                if(numBytes > 2)
                {
                    PPA_FW_SP_BYTE_WRITE(address + ii + 2, byte2);
                    if(numBytes > 3)
                    {
                        PPA_FW_SP_BYTE_WRITE(address + ii + 3, byte3);
                    }
                }
            }
        }
    }

    return;
}

typedef struct{
    char*   fieldName;/* info for the __LOG */
    GT_U32  endBit;  /* inclusive*/
    GT_U32  startBit;/* inclusive , value SMAIN_NOT_VALID_CNS means 'equal to endBit' */
}descriptor_info;

static descriptor_info pha_internal_desc[] =
{
 /* bus: internal_desc*/
 {STR(pha_internal_desc_reserved_0                    ),     31     ,      0       }
,{STR(pha_internal_desc_udp_checksum_update_en        ),     32     ,SMAIN_NOT_VALID_CNS               }
,{STR(pha_internal_desc_udp_checksum_offset           ),     40     ,      33      }
,{STR(pha_internal_desc_add_crc                       ),     41     ,SMAIN_NOT_VALID_CNS               }
,{STR(pha_internal_desc_ptp_timestamp_queue_select    ),     42     ,SMAIN_NOT_VALID_CNS               }
,{STR(pha_internal_desc_ptp_egress_tai_sel            ),     43     ,SMAIN_NOT_VALID_CNS               }
,{STR(pha_internal_desc_ptp_action                    ),     47     ,      44      }
,{STR(pha_internal_desc_fw_bc_modification            ),     55     ,      48      }
,{STR(pha_internal_desc_timestamp_offset              ),     63     ,      56      }
,{STR(pha_internal_desc_ingress_timestamp_seconds     ),     65     ,      64      }
,{STR(pha_internal_desc_egress_pipe_delay             ),     95     ,      66      }
,{STR(pha_internal_desc_ptp_timestamp_queue_entry_id  ),     105     ,      96     }
,{STR(pha_internal_desc_ptp_cf_wraparound_check_en    ),     106     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_mac_timestamping_en           ),     107     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_ptp_packet_format             ),     110     ,      108    }
,{STR(pha_internal_desc_ptp_dispatching_en            ),     111     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_desc_reserved                 ),     115     ,      112    }
,{STR(pha_internal_desc_fw_drop                       ),     116     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_truncate_packet               ),     117     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_qcn_generated_by_local_dev    ),     118     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_generate_cnc                  ),     119     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_txdma2ha_header_ctrl_bus      ),     127     ,      120    }
,{STR(pha_internal_desc_need_crc_removal              ),     128     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_cut_through_id                ),     139     ,      129    }
,{STR(pha_internal_desc_packet_is_cut_through         ),     140     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_tc                            ),     143     ,      141    }
,{STR(pha_internal_desc_header_hw_error_detected      ),     144     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_table_hw_error_detected       ),     145     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_ingress_byte_count            ),     159     ,      146    }
,{STR(pha_internal_desc_txq_queue_id                  ),     167     ,      160    }
,{STR(pha_internal_desc_cnc_address                   ),     183     ,      168    }
,{STR(pha_internal_desc_packet_hash_bits_6_5          ),     185     ,      184    }
,{STR(pha_internal_desc_dp                            ),     187     ,      186    }
,{STR(pha_internal_desc_dei                           ),     188     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_up0                           ),     191     ,      189    }
,{STR(pha_internal_desc_outer_l3_offset               ),     197     ,      192    }
 /* QCN */
,{STR(pha_internal_desc_congested_port_number         ),     202     ,      198    }
,{STR(pha_internal_desc_qcn_q_fb                      ),     208     ,      203    }
,{STR(pha_internal_desc_congested_queue_number        ),     211     ,      209    }
,{STR(pha_internal_desc_packet_hash_4qcn_bits_4_0     ),     216     ,      212    }
,{STR(pha_internal_desc_reserved_flags_4qcn           ),     222     ,      217    }
,{STR(pha_internal_desc_mark_ecn_4qcn                 ),     223     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_qcn_q_delta                   ),     237     ,      224    }
,{STR(pha_internal_desc_reserved                      ),     238     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_orig_generated_qcn_prio       ),     241     ,      239    }
,{STR(pha_internal_desc_qcn_q_offset                  ),     255     ,      242    }
 /* non QCN */
,{STR(pha_internal_desc_ptp_offset                    ),     204     ,      198    }
,{STR(pha_internal_desc_ptp_pkt_type_idx              ),     209     ,      205    }
,{STR(pha_internal_desc_ptp_u_field                   ),     210     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_packet_hash_bits_4_0          ),     215     ,      211    }
,{STR(pha_internal_desc_reserved_flags                ),     221     ,      216    }
,{STR(pha_internal_desc_mark_ecn                      ),     222     ,SMAIN_NOT_VALID_CNS              }
,{STR(pha_internal_desc_is_ptp                        ),     223     ,      223    }
,{STR(pha_internal_desc_timestamp                     ),     255     ,      224    }
};

typedef enum{
     pha_internal_desc_reserved_0
    ,pha_internal_desc_udp_checksum_update_en
    ,pha_internal_desc_udp_checksum_offset
    ,pha_internal_desc_add_crc
    ,pha_internal_desc_ptp_timestamp_queue_select
    ,pha_internal_desc_ptp_egress_tai_sel
    ,pha_internal_desc_ptp_action
    ,pha_internal_desc_fw_bc_modification
    ,pha_internal_desc_timestamp_offset
    ,pha_internal_desc_ingress_timestamp_seconds
    ,pha_internal_desc_egress_pipe_delay
    ,pha_internal_desc_ptp_timestamp_queue_entry_id
    ,pha_internal_desc_ptp_cf_wraparound_check_en
    ,pha_internal_desc_mac_timestamping_en
    ,pha_internal_desc_ptp_packet_format
    ,pha_internal_desc_ptp_dispatching_en
    ,pha_internal_desc_desc_reserved
    ,pha_internal_desc_fw_drop
    ,pha_internal_desc_truncate_packet
    ,pha_internal_desc_qcn_generated_by_local_dev
    ,pha_internal_desc_generate_cnc
    ,pha_internal_desc_txdma2ha_header_ctrl_bus
    ,pha_internal_desc_need_crc_removal
    ,pha_internal_desc_cut_through_id
    ,pha_internal_desc_packet_is_cut_through
    ,pha_internal_desc_tc
    ,pha_internal_desc_header_hw_error_detected
    ,pha_internal_desc_table_hw_error_detected
    ,pha_internal_desc_ingress_byte_count
    ,pha_internal_desc_txq_queue_id
    ,pha_internal_desc_cnc_address
    ,pha_internal_desc_packet_hash_bits_6_5
    ,pha_internal_desc_dp
    ,pha_internal_desc_dei
    ,pha_internal_desc_up0
    ,pha_internal_desc_outer_l3_offset
    /* QCN */
    ,pha_internal_desc_congested_port_number
    ,pha_internal_desc_qcn_q_fb
    ,pha_internal_desc_congested_queue_number
    ,pha_internal_desc_packet_hash_4qcn_bits_4_0
    ,pha_internal_desc_reserved_flags_4qcn
    ,pha_internal_desc_mark_ecn_4qcn
    ,pha_internal_desc_qcn_q_delta
    ,pha_internal_desc_reserved
    ,pha_internal_desc_orig_generated_qcn_prio
    ,pha_internal_desc_qcn_q_offset
    /* non QCN*/
    ,pha_internal_desc_ptp_offset
    ,pha_internal_desc_ptp_pkt_type_idx
    ,pha_internal_desc_ptp_u_field
    ,pha_internal_desc_packet_hash_bits_4_0
    ,pha_internal_desc_reserved_flags
    ,pha_internal_desc_mark_ecn
    ,pha_internal_desc_is_ptp
    ,pha_internal_desc_timestamp
}pha_internal_desc_enum;

#define FORMAT_DESC_GET(field , targetValue) \
    targetValue = getDescriptorInfo(pha_internal_desc,littleEndianMem,field)

#define FORMAT_DESC_SET(field , sourceValue) \
    setDescriptorInfo(pha_internal_desc,littleEndianMem,field,sourceValue)

static GT_U32  getDescriptorInfo(descriptor_info *infoPtr,GT_U32 *memPtr , GT_U32 fieldName)
{
    descriptor_info*    currInfoPtr=&infoPtr[fieldName];
    GT_U32  endBit  = currInfoPtr->endBit;
    GT_U32  startBit = (currInfoPtr->startBit == SMAIN_NOT_VALID_CNS) ? endBit : currInfoPtr->startBit;
    GT_U32 numOfBits = (endBit-startBit)+1;
    GT_U32 value;

    value = snetFieldValueGet(memPtr,startBit,numOfBits);

    __LOG_NO_LOCATION_META_DATA(("[%s] get value [0x%x]\n" ,
        currInfoPtr->fieldName,value));

    return value;
}

static void    setDescriptorInfo(descriptor_info *infoPtr,GT_U32 *memPtr , GT_U32 fieldName,GT_U32 value)
{
    descriptor_info*    currInfoPtr=&infoPtr[fieldName];
    GT_U32  endBit  = currInfoPtr->endBit;
    GT_U32  startBit = (currInfoPtr->startBit == SMAIN_NOT_VALID_CNS) ? endBit : currInfoPtr->startBit;
    GT_U32 numOfBits = (endBit-startBit)+1;

    snetFieldValueSet(memPtr,startBit,numOfBits,value);
}

#define DESC_MEM_SIZE_IN_WORDS_CNS  8
static GT_BIT copyMem_nonQcn_desc__isOrigSaved = 0;
static struct nonQcn_desc copyMem_nonQcn_desc;
static struct nonQcn_desc copyMem_nonQcn_desc__orig;
struct nonQcn_desc* pipeMemCast_nonQcn_desc(uint32_t address)
{
    uint32_t  littleEndianMem[DESC_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);
    uint32_t dp,timestamp;

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    desc_copyWordsFromMem(memPtr,DESC_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    FORMAT_DESC_GET(pha_internal_desc_reserved_0                   , copyMem_nonQcn_desc.reserved_0                    );
    FORMAT_DESC_GET(pha_internal_desc_udp_checksum_update_en       , copyMem_nonQcn_desc.udp_checksum_update_en        );
    FORMAT_DESC_GET(pha_internal_desc_udp_checksum_offset          , copyMem_nonQcn_desc.udp_checksum_offset           );
    FORMAT_DESC_GET(pha_internal_desc_add_crc                      , copyMem_nonQcn_desc.add_crc                       );
    FORMAT_DESC_GET(pha_internal_desc_ptp_timestamp_queue_select   , copyMem_nonQcn_desc.ptp_timestamp_queue_select    );
    FORMAT_DESC_GET(pha_internal_desc_ptp_egress_tai_sel           , copyMem_nonQcn_desc.ptp_egress_tai_sel            );
    FORMAT_DESC_GET(pha_internal_desc_ptp_action                   , copyMem_nonQcn_desc.ptp_action                    );
    FORMAT_DESC_GET(pha_internal_desc_fw_bc_modification           , copyMem_nonQcn_desc.fw_bc_modification            );
    FORMAT_DESC_GET(pha_internal_desc_timestamp_offset             , copyMem_nonQcn_desc.timestamp_offset              );
    FORMAT_DESC_GET(pha_internal_desc_ingress_timestamp_seconds    , copyMem_nonQcn_desc.ingress_timestamp_seconds     );
    FORMAT_DESC_GET(pha_internal_desc_egress_pipe_delay            , copyMem_nonQcn_desc.egress_pipe_delay             );
    FORMAT_DESC_GET(pha_internal_desc_ptp_timestamp_queue_entry_id , copyMem_nonQcn_desc.ptp_timestamp_queue_entry_id  );
    FORMAT_DESC_GET(pha_internal_desc_ptp_cf_wraparound_check_en   , copyMem_nonQcn_desc.ptp_cf_wraparound_check_en    );
    FORMAT_DESC_GET(pha_internal_desc_mac_timestamping_en          , copyMem_nonQcn_desc.mac_timestamping_en           );
    FORMAT_DESC_GET(pha_internal_desc_ptp_packet_format            , copyMem_nonQcn_desc.ptp_packet_format             );
    FORMAT_DESC_GET(pha_internal_desc_ptp_dispatching_en           , copyMem_nonQcn_desc.ptp_dispatching_en            );
    FORMAT_DESC_GET(pha_internal_desc_desc_reserved                , copyMem_nonQcn_desc.desc_reserved                 );
    FORMAT_DESC_GET(pha_internal_desc_fw_drop                      , copyMem_nonQcn_desc.fw_drop                       );
    FORMAT_DESC_GET(pha_internal_desc_truncate_packet              , copyMem_nonQcn_desc.truncate_packet               );
    FORMAT_DESC_GET(pha_internal_desc_qcn_generated_by_local_dev   , copyMem_nonQcn_desc.qcn_generated_by_local_dev    );
    FORMAT_DESC_GET(pha_internal_desc_generate_cnc                 , copyMem_nonQcn_desc.generate_cnc                  );
    FORMAT_DESC_GET(pha_internal_desc_txdma2ha_header_ctrl_bus     , copyMem_nonQcn_desc.txdma2ha_header_ctrl_bus      );
    FORMAT_DESC_GET(pha_internal_desc_need_crc_removal             , copyMem_nonQcn_desc.need_crc_removal              );
    FORMAT_DESC_GET(pha_internal_desc_cut_through_id               , copyMem_nonQcn_desc.cut_through_id                );
    FORMAT_DESC_GET(pha_internal_desc_packet_is_cut_through        , copyMem_nonQcn_desc.packet_is_cut_through         );
    FORMAT_DESC_GET(pha_internal_desc_tc                           , copyMem_nonQcn_desc.tc                            );
    FORMAT_DESC_GET(pha_internal_desc_header_hw_error_detected     , copyMem_nonQcn_desc.header_hw_error_detected      );
    FORMAT_DESC_GET(pha_internal_desc_table_hw_error_detected      , copyMem_nonQcn_desc.table_hw_error_detected       );
    FORMAT_DESC_GET(pha_internal_desc_ingress_byte_count           , copyMem_nonQcn_desc.ingress_byte_count            );
    FORMAT_DESC_GET(pha_internal_desc_txq_queue_id                 , copyMem_nonQcn_desc.txq_queue_id                  );
    FORMAT_DESC_GET(pha_internal_desc_cnc_address                  , copyMem_nonQcn_desc.cnc_address                   );
    FORMAT_DESC_GET(pha_internal_desc_packet_hash_bits_6_5         , copyMem_nonQcn_desc.packet_hash_6_5               );
    FORMAT_DESC_GET(pha_internal_desc_dp                           , dp                                    );
    FORMAT_DESC_GET(pha_internal_desc_dei                          , copyMem_nonQcn_desc.dei                           );
    FORMAT_DESC_GET(pha_internal_desc_up0                          , copyMem_nonQcn_desc.up0                           );
    FORMAT_DESC_GET(pha_internal_desc_outer_l3_offset              , copyMem_nonQcn_desc.outer_l3_offset               );

    FORMAT_DESC_GET(pha_internal_desc_ptp_offset                   , copyMem_nonQcn_desc.ptp_offset                    );
    FORMAT_DESC_GET(pha_internal_desc_ptp_pkt_type_idx             , copyMem_nonQcn_desc.ptp_pkt_type_idx              );
    FORMAT_DESC_GET(pha_internal_desc_ptp_u_field                  , copyMem_nonQcn_desc.ptp_u_field                   );
    FORMAT_DESC_GET(pha_internal_desc_packet_hash_bits_4_0         , copyMem_nonQcn_desc.packet_hash_4_0               );
    FORMAT_DESC_GET(pha_internal_desc_reserved_flags               , copyMem_nonQcn_desc.reserved_flags                );
    FORMAT_DESC_GET(pha_internal_desc_mark_ecn                     , copyMem_nonQcn_desc.mark_ecn                      );
    FORMAT_DESC_GET(pha_internal_desc_is_ptp                       , copyMem_nonQcn_desc.is_ptp                        );
    FORMAT_DESC_GET(pha_internal_desc_timestamp                    , timestamp                             );

    copyMem_nonQcn_desc.dp_0 = dp & 1;/*1bit*/
    copyMem_nonQcn_desc.dp_1 = dp >> 1;/*1bit*/
    copyMem_nonQcn_desc.timestamp_NanoSec = timestamp & 0x3FFFFFFF;/*30bits*/
    copyMem_nonQcn_desc.timestamp_Sec = timestamp>>30;/*2bits*/

    if(copyMem_nonQcn_desc__isOrigSaved == 0)
    {
        /* save the orig descriptor , so we can compare at end of operations */
        copyMem_nonQcn_desc__orig = copyMem_nonQcn_desc;

        copyMem_nonQcn_desc__isOrigSaved = 1;
    }

    return &copyMem_nonQcn_desc;

}


void pipeMemApply_nonQcn_desc(uint32_t address)
{
    uint32_t  littleEndianMem[DESC_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);
    uint32_t dp,timestamp;

    dp = copyMem_nonQcn_desc.dp_0 |
         copyMem_nonQcn_desc.dp_1 << 1;
    timestamp =  copyMem_nonQcn_desc.timestamp_NanoSec |
                 copyMem_nonQcn_desc.timestamp_Sec << 30;

    FORMAT_DESC_SET(pha_internal_desc_reserved_0                   , copyMem_nonQcn_desc.reserved_0                    );
    FORMAT_DESC_SET(pha_internal_desc_udp_checksum_update_en       , copyMem_nonQcn_desc.udp_checksum_update_en        );
    FORMAT_DESC_SET(pha_internal_desc_udp_checksum_offset          , copyMem_nonQcn_desc.udp_checksum_offset           );
    FORMAT_DESC_SET(pha_internal_desc_add_crc                      , copyMem_nonQcn_desc.add_crc                       );
    FORMAT_DESC_SET(pha_internal_desc_ptp_timestamp_queue_select   , copyMem_nonQcn_desc.ptp_timestamp_queue_select    );
    FORMAT_DESC_SET(pha_internal_desc_ptp_egress_tai_sel           , copyMem_nonQcn_desc.ptp_egress_tai_sel            );
    FORMAT_DESC_SET(pha_internal_desc_ptp_action                   , copyMem_nonQcn_desc.ptp_action                    );
    FORMAT_DESC_SET(pha_internal_desc_fw_bc_modification           , copyMem_nonQcn_desc.fw_bc_modification            );
    FORMAT_DESC_SET(pha_internal_desc_timestamp_offset             , copyMem_nonQcn_desc.timestamp_offset              );
    FORMAT_DESC_SET(pha_internal_desc_ingress_timestamp_seconds    , copyMem_nonQcn_desc.ingress_timestamp_seconds     );
    FORMAT_DESC_SET(pha_internal_desc_egress_pipe_delay            , copyMem_nonQcn_desc.egress_pipe_delay             );
    FORMAT_DESC_SET(pha_internal_desc_ptp_timestamp_queue_entry_id , copyMem_nonQcn_desc.ptp_timestamp_queue_entry_id  );
    FORMAT_DESC_SET(pha_internal_desc_ptp_cf_wraparound_check_en   , copyMem_nonQcn_desc.ptp_cf_wraparound_check_en    );
    FORMAT_DESC_SET(pha_internal_desc_mac_timestamping_en          , copyMem_nonQcn_desc.mac_timestamping_en           );
    FORMAT_DESC_SET(pha_internal_desc_ptp_packet_format            , copyMem_nonQcn_desc.ptp_packet_format             );
    FORMAT_DESC_SET(pha_internal_desc_ptp_dispatching_en           , copyMem_nonQcn_desc.ptp_dispatching_en            );
    FORMAT_DESC_SET(pha_internal_desc_desc_reserved                , copyMem_nonQcn_desc.desc_reserved                 );
    FORMAT_DESC_SET(pha_internal_desc_fw_drop                      , copyMem_nonQcn_desc.fw_drop                       );
    FORMAT_DESC_SET(pha_internal_desc_truncate_packet              , copyMem_nonQcn_desc.truncate_packet               );
    FORMAT_DESC_SET(pha_internal_desc_qcn_generated_by_local_dev   , copyMem_nonQcn_desc.qcn_generated_by_local_dev    );
    FORMAT_DESC_SET(pha_internal_desc_generate_cnc                 , copyMem_nonQcn_desc.generate_cnc                  );
    FORMAT_DESC_SET(pha_internal_desc_txdma2ha_header_ctrl_bus     , copyMem_nonQcn_desc.txdma2ha_header_ctrl_bus      );
    FORMAT_DESC_SET(pha_internal_desc_need_crc_removal             , copyMem_nonQcn_desc.need_crc_removal              );
    FORMAT_DESC_SET(pha_internal_desc_cut_through_id               , copyMem_nonQcn_desc.cut_through_id                );
    FORMAT_DESC_SET(pha_internal_desc_packet_is_cut_through        , copyMem_nonQcn_desc.packet_is_cut_through         );
    FORMAT_DESC_SET(pha_internal_desc_tc                           , copyMem_nonQcn_desc.tc                            );
    FORMAT_DESC_SET(pha_internal_desc_header_hw_error_detected     , copyMem_nonQcn_desc.header_hw_error_detected      );
    FORMAT_DESC_SET(pha_internal_desc_table_hw_error_detected      , copyMem_nonQcn_desc.table_hw_error_detected       );
    FORMAT_DESC_SET(pha_internal_desc_ingress_byte_count           , copyMem_nonQcn_desc.ingress_byte_count            );
    FORMAT_DESC_SET(pha_internal_desc_txq_queue_id                 , copyMem_nonQcn_desc.txq_queue_id                  );
    FORMAT_DESC_SET(pha_internal_desc_cnc_address                  , copyMem_nonQcn_desc.cnc_address                   );
    FORMAT_DESC_SET(pha_internal_desc_packet_hash_bits_6_5         , copyMem_nonQcn_desc.packet_hash_6_5               );
    FORMAT_DESC_SET(pha_internal_desc_dp                           , dp                                    );
    FORMAT_DESC_SET(pha_internal_desc_dei                          , copyMem_nonQcn_desc.dei                           );
    FORMAT_DESC_SET(pha_internal_desc_up0                          , copyMem_nonQcn_desc.up0                           );
    FORMAT_DESC_SET(pha_internal_desc_outer_l3_offset              , copyMem_nonQcn_desc.outer_l3_offset               );

    FORMAT_DESC_SET(pha_internal_desc_ptp_offset                   , copyMem_nonQcn_desc.ptp_offset                    );
    FORMAT_DESC_SET(pha_internal_desc_ptp_pkt_type_idx             , copyMem_nonQcn_desc.ptp_pkt_type_idx              );
    FORMAT_DESC_SET(pha_internal_desc_ptp_u_field                  , copyMem_nonQcn_desc.ptp_u_field                   );
    FORMAT_DESC_SET(pha_internal_desc_packet_hash_bits_4_0         , copyMem_nonQcn_desc.packet_hash_4_0               );
    FORMAT_DESC_SET(pha_internal_desc_reserved_flags               , copyMem_nonQcn_desc.reserved_flags                );
    FORMAT_DESC_SET(pha_internal_desc_mark_ecn                     , copyMem_nonQcn_desc.mark_ecn                      );
    FORMAT_DESC_SET(pha_internal_desc_is_ptp                       , copyMem_nonQcn_desc.is_ptp                        );
    FORMAT_DESC_SET(pha_internal_desc_timestamp                    , timestamp                             );

    /*copy words back to memory after 'bit field' manipulations that the FIRMWARE updated */
    desc_copyWordsFromMem(littleEndianMem,DESC_MEM_SIZE_IN_WORDS_CNS,memPtr);

}

#define FORMAT_DESC_COMPARE(field) \
    if(copyMem_nonQcn_desc__orig.field != copyMem_nonQcn_desc.field)            \
    {                                                                           \
        if(isCompareExpected == GT_FALSE)                                       \
        {                                                                       \
            __LOG_NO_LOCATION_META_DATA(("nonQcn_desc[%s] modified to [0x%x] (from[0x%x]) \n", \
                #field ,                                                            \
                copyMem_nonQcn_desc.field,                                          \
                copyMem_nonQcn_desc__orig.field));                                  \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            __LOG_NO_LOCATION_META_DATA(("nonQcn_desc[%s] expected [0x%x] got[0x%x] \n", \
                #field ,                                                            \
                copyMem_nonQcn_desc__orig.field,                                    \
                copyMem_nonQcn_desc.field));                                        \
        }                                                                           \
    }


#define FORMAT_DESC_COMPARE_DECIMAL(field) \
    if(copyMem_nonQcn_desc__orig.field != copyMem_nonQcn_desc.field)            \
    {                                                                           \
        if(isCompareExpected == GT_FALSE)                                       \
        {                                                                       \
            __LOG_NO_LOCATION_META_DATA(("nonQcn_desc[%s] modified to [%d] (from[%d]) \n", \
                #field ,                                                            \
                copyMem_nonQcn_desc.field,                                          \
                copyMem_nonQcn_desc__orig.field));                                  \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            __LOG_NO_LOCATION_META_DATA(("nonQcn_desc[%s] expected [%d] got[%d] \n", \
                #field ,                                                            \
                copyMem_nonQcn_desc__orig.field,                                    \
                copyMem_nonQcn_desc.field));                                        \
        }                                                                           \
    }


/* compare the orig descriptor with the final descriptor */
static void pipeMemCompare_nonQcn_desc(GT_BOOL  isCompareExpected)
{
    if(simLogIsOpenFlag == 0)
    {
        return;
    }

    FORMAT_DESC_COMPARE(reserved_0                   );
    FORMAT_DESC_COMPARE(udp_checksum_update_en       );
    FORMAT_DESC_COMPARE(udp_checksum_offset          );
    FORMAT_DESC_COMPARE(add_crc                      );
    FORMAT_DESC_COMPARE(ptp_timestamp_queue_select   );
    FORMAT_DESC_COMPARE(ptp_egress_tai_sel           );
    FORMAT_DESC_COMPARE(ptp_action                   );
    FORMAT_DESC_COMPARE_DECIMAL(fw_bc_modification           );
    FORMAT_DESC_COMPARE(timestamp_offset             );
    FORMAT_DESC_COMPARE(ingress_timestamp_seconds    );
    FORMAT_DESC_COMPARE(egress_pipe_delay            );
    FORMAT_DESC_COMPARE(ptp_timestamp_queue_entry_id );
    FORMAT_DESC_COMPARE(ptp_cf_wraparound_check_en   );
    FORMAT_DESC_COMPARE(mac_timestamping_en          );
    FORMAT_DESC_COMPARE(ptp_packet_format            );
    FORMAT_DESC_COMPARE(ptp_dispatching_en           );
    FORMAT_DESC_COMPARE(desc_reserved                );
    FORMAT_DESC_COMPARE(fw_drop                      );
    FORMAT_DESC_COMPARE(truncate_packet              );
    FORMAT_DESC_COMPARE(qcn_generated_by_local_dev   );
    FORMAT_DESC_COMPARE(generate_cnc                 );
    FORMAT_DESC_COMPARE(txdma2ha_header_ctrl_bus     );
    FORMAT_DESC_COMPARE(need_crc_removal             );
    FORMAT_DESC_COMPARE(cut_through_id               );
    FORMAT_DESC_COMPARE(packet_is_cut_through        );
    FORMAT_DESC_COMPARE(tc                           );
    FORMAT_DESC_COMPARE(header_hw_error_detected     );
    FORMAT_DESC_COMPARE(table_hw_error_detected      );
    FORMAT_DESC_COMPARE(ingress_byte_count           );
    FORMAT_DESC_COMPARE(txq_queue_id                 );
    FORMAT_DESC_COMPARE(cnc_address                  );
    FORMAT_DESC_COMPARE(packet_hash_6_5              );
    FORMAT_DESC_COMPARE(dp_1                         );
    FORMAT_DESC_COMPARE(dp_0                         );
    FORMAT_DESC_COMPARE(dei                          );
    FORMAT_DESC_COMPARE(up0                          );
    FORMAT_DESC_COMPARE(outer_l3_offset              );
    FORMAT_DESC_COMPARE(ptp_offset                   );
    FORMAT_DESC_COMPARE(ptp_pkt_type_idx             );
    FORMAT_DESC_COMPARE(ptp_u_field                  );
    FORMAT_DESC_COMPARE(packet_hash_4_0              );
    FORMAT_DESC_COMPARE(reserved_flags               );
    FORMAT_DESC_COMPARE(mark_ecn                     );
    FORMAT_DESC_COMPARE(is_ptp                       );
    FORMAT_DESC_COMPARE(timestamp_NanoSec            );
    FORMAT_DESC_COMPARE(timestamp_Sec                );
}


/********************************
 *  QCN descriptor definitions  *
 ********************************/
static GT_BIT copyMem_Qcn_desc__isOrigSaved = 0;
static struct Qcn_desc copyMem_Qcn_desc;
static struct Qcn_desc copyMem_Qcn_desc__orig;
static GT_BIT isQcnDescriptor;
struct Qcn_desc* pipeMemCast_Qcn_desc(uint32_t address)
{
    uint32_t  littleEndianMem[DESC_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    desc_copyWordsFromMem(memPtr,DESC_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    FORMAT_DESC_GET(pha_internal_desc_reserved_0                   , copyMem_Qcn_desc.reserved_0                    );
    FORMAT_DESC_GET(pha_internal_desc_udp_checksum_update_en       , copyMem_Qcn_desc.UDP_checksum_update_En        );
    FORMAT_DESC_GET(pha_internal_desc_udp_checksum_offset          , copyMem_Qcn_desc.UDP_checksum_offset           );
    FORMAT_DESC_GET(pha_internal_desc_add_crc                      , copyMem_Qcn_desc.add_CRC                       );
    FORMAT_DESC_GET(pha_internal_desc_ptp_timestamp_queue_select   , copyMem_Qcn_desc.PTP_timestamp_Queue_select    );
    FORMAT_DESC_GET(pha_internal_desc_ptp_egress_tai_sel           , copyMem_Qcn_desc.PTP_Egress_TAI_sel            );
    FORMAT_DESC_GET(pha_internal_desc_ptp_action                   , copyMem_Qcn_desc.PTP_action                    );
    FORMAT_DESC_GET(pha_internal_desc_fw_bc_modification           , copyMem_Qcn_desc.fw_bc_modification            );
    FORMAT_DESC_GET(pha_internal_desc_timestamp_offset             , copyMem_Qcn_desc.timestamp_offset              );
    FORMAT_DESC_GET(pha_internal_desc_ingress_timestamp_seconds    , copyMem_Qcn_desc.ingress_timestamp_seconds     );
    FORMAT_DESC_GET(pha_internal_desc_egress_pipe_delay            , copyMem_Qcn_desc.egress_pipe_delay             );
    FORMAT_DESC_GET(pha_internal_desc_ptp_timestamp_queue_entry_id , copyMem_Qcn_desc.PTP_timestamp_Queue_Entry_ID  );
    FORMAT_DESC_GET(pha_internal_desc_ptp_cf_wraparound_check_en   , copyMem_Qcn_desc.PTP_CF_wraparound_check_en    );
    FORMAT_DESC_GET(pha_internal_desc_mac_timestamping_en          , copyMem_Qcn_desc.MAC_timestamping_en           );
    FORMAT_DESC_GET(pha_internal_desc_ptp_packet_format            , copyMem_Qcn_desc.PTP_packet_format             );
    FORMAT_DESC_GET(pha_internal_desc_ptp_dispatching_en           , copyMem_Qcn_desc.PTP_dispatching_En            );
    FORMAT_DESC_GET(pha_internal_desc_desc_reserved                , copyMem_Qcn_desc.desc_reserved                 );
    FORMAT_DESC_GET(pha_internal_desc_fw_drop                      , copyMem_Qcn_desc.fw_drop                       );
    FORMAT_DESC_GET(pha_internal_desc_truncate_packet              , copyMem_Qcn_desc.Truncate_packet               );
    FORMAT_DESC_GET(pha_internal_desc_qcn_generated_by_local_dev   , copyMem_Qcn_desc.QCN_generated_by_local_Dev    );
    FORMAT_DESC_GET(pha_internal_desc_generate_cnc                 , copyMem_Qcn_desc.Generate_CNC                  );
    FORMAT_DESC_GET(pha_internal_desc_txdma2ha_header_ctrl_bus     , copyMem_Qcn_desc.txdma2ha_header_ctrl_bus      );
    FORMAT_DESC_GET(pha_internal_desc_need_crc_removal             , copyMem_Qcn_desc.need_CRC_removal              );
    FORMAT_DESC_GET(pha_internal_desc_cut_through_id               , copyMem_Qcn_desc.cut_through_id                );
    FORMAT_DESC_GET(pha_internal_desc_packet_is_cut_through        , copyMem_Qcn_desc.packet_is_cut_through         );
    FORMAT_DESC_GET(pha_internal_desc_tc                           , copyMem_Qcn_desc.TC                            );
    FORMAT_DESC_GET(pha_internal_desc_header_hw_error_detected     , copyMem_Qcn_desc.header_HW_error_detected      );
    FORMAT_DESC_GET(pha_internal_desc_table_hw_error_detected      , copyMem_Qcn_desc.table_HW_error_detected       );
    FORMAT_DESC_GET(pha_internal_desc_ingress_byte_count           , copyMem_Qcn_desc.ingress_byte_count            );
    FORMAT_DESC_GET(pha_internal_desc_txq_queue_id                 , copyMem_Qcn_desc.txq_queue_id                  );
    FORMAT_DESC_GET(pha_internal_desc_cnc_address                  , copyMem_Qcn_desc.CNC_address                   );
    FORMAT_DESC_GET(pha_internal_desc_packet_hash_bits_6_5         , copyMem_Qcn_desc.packet_hash_bits_6_5          );
    FORMAT_DESC_GET(pha_internal_desc_dp                           , copyMem_Qcn_desc.dp                            );
    FORMAT_DESC_GET(pha_internal_desc_dei                          , copyMem_Qcn_desc.dei                           );
    FORMAT_DESC_GET(pha_internal_desc_up0                          , copyMem_Qcn_desc.up0                           );
    FORMAT_DESC_GET(pha_internal_desc_outer_l3_offset              , copyMem_Qcn_desc.Outer_l3_offset               );

    FORMAT_DESC_GET(pha_internal_desc_congested_port_number        , copyMem_Qcn_desc.congested_port_number         );
    FORMAT_DESC_GET(pha_internal_desc_qcn_q_fb                     , copyMem_Qcn_desc.qcn_q_fb                      );
    FORMAT_DESC_GET(pha_internal_desc_congested_queue_number       , copyMem_Qcn_desc.congested_queue_number        );
    FORMAT_DESC_GET(pha_internal_desc_packet_hash_4qcn_bits_4_0    , copyMem_Qcn_desc.packet_hash_4qcn_bits_4_0     );
    FORMAT_DESC_GET(pha_internal_desc_reserved_flags_4qcn          , copyMem_Qcn_desc.reserved_flags_4qcn           );
    FORMAT_DESC_GET(pha_internal_desc_mark_ecn_4qcn                , copyMem_Qcn_desc.mark_ecn_4qcn                 );
    FORMAT_DESC_GET(pha_internal_desc_qcn_q_delta                  , copyMem_Qcn_desc.qcn_q_delta                   );
    FORMAT_DESC_GET(pha_internal_desc_reserved                     , copyMem_Qcn_desc.reserved                      );
    FORMAT_DESC_GET(pha_internal_desc_orig_generated_qcn_prio      , copyMem_Qcn_desc.orig_generated_QCN_prio       );
    FORMAT_DESC_GET(pha_internal_desc_qcn_q_offset                 , copyMem_Qcn_desc.qcn_q_offset                  );

    if(copyMem_Qcn_desc__isOrigSaved == 0)
    {
        /* save the orig descriptor , so we can compare at end of operations */
        copyMem_Qcn_desc__orig = copyMem_Qcn_desc;

        copyMem_Qcn_desc__isOrigSaved = 1;
    }

    return &copyMem_Qcn_desc;
}

void pipeMemApply_Qcn_desc(uint32_t address)
{
    uint32_t  littleEndianMem[DESC_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    FORMAT_DESC_SET(pha_internal_desc_reserved_0                   , copyMem_Qcn_desc.reserved_0                    );
    FORMAT_DESC_SET(pha_internal_desc_udp_checksum_update_en       , copyMem_Qcn_desc.UDP_checksum_update_En        );
    FORMAT_DESC_SET(pha_internal_desc_udp_checksum_offset          , copyMem_Qcn_desc.UDP_checksum_offset           );
    FORMAT_DESC_SET(pha_internal_desc_add_crc                      , copyMem_Qcn_desc.add_CRC                       );
    FORMAT_DESC_SET(pha_internal_desc_ptp_timestamp_queue_select   , copyMem_Qcn_desc.PTP_timestamp_Queue_select    );
    FORMAT_DESC_SET(pha_internal_desc_ptp_egress_tai_sel           , copyMem_Qcn_desc.PTP_Egress_TAI_sel            );
    FORMAT_DESC_SET(pha_internal_desc_ptp_action                   , copyMem_Qcn_desc.PTP_action                    );
    FORMAT_DESC_SET(pha_internal_desc_fw_bc_modification           , copyMem_Qcn_desc.fw_bc_modification            );
    FORMAT_DESC_SET(pha_internal_desc_timestamp_offset             , copyMem_Qcn_desc.timestamp_offset              );
    FORMAT_DESC_SET(pha_internal_desc_ingress_timestamp_seconds    , copyMem_Qcn_desc.ingress_timestamp_seconds     );
    FORMAT_DESC_SET(pha_internal_desc_egress_pipe_delay            , copyMem_Qcn_desc.egress_pipe_delay             );
    FORMAT_DESC_SET(pha_internal_desc_ptp_timestamp_queue_entry_id , copyMem_Qcn_desc.PTP_timestamp_Queue_Entry_ID  );
    FORMAT_DESC_SET(pha_internal_desc_ptp_cf_wraparound_check_en   , copyMem_Qcn_desc.PTP_CF_wraparound_check_en    );
    FORMAT_DESC_SET(pha_internal_desc_mac_timestamping_en          , copyMem_Qcn_desc.MAC_timestamping_en           );
    FORMAT_DESC_SET(pha_internal_desc_ptp_packet_format            , copyMem_Qcn_desc.PTP_packet_format             );
    FORMAT_DESC_SET(pha_internal_desc_ptp_dispatching_en           , copyMem_Qcn_desc.PTP_dispatching_En            );
    FORMAT_DESC_SET(pha_internal_desc_desc_reserved                , copyMem_Qcn_desc.desc_reserved                 );
    FORMAT_DESC_SET(pha_internal_desc_fw_drop                      , copyMem_Qcn_desc.fw_drop                       );
    FORMAT_DESC_SET(pha_internal_desc_truncate_packet              , copyMem_Qcn_desc.Truncate_packet               );
    FORMAT_DESC_SET(pha_internal_desc_qcn_generated_by_local_dev   , copyMem_Qcn_desc.QCN_generated_by_local_Dev    );
    FORMAT_DESC_SET(pha_internal_desc_generate_cnc                 , copyMem_Qcn_desc.Generate_CNC                  );
    FORMAT_DESC_SET(pha_internal_desc_txdma2ha_header_ctrl_bus     , copyMem_Qcn_desc.txdma2ha_header_ctrl_bus      );
    FORMAT_DESC_SET(pha_internal_desc_need_crc_removal             , copyMem_Qcn_desc.need_CRC_removal              );
    FORMAT_DESC_SET(pha_internal_desc_cut_through_id               , copyMem_Qcn_desc.cut_through_id                );
    FORMAT_DESC_SET(pha_internal_desc_packet_is_cut_through        , copyMem_Qcn_desc.packet_is_cut_through         );
    FORMAT_DESC_SET(pha_internal_desc_tc                           , copyMem_Qcn_desc.TC                            );
    FORMAT_DESC_SET(pha_internal_desc_header_hw_error_detected     , copyMem_Qcn_desc.header_HW_error_detected      );
    FORMAT_DESC_SET(pha_internal_desc_table_hw_error_detected      , copyMem_Qcn_desc.table_HW_error_detected       );
    FORMAT_DESC_SET(pha_internal_desc_ingress_byte_count           , copyMem_Qcn_desc.ingress_byte_count            );
    FORMAT_DESC_SET(pha_internal_desc_txq_queue_id                 , copyMem_Qcn_desc.txq_queue_id                  );
    FORMAT_DESC_SET(pha_internal_desc_cnc_address                  , copyMem_Qcn_desc.CNC_address                   );
    FORMAT_DESC_SET(pha_internal_desc_packet_hash_bits_6_5         , copyMem_Qcn_desc.packet_hash_bits_6_5          );
    FORMAT_DESC_SET(pha_internal_desc_dp                           , copyMem_Qcn_desc.dp                            );
    FORMAT_DESC_SET(pha_internal_desc_dei                          , copyMem_Qcn_desc.dei                           );
    FORMAT_DESC_SET(pha_internal_desc_up0                          , copyMem_Qcn_desc.up0                           );
    FORMAT_DESC_SET(pha_internal_desc_outer_l3_offset              , copyMem_Qcn_desc.Outer_l3_offset               );

    FORMAT_DESC_SET(pha_internal_desc_congested_port_number        , copyMem_Qcn_desc.congested_port_number         );
    FORMAT_DESC_SET(pha_internal_desc_qcn_q_fb                     , copyMem_Qcn_desc.qcn_q_fb                      );
    FORMAT_DESC_SET(pha_internal_desc_congested_queue_number       , copyMem_Qcn_desc.congested_queue_number        );
    FORMAT_DESC_SET(pha_internal_desc_packet_hash_4qcn_bits_4_0    , copyMem_Qcn_desc.packet_hash_4qcn_bits_4_0     );
    FORMAT_DESC_SET(pha_internal_desc_reserved_flags_4qcn          , copyMem_Qcn_desc.reserved_flags_4qcn           );
    FORMAT_DESC_SET(pha_internal_desc_mark_ecn_4qcn                , copyMem_Qcn_desc.mark_ecn_4qcn                 );
    FORMAT_DESC_SET(pha_internal_desc_qcn_q_delta                  , copyMem_Qcn_desc.qcn_q_delta                   );
    FORMAT_DESC_SET(pha_internal_desc_reserved                     , copyMem_Qcn_desc.reserved                      );
    FORMAT_DESC_SET(pha_internal_desc_orig_generated_qcn_prio      , copyMem_Qcn_desc.orig_generated_QCN_prio       );
    FORMAT_DESC_SET(pha_internal_desc_qcn_q_offset                 , copyMem_Qcn_desc.qcn_q_offset                  );

    /*copy words back to memory after 'bit field' manipulations that the FIRMWARE updated */
    desc_copyWordsFromMem(littleEndianMem,DESC_MEM_SIZE_IN_WORDS_CNS,memPtr);
}


/* this is BIG ENDIAN ORDER OF ALL BITS !!! */
typedef struct{
    char*   fieldName;/* info for the __LOG */
    GT_U32  endBit;  /* inclusive*/
    GT_U32  startBit;/* inclusive*/
}header_info;

static GT_U32  getHeaderInfo(header_info *infoPtr,GT_U32 *memPtr , GT_U32 fieldName)
{
    header_info*    currInfoPtr=&infoPtr[fieldName];
    GT_U32  endBit  = currInfoPtr->endBit;
    GT_U32  startBit = (currInfoPtr->startBit == SMAIN_NOT_VALID_CNS) ? endBit : currInfoPtr->startBit;
    GT_U32 numOfBits = (endBit-startBit)+1;
    GT_U32 bitIndex;
    GT_U32  complemet31;
    GT_U32  value;
    /*handle src*/
    complemet31 = 31 - (startBit & 0x1f);
    /* clear the 5 LSBits , and set with proper offset*/
    bitIndex = (startBit & 0xffffffe0) + (complemet31 - (numOfBits-1));

    value = snetFieldValueGet(memPtr,bitIndex,numOfBits);

    __LOG_NO_LOCATION_META_DATA(("[%s] get value [0x%x]\n" ,
        currInfoPtr->fieldName,value));

    return value;
}

static void    setHeaderInfo(header_info *infoPtr,GT_U32 *memPtr , GT_U32 fieldName,GT_U32 value)
{
    header_info*    currInfoPtr=&infoPtr[fieldName];
    GT_U32  endBit  = currInfoPtr->endBit;
    GT_U32  startBit = (currInfoPtr->startBit == SMAIN_NOT_VALID_CNS) ? endBit : currInfoPtr->startBit;
    GT_U32 numOfBits = (endBit-startBit)+1;
    GT_U32 bitIndex;
    GT_U32  complemet31;
    /*handle src*/
    complemet31 = 31 - (startBit & 0x1f);
    /* clear the 5 LSBits , and set with proper offset*/
    bitIndex = (startBit & 0xffffffe0) + (complemet31 - (numOfBits-1));

    snetFieldValueSet(memPtr,bitIndex,numOfBits,value);

    __LOG_NO_LOCATION_META_DATA(("[%s] set value [0x%x]\n" ,
        currInfoPtr->fieldName,value));

}

static header_info pha_ptp_header[] =
{
 /* ptp header*/
 {STR(pha_internal_ptp_header_transportSpecific       ),   3  ,  0  }     /* offset:0 */
,{STR(pha_internal_ptp_header_messageType             ),   7  ,  4  }     /* offset:4 */
,{STR(pha_internal_ptp_header_reserved0               ),   11 ,  8  }     /* offset:8 */
,{STR(pha_internal_ptp_header_versionPTP              ),   15 ,  12 }     /* offset:12 */
,{STR(pha_internal_ptp_header_messageLength           ),   31 ,  16 }     /* offset:16 */
,{STR(pha_internal_ptp_header_domainNumber            ),   39 ,  32 }     /* offset:32 */
,{STR(pha_internal_ptp_header_reserved1               ),   47 ,  40 }     /* offset:40 */
,{STR(pha_internal_ptp_header_flagField               ),   63 ,  48 }     /* offset:48 */
,{STR(pha_internal_ptp_header_correctionField_63_48   ),   79 ,  64 }     /* offset:64 */
,{STR(pha_internal_ptp_header_correctionField_47_32   ),   95 ,  80 }     /* offset:80 */
,{STR(pha_internal_ptp_header_correctionField_31_16   ),   111,  96 }     /* offset:96 */
,{STR(pha_internal_ptp_header_correctionField_15_0    ),   127,  112}     /* offset:112 */
,{STR(pha_internal_ptp_header_reserved2               ),   143,  128}     /* offset:128 */
,{STR(pha_internal_ptp_header_reserved3               ),   159,  144}     /* offset:144 */
,{STR(pha_internal_ptp_header_sourcePortIdentity_79_64),   175,  160}     /* offset:160 */
,{STR(pha_internal_ptp_header_sourcePortIdentity_63_48),   191,  176}     /* offset:176 */
,{STR(pha_internal_ptp_header_sourcePortIdentity_47_32),   207,  192}     /* offset:192 */
,{STR(pha_internal_ptp_header_sourcePortIdentity_31_16),   223,  208}     /* offset:208 */
,{STR(pha_internal_ptp_header_sourcePortIdentity_15_0 ),   239,  224}     /* offset:224 */
,{STR(pha_internal_ptp_header_sequenceId              ),   255,  240}     /* offset:240 */
,{STR(pha_internal_ptp_header_controlField            ),   263,  256}     /* offset:256 */
,{STR(pha_internal_ptp_header_logMessageInterval      ),   271,  264}     /* offset:264 */
};

typedef enum{
     pha_internal_ptp_header_transportSpecific             /* offset:0 */
    ,pha_internal_ptp_header_messageType                   /* offset:4 */
    ,pha_internal_ptp_header_reserved0                     /* offset:8 */
    ,pha_internal_ptp_header_versionPTP                    /* offset:12 */
    ,pha_internal_ptp_header_messageLength                 /* offset:16 */
    ,pha_internal_ptp_header_domainNumber                  /* offset:32 */
    ,pha_internal_ptp_header_reserved1                     /* offset:40 */
    ,pha_internal_ptp_header_flagField                     /* offset:48 */
    ,pha_internal_ptp_header_correctionField_63_48         /* offset:64 */
    ,pha_internal_ptp_header_correctionField_47_32         /* offset:80 */
    ,pha_internal_ptp_header_correctionField_31_16         /* offset:96 */
    ,pha_internal_ptp_header_correctionField_15_0          /* offset:112 */
    ,pha_internal_ptp_header_reserved2                     /* offset:128 */
    ,pha_internal_ptp_header_reserved3                     /* offset:144 */
    ,pha_internal_ptp_header_sourcePortIdentity_79_64      /* offset:160 */
    ,pha_internal_ptp_header_sourcePortIdentity_63_48      /* offset:176 */
    ,pha_internal_ptp_header_sourcePortIdentity_47_32      /* offset:192 */
    ,pha_internal_ptp_header_sourcePortIdentity_31_16      /* offset:208 */
    ,pha_internal_ptp_header_sourcePortIdentity_15_0       /* offset:224 */
    ,pha_internal_ptp_header_sequenceId                    /* offset:240 */
    ,pha_internal_ptp_header_controlField                  /* offset:256 */
    ,pha_internal_ptp_header_logMessageInterval            /* offset:264 */
}pha_internal_ptp_header_enum;

#define PTP_HERADER_GET(field , targetValue) \
    targetValue = getHeaderInfo(pha_ptp_header,littleEndianMem,field)

#define PTP_HERADER_SET(field , sourceValue) \
    setHeaderInfo(pha_ptp_header,littleEndianMem,field,sourceValue)

#define PTP_MEM_SIZE_IN_WORDS_CNS   ((sizeof(struct ptp_header)+3) / 4)/* supporting 34 bytes*/
static struct ptp_header copyMem_ptp_header;
static GT_BIT isPtpValid=0;
static GT_U32 savedAddr_ptp_header;
struct ptp_header* pipeMemCast_ptp_header(uint32_t address)
{
    uint32_t  littleEndianMem[PTP_MEM_SIZE_IN_WORDS_CNS];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    PTP_HERADER_GET(pha_internal_ptp_header_transportSpecific        ,  copyMem_ptp_header.transportSpecific          );
    PTP_HERADER_GET(pha_internal_ptp_header_messageType              ,  copyMem_ptp_header.messageType                );
    PTP_HERADER_GET(pha_internal_ptp_header_reserved0                ,  copyMem_ptp_header.reserved0                  );
    PTP_HERADER_GET(pha_internal_ptp_header_versionPTP               ,  copyMem_ptp_header.versionPTP                 );
    PTP_HERADER_GET(pha_internal_ptp_header_messageLength            ,  copyMem_ptp_header.messageLength              );
    PTP_HERADER_GET(pha_internal_ptp_header_domainNumber             ,  copyMem_ptp_header.domainNumber               );
    PTP_HERADER_GET(pha_internal_ptp_header_reserved1                ,  copyMem_ptp_header.reserved1                  );
    PTP_HERADER_GET(pha_internal_ptp_header_flagField                ,  copyMem_ptp_header.flagField                  );
    PTP_HERADER_GET(pha_internal_ptp_header_correctionField_63_48    ,  copyMem_ptp_header.correctionField_63_48      );
    PTP_HERADER_GET(pha_internal_ptp_header_correctionField_47_32    ,  copyMem_ptp_header.correctionField_47_32      );
    PTP_HERADER_GET(pha_internal_ptp_header_correctionField_31_16    ,  copyMem_ptp_header.correctionField_31_16      );
    PTP_HERADER_GET(pha_internal_ptp_header_correctionField_15_0     ,  copyMem_ptp_header.correctionField_15_0       );
    PTP_HERADER_GET(pha_internal_ptp_header_reserved2                ,  copyMem_ptp_header.reserved2                  );
    PTP_HERADER_GET(pha_internal_ptp_header_reserved3                ,  copyMem_ptp_header.reserved3                  );
    PTP_HERADER_GET(pha_internal_ptp_header_sourcePortIdentity_79_64 ,  copyMem_ptp_header.sourcePortIdentity_79_64   );
    PTP_HERADER_GET(pha_internal_ptp_header_sourcePortIdentity_63_48 ,  copyMem_ptp_header.sourcePortIdentity_63_48   );
    PTP_HERADER_GET(pha_internal_ptp_header_sourcePortIdentity_47_32 ,  copyMem_ptp_header.sourcePortIdentity_47_32   );
    PTP_HERADER_GET(pha_internal_ptp_header_sourcePortIdentity_31_16 ,  copyMem_ptp_header.sourcePortIdentity_31_16   );
    PTP_HERADER_GET(pha_internal_ptp_header_sourcePortIdentity_15_0  ,  copyMem_ptp_header.sourcePortIdentity_15_0    );
    PTP_HERADER_GET(pha_internal_ptp_header_sequenceId               ,  copyMem_ptp_header.sequenceId                 );
    PTP_HERADER_GET(pha_internal_ptp_header_controlField             ,  copyMem_ptp_header.controlField               );
    PTP_HERADER_GET(pha_internal_ptp_header_logMessageInterval       ,  copyMem_ptp_header.logMessageInterval         );

    savedAddr_ptp_header = address;
    isPtpValid = 1;

    return &copyMem_ptp_header;
}

void pipeMemApply_ptp_header(uint32_t address/*caller need to use saved address*/)
{
    uint32_t  littleEndianMem[PTP_MEM_SIZE_IN_WORDS_CNS];

    /*fix clockwork : 'littleEndianMem' array elements are used uninitialized in this function.*/
    memset(littleEndianMem,0,sizeof(littleEndianMem));

    PTP_HERADER_SET(pha_internal_ptp_header_transportSpecific        ,  copyMem_ptp_header.transportSpecific          );
    PTP_HERADER_SET(pha_internal_ptp_header_messageType              ,  copyMem_ptp_header.messageType                );
    PTP_HERADER_SET(pha_internal_ptp_header_reserved0                ,  copyMem_ptp_header.reserved0                  );
    PTP_HERADER_SET(pha_internal_ptp_header_versionPTP               ,  copyMem_ptp_header.versionPTP                 );
    PTP_HERADER_SET(pha_internal_ptp_header_messageLength            ,  copyMem_ptp_header.messageLength              );
    PTP_HERADER_SET(pha_internal_ptp_header_domainNumber             ,  copyMem_ptp_header.domainNumber               );
    PTP_HERADER_SET(pha_internal_ptp_header_reserved1                ,  copyMem_ptp_header.reserved1                  );
    PTP_HERADER_SET(pha_internal_ptp_header_flagField                ,  copyMem_ptp_header.flagField                  );
    PTP_HERADER_SET(pha_internal_ptp_header_correctionField_63_48    ,  copyMem_ptp_header.correctionField_63_48      );
    PTP_HERADER_SET(pha_internal_ptp_header_correctionField_47_32    ,  copyMem_ptp_header.correctionField_47_32      );
    PTP_HERADER_SET(pha_internal_ptp_header_correctionField_31_16    ,  copyMem_ptp_header.correctionField_31_16      );
    PTP_HERADER_SET(pha_internal_ptp_header_correctionField_15_0     ,  copyMem_ptp_header.correctionField_15_0       );
    PTP_HERADER_SET(pha_internal_ptp_header_reserved2                ,  copyMem_ptp_header.reserved2                  );
    PTP_HERADER_SET(pha_internal_ptp_header_reserved3                ,  copyMem_ptp_header.reserved3                  );
    PTP_HERADER_SET(pha_internal_ptp_header_sourcePortIdentity_79_64 ,  copyMem_ptp_header.sourcePortIdentity_79_64   );
    PTP_HERADER_SET(pha_internal_ptp_header_sourcePortIdentity_63_48 ,  copyMem_ptp_header.sourcePortIdentity_63_48   );
    PTP_HERADER_SET(pha_internal_ptp_header_sourcePortIdentity_47_32 ,  copyMem_ptp_header.sourcePortIdentity_47_32   );
    PTP_HERADER_SET(pha_internal_ptp_header_sourcePortIdentity_31_16 ,  copyMem_ptp_header.sourcePortIdentity_31_16   );
    PTP_HERADER_SET(pha_internal_ptp_header_sourcePortIdentity_15_0  ,  copyMem_ptp_header.sourcePortIdentity_15_0    );
    PTP_HERADER_SET(pha_internal_ptp_header_sequenceId               ,  copyMem_ptp_header.sequenceId                 );
    PTP_HERADER_SET(pha_internal_ptp_header_controlField             ,  copyMem_ptp_header.controlField               );
    PTP_HERADER_SET(pha_internal_ptp_header_logMessageInterval       ,  copyMem_ptp_header.logMessageInterval         );

    /*copy bytes back to memory after 'bit field' manipulations that the FIRMWARE updated */
    copyBytesFromHeader(GT_FALSE,address,littleEndianMem,sizeof(struct ptp_header));
}


static header_info pha_ipv4_header[] =
{
 /* ipv4 header*/
 {STR(pha_internal_ipv4_header_version            ),     3 , 0  }   /* offset:0 */
,{STR(pha_internal_ipv4_header_ihl                ),     7 , 4  }   /* offset:4 */
,{STR(pha_internal_ipv4_header_dscp               ),   13  , 8  }   /* offset:8 */
,{STR(pha_internal_ipv4_header_ecn                ),   15  , 14 }   /* offset:14 */
,{STR(pha_internal_ipv4_header_total_length       ),   31  , 16 }   /* offset:16 */
,{STR(pha_internal_ipv4_header_identification     ),   47  , 32 }   /* offset:32 */
,{STR(pha_internal_ipv4_header_flag_reserved      ),   48  , 48 }   /* offset:48 */
,{STR(pha_internal_ipv4_header_flag_dont_fragment ),   49  , 49 }   /* offset:49 */
,{STR(pha_internal_ipv4_header_flag_more_fragment ),   50  , 50 }   /* offset:50 */
,{STR(pha_internal_ipv4_header_fragment_offset    ),   63  , 51 }   /* offset:51 */
,{STR(pha_internal_ipv4_header_ttl                ),   71  , 64 }   /* offset:64 */
,{STR(pha_internal_ipv4_header_protocol           ),   79  , 72 }   /* offset:72 */
,{STR(pha_internal_ipv4_header_header_checksum    ),   95  , 80 }   /* offset:80 */
,{STR(pha_internal_ipv4_header_sip_high           ),   111 , 96 }   /* offset:96 */
,{STR(pha_internal_ipv4_header_sip_low            ),   127 , 112}   /* offset:112 */
,{STR(pha_internal_ipv4_header_dip_high           ),   143 , 128}   /* offset:128 */
,{STR(pha_internal_ipv4_header_dip_low            ),   159 , 144}   /* offset:144 */
};

typedef enum{
    pha_internal_ipv4_header_version,
    pha_internal_ipv4_header_ihl,
    pha_internal_ipv4_header_dscp,
    pha_internal_ipv4_header_ecn,
    pha_internal_ipv4_header_total_length,
    pha_internal_ipv4_header_identification,
    pha_internal_ipv4_header_flag_reserved,
    pha_internal_ipv4_header_flag_dont_fragment,
    pha_internal_ipv4_header_flag_more_fragment,
    pha_internal_ipv4_header_fragment_offset,
    pha_internal_ipv4_header_ttl,
    pha_internal_ipv4_header_protocol,
    pha_internal_ipv4_header_header_checksum,
    pha_internal_ipv4_header_sip_high,
    pha_internal_ipv4_header_sip_low,
    pha_internal_ipv4_header_dip_high,
    pha_internal_ipv4_header_dip_low
}pha_internal_ipv4_header_enum;

#define IPV4_HERADER_GET(field , targetValue) \
    targetValue = getHeaderInfo(pha_ipv4_header,littleEndianMem,field)

#define IPV4_HERADER_SET(field , sourceValue) \
    setHeaderInfo(pha_ipv4_header,littleEndianMem,field,sourceValue)


#define IPV4_MEM_SIZE_IN_WORDS_CNS  5

static struct IPv4_Header copyMem_IPv4_Header;
static GT_BIT isIpv4Valid=0;
static GT_U32 savedAddr_IPv4_Header;
struct IPv4_Header* pipeMemCast_IPv4_Header(uint32_t address)
{
    uint32_t  littleEndianMem[IPV4_MEM_SIZE_IN_WORDS_CNS];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    IPV4_HERADER_GET(pha_internal_ipv4_header_version                  , copyMem_IPv4_Header.version                    );
    IPV4_HERADER_GET(pha_internal_ipv4_header_ihl                      , copyMem_IPv4_Header.ihl                        );
    IPV4_HERADER_GET(pha_internal_ipv4_header_dscp                     , copyMem_IPv4_Header.dscp                       );
    IPV4_HERADER_GET(pha_internal_ipv4_header_ecn                      , copyMem_IPv4_Header.ecn                        );
    IPV4_HERADER_GET(pha_internal_ipv4_header_total_length             , copyMem_IPv4_Header.total_length               );
    IPV4_HERADER_GET(pha_internal_ipv4_header_identification           , copyMem_IPv4_Header.identification             );
    IPV4_HERADER_GET(pha_internal_ipv4_header_flag_reserved            , copyMem_IPv4_Header.flag_reserved              );
    IPV4_HERADER_GET(pha_internal_ipv4_header_flag_dont_fragment       , copyMem_IPv4_Header.flag_dont_fragment         );
    IPV4_HERADER_GET(pha_internal_ipv4_header_flag_more_fragment       , copyMem_IPv4_Header.flag_more_fragment         );
    IPV4_HERADER_GET(pha_internal_ipv4_header_fragment_offset          , copyMem_IPv4_Header.fragment_offset            );
    IPV4_HERADER_GET(pha_internal_ipv4_header_ttl                      , copyMem_IPv4_Header.ttl                        );
    IPV4_HERADER_GET(pha_internal_ipv4_header_protocol                 , copyMem_IPv4_Header.protocol                   );
    IPV4_HERADER_GET(pha_internal_ipv4_header_header_checksum          , copyMem_IPv4_Header.header_checksum            );
    IPV4_HERADER_GET(pha_internal_ipv4_header_sip_high                 , copyMem_IPv4_Header.sip_high                   );
    IPV4_HERADER_GET(pha_internal_ipv4_header_sip_low                  , copyMem_IPv4_Header.sip_low                    );
    IPV4_HERADER_GET(pha_internal_ipv4_header_dip_high                 , copyMem_IPv4_Header.dip_high                   );
    IPV4_HERADER_GET(pha_internal_ipv4_header_dip_low                  , copyMem_IPv4_Header.dip_low                    );

    isIpv4Valid = (copyMem_IPv4_Header.version == IPv4_VER) ? 1 : 0;
    savedAddr_IPv4_Header = address;

    return &copyMem_IPv4_Header;
}

void pipeMemApply_IPv4_Header(uint32_t address/*caller need to use saved address*/)
{
    uint32_t  littleEndianMem[IPV4_MEM_SIZE_IN_WORDS_CNS];

    /*fix clockwork : 'littleEndianMem' array elements are used uninitialized in this function.*/
    memset(littleEndianMem,0,sizeof(littleEndianMem));

    IPV4_HERADER_SET(pha_internal_ipv4_header_version                  , copyMem_IPv4_Header.version                    );
    IPV4_HERADER_SET(pha_internal_ipv4_header_ihl                      , copyMem_IPv4_Header.ihl                        );
    IPV4_HERADER_SET(pha_internal_ipv4_header_dscp                     , copyMem_IPv4_Header.dscp                       );
    IPV4_HERADER_SET(pha_internal_ipv4_header_ecn                      , copyMem_IPv4_Header.ecn                        );
    IPV4_HERADER_SET(pha_internal_ipv4_header_total_length             , copyMem_IPv4_Header.total_length               );
    IPV4_HERADER_SET(pha_internal_ipv4_header_identification           , copyMem_IPv4_Header.identification             );
    IPV4_HERADER_SET(pha_internal_ipv4_header_flag_reserved            , copyMem_IPv4_Header.flag_reserved              );
    IPV4_HERADER_SET(pha_internal_ipv4_header_flag_dont_fragment       , copyMem_IPv4_Header.flag_dont_fragment         );
    IPV4_HERADER_SET(pha_internal_ipv4_header_flag_more_fragment       , copyMem_IPv4_Header.flag_more_fragment         );
    IPV4_HERADER_SET(pha_internal_ipv4_header_fragment_offset          , copyMem_IPv4_Header.fragment_offset            );
    IPV4_HERADER_SET(pha_internal_ipv4_header_ttl                      , copyMem_IPv4_Header.ttl                        );
    IPV4_HERADER_SET(pha_internal_ipv4_header_protocol                 , copyMem_IPv4_Header.protocol                   );
    IPV4_HERADER_SET(pha_internal_ipv4_header_header_checksum          , copyMem_IPv4_Header.header_checksum            );
    IPV4_HERADER_SET(pha_internal_ipv4_header_sip_high                 , copyMem_IPv4_Header.sip_high                   );
    IPV4_HERADER_SET(pha_internal_ipv4_header_sip_low                  , copyMem_IPv4_Header.sip_low                    );
    IPV4_HERADER_SET(pha_internal_ipv4_header_dip_high                 , copyMem_IPv4_Header.dip_high                   );
    IPV4_HERADER_SET(pha_internal_ipv4_header_dip_low                  , copyMem_IPv4_Header.dip_low                    );

    /*copy bytes back to memory after 'bit field' manipulations that the FIRMWARE updated */
    copyBytesFromHeader(GT_FALSE,address,littleEndianMem,sizeof(struct IPv4_Header));
}


static header_info pha_ipv6_header[] =
{
 /* ipv6 header*/
 {STR(pha_internal_ipv6_header_version           ),    3    ,  0    }     /* offset:0 */
,{STR(pha_internal_ipv6_header_dscp              ),    9    ,  4    }     /* offset:4 */
,{STR(pha_internal_ipv6_header_ecn               ),    11   ,   10  }     /* offset:10 */
,{STR(pha_internal_ipv6_header_flow_label_19_16  ),    15   ,   12  }     /* offset:12 */
,{STR(pha_internal_ipv6_header_flow_label_15_0   ),    31   ,   16  }     /* offset:16 */
,{STR(pha_internal_ipv6_header_payload_length    ),    47   ,   32  }     /* offset:32 */
,{STR(pha_internal_ipv6_header_next_header       ),    55   ,   48  }     /* offset:48 */
,{STR(pha_internal_ipv6_header_hop_limit         ),    63   ,   56  }     /* offset:56 */
,{STR(pha_internal_ipv6_header_sip0_high         ),    79   ,   64  }     /* offset:64 */
,{STR(pha_internal_ipv6_header_sip0_low          ),    95   ,   80  }     /* offset:80 */
,{STR(pha_internal_ipv6_header_sip1_high         ),    111  ,    96 }     /* offset:96 */
,{STR(pha_internal_ipv6_header_sip1_low          ),    127  ,   112 }     /* offset:112 */
,{STR(pha_internal_ipv6_header_sip2_high         ),    143  ,   128 }     /* offset:128 */
,{STR(pha_internal_ipv6_header_sip2_low          ),    159  ,   144 }     /* offset:144 */
,{STR(pha_internal_ipv6_header_sip3_high         ),    175  ,   160 }     /* offset:160 */
,{STR(pha_internal_ipv6_header_sip3_low          ),    191  ,   176 }     /* offset:176 */
,{STR(pha_internal_ipv6_header_dip0_high         ),    207  ,   192 }     /* offset:192 */
,{STR(pha_internal_ipv6_header_dip0_low          ),    223  ,   208 }     /* offset:208 */
,{STR(pha_internal_ipv6_header_dip1_high         ),    239  ,   224 }     /* offset:224 */
,{STR(pha_internal_ipv6_header_dip1_low          ),    255  ,   240 }     /* offset:240 */
,{STR(pha_internal_ipv6_header_dip2_high         ),    271  ,   256 }     /* offset:256 */
,{STR(pha_internal_ipv6_header_dip2_low          ),    287  ,   272 }     /* offset:272 */
,{STR(pha_internal_ipv6_header_dip3_high         ),    303  ,   288 }     /* offset:288 */
,{STR(pha_internal_ipv6_header_dip3_low          ),    319  ,   304 }     /* offset:304 */
};

typedef enum{
     pha_internal_ipv6_header_version
    ,pha_internal_ipv6_header_dscp
    ,pha_internal_ipv6_header_ecn
    ,pha_internal_ipv6_header_flow_label_19_16
    ,pha_internal_ipv6_header_flow_label_15_0
    ,pha_internal_ipv6_header_payload_length
    ,pha_internal_ipv6_header_next_header
    ,pha_internal_ipv6_header_hop_limit
    ,pha_internal_ipv6_header_sip0_high
    ,pha_internal_ipv6_header_sip0_low
    ,pha_internal_ipv6_header_sip1_high
    ,pha_internal_ipv6_header_sip1_low
    ,pha_internal_ipv6_header_sip2_high
    ,pha_internal_ipv6_header_sip2_low
    ,pha_internal_ipv6_header_sip3_high
    ,pha_internal_ipv6_header_sip3_low
    ,pha_internal_ipv6_header_dip0_high
    ,pha_internal_ipv6_header_dip0_low
    ,pha_internal_ipv6_header_dip1_high
    ,pha_internal_ipv6_header_dip1_low
    ,pha_internal_ipv6_header_dip2_high
    ,pha_internal_ipv6_header_dip2_low
    ,pha_internal_ipv6_header_dip3_high
    ,pha_internal_ipv6_header_dip3_low
}pha_internal_ipv6_header_enum ;


#define IPV6_HERADER_GET(field , targetValue) \
    targetValue = getHeaderInfo(pha_ipv6_header,littleEndianMem,field)

#define IPV6_HERADER_SET(field , sourceValue) \
    setHeaderInfo(pha_ipv6_header,littleEndianMem,field,sourceValue)

#define IPV6_MEM_SIZE_IN_WORDS_CNS  10

static struct IPv6_Header copyMem_IPv6_Header;
static GT_BIT isIpv6Valid=0;
static GT_U32 savedAddr_IPv6_Header;
struct IPv6_Header* pipeMemCast_IPv6_Header(uint32_t address)
{
    uint32_t  littleEndianMem[IPV6_MEM_SIZE_IN_WORDS_CNS];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    IPV6_HERADER_GET(pha_internal_ipv6_header_version           ,     copyMem_IPv6_Header.version           );
    IPV6_HERADER_GET(pha_internal_ipv6_header_dscp              ,     copyMem_IPv6_Header.dscp              );
    IPV6_HERADER_GET(pha_internal_ipv6_header_ecn               ,     copyMem_IPv6_Header.ecn               );
    IPV6_HERADER_GET(pha_internal_ipv6_header_flow_label_19_16  ,     copyMem_IPv6_Header.flow_label_19_16  );
    IPV6_HERADER_GET(pha_internal_ipv6_header_flow_label_15_0   ,     copyMem_IPv6_Header.flow_label_15_0   );
    IPV6_HERADER_GET(pha_internal_ipv6_header_payload_length    ,     copyMem_IPv6_Header.payload_length    );
    IPV6_HERADER_GET(pha_internal_ipv6_header_next_header       ,     copyMem_IPv6_Header.next_header       );
    IPV6_HERADER_GET(pha_internal_ipv6_header_hop_limit         ,     copyMem_IPv6_Header.hop_limit         );
    IPV6_HERADER_GET(pha_internal_ipv6_header_sip0_high         ,     copyMem_IPv6_Header.sip0_high         );
    IPV6_HERADER_GET(pha_internal_ipv6_header_sip0_low          ,     copyMem_IPv6_Header.sip0_low          );
    IPV6_HERADER_GET(pha_internal_ipv6_header_sip1_high         ,     copyMem_IPv6_Header.sip1_high         );
    IPV6_HERADER_GET(pha_internal_ipv6_header_sip1_low          ,     copyMem_IPv6_Header.sip1_low          );
    IPV6_HERADER_GET(pha_internal_ipv6_header_sip2_high         ,     copyMem_IPv6_Header.sip2_high         );
    IPV6_HERADER_GET(pha_internal_ipv6_header_sip2_low          ,     copyMem_IPv6_Header.sip2_low          );
    IPV6_HERADER_GET(pha_internal_ipv6_header_sip3_high         ,     copyMem_IPv6_Header.sip3_high         );
    IPV6_HERADER_GET(pha_internal_ipv6_header_sip3_low          ,     copyMem_IPv6_Header.sip3_low          );
    IPV6_HERADER_GET(pha_internal_ipv6_header_dip0_high         ,     copyMem_IPv6_Header.dip0_high         );
    IPV6_HERADER_GET(pha_internal_ipv6_header_dip0_low          ,     copyMem_IPv6_Header.dip0_low          );
    IPV6_HERADER_GET(pha_internal_ipv6_header_dip1_high         ,     copyMem_IPv6_Header.dip1_high         );
    IPV6_HERADER_GET(pha_internal_ipv6_header_dip1_low          ,     copyMem_IPv6_Header.dip1_low          );
    IPV6_HERADER_GET(pha_internal_ipv6_header_dip2_high         ,     copyMem_IPv6_Header.dip2_high         );
    IPV6_HERADER_GET(pha_internal_ipv6_header_dip2_low          ,     copyMem_IPv6_Header.dip2_low          );
    IPV6_HERADER_GET(pha_internal_ipv6_header_dip3_high         ,     copyMem_IPv6_Header.dip3_high         );
    IPV6_HERADER_GET(pha_internal_ipv6_header_dip3_low          ,     copyMem_IPv6_Header.dip3_low          );

    isIpv6Valid = (copyMem_IPv6_Header.version != IPv4_VER) ? 1 : 0;
    savedAddr_IPv6_Header = address;

    return &copyMem_IPv6_Header;

}

void pipeMemApply_IPv6_Header(uint32_t address/*caller need to use saved address*/)
{
    uint32_t  littleEndianMem[IPV6_MEM_SIZE_IN_WORDS_CNS];

    /*fix clockwork : 'littleEndianMem' array elements are used uninitialized in this function.*/
    memset(littleEndianMem,0,sizeof(littleEndianMem));

    IPV6_HERADER_SET(pha_internal_ipv6_header_version           ,     copyMem_IPv6_Header.version           );
    IPV6_HERADER_SET(pha_internal_ipv6_header_dscp              ,     copyMem_IPv6_Header.dscp              );
    IPV6_HERADER_SET(pha_internal_ipv6_header_ecn               ,     copyMem_IPv6_Header.ecn               );
    IPV6_HERADER_SET(pha_internal_ipv6_header_flow_label_19_16  ,     copyMem_IPv6_Header.flow_label_19_16  );
    IPV6_HERADER_SET(pha_internal_ipv6_header_flow_label_15_0   ,     copyMem_IPv6_Header.flow_label_15_0   );
    IPV6_HERADER_SET(pha_internal_ipv6_header_payload_length    ,     copyMem_IPv6_Header.payload_length    );
    IPV6_HERADER_SET(pha_internal_ipv6_header_next_header       ,     copyMem_IPv6_Header.next_header       );
    IPV6_HERADER_SET(pha_internal_ipv6_header_hop_limit         ,     copyMem_IPv6_Header.hop_limit         );
    IPV6_HERADER_SET(pha_internal_ipv6_header_sip0_high         ,     copyMem_IPv6_Header.sip0_high         );
    IPV6_HERADER_SET(pha_internal_ipv6_header_sip0_low          ,     copyMem_IPv6_Header.sip0_low          );
    IPV6_HERADER_SET(pha_internal_ipv6_header_sip1_high         ,     copyMem_IPv6_Header.sip1_high         );
    IPV6_HERADER_SET(pha_internal_ipv6_header_sip1_low          ,     copyMem_IPv6_Header.sip1_low          );
    IPV6_HERADER_SET(pha_internal_ipv6_header_sip2_high         ,     copyMem_IPv6_Header.sip2_high         );
    IPV6_HERADER_SET(pha_internal_ipv6_header_sip2_low          ,     copyMem_IPv6_Header.sip2_low          );
    IPV6_HERADER_SET(pha_internal_ipv6_header_sip3_high         ,     copyMem_IPv6_Header.sip3_high         );
    IPV6_HERADER_SET(pha_internal_ipv6_header_sip3_low          ,     copyMem_IPv6_Header.sip3_low          );
    IPV6_HERADER_SET(pha_internal_ipv6_header_dip0_high         ,     copyMem_IPv6_Header.dip0_high         );
    IPV6_HERADER_SET(pha_internal_ipv6_header_dip0_low          ,     copyMem_IPv6_Header.dip0_low          );
    IPV6_HERADER_SET(pha_internal_ipv6_header_dip1_high         ,     copyMem_IPv6_Header.dip1_high         );
    IPV6_HERADER_SET(pha_internal_ipv6_header_dip1_low          ,     copyMem_IPv6_Header.dip1_low          );
    IPV6_HERADER_SET(pha_internal_ipv6_header_dip2_high         ,     copyMem_IPv6_Header.dip2_high         );
    IPV6_HERADER_SET(pha_internal_ipv6_header_dip2_low          ,     copyMem_IPv6_Header.dip2_low          );
    IPV6_HERADER_SET(pha_internal_ipv6_header_dip3_high         ,     copyMem_IPv6_Header.dip3_high         );
    IPV6_HERADER_SET(pha_internal_ipv6_header_dip3_low          ,     copyMem_IPv6_Header.dip3_low          );

    /*copy bytes back to memory after 'bit field' manipulations that the FIRMWARE updated */
    copyBytesFromHeader(GT_FALSE,address,littleEndianMem,sizeof(struct IPv6_Header));
}

static header_info pha_internal_in_hdr[] =
{
 /* packet header*/
 {STR(pha_internal_in_hdr_expansion_space_reserved_0 ),                             31 ,    0 }/* offset:  0 */
,{STR(pha_internal_in_hdr_expansion_space_reserved_1 ),                             63 ,   32 }/* offset: 32 */
,{STR(pha_internal_in_hdr_expansion_space_reserved_2 ),                             95 ,   64 }/* offset: 64 */
,{STR(pha_internal_in_hdr_expansion_space_reserved_3 ),                            127 ,   96 }/* offset: 96 */
,{STR(pha_internal_in_hdr_expansion_space_reserved_4 ),                            159 ,  128 }/* offset:128 */


,{STR(pha_internal_in_hdr_pre_da_tag_reserved        ),                            167 ,  160 }/* offset:160 */
,{STR(pha_internal_in_hdr_pre_da_tag_port_info       ),                            175 ,  168 }/* offset:168 */


,{STR(pha_internal_in_hdr_mac_header_mac_da_47_32    ),                            175 ,  160 }/* offset:160 */
,{STR(pha_internal_in_hdr_mac_header_mac_da_31_16    ),                            191 ,  176 }/* offset:176 */
,{STR(pha_internal_in_hdr_mac_header_mac_da_15_0     ),                            207 ,  192 }/* offset:192 */
,{STR(pha_internal_in_hdr_mac_header_mac_sa_47_32    ),                            223 ,  208 }/* offset:208 */
,{STR(pha_internal_in_hdr_mac_header_mac_sa_31_16    ),                            239 ,  224 }/* offset:224 */
,{STR(pha_internal_in_hdr_mac_header_mac_sa_15_0     ),                            255 ,  240 }/* offset:240 */


,{STR(pha_internal_in_hdr_etag_TPID                  ),/* etag after mac addr */   271 ,  256 }/* offset:256 */
,{STR(pha_internal_in_hdr_etag_E_PCP                 ),/* etag after mac addr */   274 ,  272 }/* offset:272 */
,{STR(pha_internal_in_hdr_etag_E_DEI                 ),/* etag after mac addr */   275 ,  275 }/* offset:275 */
,{STR(pha_internal_in_hdr_etag_Ingress_E_CID_base    ),/* etag after mac addr */   287 ,  276 }/* offset:276 */
,{STR(pha_internal_in_hdr_etag_IPL_Direction         ),/* etag after mac addr */   288 ,  288 }/* offset:288 */
,{STR(pha_internal_in_hdr_etag_Reserved              ),/* etag after mac addr */   289 ,  289 }/* offset:289 */
,{STR(pha_internal_in_hdr_etag_GRP                   ),/* etag after mac addr */   291 ,  290 }/* offset:290 */
,{STR(pha_internal_in_hdr_etag_E_CID_base            ),/* etag after mac addr */   303 ,  292 }/* offset:292 */
,{STR(pha_internal_in_hdr_etag_Ingress_E_CID_ext     ),/* etag after mac addr */   311 ,  304 }/* offset:304 */
,{STR(pha_internal_in_hdr_etag_E_CID_ext             ),/* etag after mac addr */   319 ,  312 }/* offset:312 */


,{STR(pha_internal_in_hdr_vlan_after_Etag_TPID    ),/* vlan after etag */          335 ,  320 }/* offset:320 */
,{STR(pha_internal_in_hdr_vlan_after_Etag_up      ),/* vlan after etag */          338 ,  336 }/* offset:336 */
,{STR(pha_internal_in_hdr_vlan_after_Etag_cfi     ),/* vlan after etag */          339 ,  339 }/* offset:339 */
,{STR(pha_internal_in_hdr_vlan_after_Etag_vid     ),/* vlan after etag */          351 ,  340 }/* offset:340 */


,{STR(pha_internal_in_hdr_extDSA_fwd_w0_tagCommand   ),   /* dsa after mac addr */       257 ,256}/* offset:256 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w0_srcTagged    ),   /* dsa after mac addr */       258 ,258}/* offset:258 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w0_srcDev       ),   /* dsa after mac addr */       263 ,259}/* offset:259 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w0_SrcPort_4_0  ),   /* dsa after mac addr */       268 ,264}/* offset:264 */ /* validity: srcIsTrunk == 0 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w0_srcIsTrunk   ),   /* dsa after mac addr */       269 ,269}/* offset:269 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w0_core_id_2    ),   /* dsa after mac addr */       270 ,270}/* offset:270 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w0_cfi          ),   /* dsa after mac addr */       271 ,271}/* offset:271 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w0_up           ),   /* dsa after mac addr */       274 ,272}/* offset:272 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w0_extended     ),   /* dsa after mac addr */       275 ,275}/* offset:275 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w0_vid          ),   /* dsa after mac addr */       287 ,276}/* offset:276 */

,{STR(pha_internal_in_hdr_extDSA_fwd_w1_Extend       ),   /* dsa after mac addr */       288 ,288}/* offset:288 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_SrcPort_6_5  ),   /* dsa after mac addr */       290 ,289}/* offset:289 */ /* validity: srcIsTrunk == 0 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_EgressFilterRegistered), /* dsa after mac addr */291 ,291}/* offset:291 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_DropOnSource  ),  /* dsa after mac addr */       292 ,292}/* offset:292 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_PacketIsLooped),  /* dsa after mac addr */       293 ,293}/* offset:293 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_routed        ),  /* dsa after mac addr */       294 ,294}/* offset:294 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_Src_ID        ),  /* dsa after mac addr */       299 ,295}/* offset:295 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_QosProfile    ),  /* dsa after mac addr */       306 ,300}/* offset:300 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_use_vidx      ),  /* dsa after mac addr */       307 ,307}/* offset:307 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_Reserved      ),  /* dsa after mac addr */       308 ,308}/* offset:308 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_TrgPort       ),  /* dsa after mac addr */       314 ,309}/* offset:309 */
,{STR(pha_internal_in_hdr_extDSA_fwd_w1_TrgDev        ),  /* dsa after mac addr */       319 ,315}/* offset:315 */


,{STR(pha_internal_in_hdr_DSA_from_cpu_tagCommand     ),  /* dsa after mac addr */       256 ,256} /* offset:256 */
,{STR(pha_internal_in_hdr_DSA_from_cpu_TrgTagged      ),  /* dsa after mac addr */       258 ,258} /* offset:258 */
,{STR(pha_internal_in_hdr_DSA_from_cpu_TrgDev         ),  /* dsa after mac addr */       259 ,259} /* offset:259 */
,{STR(pha_internal_in_hdr_DSA_from_cpu_TrgPhyPort_4_0 ),  /* dsa after mac addr */       264 ,264} /* offset:264 */
,{STR(pha_internal_in_hdr_DSA_from_cpu_use_eVIDX      ),  /* dsa after mac addr */       269 ,269} /* offset:269 */
,{STR(pha_internal_in_hdr_DSA_from_cpu_TC0            ),  /* dsa after mac addr */       270 ,270} /* offset:270 */
,{STR(pha_internal_in_hdr_DSA_from_cpu_cfi            ),  /* dsa after mac addr */       271 ,271} /* offset:271 */
,{STR(pha_internal_in_hdr_DSA_from_cpu_up             ),  /* dsa after mac addr */       272 ,272} /* offset:272 */
,{STR(pha_internal_in_hdr_DSA_from_cpu_Extend         ),  /* dsa after mac addr */       275 ,275} /* offset:275 */
,{STR(pha_internal_in_hdr_DSA_from_cpu_eVLAN          ),  /* dsa after mac addr */       276 ,276} /* offset:276 */


};

typedef enum{
     pha_internal_in_hdr_expansion_space_reserved_0                                /* offset:  0 */
    ,pha_internal_in_hdr_expansion_space_reserved_1                                /* offset: 32 */
    ,pha_internal_in_hdr_expansion_space_reserved_2                                /* offset: 64 */
    ,pha_internal_in_hdr_expansion_space_reserved_3                                /* offset: 96 */
    ,pha_internal_in_hdr_expansion_space_reserved_4                                /* offset:128 */

    ,pha_internal_in_hdr_pre_da_tag_reserved                                       /* offset:160 */
    ,pha_internal_in_hdr_pre_da_tag_port_info                                      /* offset:168 */

    ,pha_internal_in_hdr_mac_header_mac_da_47_32                                   /* offset:160 */
    ,pha_internal_in_hdr_mac_header_mac_da_31_16                                   /* offset:176 */
    ,pha_internal_in_hdr_mac_header_mac_da_15_0                                    /* offset:192 */
    ,pha_internal_in_hdr_mac_header_mac_sa_47_32                                   /* offset:208 */
    ,pha_internal_in_hdr_mac_header_mac_sa_31_16                                   /* offset:224 */
    ,pha_internal_in_hdr_mac_header_mac_sa_15_0                                    /* offset:240 */


    ,pha_internal_in_hdr_etag_TPID                  /* etag after mac addr */      /* offset:256 */
    ,pha_internal_in_hdr_etag_E_PCP                 /* etag after mac addr */      /* offset:272 */
    ,pha_internal_in_hdr_etag_E_DEI                 /* etag after mac addr */      /* offset:275 */
    ,pha_internal_in_hdr_etag_Ingress_E_CID_base    /* etag after mac addr */      /* offset:276 */
    ,pha_internal_in_hdr_etag_IPL_Direction         /* etag after mac addr */      /* offset:287 */
    ,pha_internal_in_hdr_etag_Reserved              /* etag after mac addr */      /* offset:288 */
    ,pha_internal_in_hdr_etag_GRP                   /* etag after mac addr */      /* offset:290 */
    ,pha_internal_in_hdr_etag_E_CID_base            /* etag after mac addr */      /* offset:292 */
    ,pha_internal_in_hdr_etag_Ingress_E_CID_ext     /* etag after mac addr */      /* offset:304 */
    ,pha_internal_in_hdr_etag_E_CID_ext             /* etag after mac addr */      /* offset:312 */


    ,pha_internal_in_hdr_vlan_after_Etag_TPID    /* vlan after etag */             /* offset:320 */
    ,pha_internal_in_hdr_vlan_after_Etag_up      /* vlan after etag */             /* offset:336 */
    ,pha_internal_in_hdr_vlan_after_Etag_cfi     /* vlan after etag */             /* offset:339 */
    ,pha_internal_in_hdr_vlan_after_Etag_vid     /* vlan after etag */             /* offset:340 */


    ,pha_internal_in_hdr_extDSA_fwd_w0_tagCommand      /* dsa after mac addr */        /* offset:256 */
    ,pha_internal_in_hdr_extDSA_fwd_w0_srcTagged       /* dsa after mac addr */        /* offset:258 */
    ,pha_internal_in_hdr_extDSA_fwd_w0_srcDev          /* dsa after mac addr */        /* offset:259 */
    ,pha_internal_in_hdr_extDSA_fwd_w0_SrcPort_4_0     /* dsa after mac addr */        /* offset:264 */ /* validity: srcIsTrunk == 0 */
    ,pha_internal_in_hdr_extDSA_fwd_w0_srcIsTrunk      /* dsa after mac addr */        /* offset:269 */
    ,pha_internal_in_hdr_extDSA_fwd_w0_core_id_2       /* dsa after mac addr */        /* offset:270 */
    ,pha_internal_in_hdr_extDSA_fwd_w0_cfi             /* dsa after mac addr */        /* offset:271 */
    ,pha_internal_in_hdr_extDSA_fwd_w0_up              /* dsa after mac addr */        /* offset:272 */
    ,pha_internal_in_hdr_extDSA_fwd_w0_extended        /* dsa after mac addr */        /* offset:275 */
    ,pha_internal_in_hdr_extDSA_fwd_w0_vid             /* dsa after mac addr */        /* offset:276 */

    ,pha_internal_in_hdr_extDSA_fwd_w1_Extend          /* dsa word 1 after word 0 */        /* offset:288 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_SrcPort_6_5     /* dsa word 1 after word 0 */        /* offset:289 */ /* validity: srcIsTrunk == 0 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_EgressFilterRegistered/* dsa word 1 after word 0 */  /* offset:291 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_DropOnSource    /* dsa word 1 after word 0 */        /* offset:292 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_PacketIsLooped  /* dsa word 1 after word 0 */        /* offset:293 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_routed          /* dsa word 1 after word 0 */        /* offset:294 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_Src_ID          /* dsa word 1 after word 0 */        /* offset:295 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_QosProfile      /* dsa word 1 after word 0 */        /* offset:300 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_use_vidx        /* dsa word 1 after word 0 */        /* offset:307 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_Reserved        /* dsa word 1 after word 0 */        /* offset:308 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_TrgPort         /* dsa word 1 after word 0 */        /* offset:309 */
    ,pha_internal_in_hdr_extDSA_fwd_w1_TrgDev          /* dsa word 1 after word 0 */        /* offset:315 */


    ,pha_internal_in_hdr_DSA_from_cpu_tagCommand       /* dsa after mac addr */        /* offset:256 */
    ,pha_internal_in_hdr_DSA_from_cpu_TrgTagged        /* dsa after mac addr */        /* offset:258 */
    ,pha_internal_in_hdr_DSA_from_cpu_TrgDev           /* dsa after mac addr */        /* offset:259 */
    ,pha_internal_in_hdr_DSA_from_cpu_TrgPhyPort_4_0   /* dsa after mac addr */        /* offset:264 */
    ,pha_internal_in_hdr_DSA_from_cpu_use_eVIDX        /* dsa after mac addr */        /* offset:269 */
    ,pha_internal_in_hdr_DSA_from_cpu_TC0              /* dsa after mac addr */        /* offset:270 */
    ,pha_internal_in_hdr_DSA_from_cpu_cfi              /* dsa after mac addr */        /* offset:271 */
    ,pha_internal_in_hdr_DSA_from_cpu_up               /* dsa after mac addr */        /* offset:272 */
    ,pha_internal_in_hdr_DSA_from_cpu_Extend           /* dsa after mac addr */        /* offset:275 */
    ,pha_internal_in_hdr_DSA_from_cpu_eVLAN            /* dsa after mac addr */        /* offset:276 */




}pha_internal_in_hdr_enum ;

#define in_hdr_HERADER_GET(field , targetValue) \
    targetValue = getHeaderInfo(pha_internal_in_hdr,littleEndianMem,field)

#define in_hdr_HERADER_SET(field , sourceValue) \
    setHeaderInfo(pha_internal_in_hdr,littleEndianMem,field,sourceValue)


static struct thr2_u2e_in_hdr copyMem_thr2_u2e_in_hdr;
struct thr2_u2e_in_hdr* pipeMemCast_thr2_u2e_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr2_u2e_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr2_u2e_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr2_u2e_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr2_u2e_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr2_u2e_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr2_u2e_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr2_u2e_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr2_u2e_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr2_u2e_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr2_u2e_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr2_u2e_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr2_u2e_in_hdr.mac_header.mac_sa_15_0  );


    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_TPID                  , copyMem_thr2_u2e_in_hdr.etag.TPID               );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_PCP                 , copyMem_thr2_u2e_in_hdr.etag.E_PCP              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_DEI                 , copyMem_thr2_u2e_in_hdr.etag.E_DEI              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_base    , copyMem_thr2_u2e_in_hdr.etag.Ingress_E_CID_base );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_IPL_Direction         , copyMem_thr2_u2e_in_hdr.etag.IPL_Direction      );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Reserved              , copyMem_thr2_u2e_in_hdr.etag.Reserved           );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_GRP                   , copyMem_thr2_u2e_in_hdr.etag.GRP                );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_base            , copyMem_thr2_u2e_in_hdr.etag.E_CID_base         );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_ext     , copyMem_thr2_u2e_in_hdr.etag.Ingress_E_CID_ext  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_ext             , copyMem_thr2_u2e_in_hdr.etag.E_CID_ext          );


    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_TPID       , copyMem_thr2_u2e_in_hdr.vlan.TPID               );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_up         , copyMem_thr2_u2e_in_hdr.vlan.up                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_cfi        , copyMem_thr2_u2e_in_hdr.vlan.cfi                );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_vid        , copyMem_thr2_u2e_in_hdr.vlan.vid                );

    return &copyMem_thr2_u2e_in_hdr;
}

static struct thr3_u2c_in_hdr copyMem_thr3_u2c_in_hdr;
struct thr3_u2c_in_hdr * pipeMemCast_thr3_u2c_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr3_u2c_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr3_u2c_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr3_u2c_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr3_u2c_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr3_u2c_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr3_u2c_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr3_u2c_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr3_u2c_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr3_u2c_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr3_u2c_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr3_u2c_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr3_u2c_in_hdr.mac_header.mac_sa_15_0  );


    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_TPID                  , copyMem_thr3_u2c_in_hdr.etag.TPID               );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_PCP                 , copyMem_thr3_u2c_in_hdr.etag.E_PCP              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_DEI                 , copyMem_thr3_u2c_in_hdr.etag.E_DEI              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_base    , copyMem_thr3_u2c_in_hdr.etag.Ingress_E_CID_base );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_IPL_Direction         , copyMem_thr3_u2c_in_hdr.etag.IPL_Direction      );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Reserved              , copyMem_thr3_u2c_in_hdr.etag.Reserved           );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_GRP                   , copyMem_thr3_u2c_in_hdr.etag.GRP                );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_base            , copyMem_thr3_u2c_in_hdr.etag.E_CID_base         );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_ext     , copyMem_thr3_u2c_in_hdr.etag.Ingress_E_CID_ext  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_ext             , copyMem_thr3_u2c_in_hdr.etag.E_CID_ext          );

    return &copyMem_thr3_u2c_in_hdr;
}

static struct thr6_u2e_in_hdr copyMem_thr6_u2e_in_hdr;
struct thr6_u2e_in_hdr * pipeMemCast_thr6_u2e_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr6_u2e_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));


    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr6_u2e_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr6_u2e_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr6_u2e_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr6_u2e_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr6_u2e_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr6_u2e_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr6_u2e_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr6_u2e_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr6_u2e_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr6_u2e_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr6_u2e_in_hdr.mac_header.mac_sa_15_0  );


    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_tagCommand   , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.tagCommand );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_srcTagged    , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.srcTagged  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_srcDev       , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.srcDev     );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_SrcPort_4_0  , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.SrcPort_4_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_srcIsTrunk   , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.srcIsTrunk );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_core_id_2    , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.core_id_2  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_cfi          , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.cfi        );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_up           , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.up         );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_extended     , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.extended   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w0_vid          , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w0.vid        );

    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_Extend                 , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.Extend                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_SrcPort_6_5            , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.SrcPort_6_5            );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_EgressFilterRegistered , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.EgressFilterRegistered );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_DropOnSource           , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.DropOnSource           );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_PacketIsLooped         , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.PacketIsLooped         );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_routed                 , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.routed                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_Src_ID                 , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.Src_ID                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_QosProfile             , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.QosProfile             );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_use_vidx               , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.use_vidx               );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_Reserved               , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.Reserved               );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_TrgPort                , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.TrgPort                );
    in_hdr_HERADER_GET(pha_internal_in_hdr_extDSA_fwd_w1_TrgDev                 , copyMem_thr6_u2e_in_hdr.extDSA_fwd_w1.TrgDev                 );

    return &copyMem_thr6_u2e_in_hdr;
}

static GT_BIT isInHdrThr17Valid=0;
static GT_U32 savedAddr_inhdr_thr17;

static struct thr17_u2ipl_in_hdr copyMem_thr17_u2ipl_in_hdr;
struct thr17_u2ipl_in_hdr * pipeMemCast_thr17_u2ipl_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr17_u2ipl_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr17_u2ipl_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr17_u2ipl_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr17_u2ipl_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr17_u2ipl_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr17_u2ipl_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr17_u2ipl_in_hdr.mac_header.mac_sa_15_0  );


    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_TPID                  , copyMem_thr17_u2ipl_in_hdr.etag.TPID               );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_PCP                 , copyMem_thr17_u2ipl_in_hdr.etag.E_PCP              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_DEI                 , copyMem_thr17_u2ipl_in_hdr.etag.E_DEI              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_base    , copyMem_thr17_u2ipl_in_hdr.etag.Ingress_E_CID_base );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_IPL_Direction         , copyMem_thr17_u2ipl_in_hdr.etag.IPL_Direction      );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Reserved              , copyMem_thr17_u2ipl_in_hdr.etag.Reserved           );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_GRP                   , copyMem_thr17_u2ipl_in_hdr.etag.GRP                );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_base            , copyMem_thr17_u2ipl_in_hdr.etag.E_CID_base         );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_ext     , copyMem_thr17_u2ipl_in_hdr.etag.Ingress_E_CID_ext  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_ext             , copyMem_thr17_u2ipl_in_hdr.etag.E_CID_ext          );

    savedAddr_inhdr_thr17 = address;
    isInHdrThr17Valid = 1;
    return &copyMem_thr17_u2ipl_in_hdr;
}


void pipeMemApply_thr17_u2ipl_in_hdr(uint32_t address/*caller need to use saved address*/)
{
    uint32_t  littleEndianMem[sizeof(struct thr17_u2ipl_in_hdr)/4];

    memset(littleEndianMem,0,sizeof(littleEndianMem));

    in_hdr_HERADER_SET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_SET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_SET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_SET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_SET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr17_u2ipl_in_hdr.expansion_space.reserved_4);

    in_hdr_HERADER_SET(pha_internal_in_hdr_mac_header_mac_da_47_32     ,  copyMem_thr17_u2ipl_in_hdr.mac_header.mac_da_47_32 );
    in_hdr_HERADER_SET(pha_internal_in_hdr_mac_header_mac_da_31_16     ,  copyMem_thr17_u2ipl_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_SET(pha_internal_in_hdr_mac_header_mac_da_15_0      ,  copyMem_thr17_u2ipl_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_SET(pha_internal_in_hdr_mac_header_mac_sa_47_32     ,  copyMem_thr17_u2ipl_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_SET(pha_internal_in_hdr_mac_header_mac_sa_31_16     ,  copyMem_thr17_u2ipl_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_SET(pha_internal_in_hdr_mac_header_mac_sa_15_0      ,  copyMem_thr17_u2ipl_in_hdr.mac_header.mac_sa_15_0  );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_TPID                   ,  copyMem_thr17_u2ipl_in_hdr.etag.TPID               );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_E_PCP                  ,  copyMem_thr17_u2ipl_in_hdr.etag.E_PCP              );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_E_DEI                  ,  copyMem_thr17_u2ipl_in_hdr.etag.E_DEI              );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_Ingress_E_CID_base     ,  copyMem_thr17_u2ipl_in_hdr.etag.Ingress_E_CID_base );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_IPL_Direction          ,  copyMem_thr17_u2ipl_in_hdr.etag.IPL_Direction      );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_Reserved               ,  copyMem_thr17_u2ipl_in_hdr.etag.Reserved           );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_GRP                    ,  copyMem_thr17_u2ipl_in_hdr.etag.GRP                );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_E_CID_base             ,  copyMem_thr17_u2ipl_in_hdr.etag.E_CID_base         );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_Ingress_E_CID_ext      ,  copyMem_thr17_u2ipl_in_hdr.etag.Ingress_E_CID_ext  );
    in_hdr_HERADER_SET(pha_internal_in_hdr_etag_E_CID_ext              ,  copyMem_thr17_u2ipl_in_hdr.etag.E_CID_ext          );

    /*copy bytes back to memory after 'bit field' manipulations that the FIRMWARE updated */
    copyBytesFromHeader(GT_FALSE,address,littleEndianMem,sizeof(struct thr17_u2ipl_in_hdr));
}


static struct thr18_ipl2ipl_in_hdr copyMem_thr18_ipl2ipl_in_hdr;
struct thr18_ipl2ipl_in_hdr * pipeMemCast_thr18_ipl2ipl_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr18_ipl2ipl_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr18_ipl2ipl_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr18_ipl2ipl_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr18_ipl2ipl_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr18_ipl2ipl_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr18_ipl2ipl_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr18_ipl2ipl_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr18_ipl2ipl_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr18_ipl2ipl_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr18_ipl2ipl_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr18_ipl2ipl_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr18_ipl2ipl_in_hdr.mac_header.mac_sa_15_0  );

    return &copyMem_thr18_ipl2ipl_in_hdr;
}

static struct thr20_u2e_m4_in_hdr copyMem_thr20_u2e_m4_in_hdr;
struct thr20_u2e_m4_in_hdr * pipeMemCast_thr20_u2e_m4_in_hdr (uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr20_u2e_m4_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr20_u2e_m4_in_hdr.expansion_space.reserved_0 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr20_u2e_m4_in_hdr.expansion_space.reserved_1 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr20_u2e_m4_in_hdr.expansion_space.reserved_2 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr20_u2e_m4_in_hdr.expansion_space.reserved_3 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr20_u2e_m4_in_hdr.expansion_space.reserved_4 );

    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr20_u2e_m4_in_hdr.mac_header.mac_da_47_32    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr20_u2e_m4_in_hdr.mac_header.mac_da_31_16    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr20_u2e_m4_in_hdr.mac_header.mac_da_15_0     );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr20_u2e_m4_in_hdr.mac_header.mac_sa_47_32    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr20_u2e_m4_in_hdr.mac_header.mac_sa_31_16    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr20_u2e_m4_in_hdr.mac_header.mac_sa_15_0     );


    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_TPID                  , copyMem_thr20_u2e_m4_in_hdr.etag.TPID                  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_PCP                 , copyMem_thr20_u2e_m4_in_hdr.etag.E_PCP                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_DEI                 , copyMem_thr20_u2e_m4_in_hdr.etag.E_DEI                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_base    , copyMem_thr20_u2e_m4_in_hdr.etag.Ingress_E_CID_base    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_IPL_Direction         , copyMem_thr20_u2e_m4_in_hdr.etag.IPL_Direction         );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Reserved              , copyMem_thr20_u2e_m4_in_hdr.etag.Reserved              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_GRP                   , copyMem_thr20_u2e_m4_in_hdr.etag.GRP                   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_base            , copyMem_thr20_u2e_m4_in_hdr.etag.E_CID_base            );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_ext     , copyMem_thr20_u2e_m4_in_hdr.etag.Ingress_E_CID_ext     );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_ext             , copyMem_thr20_u2e_m4_in_hdr.etag.E_CID_ext             );

    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_TPID       , copyMem_thr20_u2e_m4_in_hdr.vlan.TPID                  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_up         , copyMem_thr20_u2e_m4_in_hdr.vlan.up                    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_cfi        , copyMem_thr20_u2e_m4_in_hdr.vlan.cfi                   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_vid        , copyMem_thr20_u2e_m4_in_hdr.vlan.vid                   );

    return &copyMem_thr20_u2e_m4_in_hdr;
}


static struct thr21_u2e_m8_in_hdr copyMem_thr21_u2e_m8_in_hdr;
struct thr21_u2e_m8_in_hdr * pipeMemCast_thr21_u2e_m8_in_hdr (uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr21_u2e_m8_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr21_u2e_m8_in_hdr.expansion_space.reserved_0 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr21_u2e_m8_in_hdr.expansion_space.reserved_1 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr21_u2e_m8_in_hdr.expansion_space.reserved_2 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr21_u2e_m8_in_hdr.expansion_space.reserved_3 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr21_u2e_m8_in_hdr.expansion_space.reserved_4 );

    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr21_u2e_m8_in_hdr.mac_header.mac_da_47_32    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr21_u2e_m8_in_hdr.mac_header.mac_da_31_16    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr21_u2e_m8_in_hdr.mac_header.mac_da_15_0     );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr21_u2e_m8_in_hdr.mac_header.mac_sa_47_32    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr21_u2e_m8_in_hdr.mac_header.mac_sa_31_16    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr21_u2e_m8_in_hdr.mac_header.mac_sa_15_0     );


    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_TPID                  , copyMem_thr21_u2e_m8_in_hdr.etag.TPID                  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_PCP                 , copyMem_thr21_u2e_m8_in_hdr.etag.E_PCP                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_DEI                 , copyMem_thr21_u2e_m8_in_hdr.etag.E_DEI                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_base    , copyMem_thr21_u2e_m8_in_hdr.etag.Ingress_E_CID_base    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_IPL_Direction         , copyMem_thr21_u2e_m8_in_hdr.etag.IPL_Direction         );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Reserved              , copyMem_thr21_u2e_m8_in_hdr.etag.Reserved              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_GRP                   , copyMem_thr21_u2e_m8_in_hdr.etag.GRP                   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_base            , copyMem_thr21_u2e_m8_in_hdr.etag.E_CID_base            );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_ext     , copyMem_thr21_u2e_m8_in_hdr.etag.Ingress_E_CID_ext     );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_ext             , copyMem_thr21_u2e_m8_in_hdr.etag.E_CID_ext             );

    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_TPID       , copyMem_thr21_u2e_m8_in_hdr.vlan.TPID                  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_up         , copyMem_thr21_u2e_m8_in_hdr.vlan.up                    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_cfi        , copyMem_thr21_u2e_m8_in_hdr.vlan.cfi                   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_vid        , copyMem_thr21_u2e_m8_in_hdr.vlan.vid                   );

    return &copyMem_thr21_u2e_m8_in_hdr;
}

typedef struct{
    char*   fieldName;/* info for the __LOG */
    GT_U32  endBit;  /* inclusive*/
    GT_U32  startBit;/* inclusive , value SMAIN_NOT_VALID_CNS means 'equal to endBit' */
}config_info;

static GT_U32  getConfigInfo(config_info *infoPtr,GT_U32 *memPtr , GT_U32 fieldName)
{
    config_info*    currInfoPtr=&infoPtr[fieldName];
    GT_U32  endBit  = currInfoPtr->endBit;
    GT_U32  startBit = (currInfoPtr->startBit == SMAIN_NOT_VALID_CNS) ? endBit : currInfoPtr->startBit;
    GT_U32 numOfBits = (endBit-startBit)+1;
    GT_U32 bitIndex;
    GT_U32  complemet31;
    GT_U32  value;
    /*handle src*/
    complemet31 = 31 - (startBit & 0x1f);
    /* clear the 5 LSBits , and set with proper offset*/
    bitIndex = (startBit & 0xffffffe0) + (complemet31 - (numOfBits-1));

    value = snetFieldValueGet(memPtr,bitIndex,numOfBits);

    __LOG_NO_LOCATION_META_DATA(("[%s] get value [0x%x]\n" ,
        currInfoPtr->fieldName,value));

    return value;
}

static config_info pha_internal_thr1_e2u_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr1_e2u_cfg_etag_E_PCP                         ), 2  , 0 }/* offset:0 */
,{STR(pha_internal_thr1_e2u_cfg_etag_E_DEI                         ), 3  , 3 }/* offset:3 */
,{STR(pha_internal_thr1_e2u_cfg_etag_Ingress_E_CID_base            ),15  , 4 }/* offset:4*/
,{STR(pha_internal_thr1_e2u_cfg_etag_IPL_Direction                 ),16  ,16 }/* offset:16 */
,{STR(pha_internal_thr1_e2u_cfg_etag_Reserved                      ),17  ,17 }/* offset:17*/
,{STR(pha_internal_thr1_e2u_cfg_etag_GRP                           ),19  ,18 }/* offset:18 */
,{STR(pha_internal_thr1_e2u_cfg_etag_E_CID_base                    ),31  ,20 }/* offset:20 */
,{STR(pha_internal_thr1_e2u_cfg_etag_Ingress_E_CID_ext             ),39  ,32 }/* offset:32 */
,{STR(pha_internal_thr1_e2u_cfg_etag_E_CID_ext                     ),47  ,40 }/* offset:40 */
,{STR(pha_internal_thr1_e2u_cfg_etag_E_CID_reserved                ),63  ,48 }/* offset:48 */

,{STR(pha_internal_thr1_e2u_cfg_HA_Table_reserved_space_reserved_0 ),95  ,64 }/* offset:64 */
,{STR(pha_internal_thr1_e2u_cfg_HA_Table_reserved_space_reserved_1 ),127 ,96 }/* offset:96 */


,{STR(pha_internal_thr1_e2u_cfg_srcPortEntry_PCID                  ),139 ,128}/* offset:128 */
,{STR(pha_internal_thr1_e2u_cfg_srcPortEntry_Default_VLAN_Tag          ),155 ,140}/* offset:140 */
,{STR(pha_internal_thr1_e2u_cfg_srcPortEntry_Uplink_Port           ),156 ,156}/* offset:156 */
,{STR(pha_internal_thr1_e2u_cfg_srcPortEntry_reserved              ),159 ,157}/* offset:157 */


,{STR(pha_internal_thr1_e2u_cfg_targetPortEntry_TPID               ),175 ,160}/* offset:160 */
,{STR(pha_internal_thr1_e2u_cfg_targetPortEntry_PVID               ),187 ,176}/* offset:176 */
,{STR(pha_internal_thr1_e2u_cfg_targetPortEntry_VlanEgrTagState    ),188 ,188}/* offset:188 */
,{STR(pha_internal_thr1_e2u_cfg_targetPortEntry_reserved           ),191 ,189}/* offset:189 */
,{STR(pha_internal_thr1_e2u_cfg_targetPortEntry_PCID               ),203 ,192}/* offset:192 */
,{STR(pha_internal_thr1_e2u_cfg_targetPortEntry_Egress_Pipe_Delay  ),223 ,204}/* offset:204 */

};

typedef enum {
     pha_internal_thr1_e2u_cfg_etag_E_PCP                           /* offset:0 */
    ,pha_internal_thr1_e2u_cfg_etag_E_DEI                           /* offset:16 */
    ,pha_internal_thr1_e2u_cfg_etag_Ingress_E_CID_base              /* offset:19 */
    ,pha_internal_thr1_e2u_cfg_etag_IPL_Direction                   /* offset:20 */
    ,pha_internal_thr1_e2u_cfg_etag_Reserved                        /* offset:32 */
    ,pha_internal_thr1_e2u_cfg_etag_GRP                             /* offset:33 */
    ,pha_internal_thr1_e2u_cfg_etag_E_CID_base                      /* offset:34 */
    ,pha_internal_thr1_e2u_cfg_etag_Ingress_E_CID_ext               /* offset:36 */
    ,pha_internal_thr1_e2u_cfg_etag_E_CID_ext                       /* offset:48 */
    ,pha_internal_thr1_e2u_cfg_etag_E_CID_reserved                  /* offset:56 */


    ,pha_internal_thr1_e2u_cfg_HA_Table_reserved_space_reserved_0  /* offset:64 */
    ,pha_internal_thr1_e2u_cfg_HA_Table_reserved_space_reserved_1  /* offset:96 */


    ,pha_internal_thr1_e2u_cfg_srcPortEntry_PCID                   /* offset:128 */
    ,pha_internal_thr1_e2u_cfg_srcPortEntry_Default_VLAN_Tag       /* offset:140 */
    ,pha_internal_thr1_e2u_cfg_srcPortEntry_Uplink_Port            /* offset:156 */
    ,pha_internal_thr1_e2u_cfg_srcPortEntry_reserved               /* offset:157 */


    ,pha_internal_thr1_e2u_cfg_targetPortEntry_TPID                /* offset:160 */
    ,pha_internal_thr1_e2u_cfg_targetPortEntry_PVID                /* offset:176 */
    ,pha_internal_thr1_e2u_cfg_targetPortEntry_VlanEgrTagState     /* offset:188 */
    ,pha_internal_thr1_e2u_cfg_targetPortEntry_reserved            /* offset:189 */
    ,pha_internal_thr1_e2u_cfg_targetPortEntry_PCID                /* offset:192 */
    ,pha_internal_thr1_e2u_cfg_targetPortEntry_Egress_Pipe_Delay   /* offset:204 */

}pha_internal_thr1_e2u_cfg_enum;

#define CONFIG_MEM_SIZE_IN_WORDS_CNS  7

#define thr1_e2u_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr1_e2u_cfg,littleEndianMem,field)

static struct thr1_e2u_cfg copyMem_thr1_e2u_cfg;
struct thr1_e2u_cfg* pipeMemCast_thr1_e2u_cfg(uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_E_PCP                         , copyMem_thr1_e2u_cfg.etag.E_PCP                         );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_E_DEI                         , copyMem_thr1_e2u_cfg.etag.E_DEI                         );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_Ingress_E_CID_base            , copyMem_thr1_e2u_cfg.etag.Ingress_E_CID_base            );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_IPL_Direction                 , copyMem_thr1_e2u_cfg.etag.IPL_Direction                 );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_Reserved                      , copyMem_thr1_e2u_cfg.etag.Reserved                      );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_GRP                           , copyMem_thr1_e2u_cfg.etag.GRP                           );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_E_CID_base                    , copyMem_thr1_e2u_cfg.etag.E_CID_base                    );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_Ingress_E_CID_ext             , copyMem_thr1_e2u_cfg.etag.Ingress_E_CID_ext             );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_E_CID_ext                     , copyMem_thr1_e2u_cfg.etag.E_CID_ext                     );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_etag_E_CID_reserved                , copyMem_thr1_e2u_cfg.etag.reserved                      );


    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_HA_Table_reserved_space_reserved_0 , copyMem_thr1_e2u_cfg.HA_Table_reserved_space.reserved_0 );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_HA_Table_reserved_space_reserved_1 , copyMem_thr1_e2u_cfg.HA_Table_reserved_space.reserved_1 );


    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_srcPortEntry_PCID                  , copyMem_thr1_e2u_cfg.srcPortEntry.PCID                  );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_srcPortEntry_Default_VLAN_Tag      , copyMem_thr1_e2u_cfg.srcPortEntry.Default_VLAN_Tag      );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_srcPortEntry_Uplink_Port           , copyMem_thr1_e2u_cfg.srcPortEntry.Uplink_Port           );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_srcPortEntry_reserved              , copyMem_thr1_e2u_cfg.srcPortEntry.reserved              );


    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_targetPortEntry_TPID               , copyMem_thr1_e2u_cfg.targetPortEntry.TPID               );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_targetPortEntry_PVID               , copyMem_thr1_e2u_cfg.targetPortEntry.PVID               );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_targetPortEntry_VlanEgrTagState    , copyMem_thr1_e2u_cfg.targetPortEntry.VlanEgrTagState    );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_targetPortEntry_reserved           , copyMem_thr1_e2u_cfg.targetPortEntry.reserved           );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_targetPortEntry_PCID               , copyMem_thr1_e2u_cfg.targetPortEntry.PCID               );
    thr1_e2u_cfg_CONFIG_GET(pha_internal_thr1_e2u_cfg_targetPortEntry_Egress_Pipe_Delay  , copyMem_thr1_e2u_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr1_e2u_cfg;
}

static config_info pha_internal_thr19_e2u_untagged_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_PCP                   ), 2  , 0 }/* offset:0  */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_DEI                   ), 3  , 3 }/* offset:3  */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_Ingress_E_CID_base      ),15  , 4 }/* offset:4  */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_IPL_Direction           ),16  ,16 }/* offset:16 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_Reserved                ),17  ,17 }/* offset:17 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_GRP                     ),19  ,18 }/* offset:18 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_CID_base              ),31  ,20 }/* offset:20 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_Ingress_E_CID_ext       ),39  ,32 }/* offset:32 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_CID_ext               ),47  ,40 }/* offset:40 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_VLAN_Tag_TPID           ),63  ,48 }/* offset:48 */

,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_reserved_space_reserved_0              ),95  ,64 }/* offset:64 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_HA_Table_reserved_space_reserved_1              ),127 ,96 }/* offset:96 */


,{STR(pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_PCID                               ),139 ,128}/* offset:128 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_Default_VLAN_Tag                   ),155 ,140}/* offset:140 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_Uplink_Port                        ),156 ,156}/* offset:156 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_reserved                           ),159 ,157}/* offset:157 */


,{STR(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_TPID                            ),175 ,160}/* offset:160 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_PVID                            ),187 ,176}/* offset:176 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_VlanEgrTagState                 ),188 ,188}/* offset:188 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_reserved                        ),191 ,189}/* offset:189 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_PCID                            ),203 ,192}/* offset:192 */
,{STR(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_Egress_Pipe_Delay               ),223 ,204}/* offset:204 */

,{STR(pha_internal_thr19_e2u_untagged_cfg_cfgReservedSpace_reserved_0                     ),255 ,224}/* offset:224 */

};

typedef enum {
     pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_PCP                               /* offset:0  */
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_DEI                               /* offset:3  */
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_Ingress_E_CID_base                  /* offset:4  */
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_IPL_Direction                       /* offset:16 */
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_Reserved                            /* offset:17 */
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_GRP                                 /* offset:18 */
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_CID_base                          /* offset:20 */
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_Ingress_E_CID_ext                   /* offset:32 */
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_CID_ext                           /* offset:40 */
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_VLAN_Tag_TPID                       /* offset:48 */

    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_reserved_space_reserved_0
    ,pha_internal_thr19_e2u_untagged_cfg_HA_Table_reserved_space_reserved_1                          /* offset:64 */
                                                                                                     /* offset:96 */

    ,pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_PCID
    ,pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_Default_VLAN_Tag                               /* offset:128 */
    ,pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_Uplink_Port                                    /* offset:140 */
    ,pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_reserved                                       /* offset:156 */
                                                                                                     /* offset:157 */

    ,pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_TPID
    ,pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_PVID                                        /* offset:160 */
    ,pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_VlanEgrTagState                             /* offset:176 */
    ,pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_reserved                                    /* offset:188 */
    ,pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_PCID                                        /* offset:189 */
    ,pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_Egress_Pipe_Delay                           /* offset:192 */
                                                                                                     /* offset:204 */
    ,pha_internal_thr19_e2u_untagged_cfg_cfgReservedSpace_reserved_0                                 /* offset:224 */
}pha_internal_thr19_e2u_untagged_cfg_enum;

#define thr19_e2u_untagged_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr19_e2u_untagged_cfg,littleEndianMem,field)

static struct thr19_e2u_untagged_cfg copyMem_thr19_e2u_untagged_cfg;
struct thr19_e2u_untagged_cfg* pipeMemCast_thr19_e2u_untagged_cfg(uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_PCP                         , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.E_PCP               );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_DEI                         , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.E_DEI               );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_Ingress_E_CID_base            , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.Ingress_E_CID_base  );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_IPL_Direction                 , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.IPL_Direction       );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_Reserved                      , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.Reserved            );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_GRP                           , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.GRP                 );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_CID_base                    , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.E_CID_base          );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_Ingress_E_CID_ext             , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.Ingress_E_CID_ext   );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_E_CID_ext                     , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.E_CID_ext           );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_Upstream_Ports_VLAN_Tag_TPID                 , copyMem_thr19_e2u_untagged_cfg.HA_Table_Upstream_Ports.VLAN_Tag_TPID       );


    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_reserved_space_reserved_0                    , copyMem_thr19_e2u_untagged_cfg.HA_Table_reserved_space.reserved_0          );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_HA_Table_reserved_space_reserved_1                    , copyMem_thr19_e2u_untagged_cfg.HA_Table_reserved_space.reserved_1          );


    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_PCID                                     , copyMem_thr19_e2u_untagged_cfg.srcPortEntry.PCID                           );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_Default_VLAN_Tag                         , copyMem_thr19_e2u_untagged_cfg.srcPortEntry.Default_VLAN_Tag               );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_Uplink_Port                              , copyMem_thr19_e2u_untagged_cfg.srcPortEntry.Uplink_Port                    );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_srcPortEntry_reserved                                 , copyMem_thr19_e2u_untagged_cfg.srcPortEntry.reserved                       );


    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_TPID                                  , copyMem_thr19_e2u_untagged_cfg.targetPortEntry.TPID                        );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_PVID                                  , copyMem_thr19_e2u_untagged_cfg.targetPortEntry.PVID                        );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_VlanEgrTagState                       , copyMem_thr19_e2u_untagged_cfg.targetPortEntry.VlanEgrTagState             );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_reserved                              , copyMem_thr19_e2u_untagged_cfg.targetPortEntry.reserved                    );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_PCID                                  , copyMem_thr19_e2u_untagged_cfg.targetPortEntry.PCID                        );
    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_targetPortEntry_Egress_Pipe_Delay                     , copyMem_thr19_e2u_untagged_cfg.targetPortEntry.Egress_Pipe_Delay           );

    thr19_e2u_untagged_cfg_CONFIG_GET(pha_internal_thr19_e2u_untagged_cfg_cfgReservedSpace_reserved_0                           , copyMem_thr19_e2u_untagged_cfg.cfgReservedSpace.reserved_0                 );

    return &copyMem_thr19_e2u_untagged_cfg;
}

static config_info pha_internal_thr2_u2e_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_0),    31 ,    0 }/* offset:  0 */
,{STR(pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_1),    63 ,   32 }/* offset: 32 */
,{STR(pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_2),    95 ,   64 }/* offset: 64 */
,{STR(pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_3),   127 ,   96 }/* offset: 96 */


,{STR(pha_internal_thr2_u2e_cfg_srcPortEntry_PCID                                ),   139 , 128 }/* offset:128 */
,{STR(pha_internal_thr2_u2e_cfg_srcPortEntry_Source_Filtering_Bitvector          ),   155 , 140}/* offset:140 */
,{STR(pha_internal_thr2_u2e_cfg_srcPortEntry_Uplink_Port                         ),   156 , 156}/* offset:156 */
,{STR(pha_internal_thr2_u2e_cfg_srcPortEntry_reserved                            ),   159 , 157}/* offset:157 */


,{STR(pha_internal_thr2_u2e_cfg_targetPortEntry_TPID              ),   175 , 160 }/* offset:160 */
,{STR(pha_internal_thr2_u2e_cfg_targetPortEntry_TargetPort        ),   179 , 176 }/* offset:176 */
,{STR(pha_internal_thr2_u2e_cfg_targetPortEntry_reserved          ),   191 , 180 }/* offset:180 */
,{STR(pha_internal_thr2_u2e_cfg_targetPortEntry_PCID              ),   203 , 192 }/* offset:192 */
,{STR(pha_internal_thr2_u2e_cfg_targetPortEntry_Egress_Pipe_Delay ),   223 , 204 }/* offset:204 */
};


typedef enum{
     pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_0      /* offset:0 */
    ,pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_1      /* offset:32 */
    ,pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_2      /* offset:64 */
    ,pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_3      /* offset:96 */


    ,pha_internal_thr2_u2e_cfg_srcPortEntry_PCID                       /* offset:128 */
    ,pha_internal_thr2_u2e_cfg_srcPortEntry_Source_Filtering_Bitvector /* offset:140 */
    ,pha_internal_thr2_u2e_cfg_srcPortEntry_Uplink_Port                /* offset:156 */
    ,pha_internal_thr2_u2e_cfg_srcPortEntry_reserved                   /* offset:144 */


    ,pha_internal_thr2_u2e_cfg_targetPortEntry_TPID                    /* offset:160 */
    ,pha_internal_thr2_u2e_cfg_targetPortEntry_TargetPort              /* offset:176 */
    ,pha_internal_thr2_u2e_cfg_targetPortEntry_reserved                /* offset:180 */
    ,pha_internal_thr2_u2e_cfg_targetPortEntry_PCID                    /* offset:192 */
    ,pha_internal_thr2_u2e_cfg_targetPortEntry_Egress_Pipe_Delay       /* offset:204 */
}pha_internal_thr2_u2e_cfg_enum;

#define thr2_u2e_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr2_u2e_cfg,littleEndianMem,field)

static struct thr2_u2e_cfg copyMem_thr2_u2e_cfg;
struct thr2_u2e_cfg * pipeMemCast_thr2_u2e_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_0 , copyMem_thr2_u2e_cfg.HA_Table_reserved_space.reserved_0 );
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_1 , copyMem_thr2_u2e_cfg.HA_Table_reserved_space.reserved_1 );
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_2 , copyMem_thr2_u2e_cfg.HA_Table_reserved_space.reserved_2 );
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_HA_Table_reserved_space_reserved_3 , copyMem_thr2_u2e_cfg.HA_Table_reserved_space.reserved_3 );


    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_srcPortEntry_PCID                  , copyMem_thr2_u2e_cfg.srcPortEntry.PCID                  );
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_srcPortEntry_Source_Filtering_Bitvector          , copyMem_thr2_u2e_cfg.srcPortEntry.Source_Filtering_Bitvector);
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_srcPortEntry_Uplink_Port           , copyMem_thr2_u2e_cfg.srcPortEntry.Uplink_Port);
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_srcPortEntry_reserved              , copyMem_thr2_u2e_cfg.srcPortEntry.reserved              );


    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_targetPortEntry_TPID               , copyMem_thr2_u2e_cfg.targetPortEntry.TPID               );
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_targetPortEntry_TargetPort         , copyMem_thr2_u2e_cfg.targetPortEntry.TargetPort         );
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_targetPortEntry_reserved           , copyMem_thr2_u2e_cfg.targetPortEntry.reserved           );
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_targetPortEntry_PCID               , copyMem_thr2_u2e_cfg.targetPortEntry.PCID               );
    thr2_u2e_cfg_CONFIG_GET(pha_internal_thr2_u2e_cfg_targetPortEntry_Egress_Pipe_Delay  , copyMem_thr2_u2e_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr2_u2e_cfg;
}

static config_info pha_internal_thr3_u2c_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_0),    31 ,    0 }/* offset:  0 */
,{STR(pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_1),    63 ,   32 }/* offset: 32 */
,{STR(pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_2),    95 ,   64 }/* offset: 64 */
,{STR(pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_3),   127 ,   96 }/* offset: 96 */


,{STR(pha_internal_thr3_u2c_cfg_srcPortEntry_PCID                 ),   139 , 128 }/* offset:128 */
,{STR(pha_internal_thr3_u2c_cfg_Source_Filtering_Bitvector        ),   155 , 140 }/* offset:140 */
,{STR(pha_internal_thr3_u2c_cfg_Uplink_Port                       ),   156 , 156 }/* offset:156 */
,{STR(pha_internal_thr3_u2c_cfg_srcPortEntry_reserved             ),   159 , 157 }/* offset:157 */


,{STR(pha_internal_thr3_u2c_cfg_targetPortEntry_TPID              ),   175 , 160 }/* offset:160 */
,{STR(pha_internal_thr3_u2c_cfg_targetPortEntry_PVID              ),   187 , 176 }/* offset:176 */
,{STR(pha_internal_thr3_u2c_cfg_targetPortEntry_VlanEgrTagState   ),   188 , 188 }/* offset:188 */
,{STR(pha_internal_thr3_u2c_cfg_targetPortEntry_reserved          ),   191 , 189 }/* offset:189 */
,{STR(pha_internal_thr3_u2c_cfg_targetPortEntry_PCID              ),   203 , 192 }/* offset:192 */
,{STR(pha_internal_thr3_u2c_cfg_targetPortEntry_Egress_Pipe_Delay ),   223 , 204 }/* offset:204 */
};

typedef enum{
     pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_0      /* offset:0 */
    ,pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_1      /* offset:32 */
    ,pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_2      /* offset:64 */
    ,pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_3      /* offset:96 */


    ,pha_internal_thr3_u2c_cfg_srcPortEntry_PCID                       /* offset:128 */
    ,pha_internal_thr3_u2c_cfg_Source_Filtering_Bitvector              /* offset:140 */
    ,pha_internal_thr3_u2c_cfg_Uplink_Port                             /* offset:156 */
    ,pha_internal_thr3_u2c_cfg_srcPortEntry_reserved                   /* offset:157 */


    ,pha_internal_thr3_u2c_cfg_targetPortEntry_TPID                    /* offset:160 */
    ,pha_internal_thr3_u2c_cfg_targetPortEntry_PVID                    /* offset:176 */
    ,pha_internal_thr3_u2c_cfg_targetPortEntry_VlanEgrTagState         /* offset:188 */
    ,pha_internal_thr3_u2c_cfg_targetPortEntry_reserved                /* offset:189 */
    ,pha_internal_thr3_u2c_cfg_targetPortEntry_PCID                    /* offset:192 */
    ,pha_internal_thr3_u2c_cfg_targetPortEntry_Egress_Pipe_Delay       /* offset:204 */
}pha_internal_thr3_u2c_cfg_enum;

#define thr3_u2c_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr3_u2c_cfg,littleEndianMem,field)

static struct thr3_u2c_cfg copyMem_thr3_u2c_cfg;
struct thr3_u2c_cfg * pipeMemCast_thr3_u2c_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_0 , copyMem_thr3_u2c_cfg.HA_Table_reserved_space.reserved_0 );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_1 , copyMem_thr3_u2c_cfg.HA_Table_reserved_space.reserved_1 );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_2 , copyMem_thr3_u2c_cfg.HA_Table_reserved_space.reserved_2 );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_HA_Table_reserved_space_reserved_3 , copyMem_thr3_u2c_cfg.HA_Table_reserved_space.reserved_3 );


    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_srcPortEntry_PCID                  , copyMem_thr3_u2c_cfg.srcPortEntry.PCID                  );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_Source_Filtering_Bitvector         , copyMem_thr3_u2c_cfg.srcPortEntry.Source_Filtering_Bitvector);
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_Uplink_Port                        , copyMem_thr3_u2c_cfg.srcPortEntry.Uplink_Port           );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_srcPortEntry_reserved              , copyMem_thr3_u2c_cfg.srcPortEntry.reserved              );


    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_targetPortEntry_TPID               , copyMem_thr3_u2c_cfg.targetPortEntry.TPID               );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_targetPortEntry_PVID               , copyMem_thr3_u2c_cfg.targetPortEntry.PVID               );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_targetPortEntry_VlanEgrTagState    , copyMem_thr3_u2c_cfg.targetPortEntry.VlanEgrTagState    );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_targetPortEntry_reserved           , copyMem_thr3_u2c_cfg.targetPortEntry.reserved           );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_targetPortEntry_PCID               , copyMem_thr3_u2c_cfg.targetPortEntry.PCID               );
    thr3_u2c_cfg_CONFIG_GET(pha_internal_thr3_u2c_cfg_targetPortEntry_Egress_Pipe_Delay  , copyMem_thr3_u2c_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr3_u2c_cfg;
}

static config_info pha_internal_thr4_et2u_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr4_et2u_cfg_DSA_fwd_tagCommand                 ) ,1     ,0   }  /* offset:0 */
,{STR(pha_internal_thr4_et2u_cfg_DSA_fwd_srcTagged                  ) ,2     ,2   }  /* offset:2 */
,{STR(pha_internal_thr4_et2u_cfg_DSA_fwd_srcDev                     ) ,7     ,3   }  /* offset:3 */
,{STR(pha_internal_thr4_et2u_cfg_DSA_fwd_SrcPort_4_0                ) ,12    ,8   }  /* offset:8 */ /* validity: srcIsTrunk == 0 */
,{STR(pha_internal_thr4_et2u_cfg_DSA_fwd_srcIsTrunk                 ) ,13    ,13  }  /* offset:13 */
,{STR(pha_internal_thr4_et2u_cfg_DSA_fwd_core_id_2                  ) ,14    ,14  }  /* offset:14 */
,{STR(pha_internal_thr4_et2u_cfg_DSA_fwd_cfi                        ) ,15    ,15  }  /* offset:15 */
,{STR(pha_internal_thr4_et2u_cfg_DSA_fwd_up                         ) ,18    ,16  }  /* offset:16 */
,{STR(pha_internal_thr4_et2u_cfg_DSA_fwd_extended                   ) ,19    ,19  }  /* offset:19 */
,{STR(pha_internal_thr4_et2u_cfg_DSA_fwd_vid                        ) ,31    ,20  }  /* offset:20 */


,{STR(pha_internal_thr4_et2u_cfg_HA_Table_reserved_space_reserved_0 ) ,63    ,32  }  /* offset:32 */
,{STR(pha_internal_thr4_et2u_cfg_HA_Table_reserved_space_reserved_1 ) ,95    ,64  }  /* offset:64 */
,{STR(pha_internal_thr4_et2u_cfg_HA_Table_reserved_space_reserved_2 ) ,127   ,96  }  /* offset:96 */


,{STR(pha_internal_thr4_et2u_cfg_srcPortEntry_PVID                  ) ,139   ,128 }  /* offset:128 */
,{STR(pha_internal_thr4_et2u_cfg_srcPortEntry_src_port_num          ) ,143   ,140 }  /* offset:140 */
,{STR(pha_internal_thr4_et2u_cfg_srcPortEntry_reserved              ) ,159   ,144 }  /* offset:144 */


,{STR(pha_internal_thr4_et2u_cfg_targetPortEntry_TPID               ) ,175   ,160 }  /* offset:160 */
,{STR(pha_internal_thr4_et2u_cfg_targetPortEntry_PVID               ) ,187   ,176 }  /* offset:176 */
,{STR(pha_internal_thr4_et2u_cfg_targetPortEntry_VlanEgrTagState    ) ,188   ,188 }  /* offset:188 */
,{STR(pha_internal_thr4_et2u_cfg_targetPortEntry_reserved_0         ) ,191   ,189 }  /* offset:189 */
,{STR(pha_internal_thr4_et2u_cfg_targetPortEntry_reserved_1         ) ,203   ,192 }  /* offset:192 */
,{STR(pha_internal_thr4_et2u_cfg_targetPortEntry_Egress_Pipe_Delay  ) ,223   ,204 }  /* offset:204 */

};

typedef enum{

     pha_internal_thr4_et2u_cfg_DSA_fwd_tagCommand                 /* offset:0 */
    ,pha_internal_thr4_et2u_cfg_DSA_fwd_srcTagged                  /* offset:2 */
    ,pha_internal_thr4_et2u_cfg_DSA_fwd_srcDev                     /* offset:3 */
    ,pha_internal_thr4_et2u_cfg_DSA_fwd_SrcPort_4_0                /* offset:8 */ /* validity: srcIsTrunk == 0 */
    ,pha_internal_thr4_et2u_cfg_DSA_fwd_srcIsTrunk                 /* offset:13 */
    ,pha_internal_thr4_et2u_cfg_DSA_fwd_core_id_2                  /* offset:14 */
    ,pha_internal_thr4_et2u_cfg_DSA_fwd_cfi                        /* offset:15 */
    ,pha_internal_thr4_et2u_cfg_DSA_fwd_up                         /* offset:16 */
    ,pha_internal_thr4_et2u_cfg_DSA_fwd_extended                   /* offset:19 */
    ,pha_internal_thr4_et2u_cfg_DSA_fwd_vid                        /* offset:20 */


    ,pha_internal_thr4_et2u_cfg_HA_Table_reserved_space_reserved_0 /* offset:32 */
    ,pha_internal_thr4_et2u_cfg_HA_Table_reserved_space_reserved_1 /* offset:64 */
    ,pha_internal_thr4_et2u_cfg_HA_Table_reserved_space_reserved_2 /* offset:96 */


    ,pha_internal_thr4_et2u_cfg_srcPortEntry_PVID                  /* offset:128 */
    ,pha_internal_thr4_et2u_cfg_srcPortEntry_src_port_num          /* offset:140 */
    ,pha_internal_thr4_et2u_cfg_srcPortEntry_reserved              /* offset:144 */


    ,pha_internal_thr4_et2u_cfg_targetPortEntry_TPID               /* offset:160 */
    ,pha_internal_thr4_et2u_cfg_targetPortEntry_PVID               /* offset:176 */
    ,pha_internal_thr4_et2u_cfg_targetPortEntry_VlanEgrTagState    /* offset:188 */
    ,pha_internal_thr4_et2u_cfg_targetPortEntry_reserved_0         /* offset:189 */
    ,pha_internal_thr4_et2u_cfg_targetPortEntry_reserved_1         /* offset:192 */
    ,pha_internal_thr4_et2u_cfg_targetPortEntry_Egress_Pipe_Delay  /* offset:204 */

}pha_internal_thr4_et2u_cfg_enum;

#define thr4_et2u_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr4_et2u_cfg,littleEndianMem,field)

static struct thr4_et2u_cfg copyMem_thr4_et2u_cfg;
struct thr4_et2u_cfg * pipeMemCast_thr4_et2u_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_tagCommand                 ,copyMem_thr4_et2u_cfg.DSA_fwd.tagCommand                 );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_srcTagged                  ,copyMem_thr4_et2u_cfg.DSA_fwd.srcTagged                  );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_srcDev                     ,copyMem_thr4_et2u_cfg.DSA_fwd.srcDev                     );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_SrcPort_4_0                ,copyMem_thr4_et2u_cfg.DSA_fwd.SrcPort_4_0                );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_srcIsTrunk                 ,copyMem_thr4_et2u_cfg.DSA_fwd.srcIsTrunk                 );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_core_id_2                  ,copyMem_thr4_et2u_cfg.DSA_fwd.core_id_2                  );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_cfi                        ,copyMem_thr4_et2u_cfg.DSA_fwd.cfi                        );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_up                         ,copyMem_thr4_et2u_cfg.DSA_fwd.up                         );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_extended                   ,copyMem_thr4_et2u_cfg.DSA_fwd.extended                   );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_DSA_fwd_vid                        ,copyMem_thr4_et2u_cfg.DSA_fwd.vid                        );


    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_HA_Table_reserved_space_reserved_0 ,copyMem_thr4_et2u_cfg.HA_Table_reserved_space.reserved_0 );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_HA_Table_reserved_space_reserved_1 ,copyMem_thr4_et2u_cfg.HA_Table_reserved_space.reserved_1 );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_HA_Table_reserved_space_reserved_2 ,copyMem_thr4_et2u_cfg.HA_Table_reserved_space.reserved_2 );


    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_srcPortEntry_PVID                  ,copyMem_thr4_et2u_cfg.srcPortEntry.PVID                  );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_srcPortEntry_src_port_num          ,copyMem_thr4_et2u_cfg.srcPortEntry.src_port_num          );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_srcPortEntry_reserved              ,copyMem_thr4_et2u_cfg.srcPortEntry.reserved              );


    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_targetPortEntry_TPID               ,copyMem_thr4_et2u_cfg.targetPortEntry.TPID               );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_targetPortEntry_PVID               ,copyMem_thr4_et2u_cfg.targetPortEntry.PVID               );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_targetPortEntry_VlanEgrTagState    ,copyMem_thr4_et2u_cfg.targetPortEntry.VlanEgrTagState    );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_targetPortEntry_reserved_0         ,copyMem_thr4_et2u_cfg.targetPortEntry.reserved_0         );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_targetPortEntry_reserved_1         ,copyMem_thr4_et2u_cfg.targetPortEntry.reserved_1         );
    thr4_et2u_cfg_CONFIG_GET(pha_internal_thr4_et2u_cfg_targetPortEntry_Egress_Pipe_Delay  ,copyMem_thr4_et2u_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr4_et2u_cfg;
}

static config_info pha_internal_thr5_eu2u_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_tagCommand                 ) ,1     ,0   }  /* offset:0 */
,{STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_srcTagged                  ) ,2     ,2   }  /* offset:2 */
,{STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_srcDev                     ) ,7     ,3   }  /* offset:3 */
,{STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_SrcPort_4_0                ) ,12    ,8   }  /* offset:8 */ /* validity: srcIsTrunk == 0 */
,{STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_srcIsTrunk                 ) ,13    ,13  }  /* offset:13 */
,{STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_core_id_2                  ) ,14    ,14  }  /* offset:14 */
,{STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_cfi                        ) ,15    ,15  }  /* offset:15 */
,{STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_up                         ) ,18    ,16  }  /* offset:16 */
,{STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_extended                   ) ,19    ,19  }  /* offset:19 */
,{STR(pha_internal_thr5_eu2u_cfg_DSA_fwd_vid                        ) ,31    ,20  }  /* offset:20 */


,{STR(pha_internal_thr5_eu2u_cfg_HA_Table_reserved_space_reserved_0 ) ,63    ,32  }  /* offset:32 */
,{STR(pha_internal_thr5_eu2u_cfg_HA_Table_reserved_space_reserved_1 ) ,95    ,64  }  /* offset:64 */
,{STR(pha_internal_thr5_eu2u_cfg_HA_Table_reserved_space_reserved_2 ) ,127   ,96  }  /* offset:96 */


,{STR(pha_internal_thr5_eu2u_cfg_srcPortEntry_PVID                  ) ,139   ,128 }  /* offset:128 */
,{STR(pha_internal_thr5_eu2u_cfg_srcPortEntry_src_port_num          ) ,143   ,140 }  /* offset:140 */
,{STR(pha_internal_thr5_eu2u_cfg_srcPortEntry_reserved              ) ,159   ,144 }  /* offset:144 */


,{STR(pha_internal_thr5_eu2u_cfg_targetPortEntry_TPID               ) ,175   ,160 }  /* offset:160 */
,{STR(pha_internal_thr5_eu2u_cfg_targetPortEntry_PVID               ) ,187   ,176 }  /* offset:176 */
,{STR(pha_internal_thr5_eu2u_cfg_targetPortEntry_VlanEgrTagState    ) ,188   ,188 }  /* offset:188 */
,{STR(pha_internal_thr5_eu2u_cfg_targetPortEntry_reserved_0         ) ,191   ,189 }  /* offset:189 */
,{STR(pha_internal_thr5_eu2u_cfg_targetPortEntry_reserved_1         ) ,203   ,192 }  /* offset:192 */
,{STR(pha_internal_thr5_eu2u_cfg_targetPortEntry_Egress_Pipe_Delay  ) ,223   ,204 }  /* offset:204 */

};

typedef enum{

     pha_internal_thr5_eu2u_cfg_DSA_fwd_tagCommand                 /* offset:0 */
    ,pha_internal_thr5_eu2u_cfg_DSA_fwd_srcTagged                  /* offset:2 */
    ,pha_internal_thr5_eu2u_cfg_DSA_fwd_srcDev                     /* offset:3 */
    ,pha_internal_thr5_eu2u_cfg_DSA_fwd_SrcPort_4_0                /* offset:8 */ /* validity: srcIsTrunk == 0 */
    ,pha_internal_thr5_eu2u_cfg_DSA_fwd_srcIsTrunk                 /* offset:13 */
    ,pha_internal_thr5_eu2u_cfg_DSA_fwd_core_id_2                  /* offset:14 */
    ,pha_internal_thr5_eu2u_cfg_DSA_fwd_cfi                        /* offset:15 */
    ,pha_internal_thr5_eu2u_cfg_DSA_fwd_up                         /* offset:16 */
    ,pha_internal_thr5_eu2u_cfg_DSA_fwd_extended                   /* offset:19 */
    ,pha_internal_thr5_eu2u_cfg_DSA_fwd_vid                        /* offset:20 */


    ,pha_internal_thr5_eu2u_cfg_HA_Table_reserved_space_reserved_0 /* offset:32 */
    ,pha_internal_thr5_eu2u_cfg_HA_Table_reserved_space_reserved_1 /* offset:64 */
    ,pha_internal_thr5_eu2u_cfg_HA_Table_reserved_space_reserved_2 /* offset:96 */


    ,pha_internal_thr5_eu2u_cfg_srcPortEntry_PVID                  /* offset:128 */
    ,pha_internal_thr5_eu2u_cfg_srcPortEntry_src_port_num          /* offset:140 */
    ,pha_internal_thr5_eu2u_cfg_srcPortEntry_reserved              /* offset:144 */


    ,pha_internal_thr5_eu2u_cfg_targetPortEntry_TPID               /* offset:160 */
    ,pha_internal_thr5_eu2u_cfg_targetPortEntry_PVID               /* offset:176 */
    ,pha_internal_thr5_eu2u_cfg_targetPortEntry_VlanEgrTagState    /* offset:188 */
    ,pha_internal_thr5_eu2u_cfg_targetPortEntry_reserved_0         /* offset:189 */
    ,pha_internal_thr5_eu2u_cfg_targetPortEntry_reserved_1         /* offset:192 */
    ,pha_internal_thr5_eu2u_cfg_targetPortEntry_Egress_Pipe_Delay  /* offset:204 */

}pha_internal_thr5_eu2u_cfg_enum;

#define thr5_eu2u_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr5_eu2u_cfg,littleEndianMem,field)

static struct thr5_eu2u_cfg copyMem_thr5_eu2u_cfg;
struct thr5_eu2u_cfg * pipeMemCast_thr5_eu2u_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_tagCommand                 ,copyMem_thr5_eu2u_cfg.DSA_fwd.tagCommand                 );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_srcTagged                  ,copyMem_thr5_eu2u_cfg.DSA_fwd.srcTagged                  );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_srcDev                     ,copyMem_thr5_eu2u_cfg.DSA_fwd.srcDev                     );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_SrcPort_4_0                ,copyMem_thr5_eu2u_cfg.DSA_fwd.SrcPort_4_0                );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_srcIsTrunk                 ,copyMem_thr5_eu2u_cfg.DSA_fwd.srcIsTrunk                 );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_core_id_2                  ,copyMem_thr5_eu2u_cfg.DSA_fwd.core_id_2                  );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_cfi                        ,copyMem_thr5_eu2u_cfg.DSA_fwd.cfi                        );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_up                         ,copyMem_thr5_eu2u_cfg.DSA_fwd.up                         );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_extended                   ,copyMem_thr5_eu2u_cfg.DSA_fwd.extended                   );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_DSA_fwd_vid                        ,copyMem_thr5_eu2u_cfg.DSA_fwd.vid                        );


    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_HA_Table_reserved_space_reserved_0 ,copyMem_thr5_eu2u_cfg.HA_Table_reserved_space.reserved_0 );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_HA_Table_reserved_space_reserved_1 ,copyMem_thr5_eu2u_cfg.HA_Table_reserved_space.reserved_1 );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_HA_Table_reserved_space_reserved_2 ,copyMem_thr5_eu2u_cfg.HA_Table_reserved_space.reserved_2 );


    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_srcPortEntry_PVID                  ,copyMem_thr5_eu2u_cfg.srcPortEntry.PVID                  );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_srcPortEntry_src_port_num          ,copyMem_thr5_eu2u_cfg.srcPortEntry.src_port_num          );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_srcPortEntry_reserved              ,copyMem_thr5_eu2u_cfg.srcPortEntry.reserved              );


    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_targetPortEntry_TPID               ,copyMem_thr5_eu2u_cfg.targetPortEntry.TPID               );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_targetPortEntry_PVID               ,copyMem_thr5_eu2u_cfg.targetPortEntry.PVID               );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_targetPortEntry_VlanEgrTagState    ,copyMem_thr5_eu2u_cfg.targetPortEntry.VlanEgrTagState    );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_targetPortEntry_reserved_0         ,copyMem_thr5_eu2u_cfg.targetPortEntry.reserved_0         );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_targetPortEntry_reserved_1         ,copyMem_thr5_eu2u_cfg.targetPortEntry.reserved_1         );
    thr5_eu2u_cfg_CONFIG_GET(pha_internal_thr5_eu2u_cfg_targetPortEntry_Egress_Pipe_Delay  ,copyMem_thr5_eu2u_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr5_eu2u_cfg;
}

static config_info pha_internal_thr6_u2e_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr6_u2e_cfg_HA_Table_Own_device_Own_Device     ) , 4    ,0   }     /* offset:0  */
,{STR(pha_internal_thr6_u2e_cfg_HA_Table_Own_device_Reserved       ) ,31    ,5   }     /* offset:5  */

,{STR(pha_internal_thr6_u2e_cfg_vlan_TPID                          ) ,47    ,32   }    /* offset:32  */
,{STR(pha_internal_thr6_u2e_cfg_vlan_up                            ) ,50    ,48  }     /* offset:48 */
,{STR(pha_internal_thr6_u2e_cfg_vlan_cfi                           ) ,51    ,51  }     /* offset:51 */
,{STR(pha_internal_thr6_u2e_cfg_vlan_vid                           ) ,63    ,52  }     /* offset:52 */


,{STR(pha_internal_thr6_u2e_cfg_HA_Table_reserved_space_reserved_0 ) ,95    ,64  }     /* offset:64 */
,{STR(pha_internal_thr6_u2e_cfg_HA_Table_reserved_space_reserved_1 ) ,127   ,96  }     /* offset:96 */


,{STR(pha_internal_thr6_u2e_cfg_srcPortEntry_PVID                  ) ,139   ,128 }     /* offset:128 */
,{STR(pha_internal_thr6_u2e_cfg_srcPortEntry_src_port_num          ) ,143   ,140 }     /* offset:140 */
,{STR(pha_internal_thr6_u2e_cfg_srcPortEntry_reserved              ) ,159   ,144 }     /* offset:144 */


,{STR(pha_internal_thr6_u2e_cfg_targetPortEntry_TPID               ) ,175   ,160 }     /* offset:160 */
,{STR(pha_internal_thr6_u2e_cfg_targetPortEntry_TargetPort         ) ,179   ,176 }     /* offset:176 */
,{STR(pha_internal_thr6_u2e_cfg_targetPortEntry_reserved_0         ) ,191   ,180 }     /* offset:189 */
,{STR(pha_internal_thr6_u2e_cfg_targetPortEntry_reserved_1         ) ,203   ,192 }     /* offset:192 */
,{STR(pha_internal_thr6_u2e_cfg_targetPortEntry_Egress_Pipe_Delay  ) ,223   ,204 }     /* offset:204 */

};

typedef enum{

     pha_internal_thr6_u2e_cfg_HA_Table_Own_device_Own_Device           /* offset:0  */
    ,pha_internal_thr6_u2e_cfg_HA_Table_Own_device_Reserved             /* offset:5  */

    ,pha_internal_thr6_u2e_cfg_vlan_TPID                                /* offset:32  */
    ,pha_internal_thr6_u2e_cfg_vlan_up                                  /* offset:48 */
    ,pha_internal_thr6_u2e_cfg_vlan_cfi                                 /* offset:51 */
    ,pha_internal_thr6_u2e_cfg_vlan_vid                                 /* offset:52 */


    ,pha_internal_thr6_u2e_cfg_HA_Table_reserved_space_reserved_0       /* offset:64 */
    ,pha_internal_thr6_u2e_cfg_HA_Table_reserved_space_reserved_1       /* offset:96 */


    ,pha_internal_thr6_u2e_cfg_srcPortEntry_PVID                        /* offset:128 */
    ,pha_internal_thr6_u2e_cfg_srcPortEntry_src_port_num                /* offset:140 */
    ,pha_internal_thr6_u2e_cfg_srcPortEntry_reserved                    /* offset:144 */


    ,pha_internal_thr6_u2e_cfg_targetPortEntry_TPID                     /* offset:160 */
    ,pha_internal_thr6_u2e_cfg_targetPortEntry_TargetPort               /* offset:176 */
    ,pha_internal_thr6_u2e_cfg_targetPortEntry_reserved_0               /* offset:189 */
    ,pha_internal_thr6_u2e_cfg_targetPortEntry_reserved_1               /* offset:192 */
    ,pha_internal_thr6_u2e_cfg_targetPortEntry_Egress_Pipe_Delay        /* offset:204 */

}pha_internal_thr6_u2e_cfg_enum;

#define thr6_u2e_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr6_u2e_cfg,littleEndianMem,field)

static struct thr6_u2e_cfg copyMem_thr6_u2e_cfg;
struct thr6_u2e_cfg * pipeMemCast_thr6_u2e_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_HA_Table_Own_device_Own_Device     ,copyMem_thr6_u2e_cfg.HA_Table_Own_device.Own_Device     );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_HA_Table_Own_device_Reserved       ,copyMem_thr6_u2e_cfg.HA_Table_Own_device.Reserved       );

    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_vlan_TPID                          ,copyMem_thr6_u2e_cfg.vlan.TPID                          );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_vlan_up                            ,copyMem_thr6_u2e_cfg.vlan.up                            );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_vlan_cfi                           ,copyMem_thr6_u2e_cfg.vlan.cfi                           );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_vlan_vid                           ,copyMem_thr6_u2e_cfg.vlan.vid                           );


    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_HA_Table_reserved_space_reserved_0 ,copyMem_thr6_u2e_cfg.HA_Table_reserved_space.reserved_0 );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_HA_Table_reserved_space_reserved_1 ,copyMem_thr6_u2e_cfg.HA_Table_reserved_space.reserved_1 );


    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_srcPortEntry_PVID                  ,copyMem_thr6_u2e_cfg.srcPortEntry.PVID                  );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_srcPortEntry_src_port_num          ,copyMem_thr6_u2e_cfg.srcPortEntry.src_port_num          );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_srcPortEntry_reserved              ,copyMem_thr6_u2e_cfg.srcPortEntry.reserved              );


    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_targetPortEntry_TPID               ,copyMem_thr6_u2e_cfg.targetPortEntry.TPID               );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_targetPortEntry_TargetPort         ,copyMem_thr6_u2e_cfg.targetPortEntry.TargetPort         );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_targetPortEntry_reserved_0         ,copyMem_thr6_u2e_cfg.targetPortEntry.reserved_0         );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_targetPortEntry_reserved_1         ,copyMem_thr6_u2e_cfg.targetPortEntry.reserved_1         );
    thr6_u2e_cfg_CONFIG_GET(pha_internal_thr6_u2e_cfg_targetPortEntry_Egress_Pipe_Delay  ,copyMem_thr6_u2e_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr6_u2e_cfg;
}

static config_info pha_internal_thr7_mrr2e_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_0 ) ,31    ,0   }     /* offset:0  */
,{STR(pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_1 ) ,63    ,32  }     /* offset:32 */
,{STR(pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_2 ) ,95    ,64  }     /* offset:64 */
,{STR(pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_3 ) ,127   ,96  }     /* offset:96 */


,{STR(pha_internal_thr7_mrr2e_cfg_srcPortEntry_PVID                  ) ,139   ,128 }     /* offset:128 */
,{STR(pha_internal_thr7_mrr2e_cfg_srcPortEntry_src_port_num          ) ,143   ,140 }     /* offset:140 */
,{STR(pha_internal_thr7_mrr2e_cfg_srcPortEntry_reserved              ) ,159   ,144 }     /* offset:144 */


,{STR(pha_internal_thr7_mrr2e_cfg_targetPortEntry_TPID               ) ,175   ,160 }     /* offset:160 */
,{STR(pha_internal_thr7_mrr2e_cfg_targetPortEntry_PVID               ) ,187   ,176 }     /* offset:176 */
,{STR(pha_internal_thr7_mrr2e_cfg_targetPortEntry_VlanEgrTagState    ) ,188   ,188 }     /* offset:188 */
,{STR(pha_internal_thr7_mrr2e_cfg_targetPortEntry_reserved_0         ) ,191   ,189 }     /* offset:189 */
,{STR(pha_internal_thr7_mrr2e_cfg_targetPortEntry_reserved_1         ) ,203   ,192 }     /* offset:192 */
,{STR(pha_internal_thr7_mrr2e_cfg_targetPortEntry_Egress_Pipe_Delay  ) ,223   ,204 }     /* offset:204 */

};

typedef enum{
     pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_0 /* offset:0  */
    ,pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_1 /* offset:32 */
    ,pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_2 /* offset:64 */
    ,pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_3 /* offset:96 */


    ,pha_internal_thr7_mrr2e_cfg_srcPortEntry_PVID                  /* offset:128 */
    ,pha_internal_thr7_mrr2e_cfg_srcPortEntry_src_port_num          /* offset:140 */
    ,pha_internal_thr7_mrr2e_cfg_srcPortEntry_reserved              /* offset:144 */


    ,pha_internal_thr7_mrr2e_cfg_targetPortEntry_TPID               /* offset:160 */
    ,pha_internal_thr7_mrr2e_cfg_targetPortEntry_PVID               /* offset:176 */
    ,pha_internal_thr7_mrr2e_cfg_targetPortEntry_VlanEgrTagState    /* offset:188 */
    ,pha_internal_thr7_mrr2e_cfg_targetPortEntry_reserved_0         /* offset:189 */
    ,pha_internal_thr7_mrr2e_cfg_targetPortEntry_reserved_1         /* offset:192 */
    ,pha_internal_thr7_mrr2e_cfg_targetPortEntry_Egress_Pipe_Delay  /* offset:204 */

}pha_internal_thr7_mrr2e_cfg_enum;

#define thr7_mrr2e_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr7_mrr2e_cfg,littleEndianMem,field)

static struct thr7_mrr2e_cfg copyMem_thr7_mrr2e_cfg;
struct thr7_mrr2e_cfg * pipeMemCast_thr7_mrr2e_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_0 ,copyMem_thr7_mrr2e_cfg.HA_Table_reserved_space.reserved_0 );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_1 ,copyMem_thr7_mrr2e_cfg.HA_Table_reserved_space.reserved_1 );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_2 ,copyMem_thr7_mrr2e_cfg.HA_Table_reserved_space.reserved_2 );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_HA_Table_reserved_space_reserved_3 ,copyMem_thr7_mrr2e_cfg.HA_Table_reserved_space.reserved_3 );


    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_srcPortEntry_PVID                  ,copyMem_thr7_mrr2e_cfg.srcPortEntry.PVID                  );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_srcPortEntry_src_port_num          ,copyMem_thr7_mrr2e_cfg.srcPortEntry.src_port_num          );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_srcPortEntry_reserved              ,copyMem_thr7_mrr2e_cfg.srcPortEntry.reserved              );


    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_targetPortEntry_TPID               ,copyMem_thr7_mrr2e_cfg.targetPortEntry.TPID               );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_targetPortEntry_PVID               ,copyMem_thr7_mrr2e_cfg.targetPortEntry.PVID               );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_targetPortEntry_VlanEgrTagState    ,copyMem_thr7_mrr2e_cfg.targetPortEntry.VlanEgrTagState    );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_targetPortEntry_reserved_0         ,copyMem_thr7_mrr2e_cfg.targetPortEntry.reserved_0         );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_targetPortEntry_reserved_1         ,copyMem_thr7_mrr2e_cfg.targetPortEntry.reserved_1         );
    thr7_mrr2e_cfg_CONFIG_GET(pha_internal_thr7_mrr2e_cfg_targetPortEntry_Egress_Pipe_Delay  ,copyMem_thr7_mrr2e_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr7_mrr2e_cfg;
}

#define in_hdr_HERADER_GET(field , targetValue) \
    targetValue = getHeaderInfo(pha_internal_in_hdr,littleEndianMem,field)

static struct thr7_mrr2e_in_hdr copyMem_thr7_mrr2e_in_hdr;
struct thr7_mrr2e_in_hdr* pipeMemCast_thr7_mrr2e_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr7_mrr2e_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr7_mrr2e_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr7_mrr2e_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr7_mrr2e_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr7_mrr2e_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr7_mrr2e_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr7_mrr2e_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr7_mrr2e_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr7_mrr2e_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr7_mrr2e_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr7_mrr2e_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr7_mrr2e_in_hdr.mac_header.mac_sa_15_0  );

    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_tagCommand    , copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.tagCommand      );
    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_TrgTagged     , copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.TrgTagged       );
    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_TrgDev        , copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.TrgDev          );
    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_TrgPhyPort_4_0, copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.TrgPhyPort_4_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_use_eVIDX     , copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.use_eVIDX       );
    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_TC0           , copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.TC0             );
    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_cfi           , copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.cfi             );
    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_up            , copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.up              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_Extend        , copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.Extend          );
    in_hdr_HERADER_GET(pha_internal_in_hdr_DSA_from_cpu_eVLAN         , copyMem_thr7_mrr2e_in_hdr.DSA_from_cpu.eVLAN           );

    return &copyMem_thr7_mrr2e_in_hdr;
};


static config_info pha_internal_thr8_e_v2u_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_TagCommand             ),  1  , 0  } /* offset:0 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Tag0SrcTagged          ),  2  , 2  } /* offset:2 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_SrcDev_4_0             ),  7  , 3  } /* offset:3 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Src_ePort_4_0          ),  12 , 8  } /* offset:8 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_SrcIsTrunk             ),  13 , 13 } /* offset:13 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Reserved0              ),  14 , 14 } /* offset:14 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_CFI                    ),  15 , 15 } /* offset:15 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_UP                     ),  18 , 16 } /* offset:16 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Extend0                ),  19 , 19 } /* offset:19 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_eVLAN_11_0             ),  31 , 20 } /* offset:20 */


,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_Extend1                ),  32 , 32 } /* offset:32 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_Src_ePort_6_5          ),  34 , 33 } /* offset:33 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_EgressFilterRegistered ),  35 , 35 } /* offset:35 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_DropOnSource           ),  36 , 36 } /* offset:36 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_PacketIsLooped         ),  37 , 37 } /* offset:37 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_Routed                 ),  38 , 38 } /* offset:38 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_SrcID_4_0              ),  43 , 39 } /* offset:39 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile      ),  50 , 44 } /* offset:44 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_use_eVIDX              ),  51 , 51 } /* offset:51 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0         ),  58 , 52 } /* offset:52 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_TrgDev_4_0             ),  63 , 59 } /* offset:59 */


,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Extend2                ),  64 , 64 } /* offset:64 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Reserved1              ),  65 , 65 } /* offset:65 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid      ),  66 , 66 } /* offset:66 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_TrgPhyPort_7           ),  67 , 67 } /* offset:67 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_SrcID_11_5             ),  74 , 68 } /* offset:68 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_SrcDev_11_5            ),  81 , 75 } /* offset:75 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Reserved2              ),  82 , 82 } /* offset:82 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Src_ePort_16_7         ),  92 , 83 } /* offset:83 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_TPID_index             ),  95 , 93 } /* offset:93 */


,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Extend3                ),  96 , 96 } /* offset:96 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_eVLAN_15_12            ),  100, 97 } /* offset:97 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Tag1SrcTagged          ),  101, 101} /* offset:101 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag      ),  102, 102} /* offset:102 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Reserved3              ),  103, 103} /* offset:103 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Trg_ePort              ),  120, 104} /* offset:104 */
,{STR(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_TrgDev                 ),  127, 121} /* offset:121 */


,{STR(pha_internal_thr8_e_v2u_cfg_srcPortEntry_src_ePort_13_7        ),  134, 128} /* offset:128 */
,{STR(pha_internal_thr8_e_v2u_cfg_srcPortEntry_src_ePort_6_5         ),  136, 135} /* offset:135 */
,{STR(pha_internal_thr8_e_v2u_cfg_srcPortEntry_src_ePort_4_0         ),  141, 137} /* offset:137 */
,{STR(pha_internal_thr8_e_v2u_cfg_srcPortEntry_reserved1             ),  158, 142} /* offset:142 */
,{STR(pha_internal_thr8_e_v2u_cfg_srcPortEntry_reserved2             ),  159, 159} /* offset:159 */


,{STR(pha_internal_thr8_e_v2u_cfg_targetPortEntry_TPID               ),  175, 160} /* offset:160 */
,{STR(pha_internal_thr8_e_v2u_cfg_targetPortEntry_PVID               ),  187, 176} /* offset:176 */
,{STR(pha_internal_thr8_e_v2u_cfg_targetPortEntry_VlanEgrTagState    ),  188, 188} /* offset:188 */
,{STR(pha_internal_thr8_e_v2u_cfg_targetPortEntry_trg_ePort_13_11    ),  191, 189} /* offset:189 */
,{STR(pha_internal_thr8_e_v2u_cfg_targetPortEntry_trg_ePort_10_0     ),  202, 192} /* offset:192 */
,{STR(pha_internal_thr8_e_v2u_cfg_targetPortEntry_reserved           ),  203, 203} /* offset:203 */
,{STR(pha_internal_thr8_e_v2u_cfg_targetPortEntry_Egress_Pipe_Delay  ),  223, 204} /* offset:204 */
};


typedef enum{
     pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_TagCommand                 /* offset:0 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Tag0SrcTagged              /* offset:2 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_SrcDev_4_0                 /* offset:3 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Src_ePort_4_0              /* offset:8 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_SrcIsTrunk                 /* offset:13 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Reserved0                  /* offset:14 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_CFI                        /* offset:15 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_UP                         /* offset:16 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Extend0                    /* offset:19 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_eVLAN_11_0                 /* offset:20 */


    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_Extend1                    /* offset:32 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_Src_ePort_6_5              /* offset:33 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_EgressFilterRegistered     /* offset:35 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_DropOnSource               /* offset:36 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_PacketIsLooped             /* offset:37 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_Routed                     /* offset:38 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_SrcID_4_0                  /* offset:39 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile          /* offset:44 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_use_eVIDX                  /* offset:51 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0             /* offset:52 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_TrgDev_4_0                 /* offset:59 */


    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Extend2                    /* offset:64 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Reserved1                  /* offset:65 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid          /* offset:66 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_TrgPhyPort_7               /* offset:67 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_SrcID_11_5                 /* offset:68 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_SrcDev_11_5                /* offset:75 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Reserved2                  /* offset:82 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Src_ePort_16_7             /* offset:83 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_TPID_index                 /* offset:93 */


    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Extend3                    /* offset:96 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_eVLAN_15_12                /* offset:97 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Tag1SrcTagged              /* offset:101 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag          /* offset:102 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Reserved3                  /* offset:103 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Trg_ePort                  /* offset:104 */
    ,pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_TrgDev                     /* offset:121 */


    ,pha_internal_thr8_e_v2u_cfg_srcPortEntry_src_ePort_13_7            /* offset:128 */
    ,pha_internal_thr8_e_v2u_cfg_srcPortEntry_src_ePort_6_5             /* offset:135 */
    ,pha_internal_thr8_e_v2u_cfg_srcPortEntry_src_ePort_4_0             /* offset:137 */
    ,pha_internal_thr8_e_v2u_cfg_srcPortEntry_reserved1                 /* offset:142 */
    ,pha_internal_thr8_e_v2u_cfg_srcPortEntry_reserved2                 /* offset:159 */


    ,pha_internal_thr8_e_v2u_cfg_targetPortEntry_TPID                   /* offset:160 */
    ,pha_internal_thr8_e_v2u_cfg_targetPortEntry_PVID                   /* offset:176 */
    ,pha_internal_thr8_e_v2u_cfg_targetPortEntry_VlanEgrTagState        /* offset:188 */
    ,pha_internal_thr8_e_v2u_cfg_targetPortEntry_trg_ePort_13_11        /* offset:189 */
    ,pha_internal_thr8_e_v2u_cfg_targetPortEntry_trg_ePort_10_0         /* offset:192 */
    ,pha_internal_thr8_e_v2u_cfg_targetPortEntry_reserved               /* offset:203 */
    ,pha_internal_thr8_e_v2u_cfg_targetPortEntry_Egress_Pipe_Delay      /* offset:204 */
}pha_internal_thr8_e_v2u_cfg_enum;

#define thr8_e_v2u_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr8_e_v2u_cfg,littleEndianMem,field)

static struct thr8_e_v2u_cfg copyMem_thr8_e_v2u_cfg;
struct thr8_e_v2u_cfg * pipeMemCast_thr8_e_v2u_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);


    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_TagCommand               , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.TagCommand             );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Tag0SrcTagged            , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.Tag0SrcTagged          );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_SrcDev_4_0               , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.SrcDev_4_0             );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Src_ePort_4_0            , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.Src_ePort_4_0          );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_SrcIsTrunk               , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.SrcIsTrunk             );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Reserved0                , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.Reserved0              );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_CFI                      , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.CFI                    );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_UP                       , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.UP                     );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_Extend0                  , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.Extend0                );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w0_eVLAN_11_0               , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w0.eVLAN_11_0             );


    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_Extend1                  , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.Extend1                );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_Src_ePort_6_5            , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.Src_ePort_6_5          );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_EgressFilterRegistered   , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.EgressFilterRegistered );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_DropOnSource             , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.DropOnSource           );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_PacketIsLooped           , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.PacketIsLooped         );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_Routed                   , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.Routed                 );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_SrcID_4_0                , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.SrcID_4_0              );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile        , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.GlobalQoS_Profile      );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_use_eVIDX                , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.use_eVIDX              );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0           , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.TrgPhyPort_6_0         );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w1_TrgDev_4_0               , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w1.TrgDev_4_0             );


    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Extend2                  , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w2.Extend2                );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Reserved1                , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w2.Reserved1              );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid        , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w2.IsTrgPhyPortValid      );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_TrgPhyPort_7             , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w2.TrgPhyPort_7           );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_SrcID_11_5               , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w2.SrcID_11_5             );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_SrcDev_11_5              , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w2.SrcDev_11_5            );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Reserved2                , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w2.Reserved2              );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_Src_ePort_16_7           , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w2.Src_ePort_16_7         );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w2_TPID_index               , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w2.TPID_index             );


    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Extend3                  , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w3.Extend3                );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_eVLAN_15_12              , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w3.eVLAN_15_12            );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Tag1SrcTagged            , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w3.Tag1SrcTagged          );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag        , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w3.SrcTag0IsOuterTag      );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Reserved3                , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w3.Reserved3              );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_Trg_ePort                , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w3.Trg_ePort              );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_eDSA_fwd_w3_TrgDev                   , copyMem_thr8_e_v2u_cfg.eDSA_fwd_w3.TrgDev                 );


    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_srcPortEntry_src_ePort_13_7          , copyMem_thr8_e_v2u_cfg.srcPortEntry.src_ePort_13_7        );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_srcPortEntry_src_ePort_6_5           , copyMem_thr8_e_v2u_cfg.srcPortEntry.src_ePort_6_5         );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_srcPortEntry_src_ePort_4_0           , copyMem_thr8_e_v2u_cfg.srcPortEntry.src_ePort_4_0         );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_srcPortEntry_reserved1               , copyMem_thr8_e_v2u_cfg.srcPortEntry.reserved1             );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_srcPortEntry_reserved2               , copyMem_thr8_e_v2u_cfg.srcPortEntry.reserved2             );


    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_targetPortEntry_TPID                 , copyMem_thr8_e_v2u_cfg.targetPortEntry.TPID               );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_targetPortEntry_PVID                 , copyMem_thr8_e_v2u_cfg.targetPortEntry.PVID               );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_targetPortEntry_VlanEgrTagState      , copyMem_thr8_e_v2u_cfg.targetPortEntry.VlanEgrTagState    );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_targetPortEntry_trg_ePort_13_11      , copyMem_thr8_e_v2u_cfg.targetPortEntry.trg_ePort_13_11    );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_targetPortEntry_trg_ePort_10_0       , copyMem_thr8_e_v2u_cfg.targetPortEntry.trg_ePort_10_0     );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_targetPortEntry_reserved             , copyMem_thr8_e_v2u_cfg.targetPortEntry.reserved           );
    thr8_e_v2u_cfg_CONFIG_GET(pha_internal_thr8_e_v2u_cfg_targetPortEntry_Egress_Pipe_Delay    , copyMem_thr8_e_v2u_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr8_e_v2u_cfg;
}

static config_info pha_internal_thr9_e2u_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_TagCommand             ),  1  , 0  } /* offset:0 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Tag0SrcTagged          ),  2  , 2  } /* offset:2 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_SrcDev_4_0             ),  7  , 3  } /* offset:3 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Src_ePort_4_0          ),  12 , 8  } /* offset:8 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_SrcIsTrunk             ),  13 , 13 } /* offset:13 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Reserved0              ),  14 , 14 } /* offset:14 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_CFI                    ),  15 , 15 } /* offset:15 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_UP                     ),  18 , 16 } /* offset:16 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Extend0                ),  19 , 19 } /* offset:19 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_eVLAN_11_0             ),  31 , 20 } /* offset:20 */


,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_Extend1                ),  32 , 32 } /* offset:32 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_Src_ePort_6_5          ),  34 , 33 } /* offset:33 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_EgressFilterRegistered ),  35 , 35 } /* offset:35 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_DropOnSource           ),  36 , 36 } /* offset:36 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_PacketIsLooped         ),  37 , 37 } /* offset:37 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_Routed                 ),  38 , 38 } /* offset:38 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_SrcID_4_0              ),  43 , 39 } /* offset:39 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile      ),  50 , 44 } /* offset:44 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_use_eVIDX              ),  51 , 51 } /* offset:51 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0         ),  58 , 52 } /* offset:52 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_TrgDev_4_0             ),  63 , 59 } /* offset:59 */


,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Extend2                ),  64 , 64 } /* offset:64 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Reserved1              ),  65 , 65 } /* offset:65 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid      ),  66 , 66 } /* offset:66 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_TrgPhyPort_7           ),  67 , 67 } /* offset:67 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_SrcID_11_5             ),  74 , 68 } /* offset:68 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_SrcDev_11_5            ),  81 , 75 } /* offset:75 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Reserved2              ),  82 , 82 } /* offset:82 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Src_ePort_16_7         ),  92 , 83 } /* offset:83 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_TPID_index             ),  95 , 93 } /* offset:93 */


,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Extend3                ),  96 , 96 } /* offset:96 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_eVLAN_15_12            ),  100, 97 } /* offset:97 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Tag1SrcTagged          ),  101, 101} /* offset:101 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag      ),  102, 102} /* offset:102 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Reserved3              ),  103, 103} /* offset:103 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Trg_ePort              ),  120, 104} /* offset:104 */
,{STR(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_TrgDev                 ),  127, 121} /* offset:121 */


,{STR(pha_internal_thr9_e2u_cfg_srcPortEntry_src_ePort_13_7        ),  134, 128} /* offset:128 */
,{STR(pha_internal_thr9_e2u_cfg_srcPortEntry_src_ePort_6_5         ),  136, 135} /* offset:135 */
,{STR(pha_internal_thr9_e2u_cfg_srcPortEntry_src_ePort_4_0         ),  141, 137} /* offset:137 */
,{STR(pha_internal_thr9_e2u_cfg_srcPortEntry_reserved1             ),  158, 142} /* offset:142 */
,{STR(pha_internal_thr9_e2u_cfg_srcPortEntry_reserved2             ),  159, 159} /* offset:159 */


,{STR(pha_internal_thr9_e2u_cfg_targetPortEntry_TPID               ),  175, 160} /* offset:160 */
,{STR(pha_internal_thr9_e2u_cfg_targetPortEntry_PVID               ),  187, 176} /* offset:176 */
,{STR(pha_internal_thr9_e2u_cfg_targetPortEntry_VlanEgrTagState    ),  188, 188} /* offset:188 */
,{STR(pha_internal_thr9_e2u_cfg_targetPortEntry_trg_ePort_13_11    ),  191, 189} /* offset:189 */
,{STR(pha_internal_thr9_e2u_cfg_targetPortEntry_trg_ePort_10_0     ),  202, 192} /* offset:192 */
,{STR(pha_internal_thr9_e2u_cfg_targetPortEntry_reserved           ),  203, 203} /* offset:203 */
,{STR(pha_internal_thr9_e2u_cfg_targetPortEntry_Egress_Pipe_Delay  ),  223, 204} /* offset:204 */
};


typedef enum{
     pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_TagCommand                 /* offset:0 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Tag0SrcTagged              /* offset:2 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_SrcDev_4_0                 /* offset:3 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Src_ePort_4_0              /* offset:8 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_SrcIsTrunk                 /* offset:13 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Reserved0                  /* offset:14 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_CFI                        /* offset:15 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_UP                         /* offset:16 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Extend0                    /* offset:19 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_eVLAN_11_0                 /* offset:20 */


    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_Extend1                    /* offset:32 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_Src_ePort_6_5              /* offset:33 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_EgressFilterRegistered     /* offset:35 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_DropOnSource               /* offset:36 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_PacketIsLooped             /* offset:37 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_Routed                     /* offset:38 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_SrcID_4_0                  /* offset:39 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile          /* offset:44 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_use_eVIDX                  /* offset:51 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0             /* offset:52 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_TrgDev_4_0                 /* offset:59 */


    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Extend2                    /* offset:64 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Reserved1                  /* offset:65 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid          /* offset:66 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_TrgPhyPort_7               /* offset:67 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_SrcID_11_5                 /* offset:68 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_SrcDev_11_5                /* offset:75 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Reserved2                  /* offset:82 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Src_ePort_16_7             /* offset:83 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_TPID_index                 /* offset:93 */


    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Extend3                    /* offset:96 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_eVLAN_15_12                /* offset:97 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Tag1SrcTagged              /* offset:101 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag          /* offset:102 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Reserved3                  /* offset:103 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Trg_ePort                  /* offset:104 */
    ,pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_TrgDev                     /* offset:121 */


    ,pha_internal_thr9_e2u_cfg_srcPortEntry_src_ePort_13_7            /* offset:128 */
    ,pha_internal_thr9_e2u_cfg_srcPortEntry_src_ePort_6_5             /* offset:135 */
    ,pha_internal_thr9_e2u_cfg_srcPortEntry_src_ePort_4_0             /* offset:137 */
    ,pha_internal_thr9_e2u_cfg_srcPortEntry_reserved1                 /* offset:142 */
    ,pha_internal_thr9_e2u_cfg_srcPortEntry_reserved2                 /* offset:159 */


    ,pha_internal_thr9_e2u_cfg_targetPortEntry_TPID                   /* offset:160 */
    ,pha_internal_thr9_e2u_cfg_targetPortEntry_PVID                   /* offset:176 */
    ,pha_internal_thr9_e2u_cfg_targetPortEntry_VlanEgrTagState        /* offset:188 */
    ,pha_internal_thr9_e2u_cfg_targetPortEntry_trg_ePort_13_11        /* offset:189 */
    ,pha_internal_thr9_e2u_cfg_targetPortEntry_trg_ePort_10_0         /* offset:192 */
    ,pha_internal_thr9_e2u_cfg_targetPortEntry_reserved               /* offset:203 */
    ,pha_internal_thr9_e2u_cfg_targetPortEntry_Egress_Pipe_Delay      /* offset:204 */
}pha_internal_thr9_e2u_cfg_enum;

#define thr9_e2u_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr9_e2u_cfg,littleEndianMem,field)

static struct thr9_e2u_cfg copyMem_thr9_e2u_cfg;
struct thr9_e2u_cfg * pipeMemCast_thr9_e2u_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);


    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_TagCommand               , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.TagCommand             );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Tag0SrcTagged            , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.Tag0SrcTagged          );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_SrcDev_4_0               , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.SrcDev_4_0             );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Src_ePort_4_0            , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.Src_ePort_4_0          );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_SrcIsTrunk               , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.SrcIsTrunk             );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Reserved0                , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.Reserved0              );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_CFI                      , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.CFI                    );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_UP                       , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.UP                     );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_Extend0                  , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.Extend0                );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w0_eVLAN_11_0               , copyMem_thr9_e2u_cfg.eDSA_fwd_w0.eVLAN_11_0             );


    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_Extend1                  , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.Extend1                );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_Src_ePort_6_5            , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.Src_ePort_6_5          );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_EgressFilterRegistered   , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.EgressFilterRegistered );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_DropOnSource             , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.DropOnSource           );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_PacketIsLooped           , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.PacketIsLooped         );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_Routed                   , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.Routed                 );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_SrcID_4_0                , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.SrcID_4_0              );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile        , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.GlobalQoS_Profile      );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_use_eVIDX                , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.use_eVIDX              );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0           , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.TrgPhyPort_6_0         );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w1_TrgDev_4_0               , copyMem_thr9_e2u_cfg.eDSA_fwd_w1.TrgDev_4_0             );


    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Extend2                  , copyMem_thr9_e2u_cfg.eDSA_fwd_w2.Extend2                );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Reserved1                , copyMem_thr9_e2u_cfg.eDSA_fwd_w2.Reserved1              );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid        , copyMem_thr9_e2u_cfg.eDSA_fwd_w2.IsTrgPhyPortValid      );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_TrgPhyPort_7             , copyMem_thr9_e2u_cfg.eDSA_fwd_w2.TrgPhyPort_7           );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_SrcID_11_5               , copyMem_thr9_e2u_cfg.eDSA_fwd_w2.SrcID_11_5             );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_SrcDev_11_5              , copyMem_thr9_e2u_cfg.eDSA_fwd_w2.SrcDev_11_5            );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Reserved2                , copyMem_thr9_e2u_cfg.eDSA_fwd_w2.Reserved2              );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_Src_ePort_16_7           , copyMem_thr9_e2u_cfg.eDSA_fwd_w2.Src_ePort_16_7         );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w2_TPID_index               , copyMem_thr9_e2u_cfg.eDSA_fwd_w2.TPID_index             );


    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Extend3                  , copyMem_thr9_e2u_cfg.eDSA_fwd_w3.Extend3                );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_eVLAN_15_12              , copyMem_thr9_e2u_cfg.eDSA_fwd_w3.eVLAN_15_12            );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Tag1SrcTagged            , copyMem_thr9_e2u_cfg.eDSA_fwd_w3.Tag1SrcTagged          );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag        , copyMem_thr9_e2u_cfg.eDSA_fwd_w3.SrcTag0IsOuterTag      );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Reserved3                , copyMem_thr9_e2u_cfg.eDSA_fwd_w3.Reserved3              );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_Trg_ePort                , copyMem_thr9_e2u_cfg.eDSA_fwd_w3.Trg_ePort              );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_eDSA_fwd_w3_TrgDev                   , copyMem_thr9_e2u_cfg.eDSA_fwd_w3.TrgDev                 );


    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_srcPortEntry_src_ePort_13_7          , copyMem_thr9_e2u_cfg.srcPortEntry.src_ePort_13_7        );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_srcPortEntry_src_ePort_6_5           , copyMem_thr9_e2u_cfg.srcPortEntry.src_ePort_6_5         );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_srcPortEntry_src_ePort_4_0           , copyMem_thr9_e2u_cfg.srcPortEntry.src_ePort_4_0         );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_srcPortEntry_reserved1               , copyMem_thr9_e2u_cfg.srcPortEntry.reserved1             );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_srcPortEntry_reserved2               , copyMem_thr9_e2u_cfg.srcPortEntry.reserved2             );


    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_targetPortEntry_TPID                 , copyMem_thr9_e2u_cfg.targetPortEntry.TPID               );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_targetPortEntry_PVID                 , copyMem_thr9_e2u_cfg.targetPortEntry.PVID               );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_targetPortEntry_VlanEgrTagState      , copyMem_thr9_e2u_cfg.targetPortEntry.VlanEgrTagState    );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_targetPortEntry_trg_ePort_13_11      , copyMem_thr9_e2u_cfg.targetPortEntry.trg_ePort_13_11    );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_targetPortEntry_trg_ePort_10_0       , copyMem_thr9_e2u_cfg.targetPortEntry.trg_ePort_10_0     );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_targetPortEntry_reserved             , copyMem_thr9_e2u_cfg.targetPortEntry.reserved           );
    thr9_e2u_cfg_CONFIG_GET(pha_internal_thr9_e2u_cfg_targetPortEntry_Egress_Pipe_Delay    , copyMem_thr9_e2u_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr9_e2u_cfg;
}

static config_info pha_internal_thr10_c_v2u_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_TagCommand             ),  1  , 0  } /* offset:0 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Tag0SrcTagged          ),  2  , 2  } /* offset:2 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_SrcDev_4_0             ),  7  , 3  } /* offset:3 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Src_ePort_4_0          ),  12 , 8  } /* offset:8 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_SrcIsTrunk             ),  13 , 13 } /* offset:13 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Reserved0              ),  14 , 14 } /* offset:14 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_CFI                    ),  15 , 15 } /* offset:15 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_UP                     ),  18 , 16 } /* offset:16 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Extend0                ),  19 , 19 } /* offset:19 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_eVLAN_11_0             ),  31 , 20 } /* offset:20 */


,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_Extend1                ),  32 , 32 } /* offset:32 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_Src_ePort_6_5          ),  34 , 33 } /* offset:33 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_EgressFilterRegistered ),  35 , 35 } /* offset:35 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_DropOnSource           ),  36 , 36 } /* offset:36 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_PacketIsLooped         ),  37 , 37 } /* offset:37 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_Routed                 ),  38 , 38 } /* offset:38 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_SrcID_4_0              ),  43 , 39 } /* offset:39 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile      ),  50 , 44 } /* offset:44 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_use_eVIDX              ),  51 , 51 } /* offset:51 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0         ),  58 , 52 } /* offset:52 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_TrgDev_4_0             ),  63 , 59 } /* offset:59 */


,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Extend2                ),  64 , 64 } /* offset:64 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Reserved1              ),  65 , 65 } /* offset:65 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid      ),  66 , 66 } /* offset:66 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_TrgPhyPort_7           ),  67 , 67 } /* offset:67 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_SrcID_11_5             ),  74 , 68 } /* offset:68 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_SrcDev_11_5            ),  81 , 75 } /* offset:75 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Reserved2              ),  82 , 82 } /* offset:82 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Src_ePort_16_7         ),  92 , 83 } /* offset:83 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_TPID_index             ),  95 , 93 } /* offset:93 */


,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Extend3                ),  96 , 96 } /* offset:96 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_eVLAN_15_12            ),  100, 97 } /* offset:97 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Tag1SrcTagged          ),  101, 101} /* offset:101 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag      ),  102, 102} /* offset:102 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Reserved3              ),  103, 103} /* offset:103 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Trg_ePort              ),  120, 104} /* offset:104 */
,{STR(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_TrgDev                 ),  127, 121} /* offset:121 */


,{STR(pha_internal_thr10_c_v2u_cfg_srcPortEntry_src_ePort_13_7        ),  134, 128} /* offset:128 */
,{STR(pha_internal_thr10_c_v2u_cfg_srcPortEntry_src_ePort_6_5         ),  136, 135} /* offset:135 */
,{STR(pha_internal_thr10_c_v2u_cfg_srcPortEntry_src_ePort_4_0         ),  141, 137} /* offset:137 */
,{STR(pha_internal_thr10_c_v2u_cfg_srcPortEntry_reserved1             ),  158, 142} /* offset:142 */
,{STR(pha_internal_thr10_c_v2u_cfg_srcPortEntry_reserved2             ),  159, 159} /* offset:159 */


,{STR(pha_internal_thr10_c_v2u_cfg_targetPortEntry_TPID               ),  175, 160} /* offset:160 */
,{STR(pha_internal_thr10_c_v2u_cfg_targetPortEntry_PVID               ),  187, 176} /* offset:176 */
,{STR(pha_internal_thr10_c_v2u_cfg_targetPortEntry_VlanEgrTagState    ),  188, 188} /* offset:188 */
,{STR(pha_internal_thr10_c_v2u_cfg_targetPortEntry_trg_ePort_13_11    ),  191, 189} /* offset:189 */
,{STR(pha_internal_thr10_c_v2u_cfg_targetPortEntry_trg_ePort_10_0     ),  202, 192} /* offset:192 */
,{STR(pha_internal_thr10_c_v2u_cfg_targetPortEntry_reserved           ),  203, 203} /* offset:203 */
,{STR(pha_internal_thr10_c_v2u_cfg_targetPortEntry_Egress_Pipe_Delay  ),  223, 204} /* offset:204 */
};


typedef enum{
     pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_TagCommand                 /* offset:0 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Tag0SrcTagged              /* offset:2 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_SrcDev_4_0                 /* offset:3 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Src_ePort_4_0              /* offset:8 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_SrcIsTrunk                 /* offset:13 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Reserved0                  /* offset:14 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_CFI                        /* offset:15 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_UP                         /* offset:16 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Extend0                    /* offset:19 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_eVLAN_11_0                 /* offset:20 */


    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_Extend1                    /* offset:32 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_Src_ePort_6_5              /* offset:33 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_EgressFilterRegistered     /* offset:35 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_DropOnSource               /* offset:36 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_PacketIsLooped             /* offset:37 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_Routed                     /* offset:38 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_SrcID_4_0                  /* offset:39 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile          /* offset:44 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_use_eVIDX                  /* offset:51 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0             /* offset:52 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_TrgDev_4_0                 /* offset:59 */


    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Extend2                    /* offset:64 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Reserved1                  /* offset:65 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid          /* offset:66 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_TrgPhyPort_7               /* offset:67 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_SrcID_11_5                 /* offset:68 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_SrcDev_11_5                /* offset:75 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Reserved2                  /* offset:82 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Src_ePort_16_7             /* offset:83 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_TPID_index                 /* offset:93 */


    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Extend3                    /* offset:96 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_eVLAN_15_12                /* offset:97 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Tag1SrcTagged              /* offset:101 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag          /* offset:102 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Reserved3                  /* offset:103 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Trg_ePort                  /* offset:104 */
    ,pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_TrgDev                     /* offset:121 */


    ,pha_internal_thr10_c_v2u_cfg_srcPortEntry_src_ePort_13_7            /* offset:128 */
    ,pha_internal_thr10_c_v2u_cfg_srcPortEntry_src_ePort_6_5             /* offset:135 */
    ,pha_internal_thr10_c_v2u_cfg_srcPortEntry_src_ePort_4_0             /* offset:137 */
    ,pha_internal_thr10_c_v2u_cfg_srcPortEntry_reserved1                 /* offset:142 */
    ,pha_internal_thr10_c_v2u_cfg_srcPortEntry_reserved2                 /* offset:159 */


    ,pha_internal_thr10_c_v2u_cfg_targetPortEntry_TPID                   /* offset:160 */
    ,pha_internal_thr10_c_v2u_cfg_targetPortEntry_PVID                   /* offset:176 */
    ,pha_internal_thr10_c_v2u_cfg_targetPortEntry_VlanEgrTagState        /* offset:188 */
    ,pha_internal_thr10_c_v2u_cfg_targetPortEntry_trg_ePort_13_11        /* offset:189 */
    ,pha_internal_thr10_c_v2u_cfg_targetPortEntry_trg_ePort_10_0         /* offset:192 */
    ,pha_internal_thr10_c_v2u_cfg_targetPortEntry_reserved               /* offset:203 */
    ,pha_internal_thr10_c_v2u_cfg_targetPortEntry_Egress_Pipe_Delay      /* offset:204 */
}pha_internal_thr10_c_v2u_cfg_enum;

#define thr10_c_v2u_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr10_c_v2u_cfg,littleEndianMem,field)

static struct thr10_c_v2u_cfg copyMem_thr10_c_v2u_cfg;
struct thr10_c_v2u_cfg * pipeMemCast_thr10_c_v2u_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);


    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_TagCommand               , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.TagCommand             );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Tag0SrcTagged            , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.Tag0SrcTagged          );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_SrcDev_4_0               , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.SrcDev_4_0             );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Src_ePort_4_0            , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.Src_ePort_4_0          );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_SrcIsTrunk               , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.SrcIsTrunk             );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Reserved0                , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.Reserved0              );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_CFI                      , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.CFI                    );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_UP                       , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.UP                     );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_Extend0                  , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.Extend0                );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w0_eVLAN_11_0               , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w0.eVLAN_11_0             );


    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_Extend1                  , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.Extend1                );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_Src_ePort_6_5            , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.Src_ePort_6_5          );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_EgressFilterRegistered   , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.EgressFilterRegistered );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_DropOnSource             , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.DropOnSource           );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_PacketIsLooped           , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.PacketIsLooped         );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_Routed                   , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.Routed                 );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_SrcID_4_0                , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.SrcID_4_0              );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile        , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.GlobalQoS_Profile      );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_use_eVIDX                , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.use_eVIDX              );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0           , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.TrgPhyPort_6_0         );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w1_TrgDev_4_0               , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w1.TrgDev_4_0             );


    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Extend2                  , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w2.Extend2                );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Reserved1                , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w2.Reserved1              );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid        , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w2.IsTrgPhyPortValid      );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_TrgPhyPort_7             , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w2.TrgPhyPort_7           );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_SrcID_11_5               , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w2.SrcID_11_5             );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_SrcDev_11_5              , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w2.SrcDev_11_5            );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Reserved2                , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w2.Reserved2              );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_Src_ePort_16_7           , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w2.Src_ePort_16_7         );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w2_TPID_index               , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w2.TPID_index             );


    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Extend3                  , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w3.Extend3                );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_eVLAN_15_12              , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w3.eVLAN_15_12            );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Tag1SrcTagged            , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w3.Tag1SrcTagged          );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag        , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w3.SrcTag0IsOuterTag      );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Reserved3                , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w3.Reserved3              );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_Trg_ePort                , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w3.Trg_ePort              );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_eDSA_fwd_w3_TrgDev                   , copyMem_thr10_c_v2u_cfg.eDSA_fwd_w3.TrgDev                 );


    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_srcPortEntry_src_ePort_13_7          , copyMem_thr10_c_v2u_cfg.srcPortEntry.src_ePort_13_7        );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_srcPortEntry_src_ePort_6_5           , copyMem_thr10_c_v2u_cfg.srcPortEntry.src_ePort_6_5         );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_srcPortEntry_src_ePort_4_0           , copyMem_thr10_c_v2u_cfg.srcPortEntry.src_ePort_4_0         );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_srcPortEntry_reserved1               , copyMem_thr10_c_v2u_cfg.srcPortEntry.reserved1             );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_srcPortEntry_reserved2               , copyMem_thr10_c_v2u_cfg.srcPortEntry.reserved2             );


    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_targetPortEntry_TPID                 , copyMem_thr10_c_v2u_cfg.targetPortEntry.TPID               );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_targetPortEntry_PVID                 , copyMem_thr10_c_v2u_cfg.targetPortEntry.PVID               );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_targetPortEntry_VlanEgrTagState      , copyMem_thr10_c_v2u_cfg.targetPortEntry.VlanEgrTagState    );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_targetPortEntry_trg_ePort_13_11      , copyMem_thr10_c_v2u_cfg.targetPortEntry.trg_ePort_13_11    );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_targetPortEntry_trg_ePort_10_0       , copyMem_thr10_c_v2u_cfg.targetPortEntry.trg_ePort_10_0     );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_targetPortEntry_reserved             , copyMem_thr10_c_v2u_cfg.targetPortEntry.reserved           );
    thr10_c_v2u_cfg_CONFIG_GET(pha_internal_thr10_c_v2u_cfg_targetPortEntry_Egress_Pipe_Delay    , copyMem_thr10_c_v2u_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr10_c_v2u_cfg;
}

static struct thr10_c_v2u_in_hdr copyMem_thr10_c_v2u_in_hdr;
struct thr10_c_v2u_in_hdr* pipeMemCast_thr10_c_v2u_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr10_c_v2u_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr10_c_v2u_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr10_c_v2u_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr10_c_v2u_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr10_c_v2u_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr10_c_v2u_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr10_c_v2u_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr10_c_v2u_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr10_c_v2u_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr10_c_v2u_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr10_c_v2u_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr10_c_v2u_in_hdr.mac_header.mac_sa_15_0  );


    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_TPID                  , copyMem_thr10_c_v2u_in_hdr.etag.TPID               );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_PCP                 , copyMem_thr10_c_v2u_in_hdr.etag.E_PCP              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_DEI                 , copyMem_thr10_c_v2u_in_hdr.etag.E_DEI              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_base    , copyMem_thr10_c_v2u_in_hdr.etag.Ingress_E_CID_base );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_IPL_Direction         , copyMem_thr10_c_v2u_in_hdr.etag.IPL_Direction      );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Reserved              , copyMem_thr10_c_v2u_in_hdr.etag.Reserved           );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_GRP                   , copyMem_thr10_c_v2u_in_hdr.etag.GRP                );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_base            , copyMem_thr10_c_v2u_in_hdr.etag.E_CID_base         );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_ext     , copyMem_thr10_c_v2u_in_hdr.etag.Ingress_E_CID_ext  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_ext             , copyMem_thr10_c_v2u_in_hdr.etag.E_CID_ext          );


    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_TPID       , copyMem_thr10_c_v2u_in_hdr.vlan.TPID               );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_up         , copyMem_thr10_c_v2u_in_hdr.vlan.up                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_cfi        , copyMem_thr10_c_v2u_in_hdr.vlan.cfi                );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_vid        , copyMem_thr10_c_v2u_in_hdr.vlan.vid                );

    return &copyMem_thr10_c_v2u_in_hdr;
}


static config_info pha_internal_thr11_c2u_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_TagCommand             ),  1  , 0  } /* offset:0 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Tag0SrcTagged          ),  2  , 2  } /* offset:2 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_SrcDev_4_0             ),  7  , 3  } /* offset:3 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Src_ePort_4_0          ),  12 , 8  } /* offset:8 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_SrcIsTrunk             ),  13 , 13 } /* offset:13 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Reserved0              ),  14 , 14 } /* offset:14 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_CFI                    ),  15 , 15 } /* offset:15 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_UP                     ),  18 , 16 } /* offset:16 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Extend0                ),  19 , 19 } /* offset:19 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_eVLAN_11_0             ),  31 , 20 } /* offset:20 */


,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_Extend1                ),  32 , 32 } /* offset:32 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_Src_ePort_6_5          ),  34 , 33 } /* offset:33 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_EgressFilterRegistered ),  35 , 35 } /* offset:35 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_DropOnSource           ),  36 , 36 } /* offset:36 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_PacketIsLooped         ),  37 , 37 } /* offset:37 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_Routed                 ),  38 , 38 } /* offset:38 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_SrcID_4_0              ),  43 , 39 } /* offset:39 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile      ),  50 , 44 } /* offset:44 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_use_eVIDX              ),  51 , 51 } /* offset:51 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0         ),  58 , 52 } /* offset:52 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_TrgDev_4_0             ),  63 , 59 } /* offset:59 */


,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Extend2                ),  64 , 64 } /* offset:64 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Reserved1              ),  65 , 65 } /* offset:65 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid      ),  66 , 66 } /* offset:66 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_TrgPhyPort_7           ),  67 , 67 } /* offset:67 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_SrcID_11_5             ),  74 , 68 } /* offset:68 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_SrcDev_11_5            ),  81 , 75 } /* offset:75 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Reserved2              ),  82 , 82 } /* offset:82 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Src_ePort_16_7         ),  92 , 83 } /* offset:83 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_TPID_index             ),  95 , 93 } /* offset:93 */


,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Extend3                ),  96 , 96 } /* offset:96 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_eVLAN_15_12            ),  100, 97 } /* offset:97 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Tag1SrcTagged          ),  101, 101} /* offset:101 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag      ),  102, 102} /* offset:102 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Reserved3              ),  103, 103} /* offset:103 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Trg_ePort              ),  120, 104} /* offset:104 */
,{STR(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_TrgDev                 ),  127, 121} /* offset:121 */


,{STR(pha_internal_thr11_c2u_cfg_srcPortEntry_src_ePort_13_7        ),  134, 128} /* offset:128 */
,{STR(pha_internal_thr11_c2u_cfg_srcPortEntry_src_ePort_6_5         ),  136, 135} /* offset:135 */
,{STR(pha_internal_thr11_c2u_cfg_srcPortEntry_src_ePort_4_0         ),  141, 137} /* offset:137 */
,{STR(pha_internal_thr11_c2u_cfg_srcPortEntry_reserved1             ),  158, 142} /* offset:142 */
,{STR(pha_internal_thr11_c2u_cfg_srcPortEntry_reserved2             ),  159, 159} /* offset:159 */


,{STR(pha_internal_thr11_c2u_cfg_targetPortEntry_TPID               ),  175, 160} /* offset:160 */
,{STR(pha_internal_thr11_c2u_cfg_targetPortEntry_PVID               ),  187, 176} /* offset:176 */
,{STR(pha_internal_thr11_c2u_cfg_targetPortEntry_VlanEgrTagState    ),  188, 188} /* offset:188 */
,{STR(pha_internal_thr11_c2u_cfg_targetPortEntry_trg_ePort_13_11    ),  191, 189} /* offset:189 */
,{STR(pha_internal_thr11_c2u_cfg_targetPortEntry_trg_ePort_10_0     ),  202, 192} /* offset:192 */
,{STR(pha_internal_thr11_c2u_cfg_targetPortEntry_reserved           ),  203, 203} /* offset:203 */
,{STR(pha_internal_thr11_c2u_cfg_targetPortEntry_Egress_Pipe_Delay  ),  223, 204} /* offset:204 */
};


typedef enum{
     pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_TagCommand                 /* offset:0 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Tag0SrcTagged              /* offset:2 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_SrcDev_4_0                 /* offset:3 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Src_ePort_4_0              /* offset:8 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_SrcIsTrunk                 /* offset:13 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Reserved0                  /* offset:14 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_CFI                        /* offset:15 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_UP                         /* offset:16 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Extend0                    /* offset:19 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_eVLAN_11_0                 /* offset:20 */


    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_Extend1                    /* offset:32 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_Src_ePort_6_5              /* offset:33 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_EgressFilterRegistered     /* offset:35 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_DropOnSource               /* offset:36 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_PacketIsLooped             /* offset:37 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_Routed                     /* offset:38 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_SrcID_4_0                  /* offset:39 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile          /* offset:44 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_use_eVIDX                  /* offset:51 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0             /* offset:52 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_TrgDev_4_0                 /* offset:59 */


    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Extend2                    /* offset:64 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Reserved1                  /* offset:65 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid          /* offset:66 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_TrgPhyPort_7               /* offset:67 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_SrcID_11_5                 /* offset:68 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_SrcDev_11_5                /* offset:75 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Reserved2                  /* offset:82 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Src_ePort_16_7             /* offset:83 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_TPID_index                 /* offset:93 */


    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Extend3                    /* offset:96 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_eVLAN_15_12                /* offset:97 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Tag1SrcTagged              /* offset:101 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag          /* offset:102 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Reserved3                  /* offset:103 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Trg_ePort                  /* offset:104 */
    ,pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_TrgDev                     /* offset:121 */


    ,pha_internal_thr11_c2u_cfg_srcPortEntry_src_ePort_13_7            /* offset:128 */
    ,pha_internal_thr11_c2u_cfg_srcPortEntry_src_ePort_6_5             /* offset:135 */
    ,pha_internal_thr11_c2u_cfg_srcPortEntry_src_ePort_4_0             /* offset:137 */
    ,pha_internal_thr11_c2u_cfg_srcPortEntry_reserved1                 /* offset:142 */
    ,pha_internal_thr11_c2u_cfg_srcPortEntry_reserved2                 /* offset:159 */


    ,pha_internal_thr11_c2u_cfg_targetPortEntry_TPID                   /* offset:160 */
    ,pha_internal_thr11_c2u_cfg_targetPortEntry_PVID                   /* offset:176 */
    ,pha_internal_thr11_c2u_cfg_targetPortEntry_VlanEgrTagState        /* offset:188 */
    ,pha_internal_thr11_c2u_cfg_targetPortEntry_trg_ePort_13_11        /* offset:189 */
    ,pha_internal_thr11_c2u_cfg_targetPortEntry_trg_ePort_10_0         /* offset:192 */
    ,pha_internal_thr11_c2u_cfg_targetPortEntry_reserved               /* offset:203 */
    ,pha_internal_thr11_c2u_cfg_targetPortEntry_Egress_Pipe_Delay      /* offset:204 */
}pha_internal_thr11_c2u_cfg_enum;

#define thr11_c2u_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr11_c2u_cfg,littleEndianMem,field)

static struct thr11_c2u_cfg copyMem_thr11_c2u_cfg;
struct thr11_c2u_cfg * pipeMemCast_thr11_c2u_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);


    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_TagCommand               , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.TagCommand             );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Tag0SrcTagged            , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.Tag0SrcTagged          );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_SrcDev_4_0               , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.SrcDev_4_0             );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Src_ePort_4_0            , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.Src_ePort_4_0          );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_SrcIsTrunk               , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.SrcIsTrunk             );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Reserved0                , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.Reserved0              );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_CFI                      , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.CFI                    );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_UP                       , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.UP                     );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_Extend0                  , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.Extend0                );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w0_eVLAN_11_0               , copyMem_thr11_c2u_cfg.eDSA_fwd_w0.eVLAN_11_0             );


    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_Extend1                  , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.Extend1                );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_Src_ePort_6_5            , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.Src_ePort_6_5          );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_EgressFilterRegistered   , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.EgressFilterRegistered );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_DropOnSource             , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.DropOnSource           );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_PacketIsLooped           , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.PacketIsLooped         );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_Routed                   , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.Routed                 );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_SrcID_4_0                , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.SrcID_4_0              );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_GlobalQoS_Profile        , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.GlobalQoS_Profile      );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_use_eVIDX                , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.use_eVIDX              );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_TrgPhyPort_6_0           , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.TrgPhyPort_6_0         );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w1_TrgDev_4_0               , copyMem_thr11_c2u_cfg.eDSA_fwd_w1.TrgDev_4_0             );


    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Extend2                  , copyMem_thr11_c2u_cfg.eDSA_fwd_w2.Extend2                );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Reserved1                , copyMem_thr11_c2u_cfg.eDSA_fwd_w2.Reserved1              );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_IsTrgPhyPortValid        , copyMem_thr11_c2u_cfg.eDSA_fwd_w2.IsTrgPhyPortValid      );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_TrgPhyPort_7             , copyMem_thr11_c2u_cfg.eDSA_fwd_w2.TrgPhyPort_7           );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_SrcID_11_5               , copyMem_thr11_c2u_cfg.eDSA_fwd_w2.SrcID_11_5             );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_SrcDev_11_5              , copyMem_thr11_c2u_cfg.eDSA_fwd_w2.SrcDev_11_5            );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Reserved2                , copyMem_thr11_c2u_cfg.eDSA_fwd_w2.Reserved2              );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_Src_ePort_16_7           , copyMem_thr11_c2u_cfg.eDSA_fwd_w2.Src_ePort_16_7         );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w2_TPID_index               , copyMem_thr11_c2u_cfg.eDSA_fwd_w2.TPID_index             );


    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Extend3                  , copyMem_thr11_c2u_cfg.eDSA_fwd_w3.Extend3                );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_eVLAN_15_12              , copyMem_thr11_c2u_cfg.eDSA_fwd_w3.eVLAN_15_12            );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Tag1SrcTagged            , copyMem_thr11_c2u_cfg.eDSA_fwd_w3.Tag1SrcTagged          );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_SrcTag0IsOuterTag        , copyMem_thr11_c2u_cfg.eDSA_fwd_w3.SrcTag0IsOuterTag      );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Reserved3                , copyMem_thr11_c2u_cfg.eDSA_fwd_w3.Reserved3              );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_Trg_ePort                , copyMem_thr11_c2u_cfg.eDSA_fwd_w3.Trg_ePort              );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_eDSA_fwd_w3_TrgDev                   , copyMem_thr11_c2u_cfg.eDSA_fwd_w3.TrgDev                 );


    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_srcPortEntry_src_ePort_13_7          , copyMem_thr11_c2u_cfg.srcPortEntry.src_ePort_13_7        );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_srcPortEntry_src_ePort_6_5           , copyMem_thr11_c2u_cfg.srcPortEntry.src_ePort_6_5         );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_srcPortEntry_src_ePort_4_0           , copyMem_thr11_c2u_cfg.srcPortEntry.src_ePort_4_0         );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_srcPortEntry_reserved1               , copyMem_thr11_c2u_cfg.srcPortEntry.reserved1             );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_srcPortEntry_reserved2               , copyMem_thr11_c2u_cfg.srcPortEntry.reserved2             );


    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_targetPortEntry_TPID                 , copyMem_thr11_c2u_cfg.targetPortEntry.TPID               );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_targetPortEntry_PVID                 , copyMem_thr11_c2u_cfg.targetPortEntry.PVID               );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_targetPortEntry_VlanEgrTagState      , copyMem_thr11_c2u_cfg.targetPortEntry.VlanEgrTagState    );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_targetPortEntry_trg_ePort_13_11      , copyMem_thr11_c2u_cfg.targetPortEntry.trg_ePort_13_11    );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_targetPortEntry_trg_ePort_10_0       , copyMem_thr11_c2u_cfg.targetPortEntry.trg_ePort_10_0     );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_targetPortEntry_reserved             , copyMem_thr11_c2u_cfg.targetPortEntry.reserved           );
    thr11_c2u_cfg_CONFIG_GET(pha_internal_thr11_c2u_cfg_targetPortEntry_Egress_Pipe_Delay    , copyMem_thr11_c2u_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr11_c2u_cfg;
}

static struct thr11_c2u_in_hdr copyMem_thr11_c2u_in_hdr;
struct thr11_c2u_in_hdr* pipeMemCast_thr11_c2u_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr11_c2u_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr11_c2u_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr11_c2u_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr11_c2u_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr11_c2u_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr11_c2u_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr11_c2u_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr11_c2u_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr11_c2u_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr11_c2u_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr11_c2u_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr11_c2u_in_hdr.mac_header.mac_sa_15_0  );


    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_TPID                  , copyMem_thr11_c2u_in_hdr.etag.TPID               );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_PCP                 , copyMem_thr11_c2u_in_hdr.etag.E_PCP              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_DEI                 , copyMem_thr11_c2u_in_hdr.etag.E_DEI              );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_base    , copyMem_thr11_c2u_in_hdr.etag.Ingress_E_CID_base );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_IPL_Direction         , copyMem_thr11_c2u_in_hdr.etag.IPL_Direction      );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Reserved              , copyMem_thr11_c2u_in_hdr.etag.Reserved           );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_GRP                   , copyMem_thr11_c2u_in_hdr.etag.GRP                );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_base            , copyMem_thr11_c2u_in_hdr.etag.E_CID_base         );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_Ingress_E_CID_ext     , copyMem_thr11_c2u_in_hdr.etag.Ingress_E_CID_ext  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_etag_E_CID_ext             , copyMem_thr11_c2u_in_hdr.etag.E_CID_ext          );

    return &copyMem_thr11_c2u_in_hdr;
}

static config_info pha_internal_thr17_u2ipl_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_0),    31 ,    0 }/* offset:  0 */
,{STR(pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_1),    63 ,   32 }/* offset: 32 */
,{STR(pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_2),    95 ,   64 }/* offset: 64 */
,{STR(pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_3),   127 ,   96 }/* offset: 96 */


,{STR(pha_internal_thr17_u2ipl_cfg_srcPortEntry_PCID                 ),   139 , 128 }/* offset:128 */
,{STR(pha_internal_thr17_u2ipl_cfg_Source_Filtering_Bitvector        ),   155 , 140 }/* offset:140 */
,{STR(pha_internal_thr17_u2ipl_cfg_Uplink_Port                       ),   156 , 156 }/* offset:156 */
,{STR(pha_internal_thr17_u2ipl_cfg_srcPortEntry_reserved             ),   159 , 157 }/* offset:157 */


,{STR(pha_internal_thr17_u2ipl_cfg_targetPortEntry_TPID              ),   175 , 160 }/* offset:160 */
,{STR(pha_internal_thr17_u2ipl_cfg_targetPortEntry_PVID              ),   187 , 176 }/* offset:176 */
,{STR(pha_internal_thr17_u2ipl_cfg_targetPortEntry_VlanEgrTagState   ),   188 , 188 }/* offset:188 */
,{STR(pha_internal_thr17_u2ipl_cfg_targetPortEntry_reserved          ),   191 , 189 }/* offset:189 */
,{STR(pha_internal_thr17_u2ipl_cfg_targetPortEntry_PCID              ),   203 , 192 }/* offset:192 */
,{STR(pha_internal_thr17_u2ipl_cfg_targetPortEntry_Egress_Pipe_Delay ),   223 , 204 }/* offset:204 */
};

typedef enum{
     pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_0      /* offset:0 */
    ,pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_1      /* offset:32 */
    ,pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_2      /* offset:64 */
    ,pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_3      /* offset:96 */


    ,pha_internal_thr17_u2ipl_cfg_srcPortEntry_PCID                       /* offset:128 */
    ,pha_internal_thr17_u2ipl_cfg_Source_Filtering_Bitvector              /* offset:140 */
    ,pha_internal_thr17_u2ipl_cfg_Uplink_Port                             /* offset:156 */
    ,pha_internal_thr17_u2ipl_cfg_srcPortEntry_reserved                   /* offset:157 */


    ,pha_internal_thr17_u2ipl_cfg_targetPortEntry_TPID                    /* offset:160 */
    ,pha_internal_thr17_u2ipl_cfg_targetPortEntry_PVID                    /* offset:176 */
    ,pha_internal_thr17_u2ipl_cfg_targetPortEntry_VlanEgrTagState         /* offset:188 */
    ,pha_internal_thr17_u2ipl_cfg_targetPortEntry_reserved                /* offset:189 */
    ,pha_internal_thr17_u2ipl_cfg_targetPortEntry_PCID                    /* offset:192 */
    ,pha_internal_thr17_u2ipl_cfg_targetPortEntry_Egress_Pipe_Delay       /* offset:204 */
}pha_internal_thr17_u2ipl_cfg_enum;

#define thr17_u2ipl_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr17_u2ipl_cfg,littleEndianMem,field)

static struct thr17_u2ipl_cfg copyMem_thr17_u2ipl_cfg;
struct thr17_u2ipl_cfg * pipeMemCast_thr17_u2ipl_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_0 , copyMem_thr17_u2ipl_cfg.HA_Table_reserved_space.reserved_0 );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_1 , copyMem_thr17_u2ipl_cfg.HA_Table_reserved_space.reserved_1 );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_2 , copyMem_thr17_u2ipl_cfg.HA_Table_reserved_space.reserved_2 );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_HA_Table_reserved_space_reserved_3 , copyMem_thr17_u2ipl_cfg.HA_Table_reserved_space.reserved_3 );


    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_srcPortEntry_PCID                  , copyMem_thr17_u2ipl_cfg.srcPortEntry.PCID                  );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_Source_Filtering_Bitvector         , copyMem_thr17_u2ipl_cfg.srcPortEntry.Source_Filtering_Bitvector);
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_Uplink_Port                        , copyMem_thr17_u2ipl_cfg.srcPortEntry.Uplink_Port           );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_srcPortEntry_reserved              , copyMem_thr17_u2ipl_cfg.srcPortEntry.reserved              );


    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_targetPortEntry_TPID               , copyMem_thr17_u2ipl_cfg.targetPortEntry.TPID               );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_targetPortEntry_PVID               , copyMem_thr17_u2ipl_cfg.targetPortEntry.PVID               );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_targetPortEntry_VlanEgrTagState    , copyMem_thr17_u2ipl_cfg.targetPortEntry.VlanEgrTagState    );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_targetPortEntry_reserved           , copyMem_thr17_u2ipl_cfg.targetPortEntry.reserved           );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_targetPortEntry_PCID               , copyMem_thr17_u2ipl_cfg.targetPortEntry.PCID               );
    thr17_u2ipl_cfg_CONFIG_GET(pha_internal_thr17_u2ipl_cfg_targetPortEntry_Egress_Pipe_Delay  , copyMem_thr17_u2ipl_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr17_u2ipl_cfg;
}

static config_info pha_internal_thr18_ipl2ipl_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_0),    31 ,    0 }/* offset:  0 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_1),    63 ,   32 }/* offset: 32 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_2),    95 ,   64 }/* offset: 64 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_3),   127 ,   96 }/* offset: 96 */


,{STR(pha_internal_thr18_ipl2ipl_cfg_srcPortEntry_PCID                 ),   139 , 128 }/* offset:128 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_Source_Filtering_Bitvector        ),   155 , 140 }/* offset:140 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_Uplink_Port                       ),   156 , 156 }/* offset:156 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_srcPortEntry_reserved             ),   159 , 157 }/* offset:157 */


,{STR(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_TPID              ),   175 , 160 }/* offset:160 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_TargetPort        ),   179 , 176 }/* offset:176 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_reserved          ),   191 , 180 }/* offset:188 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_PCID              ),   203 , 192 }/* offset:192 */
,{STR(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_Egress_Pipe_Delay ),   223 , 204 }/* offset:204 */
};

typedef enum{
     pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_0      /* offset:0 */
    ,pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_1      /* offset:32 */
    ,pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_2      /* offset:64 */
    ,pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_3      /* offset:96 */


    ,pha_internal_thr18_ipl2ipl_cfg_srcPortEntry_PCID                       /* offset:128 */
    ,pha_internal_thr18_ipl2ipl_cfg_Source_Filtering_Bitvector              /* offset:140 */
    ,pha_internal_thr18_ipl2ipl_cfg_Uplink_Port                             /* offset:156 */
    ,pha_internal_thr18_ipl2ipl_cfg_srcPortEntry_reserved                   /* offset:157 */


    ,pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_TPID                      /* offset:160 */
    ,pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_TargetPort                /* offset:176 */
    ,pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_reserved                  /* offset:188 */
    ,pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_PCID                      /* offset:189 */
    ,pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_Egress_Pipe_Delay         /* offset:192 */

}pha_internal_thr18_ipl2ipl_cfg_enum;

#define thr18_ipl2ipl_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr18_ipl2ipl_cfg,littleEndianMem,field)

static struct thr18_ipl2ipl_cfg copyMem_thr18_ipl2ipl_cfg;
struct thr18_ipl2ipl_cfg * pipeMemCast_thr18_ipl2ipl_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_0 , copyMem_thr18_ipl2ipl_cfg.HA_Table_reserved_space.reserved_0 );
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_1 , copyMem_thr18_ipl2ipl_cfg.HA_Table_reserved_space.reserved_1 );
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_2 , copyMem_thr18_ipl2ipl_cfg.HA_Table_reserved_space.reserved_2 );
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_HA_Table_reserved_space_reserved_3 , copyMem_thr18_ipl2ipl_cfg.HA_Table_reserved_space.reserved_3 );


    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_srcPortEntry_PCID                  , copyMem_thr18_ipl2ipl_cfg.srcPortEntry.PCID                  );
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_Source_Filtering_Bitvector         , copyMem_thr18_ipl2ipl_cfg.srcPortEntry.Source_Filtering_Bitvector);
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_Uplink_Port                        , copyMem_thr18_ipl2ipl_cfg.srcPortEntry.Uplink_Port           );
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_srcPortEntry_reserved              , copyMem_thr18_ipl2ipl_cfg.srcPortEntry.reserved              );

    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_TPID               , copyMem_thr18_ipl2ipl_cfg.targetPortEntry.TPID               );
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_TargetPort         , copyMem_thr18_ipl2ipl_cfg.targetPortEntry.TargetPort         );
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_reserved           , copyMem_thr18_ipl2ipl_cfg.targetPortEntry.reserved           );
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_PCID               , copyMem_thr18_ipl2ipl_cfg.targetPortEntry.PCID               );
    thr18_ipl2ipl_cfg_CONFIG_GET(pha_internal_thr18_ipl2ipl_cfg_targetPortEntry_Egress_Pipe_Delay  , copyMem_thr18_ipl2ipl_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr18_ipl2ipl_cfg;
}

static config_info pha_internal_thr20_u2e_m4_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_PCID_1          ),    11,   0   }/* offset:0   */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_PCID_2          ),    23,   12  }/* offset:12  */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_reserved_0              ),    31,   24  }/* offset:24  */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_3  ),    43,   32  }/* offset:32  */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_4  ),    55,   44  }/* offset:44  */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_reserved_1              ),    63,   56  }/* offset:56  */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_5  ),    75,   64  }/* offset:64  */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_6  ),    87,   76  }/* offset:76  */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_reserved_2              ),    95,   88  }/* offset:88  */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_7  ),    107,  96  }/* offset:96  */
,{STR(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_reserved_3      ),    127,  108 }/* offset:108 */

,{STR(pha_internal_thr20_u2e_m4_cfg_srcPortEntry_PCID                               ),    139,  128 }/* offset:128 */
,{STR(pha_internal_thr20_u2e_m4_cfg_srcPortEntry_Default_VLAN_Tag                   ),    155,  140 }/* offset:140 */
,{STR(pha_internal_thr20_u2e_m4_cfg_srcPortEntry_Uplink_Port                        ),    156,  156 }/* offset:156 */
,{STR(pha_internal_thr20_u2e_m4_cfg_srcPortEntry_reserved                           ),    159,  157 }/* offset:157 */

,{STR(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_TPID                            ),    175,  160 }/* offset:160 */
,{STR(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_TargetPort                      ),    179,  176 }/* offset:176 */
,{STR(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_reserved                        ),    191,  180 }/* offset:180 */
,{STR(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_PCID                            ),    203,  192 }/* offset:192 */
,{STR(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_Egress_Pipe_Delay               ),    223,  204 }/* offset:204 */

,{STR(pha_internal_thr20_u2e_m4_cfg_cfgReservedSpace_reserved_0                     ),    255,  224 }/* offset:224 */

};

typedef enum{
     pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_PCID_1                           /* offset:0   */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_PCID_2                           /* offset:12  */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_reserved_0                               /* offset:24  */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_3                   /* offset:32  */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_4                   /* offset:44  */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_reserved_1                               /* offset:56  */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_5                   /* offset:64  */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_6                   /* offset:76  */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_reserved_2                               /* offset:88  */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_7                   /* offset:96  */
    ,pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_reserved_3                       /* offset:108 */

    ,pha_internal_thr20_u2e_m4_cfg_srcPortEntry_PCID                                                /* offset:128 */
    ,pha_internal_thr20_u2e_m4_cfg_srcPortEntry_Default_VLAN_Tag                                    /* offset:140 */
    ,pha_internal_thr20_u2e_m4_cfg_srcPortEntry_Uplink_Port                                         /* offset:156 */
    ,pha_internal_thr20_u2e_m4_cfg_srcPortEntry_reserved                                            /* offset:157 */

    ,pha_internal_thr20_u2e_m4_cfg_targetPortEntry_TPID                                             /* offset:160 */
    ,pha_internal_thr20_u2e_m4_cfg_targetPortEntry_TargetPort                                       /* offset:176 */
    ,pha_internal_thr20_u2e_m4_cfg_targetPortEntry_reserved                                         /* offset:180 */
    ,pha_internal_thr20_u2e_m4_cfg_targetPortEntry_PCID                                             /* offset:192 */
    ,pha_internal_thr20_u2e_m4_cfg_targetPortEntry_Egress_Pipe_Delay                                /* offset:204 */

    ,pha_internal_thr20_u2e_m4_cfg_cfgReservedSpace_reserved_0                                      /* offset:224 */

}pha_internal_thr20_u2e_m4_cfg_enum;

#define pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr20_u2e_m4_cfg, littleEndianMem,field)

static struct thr20_u2e_m4_cfg copyMem_thr20_u2e_m4_cfg;
struct thr20_u2e_m4_cfg * pipeMemCast_thr20_u2e_m4_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_PCID_1         , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.ExtPort_PCID_1         );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_PCID_2         , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.ExtPort_PCID_2         );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_reserved_0             , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.reserved_0             );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_3 , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.ExtPort_PCID_3 );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_4 , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.ExtPort_PCID_4 );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_reserved_1             , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.reserved_1             );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_5 , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.ExtPort_PCID_5 );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_6 , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.ExtPort_PCID_6 );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_reserved_2             , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.reserved_2             );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_7 , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.ExtPort_PCID_7 );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_HA_Table_Extended_Ports_ExtPort_reserved_3     , copyMem_thr20_u2e_m4_cfg.HA_Table_Extended_Ports.reserved_3     );


    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_srcPortEntry_PCID                              , copyMem_thr20_u2e_m4_cfg.srcPortEntry.PCID                              );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_srcPortEntry_Default_VLAN_Tag                  , copyMem_thr20_u2e_m4_cfg.srcPortEntry.Default_VLAN_Tag                  );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_srcPortEntry_Uplink_Port                       , copyMem_thr20_u2e_m4_cfg.srcPortEntry.Uplink_Port                       );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_srcPortEntry_reserved                          , copyMem_thr20_u2e_m4_cfg.srcPortEntry.reserved                          );

    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_TPID                           , copyMem_thr20_u2e_m4_cfg.targetPortEntry.TPID                           );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_TargetPort                     , copyMem_thr20_u2e_m4_cfg.targetPortEntry.TargetPort                     );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_reserved                       , copyMem_thr20_u2e_m4_cfg.targetPortEntry.reserved                       );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_PCID                           , copyMem_thr20_u2e_m4_cfg.targetPortEntry.PCID                           );
    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_targetPortEntry_Egress_Pipe_Delay              , copyMem_thr20_u2e_m4_cfg.targetPortEntry.Egress_Pipe_Delay              );

    pha_internal_thr20_u2e_m4_cfg_CONFIG_GET(pha_internal_thr20_u2e_m4_cfg_cfgReservedSpace_reserved_0                    , copyMem_thr20_u2e_m4_cfg.cfgReservedSpace.reserved_0                    );

    return &copyMem_thr20_u2e_m4_cfg;
}


static config_info pha_internal_thr21_u2e_m8_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_PCID_1          ),    11,   0   }/* offset:0   */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_PCID_2          ),    23,   12  }/* offset:12  */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_reserved_0              ),    31,   24  }/* offset:24  */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_3  ),    43,   32  }/* offset:32  */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_4  ),    55,   44  }/* offset:44  */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_reserved_1              ),    63,   56  }/* offset:56  */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_5  ),    75,   64  }/* offset:64  */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_6  ),    87,   76  }/* offset:76  */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_reserved_2              ),    95,   88  }/* offset:88  */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_7  ),    107,  96  }/* offset:96  */
,{STR(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_reserved_3      ),    127,  108 }/* offset:108 */

,{STR(pha_internal_thr21_u2e_m8_cfg_srcPortEntry_PCID                               ),    139,  128 }/* offset:128 */
,{STR(pha_internal_thr21_u2e_m8_cfg_srcPortEntry_Default_VLAN_Tag                   ),    155,  140 }/* offset:140 */
,{STR(pha_internal_thr21_u2e_m8_cfg_srcPortEntry_Uplink_Port                        ),    156,  156 }/* offset:156 */
,{STR(pha_internal_thr21_u2e_m8_cfg_srcPortEntry_reserved                           ),    159,  157 }/* offset:157 */

,{STR(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_TPID                            ),    175,  160 }/* offset:160 */
,{STR(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_TargetPort                      ),    179,  176 }/* offset:176 */
,{STR(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_reserved                        ),    191,  180 }/* offset:180 */
,{STR(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_PCID                            ),    203,  192 }/* offset:192 */
,{STR(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_Egress_Pipe_Delay               ),    223,  204 }/* offset:204 */

,{STR(pha_internal_thr21_u2e_m8_cfg_cfgReservedSpace_reserved_0                     ),    255,  224 }/* offset:224 */

};

typedef enum{
     pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_PCID_1                           /* offset:0   */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_PCID_2                           /* offset:12  */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_reserved_0                               /* offset:24  */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_3                   /* offset:32  */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_4                   /* offset:44  */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_reserved_1                               /* offset:56  */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_5                   /* offset:64  */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_6                   /* offset:76  */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_reserved_2                               /* offset:88  */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_7                   /* offset:96  */
    ,pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_reserved_3                       /* offset:108 */

    ,pha_internal_thr21_u2e_m8_cfg_srcPortEntry_PCID                                                /* offset:128 */
    ,pha_internal_thr21_u2e_m8_cfg_srcPortEntry_Default_VLAN_Tag                                    /* offset:140 */
    ,pha_internal_thr21_u2e_m8_cfg_srcPortEntry_Uplink_Port                                         /* offset:156 */
    ,pha_internal_thr21_u2e_m8_cfg_srcPortEntry_reserved                                            /* offset:157 */

    ,pha_internal_thr21_u2e_m8_cfg_targetPortEntry_TPID                                             /* offset:160 */
    ,pha_internal_thr21_u2e_m8_cfg_targetPortEntry_TargetPort                                       /* offset:176 */
    ,pha_internal_thr21_u2e_m8_cfg_targetPortEntry_reserved                                         /* offset:180 */
    ,pha_internal_thr21_u2e_m8_cfg_targetPortEntry_PCID                                             /* offset:192 */
    ,pha_internal_thr21_u2e_m8_cfg_targetPortEntry_Egress_Pipe_Delay                                /* offset:204 */

    ,pha_internal_thr21_u2e_m8_cfg_cfgReservedSpace_reserved_0                                      /* offset:224 */

}pha_internal_thr21_u2e_m8_cfg_enum;

#define pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr21_u2e_m8_cfg, littleEndianMem,field)

static struct thr21_u2e_m8_cfg copyMem_thr21_u2e_m8_cfg;
struct thr21_u2e_m8_cfg * pipeMemCast_thr21_u2e_m8_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_PCID_1         , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.ExtPort_PCID_1         );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_PCID_2         , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.ExtPort_PCID_2         );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_reserved_0             , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.reserved_0             );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_3 , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.ExtPort_PCID_3 );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_4 , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.ExtPort_PCID_4 );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_reserved_1             , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.reserved_1             );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_5 , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.ExtPort_PCID_5 );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_6 , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.ExtPort_PCID_6 );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_reserved_2             , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.reserved_2             );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_ExtPort_PCID_7 , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.ExtPort_PCID_7 );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_HA_Table_Extended_Ports_ExtPort_reserved_3     , copyMem_thr21_u2e_m8_cfg.HA_Table_Extended_Ports.reserved_3     );


    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_srcPortEntry_PCID                              , copyMem_thr21_u2e_m8_cfg.srcPortEntry.PCID                              );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_srcPortEntry_Default_VLAN_Tag                  , copyMem_thr21_u2e_m8_cfg.srcPortEntry.Default_VLAN_Tag                  );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_srcPortEntry_Uplink_Port                       , copyMem_thr21_u2e_m8_cfg.srcPortEntry.Uplink_Port                       );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_srcPortEntry_reserved                          , copyMem_thr21_u2e_m8_cfg.srcPortEntry.reserved                          );

    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_TPID                           , copyMem_thr21_u2e_m8_cfg.targetPortEntry.TPID                           );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_TargetPort                     , copyMem_thr21_u2e_m8_cfg.targetPortEntry.TargetPort                     );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_reserved                       , copyMem_thr21_u2e_m8_cfg.targetPortEntry.reserved                       );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_PCID                           , copyMem_thr21_u2e_m8_cfg.targetPortEntry.PCID                           );
    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_targetPortEntry_Egress_Pipe_Delay              , copyMem_thr21_u2e_m8_cfg.targetPortEntry.Egress_Pipe_Delay              );

    pha_internal_thr21_u2e_m8_cfg_CONFIG_GET(pha_internal_thr21_u2e_m8_cfg_cfgReservedSpace_reserved_0                    , copyMem_thr21_u2e_m8_cfg.cfgReservedSpace.reserved_0                    );

    return &copyMem_thr21_u2e_m8_cfg;
}

static config_info pha_internal_thr23_evb_e2u_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr23_evb_e2u_cfg_vlan_TPID                                           ),    15,   0    }/* offset:0  */
,{STR(pha_internal_thr23_evb_e2u_cfg_vlan_up                                             ),    18,   16   }/* offset:16 */
,{STR(pha_internal_thr23_evb_e2u_cfg_vlan_cfi                                            ),    19,   19   }/* offset:19 */
,{STR(pha_internal_thr23_evb_e2u_cfg_vlan_vid                                            ),    31,   20   }/* offset:20 */

,{STR(pha_internal_thr23_evb_e2u_cfg_HA_Table_reserved_space_reserved_0                  ),    63,   32   }/* offset:32 */
,{STR(pha_internal_thr23_evb_e2u_cfg_HA_Table_reserved_space_reserved_1                  ),    95,   64   }/* offset:64 */
,{STR(pha_internal_thr23_evb_e2u_cfg_HA_Table_reserved_space_reserved_2                  ),    127,  96   }/* offset:96 */

,{STR(pha_internal_thr23_evb_e2u_cfg_srcPortEntry_PVID                                   ),    139,  128  }/* offset:128 */
,{STR(pha_internal_thr23_evb_e2u_cfg_srcPortEntry_src_port_num                           ),    143,  140  }/* offset:140 */
,{STR(pha_internal_thr23_evb_e2u_cfg_srcPortEntry_reserved                               ),    159,  144  }/* offset:144 */

,{STR(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_TPID                                ),    175,  160  }/* offset:160 */
,{STR(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_PVID                                ),    187,  176  }/* offset:176 */
,{STR(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_VlanEgrTagState                     ),    188,  188  }/* offset:188 */
,{STR(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_reserved                            ),    191,  189  }/* offset:189 */
,{STR(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_PCID                                ),    203,  192  }/* offset:192 */
,{STR(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_Egress_Pipe_Delay                   ),    223,  204  }/* offset:204 */

,{STR(pha_internal_thr23_evb_e2u_cfg_cfgReservedSpace_reserved_0                         ),    255,  224  }/* offset:224 */
};


typedef enum {
     pha_internal_thr23_evb_e2u_cfg_vlan_TPID                                                   /* offset:0 */
    ,pha_internal_thr23_evb_e2u_cfg_vlan_up                                                     /* offset:16 */
    ,pha_internal_thr23_evb_e2u_cfg_vlan_cfi                                                    /* offset:19 */
    ,pha_internal_thr23_evb_e2u_cfg_vlan_vid                                                    /* offset:20 */

    ,pha_internal_thr23_evb_e2u_cfg_HA_Table_reserved_space_reserved_0                          /* offset:32 */
    ,pha_internal_thr23_evb_e2u_cfg_HA_Table_reserved_space_reserved_1                          /* offset:64 */
    ,pha_internal_thr23_evb_e2u_cfg_HA_Table_reserved_space_reserved_2                          /* offset:96 */

    ,pha_internal_thr23_evb_e2u_cfg_srcPortEntry_PVID                                           /* offset:128 */
    ,pha_internal_thr23_evb_e2u_cfg_srcPortEntry_src_port_num                                   /* offset:140 */
    ,pha_internal_thr23_evb_e2u_cfg_srcPortEntry_reserved                                       /* offset:144 */

    ,pha_internal_thr23_evb_e2u_cfg_targetPortEntry_TPID                                        /* offset:160 */
    ,pha_internal_thr23_evb_e2u_cfg_targetPortEntry_PVID                                        /* offset:176 */
    ,pha_internal_thr23_evb_e2u_cfg_targetPortEntry_VlanEgrTagState                             /* offset:188 */
    ,pha_internal_thr23_evb_e2u_cfg_targetPortEntry_reserved                                    /* offset:189 */
    ,pha_internal_thr23_evb_e2u_cfg_targetPortEntry_PCID                                        /* offset:192 */
    ,pha_internal_thr23_evb_e2u_cfg_targetPortEntry_Egress_Pipe_Delay                           /* offset:204 */

    ,pha_internal_thr23_evb_e2u_cfg_cfgReservedSpace_reserved_0                                 /* offset:224 */
}pha_internal_thr23_evb_e2u_cfg_enum;

#define pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr23_evb_e2u_cfg, littleEndianMem,field)

static struct thr23_evb_e2u_cfg copyMem_thr23_evb_e2u_cfg;
struct thr23_evb_e2u_cfg * pipeMemCast_thr23_evb_e2u_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_vlan_TPID                             , copyMem_thr23_evb_e2u_cfg.vlan.TPID                          );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_vlan_up                               , copyMem_thr23_evb_e2u_cfg.vlan.up                            );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_vlan_cfi                              , copyMem_thr23_evb_e2u_cfg.vlan.cfi                           );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_vlan_vid                              , copyMem_thr23_evb_e2u_cfg.vlan.vid                           );

    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_HA_Table_reserved_space_reserved_0    , copyMem_thr23_evb_e2u_cfg.HA_Table_reserved_space.reserved_0 );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_HA_Table_reserved_space_reserved_1    , copyMem_thr23_evb_e2u_cfg.HA_Table_reserved_space.reserved_1 );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_HA_Table_reserved_space_reserved_2    , copyMem_thr23_evb_e2u_cfg.HA_Table_reserved_space.reserved_2 );

    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_srcPortEntry_PVID                     , copyMem_thr23_evb_e2u_cfg.srcPortEntry.PVID                  );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_srcPortEntry_src_port_num             , copyMem_thr23_evb_e2u_cfg.srcPortEntry.src_port_num          );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_srcPortEntry_reserved                 , copyMem_thr23_evb_e2u_cfg.srcPortEntry.reserved              );

    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_TPID                  , copyMem_thr23_evb_e2u_cfg.targetPortEntry.TPID               );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_PVID                  , copyMem_thr23_evb_e2u_cfg.targetPortEntry.PVID               );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_VlanEgrTagState       , copyMem_thr23_evb_e2u_cfg.targetPortEntry.VlanEgrTagState    );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_reserved              , copyMem_thr23_evb_e2u_cfg.targetPortEntry.reserved           );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_PCID                  , copyMem_thr23_evb_e2u_cfg.targetPortEntry.PCID               );
    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_targetPortEntry_Egress_Pipe_Delay     , copyMem_thr23_evb_e2u_cfg.targetPortEntry.Egress_Pipe_Delay  );

    pha_internal_thr23_evb_e2u_cfg_CONFIG_GET(pha_internal_thr23_evb_e2u_cfg_cfgReservedSpace_reserved_0           , copyMem_thr23_evb_e2u_cfg.cfgReservedSpace.reserved_0        );

    return &copyMem_thr23_evb_e2u_cfg;
}

static struct thr23_evb_e2u_in_hdr copyMem_thr23_evb_e2u_in_hdr;
struct thr23_evb_e2u_in_hdr * pipeMemCast_thr23_evb_e2u_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr23_evb_e2u_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));


    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr23_evb_e2u_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr23_evb_e2u_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr23_evb_e2u_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr23_evb_e2u_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr23_evb_e2u_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr23_evb_e2u_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr23_evb_e2u_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr23_evb_e2u_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr23_evb_e2u_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr23_evb_e2u_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr23_evb_e2u_in_hdr.mac_header.mac_sa_15_0  );

    return &copyMem_thr23_evb_e2u_in_hdr;
}

static config_info pha_internal_thr24_evb_u2e_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_0),    31 ,   0 }/* offset:  0 */
,{STR(pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_1),    63 ,  32 }/* offset: 32 */
,{STR(pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_2),    95 ,  64 }/* offset: 64 */
,{STR(pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_3),   127 ,  96 }/* offset: 96 */


,{STR(pha_internal_thr24_evb_u2e_cfg_srcPortEntry_PVID                 ),   139 , 128 }/* offset:128 */
,{STR(pha_internal_thr24_evb_u2e_cfg_srcPortEntry_src_port_num         ),   143 , 140 }/* offset:140 */
,{STR(pha_internal_thr24_evb_u2e_cfg_srcPortEntry_reserved             ),   159 , 144 }/* offset:144 */


,{STR(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_TPID              ),   175 , 160 } /* offset:160 */
,{STR(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_PVID              ),   179 , 176 } /* offset:176 */
,{STR(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_VlanEgrTagState   ),   191 , 188 } /* offset:188 */
,{STR(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_reserved          ),   203 , 189 } /* offset:189 */
,{STR(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_PCID              ),   223 , 192 } /* offset:192 */
,{STR(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_Egress_Pipe_Delay ),   223 , 204 } /* offset:204 */

,{STR(pha_internal_thr24_evb_u2e_cfg_cfgReservedSpace_reserved_0       ),   255 , 224 }/* offset:224 */
};


typedef enum{
     pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_0      /* offset:0 */
    ,pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_1      /* offset:32 */
    ,pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_2      /* offset:64 */
    ,pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_3      /* offset:96 */


    ,pha_internal_thr24_evb_u2e_cfg_srcPortEntry_PVID                       /* offset:128 */
    ,pha_internal_thr24_evb_u2e_cfg_srcPortEntry_src_port_num               /* offset:140 */
    ,pha_internal_thr24_evb_u2e_cfg_srcPortEntry_reserved                   /* offset:144 */


    ,pha_internal_thr24_evb_u2e_cfg_targetPortEntry_TPID                    /* offset:160 */
    ,pha_internal_thr24_evb_u2e_cfg_targetPortEntry_PVID                    /* offset:176 */
    ,pha_internal_thr24_evb_u2e_cfg_targetPortEntry_VlanEgrTagState         /* offset:188 */
    ,pha_internal_thr24_evb_u2e_cfg_targetPortEntry_reserved                /* offset:189 */
    ,pha_internal_thr24_evb_u2e_cfg_targetPortEntry_PCID                    /* offset:192 */
    ,pha_internal_thr24_evb_u2e_cfg_targetPortEntry_Egress_Pipe_Delay       /* offset:204 */

    ,pha_internal_thr24_evb_u2e_cfg_cfgReservedSpace_reserved_0             /* offset:224 */
}pha_internal_thr24_evb_u2e_cfg_enum;

#define thr24_evb_u2e_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr24_evb_u2e_cfg,littleEndianMem,field)

static struct thr24_evb_u2e_cfg copyMem_thr24_evb_u2e_cfg;
struct thr24_evb_u2e_cfg * pipeMemCast_thr24_evb_u2e_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address) ;

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_0 , copyMem_thr24_evb_u2e_cfg.HA_Table_reserved_space.reserved_0 );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_1 , copyMem_thr24_evb_u2e_cfg.HA_Table_reserved_space.reserved_1 );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_2 , copyMem_thr24_evb_u2e_cfg.HA_Table_reserved_space.reserved_2 );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_HA_Table_reserved_space_reserved_3 , copyMem_thr24_evb_u2e_cfg.HA_Table_reserved_space.reserved_3 );


    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_srcPortEntry_PVID                  , copyMem_thr24_evb_u2e_cfg.srcPortEntry.PVID                  );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_srcPortEntry_src_port_num          , copyMem_thr24_evb_u2e_cfg.srcPortEntry.src_port_num          );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_srcPortEntry_reserved              , copyMem_thr24_evb_u2e_cfg.srcPortEntry.reserved              );


    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_TPID               , copyMem_thr24_evb_u2e_cfg.targetPortEntry.TPID               );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_PVID               , copyMem_thr24_evb_u2e_cfg.targetPortEntry.PVID               );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_VlanEgrTagState    , copyMem_thr24_evb_u2e_cfg.targetPortEntry.VlanEgrTagState    );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_reserved           , copyMem_thr24_evb_u2e_cfg.targetPortEntry.reserved           );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_PCID               , copyMem_thr24_evb_u2e_cfg.targetPortEntry.PCID               );
    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_targetPortEntry_Egress_Pipe_Delay  , copyMem_thr24_evb_u2e_cfg.targetPortEntry.Egress_Pipe_Delay  );

    thr24_evb_u2e_cfg_CONFIG_GET(pha_internal_thr24_evb_u2e_cfg_cfgReservedSpace_reserved_0        , copyMem_thr24_evb_u2e_cfg.cfgReservedSpace.reserved_0        );

    return &copyMem_thr24_evb_u2e_cfg;
}

static struct thr24_evb_u2e_in_hdr copyMem_thr24_evb_u2e_in_hdr;
struct thr24_evb_u2e_in_hdr* pipeMemCast_thr24_evb_u2e_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr24_evb_u2e_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr24_evb_u2e_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr24_evb_u2e_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr24_evb_u2e_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr24_evb_u2e_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr24_evb_u2e_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr24_evb_u2e_in_hdr.mac_header.mac_da_47_32   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr24_evb_u2e_in_hdr.mac_header.mac_da_31_16   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr24_evb_u2e_in_hdr.mac_header.mac_da_15_0    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr24_evb_u2e_in_hdr.mac_header.mac_sa_47_32   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr24_evb_u2e_in_hdr.mac_header.mac_sa_31_16   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr24_evb_u2e_in_hdr.mac_header.mac_sa_15_0    );


    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_TPID       , copyMem_thr24_evb_u2e_in_hdr.vlan.TPID                 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_up         , copyMem_thr24_evb_u2e_in_hdr.vlan.up                   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_cfi        , copyMem_thr24_evb_u2e_in_hdr.vlan.cfi                  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_vlan_after_Etag_vid        , copyMem_thr24_evb_u2e_in_hdr.vlan.vid                  );

    return &copyMem_thr24_evb_u2e_in_hdr;
}


static config_info pha_internal_thr25_evb_qcn_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_sa_mac_47_32                       ),    15,   0    }/* offset:0  */
,{STR(pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_sa_mac_31_16                       ),    31,   16   }/* offset:16 */
,{STR(pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_sa_mac_15_0                        ),    47,   32   }/* offset:32 */
,{STR(pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_EtherType                          ),    63,   48   }/* offset:48 */

,{STR(pha_internal_thr25_evb_qcn_cfg_HA_Table_reserved_space_reserved_0                  ),    95,   64   }/* offset:64 */
,{STR(pha_internal_thr25_evb_qcn_cfg_HA_Table_reserved_space_reserved_1                  ),    127,  96   }/* offset:96 */

,{STR(pha_internal_thr25_evb_qcn_cfg_srcPortEntry_PVID                                   ),    139,  128  }/* offset:128 */
,{STR(pha_internal_thr25_evb_qcn_cfg_srcPortEntry_src_port_num                           ),    143,  140  }/* offset:140 */
,{STR(pha_internal_thr25_evb_qcn_cfg_srcPortEntry_reserved                               ),    159,  144  }/* offset:144 */

,{STR(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_TPID                                ),    175,  160  }/* offset:160 */
,{STR(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_PVID                                ),    187,  176  }/* offset:176 */
,{STR(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_VlanEgrTagState                     ),    188,  188  }/* offset:188 */
,{STR(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_reserved                            ),    191,  189  }/* offset:189 */
,{STR(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_PCID                                ),    203,  192  }/* offset:192 */
,{STR(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_Egress_Pipe_Delay                   ),    223,  204  }/* offset:204 */

,{STR(pha_internal_thr25_evb_qcn_cfg_cfgReservedSpace_reserved_0                         ),    255,  224  }/* offset:224 */
};


typedef enum {
     pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_sa_mac_47_32                               /* offset:0 */
    ,pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_sa_mac_31_16                               /* offset:16 */
    ,pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_sa_mac_15_0                                /* offset:32 */
    ,pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_EtherType                                  /* offset:48 */

    ,pha_internal_thr25_evb_qcn_cfg_HA_Table_reserved_space_reserved_0                          /* offset:64 */
    ,pha_internal_thr25_evb_qcn_cfg_HA_Table_reserved_space_reserved_1                          /* offset:96 */

    ,pha_internal_thr25_evb_qcn_cfg_srcPortEntry_PVID                                           /* offset:128 */
    ,pha_internal_thr25_evb_qcn_cfg_srcPortEntry_src_port_num                                   /* offset:140 */
    ,pha_internal_thr25_evb_qcn_cfg_srcPortEntry_reserved                                       /* offset:144 */

    ,pha_internal_thr25_evb_qcn_cfg_targetPortEntry_TPID                                        /* offset:160 */
    ,pha_internal_thr25_evb_qcn_cfg_targetPortEntry_PVID                                        /* offset:176 */
    ,pha_internal_thr25_evb_qcn_cfg_targetPortEntry_VlanEgrTagState                             /* offset:188 */
    ,pha_internal_thr25_evb_qcn_cfg_targetPortEntry_reserved                                    /* offset:189 */
    ,pha_internal_thr25_evb_qcn_cfg_targetPortEntry_PCID                                        /* offset:192 */
    ,pha_internal_thr25_evb_qcn_cfg_targetPortEntry_Egress_Pipe_Delay                           /* offset:204 */

    ,pha_internal_thr25_evb_qcn_cfg_cfgReservedSpace_reserved_0                                 /* offset:224 */
}pha_internal_thr25_evb_qcn_cfg_enum;

#define pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr25_evb_qcn_cfg, littleEndianMem,field)

static struct thr25_evb_qcn_cfg copyMem_thr25_evb_qcn_cfg;
struct thr25_evb_qcn_cfg * pipeMemCast_thr25_evb_qcn_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_sa_mac_47_32         , copyMem_thr25_evb_qcn_cfg.evb_qcn_ha_table.sa_mac_47_32    );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_sa_mac_31_16         , copyMem_thr25_evb_qcn_cfg.evb_qcn_ha_table.sa_mac_31_16    );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_sa_mac_15_0          , copyMem_thr25_evb_qcn_cfg.evb_qcn_ha_table.sa_mac_15_0     );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_evb_qcn_ha_table_EtherType            , copyMem_thr25_evb_qcn_cfg.evb_qcn_ha_table.EtherType       );

    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_HA_Table_reserved_space_reserved_0    , copyMem_thr25_evb_qcn_cfg.HA_Table_reserved_space.reserved_0 );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_HA_Table_reserved_space_reserved_1    , copyMem_thr25_evb_qcn_cfg.HA_Table_reserved_space.reserved_1 );

    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_srcPortEntry_PVID                     , copyMem_thr25_evb_qcn_cfg.srcPortEntry.PVID                  );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_srcPortEntry_src_port_num             , copyMem_thr25_evb_qcn_cfg.srcPortEntry.src_port_num          );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_srcPortEntry_reserved                 , copyMem_thr25_evb_qcn_cfg.srcPortEntry.reserved              );

    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_TPID                  , copyMem_thr25_evb_qcn_cfg.targetPortEntry.TPID               );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_PVID                  , copyMem_thr25_evb_qcn_cfg.targetPortEntry.PVID               );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_VlanEgrTagState       , copyMem_thr25_evb_qcn_cfg.targetPortEntry.VlanEgrTagState    );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_reserved              , copyMem_thr25_evb_qcn_cfg.targetPortEntry.reserved           );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_PCID                  , copyMem_thr25_evb_qcn_cfg.targetPortEntry.PCID               );
    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_targetPortEntry_Egress_Pipe_Delay     , copyMem_thr25_evb_qcn_cfg.targetPortEntry.Egress_Pipe_Delay  );

    pha_internal_thr25_evb_qcn_cfg_CONFIG_GET(pha_internal_thr25_evb_qcn_cfg_cfgReservedSpace_reserved_0           , copyMem_thr25_evb_qcn_cfg.cfgReservedSpace.reserved_0        );

    return &copyMem_thr25_evb_qcn_cfg;
}

static struct thr25_evb_qcn_in_hdr copyMem_thr25_evb_qcn_in_hdr;
struct thr25_evb_qcn_in_hdr * pipeMemCast_thr25_evb_qcn_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr25_evb_qcn_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));


    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr25_evb_qcn_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr25_evb_qcn_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr25_evb_qcn_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr25_evb_qcn_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr25_evb_qcn_in_hdr.expansion_space.reserved_4);


    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr25_evb_qcn_in_hdr.mac_header.mac_da_47_32);
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr25_evb_qcn_in_hdr.mac_header.mac_da_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr25_evb_qcn_in_hdr.mac_header.mac_da_15_0  );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr25_evb_qcn_in_hdr.mac_header.mac_sa_47_32 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr25_evb_qcn_in_hdr.mac_header.mac_sa_31_16 );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr25_evb_qcn_in_hdr.mac_header.mac_sa_15_0  );

    return &copyMem_thr25_evb_qcn_in_hdr;
}


static config_info pha_internal_thr26_pre_da_u2e_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_reserved_0),       31 ,   0 } /* offset:  0 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_reserved_1),       63 ,  32 } /* offset: 32 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_reserved_2),       87 ,  64 } /* offset: 64 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_ip_protocol),      95 ,  88 } /* offset: 88 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_udp_dest_port),   111 ,  96 } /* offset: 96 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_domain_number),   119 ,  112 }/* offset: 112 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_ptp_version),     123 ,  120 }/* offset: 120 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_message_type),    127 ,  124 }/* offset: 124 */

,{STR(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_reserved_0           ),   135 , 128 }/* offset:128 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_pre_da_port_info     ),   143 , 136 }/* offset:136 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_reserved_1           ),   156 , 144 }/* offset:144 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_ptpOverMplsEn        ),   157 , 157 }/* offset:157 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_ptp_mode             ),   159 , 158 }/* offset:158 */

,{STR(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_TPID              ),   175 , 160 } /* offset:160 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_PVID              ),   179 , 176 } /* offset:176 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_VlanEgrTagState   ),   191 , 188 } /* offset:188 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_reserved          ),   203 , 189 } /* offset:189 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_PCID              ),   223 , 192 } /* offset:192 */
,{STR(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_Egress_Pipe_Delay ),   223 , 204 } /* offset:204 */

,{STR(pha_internal_thr26_pre_da_u2e_cfg_cfgReservedSpace_reserved_0       ),   255 , 224 }/* offset:224 */
};


typedef enum{
     pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_reserved_0      /* offset:0 */
    ,pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_reserved_1      /* offset:32 */
    ,pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_reserved_2      /* offset:64 */
    ,pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_ip_protocol     /* offset:88 */
    ,pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_udp_dest_port   /* offset:96 */
    ,pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_domain_number   /* offset:112 */
    ,pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_ptp_version     /* offset:120 */
    ,pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_message_type    /* offset:124 */

    ,pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_reserved_0         /* offset:128 */
    ,pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_pre_da_port_info   /* offset:136 */
    ,pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_reserved_1         /* offset:144 */
    ,pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_ptpOverMplsEn      /* offset:157 */
    ,pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_ptp_mode           /* offset:158 */

    ,pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_TPID                    /* offset:160 */
    ,pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_PVID                    /* offset:176 */
    ,pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_VlanEgrTagState         /* offset:188 */
    ,pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_reserved                /* offset:189 */
    ,pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_PCID                    /* offset:192 */
    ,pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_Egress_Pipe_Delay       /* offset:204 */

    ,pha_internal_thr26_pre_da_u2e_cfg_cfgReservedSpace_reserved_0             /* offset:224 */
}pha_internal_thr26_pre_da_u2e_cfg_enum;

#define thr26_pre_da_u2e_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr26_pre_da_u2e_cfg,littleEndianMem,field)

static struct thr26_pre_da_u2e_cfg copyMem_thr26_pre_da_u2e_cfg;
struct thr26_pre_da_u2e_cfg * pipeMemCast_thr26_pre_da_u2e_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address) ;

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_reserved_0    , copyMem_thr26_pre_da_u2e_cfg.HA_Table_ptp_classification_fields.reserved_0 );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_reserved_1    , copyMem_thr26_pre_da_u2e_cfg.HA_Table_ptp_classification_fields.reserved_1 );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_reserved_2    , copyMem_thr26_pre_da_u2e_cfg.HA_Table_ptp_classification_fields.reserved_2 );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_ip_protocol   , copyMem_thr26_pre_da_u2e_cfg.HA_Table_ptp_classification_fields.ip_protocol );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_udp_dest_port , copyMem_thr26_pre_da_u2e_cfg.HA_Table_ptp_classification_fields.udp_dest_port );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_domain_number , copyMem_thr26_pre_da_u2e_cfg.HA_Table_ptp_classification_fields.domain_number );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_ptp_version   , copyMem_thr26_pre_da_u2e_cfg.HA_Table_ptp_classification_fields.ptp_version );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_HA_Table_ptp_classification_fields_message_type  , copyMem_thr26_pre_da_u2e_cfg.HA_Table_ptp_classification_fields.message_type );

    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_reserved_0         , copyMem_thr26_pre_da_u2e_cfg.srcPortEntry.reserved_0         );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_pre_da_port_info   , copyMem_thr26_pre_da_u2e_cfg.srcPortEntry.pre_da_port_info   );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_reserved_1         , copyMem_thr26_pre_da_u2e_cfg.srcPortEntry.reserved_1         );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_ptpOverMplsEn      , copyMem_thr26_pre_da_u2e_cfg.srcPortEntry.ptpOverMplsEn      );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_srcPortEntry_ptp_mode           , copyMem_thr26_pre_da_u2e_cfg.srcPortEntry.ptp_mode           );

    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_TPID               , copyMem_thr26_pre_da_u2e_cfg.targetPortEntry.TPID               );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_PVID               , copyMem_thr26_pre_da_u2e_cfg.targetPortEntry.PVID               );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_VlanEgrTagState    , copyMem_thr26_pre_da_u2e_cfg.targetPortEntry.VlanEgrTagState    );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_reserved           , copyMem_thr26_pre_da_u2e_cfg.targetPortEntry.reserved           );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_PCID               , copyMem_thr26_pre_da_u2e_cfg.targetPortEntry.PCID               );
    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_targetPortEntry_Egress_Pipe_Delay  , copyMem_thr26_pre_da_u2e_cfg.targetPortEntry.Egress_Pipe_Delay  );

    thr26_pre_da_u2e_cfg_CONFIG_GET(pha_internal_thr26_pre_da_u2e_cfg_cfgReservedSpace_reserved_0        , copyMem_thr26_pre_da_u2e_cfg.cfgReservedSpace.reserved_0        );

    return &copyMem_thr26_pre_da_u2e_cfg;
}

static struct thr26_pre_da_u2e_in_hdr copyMem_thr26_pre_da_u2e_in_hdr;
struct thr26_pre_da_u2e_in_hdr* pipeMemCast_thr26_pre_da_u2e_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr26_pre_da_u2e_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr26_pre_da_u2e_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr26_pre_da_u2e_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr26_pre_da_u2e_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr26_pre_da_u2e_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr26_pre_da_u2e_in_hdr.expansion_space.reserved_4);

    in_hdr_HERADER_GET(pha_internal_in_hdr_pre_da_tag_reserved        , copyMem_thr26_pre_da_u2e_in_hdr.pre_da_tag.reserved       );
    in_hdr_HERADER_GET(pha_internal_in_hdr_pre_da_tag_port_info       , copyMem_thr26_pre_da_u2e_in_hdr.pre_da_tag.port_info      );

    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr26_pre_da_u2e_in_hdr.mac_header.mac_da_47_32   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr26_pre_da_u2e_in_hdr.mac_header.mac_da_31_16   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr26_pre_da_u2e_in_hdr.mac_header.mac_da_15_0    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr26_pre_da_u2e_in_hdr.mac_header.mac_sa_47_32   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr26_pre_da_u2e_in_hdr.mac_header.mac_sa_31_16   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr26_pre_da_u2e_in_hdr.mac_header.mac_sa_15_0    );

    return &copyMem_thr26_pre_da_u2e_in_hdr;
}


static config_info pha_internal_thr27_pre_da_e2u_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_reserved_0),       31 ,   0 } /* offset:  0 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_reserved_1),       63 ,  32 } /* offset: 32 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_reserved_2),       87 ,  64 } /* offset: 64 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_ip_protocol),      95 ,  88 } /* offset: 88 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_udp_dest_port),   111 ,  96 } /* offset: 96 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_domain_number),   119 ,  112 }/* offset: 112 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_ptp_version),     123 ,  120 }/* offset: 120 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_message_type),    127 ,  124 }/* offset: 124 */

,{STR(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_reserved_0           ),   135 , 128 }/* offset:128 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_pre_da_port_info     ),   143 , 136 }/* offset:136 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_reserved_1           ),   156 , 144 }/* offset:144 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_ptpOverMplsEn        ),   157 , 157 }/* offset:157 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_ptp_mode             ),   159 , 158 }/* offset:158 */

,{STR(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_TPID              ),   175 , 160 } /* offset:160 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_PVID              ),   179 , 176 } /* offset:176 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_VlanEgrTagState   ),   191 , 188 } /* offset:188 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_reserved          ),   203 , 189 } /* offset:189 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_PCID              ),   223 , 192 } /* offset:192 */
,{STR(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_Egress_Pipe_Delay ),   223 , 204 } /* offset:204 */

,{STR(pha_internal_thr27_pre_da_e2u_cfg_cfgReservedSpace_reserved_0       ),   255 , 224 }/* offset:224 */
};


typedef enum{
    pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_reserved_0      /* offset:0 */
   ,pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_reserved_1      /* offset:32 */
   ,pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_reserved_2      /* offset:64 */
   ,pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_ip_protocol     /* offset:88 */
   ,pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_udp_dest_port   /* offset:96 */
   ,pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_domain_number   /* offset:112 */
   ,pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_ptp_version     /* offset:120 */
   ,pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_message_type    /* offset:124 */

   ,pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_reserved_0         /* offset:128 */
   ,pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_pre_da_port_info   /* offset:136 */
   ,pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_reserved_1         /* offset:144 */
   ,pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_ptpOverMplsEn      /* offset:157 */
   ,pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_ptp_mode           /* offset:158 */

    ,pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_TPID                    /* offset:160 */
    ,pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_PVID                    /* offset:176 */
    ,pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_VlanEgrTagState         /* offset:188 */
    ,pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_reserved                /* offset:189 */
    ,pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_PCID                    /* offset:192 */
    ,pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_Egress_Pipe_Delay       /* offset:204 */

    ,pha_internal_thr27_pre_da_e2u_cfg_cfgReservedSpace_reserved_0             /* offset:224 */
}pha_internal_thr27_pre_da_e2u_cfg_enum;

#define thr27_pre_da_e2u_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr27_pre_da_e2u_cfg,littleEndianMem,field)

static struct thr27_pre_da_e2u_cfg copyMem_thr27_pre_da_e2u_cfg;
struct thr27_pre_da_e2u_cfg * pipeMemCast_thr27_pre_da_e2u_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address) ;

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_reserved_0    , copyMem_thr27_pre_da_e2u_cfg.HA_Table_ptp_classification_fields.reserved_0 );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_reserved_1    , copyMem_thr27_pre_da_e2u_cfg.HA_Table_ptp_classification_fields.reserved_1 );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_reserved_2    , copyMem_thr27_pre_da_e2u_cfg.HA_Table_ptp_classification_fields.reserved_2 );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_ip_protocol   , copyMem_thr27_pre_da_e2u_cfg.HA_Table_ptp_classification_fields.ip_protocol );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_udp_dest_port , copyMem_thr27_pre_da_e2u_cfg.HA_Table_ptp_classification_fields.udp_dest_port );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_domain_number , copyMem_thr27_pre_da_e2u_cfg.HA_Table_ptp_classification_fields.domain_number );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_ptp_version   , copyMem_thr27_pre_da_e2u_cfg.HA_Table_ptp_classification_fields.ptp_version );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_HA_Table_ptp_classification_fields_message_type  , copyMem_thr27_pre_da_e2u_cfg.HA_Table_ptp_classification_fields.message_type );

    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_reserved_0         , copyMem_thr27_pre_da_e2u_cfg.srcPortEntry.reserved_0         );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_pre_da_port_info   , copyMem_thr27_pre_da_e2u_cfg.srcPortEntry.pre_da_port_info   );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_reserved_1         , copyMem_thr27_pre_da_e2u_cfg.srcPortEntry.reserved_1         );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_ptpOverMplsEn      , copyMem_thr27_pre_da_e2u_cfg.srcPortEntry.ptpOverMplsEn      );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_srcPortEntry_ptp_mode           , copyMem_thr27_pre_da_e2u_cfg.srcPortEntry.ptp_mode           );

    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_TPID               , copyMem_thr27_pre_da_e2u_cfg.targetPortEntry.TPID               );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_PVID               , copyMem_thr27_pre_da_e2u_cfg.targetPortEntry.PVID               );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_VlanEgrTagState    , copyMem_thr27_pre_da_e2u_cfg.targetPortEntry.VlanEgrTagState    );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_reserved           , copyMem_thr27_pre_da_e2u_cfg.targetPortEntry.reserved           );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_PCID               , copyMem_thr27_pre_da_e2u_cfg.targetPortEntry.PCID               );
    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_targetPortEntry_Egress_Pipe_Delay  , copyMem_thr27_pre_da_e2u_cfg.targetPortEntry.Egress_Pipe_Delay  );

    thr27_pre_da_e2u_cfg_CONFIG_GET(pha_internal_thr27_pre_da_e2u_cfg_cfgReservedSpace_reserved_0        , copyMem_thr27_pre_da_e2u_cfg.cfgReservedSpace.reserved_0        );

    return &copyMem_thr27_pre_da_e2u_cfg;
}

static struct thr27_pre_da_e2u_in_hdr copyMem_thr27_pre_da_e2u_in_hdr;
struct thr27_pre_da_e2u_in_hdr* pipeMemCast_thr27_pre_da_e2u_in_hdr(uint32_t address)
{
    uint32_t  littleEndianMem[sizeof(struct thr27_pre_da_e2u_in_hdr)/4];

    /*copy bytes from header so we can do 'bit field' manipulations for the FIRMWARE */
    copyBytesFromHeader(GT_TRUE,address,littleEndianMem,sizeof(littleEndianMem));

    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_0 , copyMem_thr27_pre_da_e2u_in_hdr.expansion_space.reserved_0);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_1 , copyMem_thr27_pre_da_e2u_in_hdr.expansion_space.reserved_1);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_2 , copyMem_thr27_pre_da_e2u_in_hdr.expansion_space.reserved_2);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_3 , copyMem_thr27_pre_da_e2u_in_hdr.expansion_space.reserved_3);
    in_hdr_HERADER_GET(pha_internal_in_hdr_expansion_space_reserved_4 , copyMem_thr27_pre_da_e2u_in_hdr.expansion_space.reserved_4);

    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_47_32    , copyMem_thr27_pre_da_e2u_in_hdr.mac_header.mac_da_47_32   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_31_16    , copyMem_thr27_pre_da_e2u_in_hdr.mac_header.mac_da_31_16   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_da_15_0     , copyMem_thr27_pre_da_e2u_in_hdr.mac_header.mac_da_15_0    );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_47_32    , copyMem_thr27_pre_da_e2u_in_hdr.mac_header.mac_sa_47_32   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_31_16    , copyMem_thr27_pre_da_e2u_in_hdr.mac_header.mac_sa_31_16   );
    in_hdr_HERADER_GET(pha_internal_in_hdr_mac_header_mac_sa_15_0     , copyMem_thr27_pre_da_e2u_in_hdr.mac_header.mac_sa_15_0    );

    return &copyMem_thr27_pre_da_e2u_in_hdr;
}


static config_info pha_internal_thr46_RemoveAddBytes_cfg[] =
{
 /* config format */
 {STR(pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_0),    31 ,    0 }/* offset:  0 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_1),    63 ,   32 }/* offset: 32 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_2),    95 ,   64 }/* offset: 64 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_3),   127 ,   96 }/* offset: 96 */


,{STR(pha_internal_thr46_RemoveAddBytes_cfg_srcPortEntry_PVID                 ),   139 , 128 }/* offset:128 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_srcPortEntry_src_port_num         ),   143 , 140 }/* offset:140 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_srcPortEntry_reserved             ),   159 , 144 }/* offset:144 */


,{STR(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_TPID              ),   175 , 160 }/* offset:160 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_PVID              ),   187 , 176 }/* offset:176 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_VlanEgrTagState   ),   188 , 188 }/* offset:188 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_reserved          ),   191 , 189 }/* offset:189 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_PCID              ),   203 , 192 }/* offset:192 */
,{STR(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_Egress_Pipe_Delay ),   223 , 204 }/* offset:204 */
};


typedef enum{
     pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_0      /* offset:0 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_1      /* offset:32 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_2      /* offset:64 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_3      /* offset:96 */


    ,pha_internal_thr46_RemoveAddBytes_cfg_srcPortEntry_PVID                       /* offset:128 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_srcPortEntry_src_port_num               /* offset:140 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_srcPortEntry_reserved                   /* offset:144 */


    ,pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_TPID                    /* offset:160 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_PVID                    /* offset:176 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_VlanEgrTagState         /* offset:188 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_reserved                /* offset:189 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_PCID                    /* offset:192 */
    ,pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_Egress_Pipe_Delay       /* offset:204 */
}pha_internal_thr46_RemoveAddBytes_cfg_enum;

#define thr46_RemoveAddBytes_cfg_CONFIG_GET(field , targetValue) \
    targetValue = getConfigInfo(pha_internal_thr46_RemoveAddBytes_cfg,littleEndianMem,field)

static struct thr46_RemoveAddBytes_cfg copyMem_thr46_RemoveAddBytes_cfg;
struct thr46_RemoveAddBytes_cfg * pipeMemCast_thr46_RemoveAddBytes_cfg (uint32_t address)
{
    uint32_t  littleEndianMem[CONFIG_MEM_SIZE_IN_WORDS_CNS];
    uint32_t* memPtr = pha_findMem(address);

    /*copy words from memory so we can do 'bit field' manipulations for the FIRMWARE */
    config_copyWordsFromMem(memPtr,CONFIG_MEM_SIZE_IN_WORDS_CNS,littleEndianMem);

    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_0 , copyMem_thr46_RemoveAddBytes_cfg.HA_Table_reserved_space.reserved_0 );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_1 , copyMem_thr46_RemoveAddBytes_cfg.HA_Table_reserved_space.reserved_1 );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_2 , copyMem_thr46_RemoveAddBytes_cfg.HA_Table_reserved_space.reserved_2 );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_HA_Table_reserved_space_reserved_3 , copyMem_thr46_RemoveAddBytes_cfg.HA_Table_reserved_space.reserved_3 );


    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_srcPortEntry_PVID                  , copyMem_thr46_RemoveAddBytes_cfg.srcPortEntry.PVID                  );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_srcPortEntry_src_port_num          , copyMem_thr46_RemoveAddBytes_cfg.srcPortEntry.src_port_num          );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_srcPortEntry_reserved              , copyMem_thr46_RemoveAddBytes_cfg.srcPortEntry.reserved              );


    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_TPID               , copyMem_thr46_RemoveAddBytes_cfg.targetPortEntry.TPID               );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_PVID               , copyMem_thr46_RemoveAddBytes_cfg.targetPortEntry.PVID               );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_VlanEgrTagState    , copyMem_thr46_RemoveAddBytes_cfg.targetPortEntry.VlanEgrTagState    );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_reserved           , copyMem_thr46_RemoveAddBytes_cfg.targetPortEntry.reserved           );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_PCID               , copyMem_thr46_RemoveAddBytes_cfg.targetPortEntry.PCID               );
    thr46_RemoveAddBytes_cfg_CONFIG_GET(pha_internal_thr46_RemoveAddBytes_cfg_targetPortEntry_Egress_Pipe_Delay  , copyMem_thr46_RemoveAddBytes_cfg.targetPortEntry.Egress_Pipe_Delay  );

    return &copyMem_thr46_RemoveAddBytes_cfg;
}


/* define for 'not implemented' cases */
#define pipeMemCast_NOT_IMPLEMENTED(structCast)                         \
    struct structCast  copyMem_##structCast;                            \
    struct structCast  *pipeMemCast_##structCast(uint32_t address)      \
    {                                                                   \
        __LOG_NO_LOCATION_META_DATA(("[%s] NOT IMPLEMENTED \n",#structCast)); \
        memset(&copyMem_##structCast,0,sizeof(copyMem_##structCast));   \
        return  &copyMem_##structCast;                                  \
    }

/* make dummy implementation for those that are 'not implemented' */
pipeMemCast_NOT_IMPLEMENTED(thr12_u_uc2c_cfg                );
pipeMemCast_NOT_IMPLEMENTED(thr12_u_uc2c_in_hdr             );
pipeMemCast_NOT_IMPLEMENTED(thr13_u_mc2c_cfg                );
pipeMemCast_NOT_IMPLEMENTED(thr14_u_mr2c_cfg                );
pipeMemCast_NOT_IMPLEMENTED(thr14_u_mr2c_in_hdr             );
pipeMemCast_NOT_IMPLEMENTED(thr15_qcn_cfg                   );
pipeMemCast_NOT_IMPLEMENTED(thr47_Add20Bytes_cfg            );
pipeMemCast_NOT_IMPLEMENTED(thr48_Remove20Bytes_cfg         );
pipeMemCast_NOT_IMPLEMENTED(thr49_VariableCyclesLength_cfg  );
pipeMemCast_NOT_IMPLEMENTED(pipe_fw_version                 );
pipeMemCast_NOT_IMPLEMENTED(thr45_VariableCyclesLengthWithAcclCmd_cfg);
pipeMemCast_NOT_IMPLEMENTED(thr16_u2e_in_hdr                );
pipeMemCast_NOT_IMPLEMENTED(thr16_u2e_cfg                   );

#define ENUM_NAME_AND_VALUE(var) STR(enum_##var),var

#define ENUM(val) enum_##val = val

enum{
     ENUM( COPY_BYTES_THR1_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_OFFSET                                                                 )
    ,ENUM( COPY_BITS_THR1_E2U_CFG_srcPortEntry__PCID_LEN12_TO_CFG_etag__E_CID_base_OFFSET                                                      )
    ,ENUM( COPY_BITS_THR1_E2U_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET                                                         )
    ,ENUM( COPY_BYTES_THR1_E2U_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET                                                                  )
    ,ENUM( COPY_BITS_THR1_E2U_DESC_nonQcn_desc__up0_LEN4_TO_CFG_etag__E_PCP_OFFSET                                                             )
    ,ENUM( COPY_BITS_THR1_E2U_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_etag__IPL_Direction_OFFSET                                             )

    ,ENUM( COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_PLUS8_LEN12_TO_MACSHIFTRIGHT_12_OFFSET                                                         )
    ,ENUM( COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                                )

    ,ENUM( COPY_BYTES_THR3_U2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                                )
    ,ENUM( COPY_BITS_THR3_U2C_CFG_cfgReservedSpace__reserved_0_LEN1_TO_PKT_etag__IPL_Direction_OFFSET                                          )

    ,ENUM( COPY_BITS_THR4_ET2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_OFFSET                                    )
    ,ENUM( COPY_BITS_THR4_ET2U_PKT_vlan__vid_LEN12_TO_CFG_DSA_fwd__vid_OFFSET                                                                  )
    ,ENUM( COPY_BITS_THR4_ET2U_PKT_vlan__up_LEN3_TO_CFG_DSA_fwd__up_OFFSET                                                                     )
    ,ENUM( COPY_BITS_THR4_ET2U_PKT_vlan__cfi_LEN1_TO_CFG_DSA_fwd__cfi_OFFSET                                                                   )
    ,ENUM( COPY_BYTES_THR4_ET2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_OFFSET                                                 )

    ,ENUM( COPY_BITS_THR5_EU2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_OFFSET                                    )
    ,ENUM( COPY_BYTES_THR5_EU2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET                                                                )
    ,ENUM( COPY_BYTES_THR5_EU2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_OFFSET                                                 )
    ,ENUM( COPY_BITS_THR5_EU2U_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_DSA_fwd__vid_OFFSET                                               )

    ,ENUM( COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                                )
    ,ENUM( COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__vid_LEN12_TO_CFG_vlan__vid_OFFSET                                                             )
    ,ENUM( COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__up_LEN3_TO_CFG_vlan__up_OFFSET                                                                )
    ,ENUM( COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__cfi_LEN1_TO_CFG_vlan__cfi_OFFSET                                                              )
    ,ENUM( COPY_BITS_THR6_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET                                                         )
    ,ENUM( COPY_BYTES_THR6_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET                                                                    )
    ,ENUM( COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                                )

    ,ENUM( COPY_BYTES_THR7_Mrr2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                              )
    ,ENUM( COPY_BITS_THR7_Mrr2E_PKT_DSA_from_cpu__Extend_LEN1_TO_DESC_nonQcn_desc__fw_drop_OFFSET                                              )

    ,ENUM( COPY_BYTES_THR8_E_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_OFFSET                                                              )
    ,ENUM( COPY_BITS_THR8_E_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_OFFSET                                                      )
    ,ENUM( COPY_BITS_THR8_E_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_OFFSET                                                                )
    ,ENUM( COPY_BITS_THR8_E_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_OFFSET                                                              )
    ,ENUM( COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_OFFSET                          )
    ,ENUM( COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET                                  )
    ,ENUM( COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET                                  )
    ,ENUM( COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET                                   )
    ,ENUM( COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET                                 )
    ,ENUM( COPY_BYTES_THR8_E_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET                                       )

    ,ENUM( COPY_BYTES_THR9_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_16_OFFSET                                                                )
    ,ENUM( COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_OFFSET                            )
    ,ENUM( COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET                                    )
    ,ENUM( COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET                                    )
    ,ENUM( COPY_BITS_THR9_E2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET                                     )
    ,ENUM( COPY_BITS_THR9_E2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET                                   )
    ,ENUM( COPY_BYTES_THR9_E2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET                                         )

    ,ENUM( COPY_BYTES_THR10_C_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET                                                              )
    ,ENUM( COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_OFFSET                              )
    ,ENUM( COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET                               )
    ,ENUM( COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET                              )
    ,ENUM( COPY_BITS_THR10_C_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_OFFSET                                                     )
    ,ENUM( COPY_BITS_THR10_C_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_OFFSET                                                               )
    ,ENUM( COPY_BITS_THR10_C_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_OFFSET                                                             )
    ,ENUM( COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET                                  )
    ,ENUM( COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET                                )
    ,ENUM( COPY_BYTES_THR10_C_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET                                      )
    ,ENUM( COPY_BITS_THR10_C_V2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_OFFSET                               )

    ,ENUM( COPY_BYTES_THR11_C2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_OFFSET                                                                )
    ,ENUM( COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_OFFSET                                )
    ,ENUM( COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET                                 )
    ,ENUM( COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET                                )
    ,ENUM( COPY_BITS_THR11_C2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET                                    )
    ,ENUM( COPY_BITS_THR11_C2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET                                  )
    ,ENUM( COPY_BYTES_THR11_C2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET                                        )
    ,ENUM( COPY_BITS_THR11_C2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_OFFSET                                 )

    ,ENUM( COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET                                                           )
    ,ENUM( COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_OFFSET                                                          )
    ,ENUM( COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_OFFSET                                                        )
    ,ENUM( COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET                                                    )
    ,ENUM( COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_OFFSET                                                              )
    ,ENUM( COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET                                                            )
    ,ENUM( COPY_BYTES_THR12_U_UC2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET                                                              )
    ,ENUM( COPY_BYTES_THR12_U_UC2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET                                                                )
    ,ENUM( COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                            )
    ,ENUM( COPY_BITS_THR12_U_UC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_OFFSET                                       )
    ,ENUM( COPY_BITS_THR12_U_UC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET                                                     )

    ,ENUM( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w3__eVIDX_15_12_LEN4_TO_CFG_etag_u_mc2c__eVIDX_full_OFFSET                                      )
    ,ENUM( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__eVIDX_11_0_LEN12_TO_CFG_etag_u_mc2c__eVIDX_full_PLUS4_OFFSET                                )
    ,ENUM( COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag_u_mc2c__E_PCP_OFFSET                                                   )
    ,ENUM( COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag_u_mc2c__E_DEI_OFFSET                                                 )
    ,ENUM( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_src_ePort__src_ePort_full_OFFSET                                )
    ,ENUM( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_src_ePort__src_ePort_full_PLUS10_OFFSET                           )
    ,ENUM( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_src_ePort__src_ePort_full_PLUS12_OFFSET                           )
    ,ENUM( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan_eDSA__eVLAN_OFFSET                                             )
    ,ENUM( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan_eDSA__up_OFFSET                                                         )
    ,ENUM( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan_eDSA__cfi_OFFSET                                                       )
    ,ENUM( COPY_BYTES_THR13_U_MC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                            )
    ,ENUM( COPY_BYTES_THR13_U_MC2C_CFG_etag_u_mc2c__E_PCP_LEN6_TO_PKT_etag_u_mc2c__E_PCP_OFFSET                                                )
    ,ENUM( COPY_BYTES_THR13_U_MC2C_CFG_vlan_eDSA__TPID_LEN4_TO_PKT_vlan_eDSA__TPID_OFFSET                                                      )
    ,ENUM( COPY_BITS_THR13_U_MC2C_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_OFFSET                         )
    ,ENUM( COPY_BITS_THR13_U_MC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_OFFSET                        )
    ,ENUM( COPY_BITS_THR13_U_MC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag_u_mc2c__TPID_OFFSET                                              )

    ,ENUM( COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET                                                           )
    ,ENUM( COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_OFFSET                                                          )
    ,ENUM( COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_OFFSET                                                        )
    ,ENUM( COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET                                             )
    ,ENUM( COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__UP_LEN3_TO_CFG_vlan__up_OFFSET                                                       )
    ,ENUM( COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET                                                     )
    ,ENUM( COPY_BITS_THR14_U_MR2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET                                                     )
    ,ENUM( COPY_BYTES_THR14_U_MR2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET                                                              )
    ,ENUM( COPY_BYTES_THR14_U_MR2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET                                                                )
    ,ENUM( COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                            )
    ,ENUM( COPY_BITS_THR14_U_MR2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_OFFSET                                       )

    ,ENUM( COPY_BITS_THR15_QCN_CFG_targetPortEntry__zero_0_LEN10_TO_PKT_qcn_tag__Version_OFFSET                                                )
    ,ENUM( COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN12_TO_PKT_qcn_tag__CPID_63_32_OFFSET                                            )
    ,ENUM( COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN9_TO_PKT_qcn_tag__Encapsulated_priority_OFFSET                                  )
    ,ENUM( COPY_BYTES_THR15_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_OFFSET                                                                )
    ,ENUM( COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_port_number_LEN5_TO_CFG_qcn_ha_table__DSA_w0_SrcPort_4_0_OFFSET                        )
    ,ENUM( COPY_BITS_THR15_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_OFFSET                                                         )
    ,ENUM( COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_OFFSET                                      )
    ,ENUM( COPY_BYTES_THR15_QCN_CFG_qcn_ha_table__sa_mac_47_32_LEN16_TO_PKT_mac_header__mac_sa_47_32_OFFSET                                    )

    ,ENUM( COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET                                                              )
    ,ENUM( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET                                                       )
    ,ENUM( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_OFFSET                                                                 )
    ,ENUM( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET                                                               )
    ,ENUM( COPY_BITS_THR16_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET                                                        )
    ,ENUM( COPY_BYTES_THR16_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET                                                                   )
    ,ENUM( COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET                                                              )
    ,ENUM( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS27_OFFSET           )
    ,ENUM( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS25_OFFSET           )
    ,ENUM( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS15_OFFSET         )
    ,ENUM( COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_10_0_LEN11_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_PLUS3_OFFSET           )
    ,ENUM( COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_13_11_LEN3_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_OFFSET                 )


    ,ENUM( COPY_BYTES_THR19_E2U_Untagged_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_OFFSET                                                      )
    ,ENUM( COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__PCID_LEN12_TO_CFG_HA_Table_Upstream_Ports__E_CID_base_OFFSET                         )
    ,ENUM( COPY_BITS_THR19_E2U_Untagged_DESC_nonQcn_desc__up0_LEN4_TO_CFG_HA_Table_Upstream_Ports__E_PCP_OFFSET                                )
    ,ENUM( COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_HA_Table_Upstream_Ports__IPL_Direction_OFFSET                )
    ,ENUM( COPY_BITS_THR19_E2U_Untagged_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET                                               )
    ,ENUM( COPY_BYTES_THR19_E2U_Untagged_CFG_HA_Table_Upstream_Ports__E_PCP_LEN8_TO_PKT_etag__E_PCP_OFFSET                                     )
    ,ENUM( COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Default_VLAN_Tag_LEN16_TO_PKT_vlan__up_OFFSET                                        )


    ,ENUM( COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET                                                           )
    ,ENUM( COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                            )


    ,ENUM( COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET                                                           )
    ,ENUM( COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                            )

    ,ENUM( COPY_BYTES_THR25_EVB_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_OFFSET                                                            )
    ,ENUM( COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__sa_mac_47_32_LEN6_TO_PKT_mac_header__mac_sa_47_32_OFFSET                             )
    ,ENUM( COPY_BITS_THR25_EVB_QCN_CFG_targetPortEntry__TPID_LEN16_TO_PKT_vlan__TPID_OFFSET                                                    )
    ,ENUM( COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__EtherType_LEN10_TO_PKT_qcn_tag__EtherType_OFFSET                                     )
    ,ENUM( COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN8_TO_PKT_qcn_tag__EtherType_PLUS10_OFFSET                              )
    ,ENUM( COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN7_TO_PKT_qcn_tag__EtherType_PLUS18_OFFSET                              )
    ,ENUM( COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_OFFSET                                                     )
    ,ENUM( COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_OFFSET                                  )

    ,ENUM( COPY_BYTES_THR27_PRE_DA_E2U_CFG_srcPortEntry__reserved_0_LEN2_TO_PKT_pre_da_tag__reserved_OFFSET                                    )

    ,ENUM( COPY_BITS_THR45_VariableCyclesLengthWithAcclCmd_CFG_targetPortEntry__reserved_LEN3_TO_CFG_HA_Table_reserved_space__reserved_2_OFFSET)
    ,ENUM( COPY_BYTES_THR45_VariableCyclesLengthWithAcclCmd_PKT_mac_da_47_32_LEN6_TO_MACSHIFTLEFT_6_OFFSET                                     )

    ,ENUM( COPY_BYTES_THR47_Add20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_20_OFFSET                                                        )
    ,ENUM( COPY_BYTES_THR47_Add20Bytes_CFG_HA_Table_reserved_space__reserved_0_LEN16_TO_PKT_HA_Table_reserved_space__reserved_0_OFFSET         )
    ,ENUM( COPY_BYTES_THR47_Add20Bytes_CFG_srcPortEntry__PVID_LEN4_TO_PKT_srcPortEntry__PVID_OFFSET                                            )

    ,ENUM( COPY_BYTES_THR48_Remove20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_20_OFFSET                                                    )

    ,ENUM( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET                                 )
    ,ENUM( COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET                                         )
    ,ENUM( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_egress_tai_sel_OFFSET                                  )
    ,ENUM( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_select_OFFSET                          )
    ,ENUM( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_entry_id_OFFSET                        )
    ,ENUM( COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_dispatching_en_OFFSET                                          )
    ,ENUM( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_OFFSET                          )
    ,ENUM( COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS10_OFFSET                     )
    ,ENUM( COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_PLUS10_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS20_OFFSET              )
    ,ENUM( COPY_BITS_PTP_DESC_nonQcn_desc__timestamp_Sec_LEN2_TO_DESC_nonQcn_desc__ingress_timestamp_seconds_OFFSET                            )
    ,ENUM( COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__udp_checksum_update_en_OFFSET                                      )
    ,ENUM( COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_OFFSET                                  )

    ,ENUM( ACCELERATOR_COMMAND_OFFSET_LAST_ENTRY                                                                                               )
};

static ACCEL_INFO_STC pipe_accelInfoArr[] =
{
/**************************************************************************
 * Accelerator commands offsets
 **************************************************************************/
    /*************** Thread 1 (E2U) *******************/
     {ENUM_NAME_AND_VALUE( COPY_BYTES_THR1_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_OFFSET                                                                 )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR1_E2U_CFG_srcPortEntry__PCID_LEN12_TO_CFG_etag__E_CID_base_OFFSET                                                      )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR1_E2U_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET                                                         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR1_E2U_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET                                                                  )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR1_E2U_DESC_nonQcn_desc__up0_LEN4_TO_CFG_etag__E_PCP_OFFSET                                                             )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR1_E2U_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_etag__IPL_Direction_OFFSET                                             )}
    /******** Thread 2 (U2E) *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_PLUS8_LEN12_TO_MACSHIFTRIGHT_12_OFFSET                                                         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR2_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                                )}
    /******** Thread 3 (U2C) *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR3_U2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR3_U2C_CFG_cfgReservedSpace__reserved_0_LEN1_TO_PKT_etag__IPL_Direction_OFFSET                                          )}
    /******** Thread 4 (ET2U) *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR4_ET2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_OFFSET                                    )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR4_ET2U_PKT_vlan__vid_LEN12_TO_CFG_DSA_fwd__vid_OFFSET                                                                  )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR4_ET2U_PKT_vlan__up_LEN3_TO_CFG_DSA_fwd__up_OFFSET                                                                     )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR4_ET2U_PKT_vlan__cfi_LEN1_TO_CFG_DSA_fwd__cfi_OFFSET                                                                   )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR4_ET2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_OFFSET                                                 )}
    /******** Thread 5 (EU2U) *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR5_EU2U_CFG_srcPortEntry__src_port_num_LEN4_TO_CFG_DSA_fwd__SrcPort_4_0_PLUS1_OFFSET                                    )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR5_EU2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR5_EU2U_CFG_DSA_fwd__tagCommand_LEN4_TO_PKT_DSA_fwd__tagCommand_OFFSET                                                 )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR5_EU2U_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_DSA_fwd__vid_OFFSET                                               )}
    /******** Thread 6 (U2E) *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__vid_LEN12_TO_CFG_vlan__vid_OFFSET                                                             )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__up_LEN3_TO_CFG_vlan__up_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR6_U2E_PKT_extDSA_fwd_w0__cfi_LEN1_TO_CFG_vlan__cfi_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR6_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET                                                         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR6_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET                                                                    )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR6_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                                )}
    /******** THR7_Mrr2E *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR7_Mrr2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR7_Mrr2E_PKT_DSA_from_cpu__Extend_LEN1_TO_DESC_nonQcn_desc__fw_drop_OFFSET                                              )}
    /******** THR8_E_V2U *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR8_E_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR8_E_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_OFFSET                                                      )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR8_E_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR8_E_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_OFFSET                          )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET                                  )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR8_E_V2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET                                  )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET                                   )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR8_E_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET                                 )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR8_E_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET                                       )}
    /******** THR9_E2U *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR9_E2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_16_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_13_7_LEN7_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS3_OFFSET                            )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_6_5_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET                                    )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR9_E2U_CFG_srcPortEntry__src_ePort_4_0_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET                                    )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR9_E2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET                                     )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR9_E2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET                                   )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR9_E2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET                                         )}
    /******** THR10_C_V2U *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR10_C_V2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_4_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_OFFSET                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET                               )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR10_C_V2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR10_C_V2U_PKT_vlan__vid_LEN12_TO_CFG_eDSA_fwd_w0__eVLAN_11_0_OFFSET                                                     )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR10_C_V2U_PKT_vlan__up_LEN3_TO_CFG_eDSA_fwd_w0__UP_OFFSET                                                               )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR10_C_V2U_PKT_vlan__cfi_LEN1_TO_CFG_eDSA_fwd_w0__CFI_OFFSET                                                             )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET                                  )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR10_C_V2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR10_C_V2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET                                      )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR10_C_V2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_OFFSET                               )}
    /******** THR11_C2U *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR11_C2U_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_8_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_LEN9_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_PLUS1_OFFSET                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS9_LEN2_TO_CFG_eDSA_fwd_w1__Src_ePort_6_5_OFFSET                                 )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR11_C2U_CFG_srcPortEntry__reserved2_PLUS11_LEN5_TO_CFG_eDSA_fwd_w0__Src_ePort_4_0_OFFSET                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR11_C2U_DESC_nonQcn_desc__tc_LEN3_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS2_OFFSET                                    )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR11_C2U_DESC_nonQcn_desc__dp_1_LEN2_TO_CFG_eDSA_fwd_w1__GlobalQoS_Profile_PLUS5_OFFSET                                  )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR11_C2U_CFG_eDSA_fwd_w0__TagCommand_LEN16_TO_PKT_eDSA_fwd_w0__TagCommand_OFFSET                                        )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR11_C2U_CFG_cfgReservedSpace__reserved_0_LEN1_TO_CFG_eDSA_fwd_w2__Src_ePort_16_7_OFFSET                                 )}
    /******** THR12_U_UC2C *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET                                                           )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_OFFSET                                                          )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR12_U_UC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_OFFSET                                                        )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET                                                    )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR12_U_UC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET                                                            )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR12_U_UC2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR12_U_UC2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR12_U_UC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                            )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR12_U_UC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_OFFSET                                       )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR12_U_UC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET                                                     )}
    /******** THR13_U_MC2C *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w3__eVIDX_15_12_LEN4_TO_CFG_etag_u_mc2c__eVIDX_full_OFFSET                                      )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__eVIDX_11_0_LEN12_TO_CFG_etag_u_mc2c__eVIDX_full_PLUS4_OFFSET                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag_u_mc2c__E_PCP_OFFSET                                                   )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag_u_mc2c__E_DEI_OFFSET                                                 )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_src_ePort__src_ePort_full_OFFSET                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_src_ePort__src_ePort_full_PLUS10_OFFSET                           )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_src_ePort__src_ePort_full_PLUS12_OFFSET                           )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan_eDSA__eVLAN_OFFSET                                             )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan_eDSA__up_OFFSET                                                         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan_eDSA__cfi_OFFSET                                                       )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_U_MC2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                            )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_U_MC2C_CFG_etag_u_mc2c__E_PCP_LEN6_TO_PKT_etag_u_mc2c__E_PCP_OFFSET                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR13_U_MC2C_CFG_vlan_eDSA__TPID_LEN4_TO_PKT_vlan_eDSA__TPID_OFFSET                                                      )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_CFG_cfgReservedSpace__reserved_0_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_OFFSET                         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag_u_mc2c__Ingress_E_CID_base_OFFSET                        )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR13_U_MC2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag_u_mc2c__TPID_OFFSET                                              )}
    /******** THR14_U_MR2C *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET                                                           )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__tc_LEN3_TO_CFG_etag__E_PCP_OFFSET                                                          )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR14_U_MR2C_DESC_nonQcn_desc__dp_0_LEN1_TO_CFG_etag__E_DEI_OFFSET                                                        )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET                                             )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__UP_LEN3_TO_CFG_vlan__up_OFFSET                                                       )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR14_U_MR2C_PKT_eDSA_w0_ToAnalyzer__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET                                                     )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR14_U_MR2C_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET                                                     )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_U_MR2C_CFG_etag__E_PCP_LEN6_TO_PKT_etag__E_PCP_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_U_MR2C_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR14_U_MR2C_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_4_OFFSET                                                            )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR14_U_MR2C_CFG_srcPortEntry__reserved1_PLUS4_LEN12_TO_CFG_etag__E_CID_base_OFFSET                                       )}
    /******** THR15_QCN *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR15_QCN_CFG_targetPortEntry__zero_0_LEN10_TO_PKT_qcn_tag__Version_OFFSET                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN12_TO_PKT_qcn_tag__CPID_63_32_OFFSET                                            )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_QCN_CFG_targetPortEntry__zero_0_LEN9_TO_PKT_qcn_tag__Encapsulated_priority_OFFSET                                  )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_OFFSET                                                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_port_number_LEN5_TO_CFG_qcn_ha_table__DSA_w0_SrcPort_4_0_OFFSET                        )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR15_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_OFFSET                                                         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR15_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_OFFSET                                      )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR15_QCN_CFG_qcn_ha_table__sa_mac_47_32_LEN16_TO_PKT_mac_header__mac_sa_47_32_OFFSET                                    )}
    /******** THR16_U2E *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__eVLAN_11_0_LEN12_TO_CFG_vlan__vid_OFFSET                                                       )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__UP_LEN3_TO_CFG_vlan__up_OFFSET                                                                 )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__CFI_LEN1_TO_CFG_vlan__cfi_OFFSET                                                               )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR16_U2E_CFG_targetPortEntry__TPID_LEN16_TO_CFG_vlan__TPID_OFFSET                                                        )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR16_U2E_CFG_vlan__TPID_LEN4_TO_PKT_vlan__TPID_OFFSET                                                                   )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR16_U2E_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_16_OFFSET                                                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w0__Src_ePort_4_0_LEN5_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS27_OFFSET           )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w1__Src_ePort_6_5_LEN2_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS25_OFFSET           )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR16_U2E_PKT_eDSA_fwd_w2__Src_ePort_16_7_LEN10_TO_CFG_cfgReservedSpaceWithSrc_ePort__Src_ePortFull_PLUS15_OFFSET         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_10_0_LEN11_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_PLUS3_OFFSET           )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR16_U2E_CFG_targetPortEntry__trg_ePort_13_11_LEN3_TO_CFG_HA_Table_Target_ePort__Target_ePortFull_OFFSET                 )}

/*************** THR19_E2U_Untagged *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR19_E2U_Untagged_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_12_OFFSET                                                      )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__PCID_LEN12_TO_CFG_HA_Table_Upstream_Ports__E_CID_base_OFFSET                         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR19_E2U_Untagged_DESC_nonQcn_desc__up0_LEN4_TO_CFG_HA_Table_Upstream_Ports__E_PCP_OFFSET                                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Uplink_Port_LEN1_TO_CFG_HA_Table_Upstream_Ports__IPL_Direction_OFFSET                )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR19_E2U_Untagged_CFG_targetPortEntry__TPID_LEN16_TO_PKT_etag__TPID_OFFSET                                               )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR19_E2U_Untagged_CFG_HA_Table_Upstream_Ports__E_PCP_LEN8_TO_PKT_etag__E_PCP_OFFSET                                     )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR19_E2U_Untagged_CFG_srcPortEntry__Default_VLAN_Tag_LEN16_TO_PKT_vlan__up_OFFSET                                        )}

/*************** THR20_U2E_M4 *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET                                                           )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR20_U2E_M4_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                            )}

/*************** THR21_U2E_M8 *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_12_OFFSET                                                           )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR21_U2E_M8_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_8_OFFSET                                                            )}

    /******** THR25_EVB_QCN *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR25_EVB_QCN_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_6_OFFSET                                                            )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__sa_mac_47_32_LEN6_TO_PKT_mac_header__mac_sa_47_32_OFFSET                             )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR25_EVB_QCN_CFG_targetPortEntry__TPID_LEN16_TO_PKT_vlan__TPID_OFFSET                                                    )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__EtherType_LEN10_TO_PKT_qcn_tag__EtherType_OFFSET                                     )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN8_TO_PKT_qcn_tag__EtherType_PLUS10_OFFSET                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR25_EVB_QCN_CFG_evb_qcn_ha_table__reserved_0_LEN7_TO_PKT_qcn_tag__EtherType_PLUS18_OFFSET                              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__qcn_q_fb_LEN6_TO_PKT_qcn_tag__qFb_OFFSET                                                     )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR25_EVB_QCN_DESC_Qcn_desc__congested_queue_number_LEN3_TO_PKT_qcn_tag__CPID_2_0_OFFSET                                  )}

    /******** THR27_PRE_DA_E2U *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR27_PRE_DA_E2U_CFG_srcPortEntry__reserved_0_LEN2_TO_PKT_pre_da_tag__reserved_OFFSET                                    )}

    /******** THR45_VariableCyclesLengthWithAcclCmd *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_THR45_VariableCyclesLengthWithAcclCmd_CFG_targetPortEntry__reserved_LEN3_TO_CFG_HA_Table_reserved_space__reserved_2_OFFSET)}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR45_VariableCyclesLengthWithAcclCmd_PKT_mac_da_47_32_LEN6_TO_MACSHIFTLEFT_6_OFFSET                                     )}
    /******** Thread 47 (Add20Bytes) *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR47_Add20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTLEFT_20_OFFSET                                                        )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR47_Add20Bytes_CFG_HA_Table_reserved_space__reserved_0_LEN16_TO_PKT_HA_Table_reserved_space__reserved_0_OFFSET         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR47_Add20Bytes_CFG_srcPortEntry__PVID_LEN4_TO_PKT_srcPortEntry__PVID_OFFSET                                            )}
    /******** THR48_Remove20Bytes *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BYTES_THR48_Remove20Bytes_PKT_mac_da_47_32_LEN12_TO_MACSHIFTRIGHT_20_OFFSET                                                    )}
    /******** PTP *******************/
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET                                 )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__mac_timestamping_en_OFFSET                                         )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_egress_tai_sel_OFFSET                                  )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_select_OFFSET                          )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_timestamp_queue_entry_id_OFFSET                        )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_dispatching_en_OFFSET                                          )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_CFG_cfgReservedSpace__reserved_0_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_OFFSET                          )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS10_OFFSET                     )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_CFG_targetPortEntry__Egress_Pipe_Delay_PLUS10_LEN10_TO_DESC_nonQcn_desc__egress_pipe_delay_PLUS20_OFFSET              )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_DESC_nonQcn_desc__timestamp_Sec_LEN2_TO_DESC_nonQcn_desc__ingress_timestamp_seconds_OFFSET                            )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__udp_checksum_update_en_OFFSET                                      )}
    ,{ENUM_NAME_AND_VALUE( COPY_BITS_PTP_DESC_nonQcn_desc__is_ptp_LEN1_TO_DESC_nonQcn_desc__ptp_cf_wraparound_check_en_OFFSET                                  )}
    /******** Last Entry *******************/
    ,{ENUM_NAME_AND_VALUE( ACCELERATOR_COMMAND_OFFSET_LAST_ENTRY                                                                                               )}
    /*must be last*/
    ,{NULL , 0}
};

#define ACCEL_CMDS_TRIG_BASE_ADDR___WORDS_SPACE_SIZE  (0x300 / 4)

static uint32_t activeMem_read_DRAM_BASE_ADDR(uint32_t addr, uint32_t *memPtr , uint32_t writeValue/*relevant for write only*/)
{
    SKERNEL_DEVICE_OBJECT *devObjPtr = current_fw_devObjPtr;
    GT_U32  phaBaseAddr = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_PHA); /* Cider of the PHA unit */
    GT_U32  addressInPHA_DRAM_INIT_DONE_ADDR = 0x007C0000;/*Cider offset for the shared DRAM in the PHA unit */
    GT_U32  relativeAddrInDram = addr - DRAM_BASE_ADDR;
    GT_U32  regValue;

    /* get the address from the DRAM of the current device */
    smemRegGet(devObjPtr,
        phaBaseAddr + addressInPHA_DRAM_INIT_DONE_ADDR + relativeAddrInDram,
        &regValue);

    return regValue;
}

/***********************/
/* memory for FW usage */
/***********************/
static MEMORY_RANGE pipe_fw_memoryMap[]=
{    {0x00000000 , 2048  , "scratchpad",NULL}
    ,{PIPE_CFG_REGs_lo , 32 , "packet configuration",NULL}
    ,{PIPE_DESC_REGs_lo , 32 , "packet descriptor",NULL}
    ,{PIPE_PKT_REGs_lo , 148 , "packet header",NULL}

    /*address in PPA*/ /* /Cider/EBU/PIPE/PIPE {Current}/Switching core/<PHA_IP> PHA/<PPA> PPA/PPA/Shared DMEM */
    ,{DRAM_BASE_ADDR /*0x00010000*/ , 8192 , "DRAM_BASE_ADDR",NULL}
    /*seems like address in PPN[1]*/
    ,{PKT_SWAP_INST_ADDR /*0x04000*/, 4 , "PKT_SWAP_INST_ADDR",NULL} /* where in Cider ??? */
    /*seems like address in PPN[1]*/
    ,{PKT_LOAD_PC_INST_ADDR /*0x04004*/, 4 , "PKT_LOAD_PC_INST_ADDR",NULL} /* where in Cider ??? */
    /*seems like address in PPN[1]*/
    ,{ACCEL_CMDS_TRIG_BASE_ADDR /*0x01100*/, 4*ACCEL_CMDS_TRIG_BASE_ADDR___WORDS_SPACE_SIZE , "ACCEL_CMDS_TRIG_BASE_ADDR",NULL} /* where in Cider ??? */
    /*must be last*/
    ,{0,0,0,NULL}/*must be last*/
};

/******************************/
/* active memory for FW usage */
/******************************/
static ACTIVE_MEMORY_RANGE pipe_fw_activeMemoryMap[]=
{
    {ACCEL_CMDS_TRIG_BASE_ADDR , ACCEL_CMDS_TRIG_BASE_ADDR___WORDS_SPACE_SIZE , 0x4  , "ACCEL_CMDS_TRIG_BASE_ADDR",pha_fw_activeMem_write_ACCEL_CMDS_TRIG_BASE_ADDR/*write*/,NULL/*read*/}
    /*address in PPA*//* /Cider/EBU/PIPE/PIPE {Current}/Switching core/<PHA_IP> PHA/<PPA> PPA/PPA/Shared DMEM */
    ,{DRAM_BASE_ADDR /*0x00010000*/ , 8192/4 , 4 , "DRAM_BASE_ADDR",NULL/*write*/,activeMem_read_DRAM_BASE_ADDR}

    /*must be last*/
    ,{0,0,0,NULL,NULL,NULL}/*must be last*/
};



static GT_U32   copyWordsFromTestToMemory_needToSwap = 0;
static void copyWordsFromTestToMemory(
    /*IN*/GT_U32    *origMemPtr,
    /*IN*/GT_U32    numOfWords,
    /*OUT*/GT_U32   *swappedMemPtr
)
{
    GT_U32  ii,iiOrig;

    for(ii = 0 ; ii < numOfWords; ii++)
    {
        iiOrig = copyWordsFromTestToMemory_needToSwap ? numOfWords-(ii+1) : ii;

        swappedMemPtr[ii] = origMemPtr[iiOrig];
    }

    return;
}


typedef void (*THREAD_TYPE)();
typedef struct {
    char* threadName;
    THREAD_TYPE threadType;
    char* threadDescription;
}THREAD_INFO;

#define STATE_NAME_AND_TYPE_MAC(thread) \
    #thread/*name*/ , thread
static THREAD_INFO threadNames[50] = {
    /*enum_THR0_DoNothing            */ {STATE_NAME_AND_TYPE_MAC(THR0_DoNothing            )  ,   "PHA_THREAD_DO_NOTHING : do no modifications (support ECN/PTP modifications)" }
    /*enum_THR1_E2U                  */,{STATE_NAME_AND_TYPE_MAC(THR1_E2U                  )  ,   "PHA_THREAD_ETAG_E2U : E-Tag : Extended Port to Upstream Port (E2U) Thread , Add 8B E-Tag" }
    /*enum_THR2_U2E                  */,{STATE_NAME_AND_TYPE_MAC(THR2_U2E                  )  ,   "PHA_THREAD_ETAG_U2E : E-Tag : Upstream Port to Extended Port (U2E) Thread , Remove 8B E-Tag" }
    /*enum_THR3_U2C                  */,{STATE_NAME_AND_TYPE_MAC(THR3_U2C                  )  ,   "PHA_THREAD_ETAG_U2C : E-Tag : Upstream Port to Cascade Port (U2C) Thread (optionally remove E-tag)" }
    /*enum_THR4_ET2U                 */,{STATE_NAME_AND_TYPE_MAC(THR4_ET2U                 )  ,   "PHA_THREAD_DSA_ET2U : FRW DSA(4B) : Port Tagged to Upstream Port (ET2U) Thread" }
    /*enum_THR5_EU2U                 */,{STATE_NAME_AND_TYPE_MAC(THR5_EU2U                 )  ,   "PHA_THREAD_DSA_EU2U : FRW DSA(4B) : Port Untagged to Upstream Port (EU2U) Thread" }
    /*enum_THR6_U2E                  */,{STATE_NAME_AND_TYPE_MAC(THR6_U2E                  )  ,   "PHA_THREAD_DSA_U2E : FRW DSA(8B) : 8B FORWARD DSA tag remove/replace by VLAN tag." }
    /*enum_THR7_Mrr2E                */,{STATE_NAME_AND_TYPE_MAC(THR7_Mrr2E                )  ,   "PHA_THREAD_DSA_MRR2E : FROM_CPU DSA(4B) : DSA Upstream Port 'Mirroring' to Extended Port (Mrr2E) Thread" }
    /*enum_THR8_E_V2U                */,{STATE_NAME_AND_TYPE_MAC(THR8_E_V2U                )  ,   STR(PHA_THREAD_EDSA_EV2U            ) }
    /*enum_THR9_E2U                  */,{STATE_NAME_AND_TYPE_MAC(THR9_E2U                  )  ,   STR(PHA_THREAD_EDSA_E2U             ) }
    /*enum_THR10_C_V2U               */,{STATE_NAME_AND_TYPE_MAC(THR10_C_V2U               )  ,   STR(PHA_THREAD_EDSA_CV2U            ) }
    /*enum_THR11_C2U                 */,{STATE_NAME_AND_TYPE_MAC(THR11_C2U                 )  ,   STR(PHA_THREAD_EDSA_C2U             ) }
    /*enum_THR12_U_UC2C              */,{STATE_NAME_AND_TYPE_MAC(THR12_U_UC2C              )  ,   STR(PHA_THREAD_EDSA_UUC2C           ) }
    /*enum_THR13_U_MC2C              */,{STATE_NAME_AND_TYPE_MAC(THR13_U_MC2C              )  ,   STR(PHA_THREAD_EDSA_UMC2C           ) }
    /*enum_THR14_U_MR2C              */,{STATE_NAME_AND_TYPE_MAC(THR14_U_MR2C              )  ,   STR(PHA_THREAD_EDSA_UMR2C           ) }
    /*enum_THR15_QCN                 */,{STATE_NAME_AND_TYPE_MAC(THR15_QCN                 )  ,   STR(PHA_THREAD_QCN                  ) }
    /*16*/                             ,{"unknown16",0}
    /*enum_THR17_U2IPL               */,{STATE_NAME_AND_TYPE_MAC(THR17_U2IPL               )  ,   "PHA_THREAD_U2IPL: E-Tag : Upstream Port to Inter Pipe Links Port (U2IPL) Thread"}
    /*enum_THR18_IPL2IPL             */,{STATE_NAME_AND_TYPE_MAC(THR18_IPL2IPL             )  ,   "PHA_THREAD_IPL2IPL: E-Tag : IPL Port to IPL Port (IPL2IPL) Thread"}
    /*enum_THR19_E2U_Untagged        */,{STATE_NAME_AND_TYPE_MAC(THR19_E2U_Untagged        )  ,   "PHA_THREAD_E2U_Untagged: E-Tag : Extended Port to Upstream Port for Untagged VLAN packets (E2U-Untagged) Thread"}
    /*enum_THR20_U2E_M4              */,{STATE_NAME_AND_TYPE_MAC(THR20_U2E_M4              )  ,   "PHA_THREAD_U2E_M4: E-Tag : Upstream Port to Extended Port for Multicast with 4 PCIDs (U2E-M4) Thread"}
    /*enum_THR21_U2E_M8              */,{STATE_NAME_AND_TYPE_MAC(THR21_U2E_M8              )  ,   "PHA_THREAD_U2E_M8: E-Tag : Upstream Port to Extended Port for Multicast with 8 PCIDs (U2E-M8) Thread"}
    /*enum_THR22_Discard             */,{STATE_NAME_AND_TYPE_MAC(THR22_Discard_pkt         )  ,   "PHA_THREAD_Discard : Mark the descriptor to drop packet"}
    /*enum_THR23_EVB_E2U             */,{STATE_NAME_AND_TYPE_MAC(THR23_EVB_E2U             )  ,   "PHA_THREAD_EVB_E2U : EVB Extended Port to Upstream Port (E2U) Thread , Add 4B VLAN tag"}
    /*enum_THR24_EVB_U2E             */,{STATE_NAME_AND_TYPE_MAC(THR24_EVB_U2E             )  ,   "PHA_THREAD_EVB_E2U : EVB Upstream Port to Extended Port (U2E) Thread , Remove 4B VLAN tag"}
    /*enum_THR25_EVB_QCN             */,{STATE_NAME_AND_TYPE_MAC(THR25_EVB_QCN             )  ,   "PHA_THREAD_EVB_QCN : QCN over VLAN tag Thread, Build CNM message over VLAN tag"}
    /*enum_THR26_PRE_DA_U2E          */,{STATE_NAME_AND_TYPE_MAC(THR26_PRE_DA_U2E          )  ,   "PHA_THREAD_PRE_DA_U2E : Remove 2 bytes pre da tag (include information about target port)"}
    /*enum_THR27_PRE_DA_E2U          */,{STATE_NAME_AND_TYPE_MAC(THR27_PRE_DA_E2U          )  ,   "PHA_THREAD_PRE_DA_E2U : Add 2 bytes pre da tag (include information about source port)"}
    /*28*/                             ,{"unknown28",0}
    /*29*/                             ,{"unknown29",0}
    /*30*/                             ,{"unknown30",0}
    /*31*/                             ,{"unknown31",0}
    /*32*/                             ,{"unknown32",0}
    /*33*/                             ,{"unknown33",0}
    /*34*/                             ,{"unknown34",0}
    /*35*/                             ,{"unknown35",0}
    /*36*/                             ,{"unknown36",0}
    /*37*/                             ,{"unknown37",0}
    /*38*/                             ,{"unknown38",0}
    /*39*/                             ,{"unknown39",0}
    /*40*/                             ,{"unknown40",0}
    /*41*/                             ,{"unknown41",0}
    /*42*/                             ,{"unknown42",0}
    /*43*/                             ,{"unknown43",0}
    /*44*/                             ,{"unknown44",0}
    /*45*/                             ,{"unknown45",0}
    /*enum_THR46_RemoveAddBytes      */,{STATE_NAME_AND_TYPE_MAC(THR46_RemoveAddBytes      )  ,   "PHA_THREAD_DO_NOT_MODIFY : do not modify (even no ECN/PTP)"}
    /*enum_THR47_Add20Bytes          */,{STATE_NAME_AND_TYPE_MAC(THR47_Add20Bytes          )  ,   STR(PHA_THREAD_ADD_20B              ) }
    /*enum_THR48_Remove20Bytes       */,{STATE_NAME_AND_TYPE_MAC(THR48_Remove20Bytes       )  ,   STR(PHA_THREAD_REMOVE_20B           ) }
    /*enum_THR49_VariableCyclesLength*/,{STATE_NAME_AND_TYPE_MAC(THR49_VariableCyclesLength)  ,   STR(PHA_THREAD_VARIABLE_CYCLE_LENGTH) }
};


/* init values for testing of 'test_THR2_U2E' with PTP */
/* return is passed : GT_TRUE --> passed , GT_FALSE --> failed */
static GT_BOOL test_firmware_case(
    ENUM_TREADS_ENT  testCase,
    char*   extraNotePtr,

    GT_U32  hdrWords[],/* the IN packet */
    GT_U32  pktNumWords,
    GT_U32  cfgWords[],/* the configurations (8 words)*/
    GT_U32  descWords[],/* the IN descriptor (8 words)*/

    GT_U32 outgoing_headerWords[], /* the OUT packet */
    GT_U32 outgoing_pktNumWords,
    GT_U32 outgoing_descWords[]/* the OUT descriptor*/
)
{
    GT_BOOL orig_simLogIsOpenFlag = simLogIsOpenFlag;
    GT_U32  *configMemPtr = pha_findMem(PIPE_CFG_REGs_lo);
    GT_U32  *descMemPtr = pha_findMem(PIPE_DESC_REGs_lo);
    GT_U32  *pktMemPtr = pha_findMem(PIPE_PKT_REGs_lo);
    GT_U32  configNumWords = 8;
    GT_U32  descNumWords = 8;
    GT_U32  ii;
    GT_U32  numErrors = 0;
    GT_U32  out_desc_numErrors = 0;

    __LOG_NO_LOCATION_META_DATA(("#########################################"));
    /* copy test config words and descriptor words before going into the memory */
    /* the data already in 'proper' order (big endian) */
    /* we call a function that may need to SWAP words duie to little/big endian */
    copyWordsFromTestToMemory(cfgWords,configNumWords,configMemPtr);
    copyWordsFromTestToMemory(descWords,descNumWords,descMemPtr);

    /* no swap on words of the 'packet header' */
    for(ii = 0 ; ii < pktNumWords; ii++)
    {
        pktMemPtr[ii] = hdrWords[ii];
    }

    if(extraNotePtr == NULL)
    {
        extraNotePtr = "";
    }

    /* run the thread to process the data , and then we can check egress packet */
    __LOG_NO_LOCATION_META_DATA(("Start : %s %s\n",threadNames[testCase].threadName,extraNotePtr));
    threadNames[testCase].threadType();
    __LOG_NO_LOCATION_META_DATA(("Ended : %s %s\n",threadNames[testCase].threadName,extraNotePtr));
    /* apply ALL the changes that the descriptor/header got */
    applyAllChanges();


    for(ii = 0 ; ii < outgoing_pktNumWords; ii++)
    {
        if(pktMemPtr[ii]            /*actual*/      !=
           outgoing_headerWords[ii]/*expected*/)
        {
            printf("ERROR : packet : word[%d] hold value[0x%8.8x] expected [0x%8.8x] XOR[0x%8.8x]\n",
                ii,
                pktMemPtr[ii],
                outgoing_headerWords[ii],
                pktMemPtr[ii] ^ outgoing_headerWords[ii]);
            numErrors++;
        }
        else /* As expected */
        {
            printf("GOOD : packet : word[%d] hold value[0x%8.8x] as expected \n",
                ii,pktMemPtr[ii]);
        }
    }

    {
        GT_U32 tmp_descWords[8];

        /* copy the descriptor before comparing */
        /* we call a function that may need to SWAP words due to little/big endian */
        copyWordsFromTestToMemory(descMemPtr,descNumWords,tmp_descWords);

        for(ii = 0 ; ii < descNumWords; ii++)
        {
            descMemPtr[ii] = tmp_descWords[ii];
        }
    }

    for(ii = 0 ; ii < descNumWords; ii++)
    {
        if(descMemPtr[ii]           /*actual*/      !=
           outgoing_descWords[ii]/*expected*/)
        {
            printf("ERROR : desc : word[%d] hold value[0x%8.8x] expected [0x%8.8x] XOR[0x%8.8x]\n",
                ii,
                descMemPtr[ii],
                outgoing_descWords[ii],
                descMemPtr[ii] ^ outgoing_descWords[ii]);
            numErrors++;
            out_desc_numErrors++;
        }
        else /* As expected */
        {
            printf("GOOD : desc : word[%d] hold value[0x%8.8x] as expected \n",
                ii,descMemPtr[ii]);
        }
    }

    if(out_desc_numErrors)
    {
        static struct nonQcn_desc test___copyMem_nonQcn_desc__orig;

        /* save the values that current descriptor hold */
        test___copyMem_nonQcn_desc__orig = copyMem_nonQcn_desc;
        /* state that 'orig' now is the 'expected descriptor' */
        copyMem_nonQcn_desc__isOrigSaved = 0;

        /* temporary disable the LOG , so it will not LOG the :
            PIPE_MEM_CAST(...,nonQcn_desc) */
        simLogIsOpenFlag = 0;

        /* copy the expected 'outgoing_descWords' to memory */
        copyWordsFromTestToMemory(outgoing_descWords,descNumWords,descMemPtr);
        /* trigger the updating of 'expected descriptor' into 'orig' */
        PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

        /* restore the LOG */
        simLogIsOpenFlag = orig_simLogIsOpenFlag;

        __LOG_NO_LOCATION_META_DATA(("START : compare out descriptor fields to catch mismatch \n"));
        /* restore the 'actual' */
        copyMem_nonQcn_desc = test___copyMem_nonQcn_desc__orig;
        /* __LOG the comparing of 'expected' and actual */
        pipeMemCompare_nonQcn_desc(GT_TRUE);

        __LOG_NO_LOCATION_META_DATA(("ENDED : compare out descriptor fields to catch mismatch \n"));
    }

    __LOG_NO_LOCATION_META_DATA(("#########################################"));


    if(numErrors)
    {
        printf("***********************\n"
                "Test : FAILED with [%d] errors\n",
                numErrors);
        return GT_FALSE;/*failed*/
    }
    else
    {
        printf("***********************\n"
                "Test : PASSED \n");
        return GT_TRUE;/*passed*/
    }

}

/* init values for testing of 'THR1_E2U' */
/* return is passed : GT_TRUE --> passed , GT_FALSE --> failed */
static GT_BOOL test_THR1_E2U(void)
{
/*****************start ********************/
/*
hdr: 0000000000000000000000000000000000000000111111111111222222222222
data_structure expansion_space: reserved_0(0): 0x0, reserved_1(32): 0x0, reserved_2(64): 0x0, reserved_3(96): 0x0, reserved_4(128): 0x0,
data_structure mac_header: mac_da_47_32(160): 0x1111, mac_da_31_16(176): 0x1111, mac_da_15_0(192): 0x1111, mac_sa_47_32(208): 0x2222, mac_sa_31_16(224): 0x2222, mac_sa_15_0(240): 0x2222,
*/
    GT_U32  hdrWords[]={
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x11111111,
        0x11112222,
        0x22222222};
/*****************start ********************/
/*
cfg: 12c0e03491206ec70000000000000000624f3d84eea0a8690c2aaaaa00000000
data_structure etag: TPID(0): 0x12c0, E_PCP(16): 0x7, E_DEI(19): 0x0, Ingress_E_CID_base(20): 0x34, Reserved(32): 0x2, GRP(34): 0x1, E_CID_base(36): 0x120, Ingress_E_CID_ext(48): 0x6e, E_CID_ext(56): 0xc7,
data_structure HA_Table_reserved_space: reserved_0(64): 0x0, reserved_1(96): 0x0,
data_structure srcPortEntry: PCID(128): 0x624, src_port_num(140): 0xf, reserved(144): 0x3d84,
data_structure targetPortEntry: TPID(160): 0xeea0, PVID(176): 0xa86, VlanEgrTagState(188): 0x1, reserved(189): 0x1, PCID(192): 0xc2, Egress_Pipe_Delay(204): 0xaaaaa,
data_structure cfgReservedSpace: reserved_0(224): 0x0,
*/
    GT_U32 cfgWords[] = {
        0x12c0e034,
        0x91206ec7,
        0x00000000,
        0x00000000,
        0x624f3d84,
        0xeea0a869,
        0x0c2aaaaa,
        0x00000000};
/*****************start ********************/
/*
desc: 000000000007544e10000072012cc18a000c0000000000000000000000000000
data_structure nonQcn_desc: timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x0, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0, ptp_u_field(45): 0x1, ptp_pkt_type_idx(46): 0x1a,
                            ptp_offset(51): 0x51, outer_l3_offset(58): 0xe, up0(64): 0x0, dei(67): 0x1, dp_1(68): 0x0, dp_0(69): 0x0, packet_hash_6_5(70): 0x0, cnc_address(72): 0x0, txq_queue_id(88): 0x72,
                            ingress_byte_count(96): 0x4b, table_hw_error_detected(110): 0x0, header_hw_error_detected(111): 0x0, tc(112): 0x6, packet_is_cut_through(115): 0x0, cut_through_id(116): 0xc5,
                            need_crc_removal(127): 0x0, txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0,
                            desc_reserved(140): 0xc, ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0, ptp_timestamp_queue_entry_id(150): 0x0,
                            egress_pipe_delay(160): 0x0, ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0, fw_bc_modification(200): 0x0, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0,
                            ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0, udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 descWords[] = {
        0x00000000,
        0x0007544e,
        0x10000072,
        0x012cc18a,
        0x000c0000,
        0x00000000,
        0x00000000,
        0x00000000};

/*****************start ********************/
/*
outgoing header: 111111111111222222222222eea0103496246ec7
mac_da_47_32(96): 0x1111, mac_da_31_16(112): 0x1111, mac_da_15_0(128): 0x1111, mac_sa_47_32(144): 0x2222, mac_sa_31_16(160): 0x2222, mac_sa_15_0(176): 0x2222,
data_structure etag: TPID(192): 0xeea0, E_PCP(208): 0x0, E_DEI(211): 0x1, Ingress_E_CID_base(212): 0x34, Reserved(224): 0x2, GRP(226): 0x1, E_CID_base(228): 0x624, Ingress_E_CID_ext(240): 0x6e, E_CID_ext(248): 0xc7,
*/
    GT_U32 outgoing_headerWords[] = {
        0,
        0,
        0,
        0x11111111,
        0x11112222,
        0x22222222,
        0xeea01034,
        0x96246ec7};

/*
final ppa desc: 000000000007544e10000072012cc18a000c0000000000000008000000000000

PPA::BuildOutput[723]: final ppa desc: data_structure nonQcn_desc: timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x0, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0,
ptp_u_field(45): 0x1, ptp_pkt_type_idx(46): 0x1a, ptp_offset(51): 0x51, outer_l3_offset(58): 0xe, up0(64): 0x0, dei(67): 0x1, dp_1(68): 0x0, dp_0(69): 0x0, packet_hash_6_5(70): 0x0, cnc_address(72): 0x0,
txq_queue_id(88): 0x72, ingress_byte_count(96): 0x4b, table_hw_error_detected(110): 0x0, header_hw_error_detected(111): 0x0, tc(112): 0x6, packet_is_cut_through(115): 0x0, cut_through_id(116): 0xc5,
need_crc_removal(127): 0x0, txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0, desc_reserved(140): 0xc,
ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0, ptp_timestamp_queue_entry_id(150): 0x0, egress_pipe_delay(160): 0x0,
ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0, fw_bc_modification(200): 0x8, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0, ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0,
udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 outgoing_descWords[] = {
        0x00000000,
        0x0007544e,
        0x10000072,
        0x012cc18a,
        0x000c0000,
        0x00000000,
        0x00080000,
        0x00000000};


    GT_U32  pktNumWords = 8;
    GT_U32  outgoing_pktNumWords = pktNumWords;

    return test_firmware_case(enum_THR1_E2U,NULL,hdrWords,pktNumWords,cfgWords,descWords,outgoing_headerWords,outgoing_pktNumWords,outgoing_descWords);
}

/* init values for testing of 'THR1_E2U' */
/* return is passed : GT_TRUE --> passed , GT_FALSE --> failed */
static GT_BOOL test_THR1_E2U_with_ECN_ipv4(void)
{
/*****************start ********************/
/*
hdr: 0000000000000000000000000000000000000000111111111111222222222222
data_structure expansion_space: reserved_0(0): 0x0, reserved_1(32): 0x0, reserved_2(64): 0x0, reserved_3(96): 0x0, reserved_4(128): 0x0,
data_structure mac_header: mac_da_47_32(160): 0x1111, mac_da_31_16(176): 0x1111, mac_da_15_0(192): 0x1111, mac_sa_47_32(208): 0x2222, mac_sa_31_16(224): 0x2222, mac_sa_15_0(240): 0x2222,
*/
    GT_U32  hdrWords[]={
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x11111111,
        0x11112222,
        0x22222222,
        0x08004500,/* ipv4 header */
        0x11223344,
        0x55667788,
        0x99aabbcc,
        0xddeeff00,
        0x88776655,
        };
/*****************start ********************/
/*
cfg: 12c0e03491206ec70000000000000000624f3d84eea0a8690c2aaaaa00000000
data_structure etag: TPID(0): 0x12c0, E_PCP(16): 0x7, E_DEI(19): 0x0, Ingress_E_CID_base(20): 0x34, Reserved(32): 0x2, GRP(34): 0x1, E_CID_base(36): 0x120, Ingress_E_CID_ext(48): 0x6e, E_CID_ext(56): 0xc7,
data_structure HA_Table_reserved_space: reserved_0(64): 0x0, reserved_1(96): 0x0,
data_structure srcPortEntry: PCID(128): 0x624, src_port_num(140): 0xf, reserved(144): 0x3d84,
data_structure targetPortEntry: TPID(160): 0xeea0, PVID(176): 0xa86, VlanEgrTagState(188): 0x1, reserved(189): 0x1, PCID(192): 0xc2, Egress_Pipe_Delay(204): 0xaaaaa,
data_structure cfgReservedSpace: reserved_0(224): 0x0,
*/
    GT_U32 cfgWords[] = {
        0x12c0e034,
        0x91206ec7,
        0x00000000,
        0x00000000,
        0x624f3d84,
        0xeea0a869,
        0x0c2aaaaa,
        0x00000000};
/*****************start ********************/
/*
desc: 000000000007544e10000072012cc18a000c0000000000000000000000000000
data_structure nonQcn_desc: timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x0, mark_ecn(33): 0x1, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0, ptp_u_field(45): 0x1, ptp_pkt_type_idx(46): 0x1a,
                            ptp_offset(51): 0x51, outer_l3_offset(58): 0xe, up0(64): 0x0, dei(67): 0x1, dp_1(68): 0x0, dp_0(69): 0x0, packet_hash_6_5(70): 0x0, cnc_address(72): 0x0, txq_queue_id(88): 0x72,
                            ingress_byte_count(96): 0x4b, table_hw_error_detected(110): 0x0, header_hw_error_detected(111): 0x0, tc(112): 0x6, packet_is_cut_through(115): 0x0, cut_through_id(116): 0xc5,
                            need_crc_removal(127): 0x0, txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0,
                            desc_reserved(140): 0xc, ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0, ptp_timestamp_queue_entry_id(150): 0x0,
                            egress_pipe_delay(160): 0x0, ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0, fw_bc_modification(200): 0x0, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0,
                            ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0, udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 descWords[] = {
        0x00000000,
        0x4007544c, /*mark_ecn(33): 0x1, outer_l3_offset = 12 (0xc)*/
        0x10000072,
        0x012cc18a,
        0x000c0000,
        0x00000000,
        0x00000000,
        0x00000000};

/*****************start ********************/
/*
outgoing header: 111111111111222222222222eea0103496246ec7
mac_da_47_32(96): 0x1111, mac_da_31_16(112): 0x1111, mac_da_15_0(128): 0x1111, mac_sa_47_32(144): 0x2222, mac_sa_31_16(160): 0x2222, mac_sa_15_0(176): 0x2222,
data_structure etag: TPID(192): 0xeea0, E_PCP(208): 0x0, E_DEI(211): 0x1, Ingress_E_CID_base(212): 0x34, Reserved(224): 0x2, GRP(226): 0x1, E_CID_base(228): 0x624, Ingress_E_CID_ext(240): 0x6e, E_CID_ext(248): 0xc7,
*/
    GT_U32 outgoing_headerWords[] = {
        0,
        0,
        0,
        0x11111111,
        0x11112222,
        0x22222222,
        0xeea01034,
        0x96246ec7,
        0x08004503,/* ipv4 header , with ecn = 3*/
        0x11223344,
        0x55667788,
        0x99a7bbcc,/* with the updated checksum ! */
        0xddeeff00,
        0x88776655,
        };

/*
final ppa desc: 000000000007544e10000072012cc18a000c0000000000000008000000000000

PPA::BuildOutput[723]: final ppa desc: data_structure nonQcn_desc: timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x0, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0,
ptp_u_field(45): 0x1, ptp_pkt_type_idx(46): 0x1a, ptp_offset(51): 0x51, outer_l3_offset(58): 0xe, up0(64): 0x0, dei(67): 0x1, dp_1(68): 0x0, dp_0(69): 0x0, packet_hash_6_5(70): 0x0, cnc_address(72): 0x0,
txq_queue_id(88): 0x72, ingress_byte_count(96): 0x4b, table_hw_error_detected(110): 0x0, header_hw_error_detected(111): 0x0, tc(112): 0x6, packet_is_cut_through(115): 0x0, cut_through_id(116): 0xc5,
need_crc_removal(127): 0x0, txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0, desc_reserved(140): 0xc,
ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0, ptp_timestamp_queue_entry_id(150): 0x0, egress_pipe_delay(160): 0x0,
ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0, fw_bc_modification(200): 0x8, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0, ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0,
udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 outgoing_descWords[] = {
        0x00000000,
        0x4007544c, /*mark_ecn(33): 0x1, outer_l3_offset = 12 (0xc)*/
        0x10000072,
        0x012cc18a,
        0x000c0000,
        0x00000000,
        0x00080000,
        0x00000000};


    GT_U32  pktNumWords = sizeof(hdrWords)/4;
    GT_U32  outgoing_pktNumWords = pktNumWords;

    return test_firmware_case(enum_THR1_E2U,"ECN_ipv4",hdrWords,pktNumWords,cfgWords,descWords,outgoing_headerWords,outgoing_pktNumWords,outgoing_descWords);
}

/* init values for testing of 'THR1_E2U' */
/* return is passed : GT_TRUE --> passed , GT_FALSE --> failed */
static GT_BOOL test_THR1_E2U_with_ECN_ipv6(void)
{
/*****************start ********************/
/*
hdr: 0000000000000000000000000000000000000000111111111111222222222222
data_structure expansion_space: reserved_0(0): 0x0, reserved_1(32): 0x0, reserved_2(64): 0x0, reserved_3(96): 0x0, reserved_4(128): 0x0,
data_structure mac_header: mac_da_47_32(160): 0x1111, mac_da_31_16(176): 0x1111, mac_da_15_0(192): 0x1111, mac_sa_47_32(208): 0x2222, mac_sa_31_16(224): 0x2222, mac_sa_15_0(240): 0x2222,
*/
    GT_U32  hdrWords[]={
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x11111111,
        0x11112222,
        0x22222222,
        0xd8dd6500,/* ipv6 header */
        0x11223344,
        0x55667788,
        0x99aabbcc,
        0xddeeff00,
        0x88776655,
        0x11111111,
        0x11112222,
        0x22222222,
        0x11111111,
        0x11112222,
        0x22222222,
        };
/*****************start ********************/
/*
cfg: 12c0e03491206ec70000000000000000624f3d84eea0a8690c2aaaaa00000000
data_structure etag: TPID(0): 0x12c0, E_PCP(16): 0x7, E_DEI(19): 0x0, Ingress_E_CID_base(20): 0x34, Reserved(32): 0x2, GRP(34): 0x1, E_CID_base(36): 0x120, Ingress_E_CID_ext(48): 0x6e, E_CID_ext(56): 0xc7,
data_structure HA_Table_reserved_space: reserved_0(64): 0x0, reserved_1(96): 0x0,
data_structure srcPortEntry: PCID(128): 0x624, src_port_num(140): 0xf, reserved(144): 0x3d84,
data_structure targetPortEntry: TPID(160): 0xeea0, PVID(176): 0xa86, VlanEgrTagState(188): 0x1, reserved(189): 0x1, PCID(192): 0xc2, Egress_Pipe_Delay(204): 0xaaaaa,
data_structure cfgReservedSpace: reserved_0(224): 0x0,
*/
    GT_U32 cfgWords[] = {
        0x12c0e034,
        0x91206ec7,
        0x00000000,
        0x00000000,
        0x624f3d84,
        0xeea0a869,
        0x0c2aaaaa,
        0x00000000};
/*****************start ********************/
/*
desc: 000000000007544e10000072012cc18a000c0000000000000000000000000000
data_structure nonQcn_desc: timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x0, mark_ecn(33): 0x1, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0, ptp_u_field(45): 0x1, ptp_pkt_type_idx(46): 0x1a,
                            ptp_offset(51): 0x51, outer_l3_offset(58): 0xe, up0(64): 0x0, dei(67): 0x1, dp_1(68): 0x0, dp_0(69): 0x0, packet_hash_6_5(70): 0x0, cnc_address(72): 0x0, txq_queue_id(88): 0x72,
                            ingress_byte_count(96): 0x4b, table_hw_error_detected(110): 0x0, header_hw_error_detected(111): 0x0, tc(112): 0x6, packet_is_cut_through(115): 0x0, cut_through_id(116): 0xc5,
                            need_crc_removal(127): 0x0, txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0,
                            desc_reserved(140): 0xc, ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0, ptp_timestamp_queue_entry_id(150): 0x0,
                            egress_pipe_delay(160): 0x0, ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0, fw_bc_modification(200): 0x0, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0,
                            ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0, udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 descWords[] = {
        0x00000000,
        0x4007544c, /*mark_ecn(33): 0x1, outer_l3_offset = 12 (0xc)*/
        0x10000072,
        0x012cc18a,
        0x000c0000,
        0x00000000,
        0x00000000,
        0x00000000};

/*****************start ********************/
/*
outgoing header: 111111111111222222222222eea0103496246ec7
mac_da_47_32(96): 0x1111, mac_da_31_16(112): 0x1111, mac_da_15_0(128): 0x1111, mac_sa_47_32(144): 0x2222, mac_sa_31_16(160): 0x2222, mac_sa_15_0(176): 0x2222,
data_structure etag: TPID(192): 0xeea0, E_PCP(208): 0x0, E_DEI(211): 0x1, Ingress_E_CID_base(212): 0x34, Reserved(224): 0x2, GRP(226): 0x1, E_CID_base(228): 0x624, Ingress_E_CID_ext(240): 0x6e, E_CID_ext(248): 0xc7,
*/
    GT_U32 outgoing_headerWords[] = {
        0,
        0,
        0,
        0x11111111,
        0x11112222,
        0x22222222,
        0xeea01034,
        0x96246ec7,
        0xd8dd6530,/* ipv6 header , with ecn = 3 */
        0x11223344,
        0x55667788,
        0x99aabbcc,
        0xddeeff00,
        0x88776655,
        0x11111111,
        0x11112222,
        0x22222222,
        0x11111111,
        0x11112222,
        0x22222222,
        };

/*
final ppa desc: 000000000007544e10000072012cc18a000c0000000000000008000000000000

PPA::BuildOutput[723]: final ppa desc: data_structure nonQcn_desc: timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x0, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0,
ptp_u_field(45): 0x1, ptp_pkt_type_idx(46): 0x1a, ptp_offset(51): 0x51, outer_l3_offset(58): 0xe, up0(64): 0x0, dei(67): 0x1, dp_1(68): 0x0, dp_0(69): 0x0, packet_hash_6_5(70): 0x0, cnc_address(72): 0x0,
txq_queue_id(88): 0x72, ingress_byte_count(96): 0x4b, table_hw_error_detected(110): 0x0, header_hw_error_detected(111): 0x0, tc(112): 0x6, packet_is_cut_through(115): 0x0, cut_through_id(116): 0xc5,
need_crc_removal(127): 0x0, txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0, desc_reserved(140): 0xc,
ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0, ptp_timestamp_queue_entry_id(150): 0x0, egress_pipe_delay(160): 0x0,
ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0, fw_bc_modification(200): 0x8, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0, ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0,
udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 outgoing_descWords[] = {
        0x00000000,
        0x4007544c, /*mark_ecn(33): 0x1, outer_l3_offset = 12 (0xc)*/
        0x10000072,
        0x012cc18a,
        0x000c0000,
        0x00000000,
        0x00080000,
        0x00000000};


    GT_U32  pktNumWords = sizeof(hdrWords)/4;
    GT_U32  outgoing_pktNumWords = pktNumWords;

    return test_firmware_case(enum_THR1_E2U,"ECN_ipv6",hdrWords,pktNumWords,cfgWords,descWords,outgoing_headerWords,outgoing_pktNumWords,outgoing_descWords);
}

/* init values for testing of 'test_THR2_U2E' */
/* return is passed : GT_TRUE --> passed , GT_FALSE --> failed */
static GT_BOOL test_THR2_U2E(void)
{
/*****************start ********************/
/*
hdr: 00000000000000000000000000000000000000002222222222223333333333335095687d47ba1d3608889e23
data_structure expansion_space: reserved_0(0): 0x0, reserved_1(32): 0x0, reserved_2(64): 0x0, reserved_3(96): 0x0, reserved_4(128): 0x0,
data_structure mac_header: mac_da_47_32(160): 0x2222, mac_da_31_16(176): 0x2222, mac_da_15_0(192): 0x2222, mac_sa_47_32(208): 0x3333, mac_sa_31_16(224): 0x3333, mac_sa_15_0(240): 0x3333,
data_structure etag: TPID(256): 0x5095, E_PCP(272): 0x3, E_DEI(275): 0x0, Ingress_E_CID_base(276): 0x87d, Reserved(288): 0x1, GRP(290): 0x0, E_CID_base(292): 0x7ba, Ingress_E_CID_ext(304): 0x1d, E_CID_ext(312): 0x36,
data_structure vlan: TPID(320): 0x888, up(336): 0x4, cfi(339): 0x1, vid(340): 0xe23,
*/
    GT_U32  hdrWords[]={
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x22222222,
        0x22223333,
        0x33333333,
        0x5095687d,
        0x47ba1d36,
        0x08889e23,/*payload*/
        0x11223344,/*payload*/
        0x55667788 /*payload*/
        };
/*****************start ********************/
/*
cfg: 00000000000000000000000000000000624f3d840888827728c0ef6a00000000
data_structure HA_Table_reserved_space: reserved_0(0): 0x0, reserved_1(32): 0x0, reserved_2(64): 0x0, reserved_3(96): 0x0,
data_structure srcPortEntry: PCID(128): 0x624, src_port_num(140): 0xf, reserved(144): 0x3d84,
data_structure targetPortEntry: TPID(160): 0x888, PVID(176): 0x827, VlanEgrTagState(188): 0x0, reserved(189): 0x7, PCID(192): 0x28c, Egress_Pipe_Delay(204): 0xef6a,
data_structure cfgReservedSpace: reserved_0(224): 0x0,
*/
    GT_U32 cfgWords[] = {
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x624f3d84,
        0x08888277,
        0x28c0ef6a,
        0x00000000};
/*****************start ********************/
/*
desc: 000000000007475a700000f10110057600040000000000000000000000000000
timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x0, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0,
ptp_u_field(45): 0x1, ptp_pkt_type_idx(46): 0x1a, ptp_offset(51): 0x1d, outer_l3_offset(58): 0x1a, up0(64): 0x3, dei(67): 0x1, dp_1(68): 0x0,
dp_0(69): 0x0, packet_hash_6_5(70): 0x0, cnc_address(72): 0x0, txq_queue_id(88): 0xf1, ingress_byte_count(96): 0x44, table_hw_error_detected(110): 0x0,
header_hw_error_detected(111): 0x0, tc(112): 0x0, packet_is_cut_through(115): 0x0, cut_through_id(116): 0x2bb, need_crc_removal(127): 0x0,
txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0,
desc_reserved(140): 0x4, ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0,
ptp_timestamp_queue_entry_id(150): 0x0, egress_pipe_delay(160): 0x0, ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0,
fw_bc_modification(200): 0x0, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0, ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0,
udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 descWords[] = {
        0x00000000,
        0x0007475a,
        0x700000f1,
        0x01100576,
        0x00040000,
        0x00000000,
        0x00000000,
        0x00000000};
/*****************start ********************/
/*
outgoing header: 2222222222223333333333333
data_structure mac_header: mac_da_47_32(256): 0x2222, mac_da_31_16(272): 0x2222, mac_da_15_0(288): 0x2222, mac_sa_47_32(304): 0x3333,
mac_sa_31_16(320): 0x3333, mac_sa_15_0(336): 0x3333,
*/
    GT_U32 outgoing_headerWords[] = {
        0x00000000,/*old info*/
        0x00000000,/*old info*/
        0x00000000,/*old info*/
        0x00000000,/*old info*/
        0x00000000,/*old info*/
        0x22222222,/*old info*/
        0x22223333,/*old info*/
        0x33333333,/*old info*/
        0x22222222,/* new start for packet */
        0x22223333,
        0x33333333,
        0x11223344,/*payload*/
        0x55667788 /*payload*/
        };

/*
out ppa desc: 000000000007475a700000f101100576000400000000000000f4000000000000
timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x0, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0,
ptp_u_field(45): 0x1, ptp_pkt_type_idx(46): 0x1a, ptp_offset(51): 0x1d, outer_l3_offset(58): 0x1a, up0(64): 0x3, dei(67): 0x1, dp_1(68): 0x0,
dp_0(69): 0x0, packet_hash_6_5(70): 0x0, cnc_address(72): 0x0, txq_queue_id(88): 0xf1, ingress_byte_count(96): 0x44, table_hw_error_detected(110): 0x0,
header_hw_error_detected(111): 0x0, tc(112): 0x0, packet_is_cut_through(115): 0x0, cut_through_id(116): 0x2bb, need_crc_removal(127): 0x0,
txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0,
desc_reserved(140): 0x4, ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0,
ptp_timestamp_queue_entry_id(150): 0x0, egress_pipe_delay(160): 0x0, ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0,
fw_bc_modification(200): 0xf4, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0, ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0,
udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 outgoing_descWords[] = {
        0x00000000,
        0x0007475a,
        0x700000f1,
        0x01100576,
        0x00040000,
        0x00000000,
        0x00f40000,
        0x00000000};


    GT_U32  pktNumWords = sizeof(hdrWords)/4;
    GT_U32  outgoing_pktNumWords = sizeof(outgoing_headerWords)/4;

    return test_firmware_case(enum_THR2_U2E,NULL,hdrWords,pktNumWords,cfgWords,descWords,outgoing_headerWords,outgoing_pktNumWords,outgoing_descWords);
}

/* init values for testing of 'test_THR2_U2E' with PTP */
/* return is passed : GT_TRUE --> passed , GT_FALSE --> failed */
static GT_BOOL test_THR2_U2E_with_ptp(void)
{
/*
<THR2_U2E>
                <ptp_type>IPV6</ptp_type>
                <ovrd>txdma2ha_desc__timestamp = 0xA</ovrd>
                <ovrd>txdma2ha_desc__outer_l3_offset = 0x18</ovrd>
                <ovrd>txdma2ha_desc__ptp_offset = 0x32</ovrd>
                <ovrd>hdr__ptp_header__transportSpecific = 0xe</ovrd>
                <ovrd>hdr__ptp_header__messageType = 0xe</ovrd>
                <ovrd>hdr__ptp_header__reserved0 = 0xe</ovrd>
                <ovrd>hdr__ptp_header__versionPTP = 0xe</ovrd>
                <ovrd>hdr__ptp_header__correctionField_15_0 = 0x4444</ovrd>
                <ovrd>hdr__ptp_header__correctionField_31_16 = 0x5555</ovrd>
                <ovrd>hdr__ptp_header__correctionField_47_32 = 0x6666</ovrd>
                <ovrd>hdr__ptp_header__correctionField_63_48 = 0x7777</ovrd>
                <ovrd>cfg__targetPortEntry__Egress_Pipe_Delay = 0xaaaaa</ovrd>
                <ovrd>txdma2ha_desc__mark_ecn = 0x0</ovrd>
                <ovrd>hdr__mac_da_47_32 = 0x2222</ovrd>
                <ovrd>hdr__mac_da_31_16 = 0x2222</ovrd>
                <ovrd>hdr__mac_da_15_0 = 0x2222</ovrd>
                <ovrd>hdr__mac_sa_47_32 = 0x3333</ovrd>
                <ovrd>hdr__mac_sa_31_16 = 0x3333</ovrd>
                <ovrd>hdr__mac_sa_15_0 = 0x3333</ovrd>
                <ovrd>hdr__vlan__TPID = 0x788</ovrd>
                <ovrd>cfg__cfgReservedSpace__reserved_0 = 0x0</ovrd>
                <ovrd>cfg__targetPortEntry__TPID = 0x888</ovrd>
                <ovrd>cfg__targetPortEntry__VlanEgrTagState = 0x0</ovrd>
                <ovrd>txdma2ha_desc__local_dev_trg_phy_port = 0x2</ovrd>
</THR2_U2E>

*/
/*****************start ********************/
/*
hdr: 0000000000000000000000000000000000000000222222222222333333333333e31d202f2d6228d30788175d
data_structure expansion_space: reserved_0(0): 0x0, reserved_1(32): 0x0, reserved_2(64): 0x0, reserved_3(96): 0x0, reserved_4(128): 0x0,
data_structure mac_header: mac_da_47_32(160): 0x2222, mac_da_31_16(176): 0x2222, mac_da_15_0(192): 0x2222, mac_sa_47_32(208): 0x3333, mac_sa_31_16(224): 0x3333, mac_sa_15_0(240): 0x3333,
data_structure etag: TPID(256): 0xe31d, E_PCP(272): 0x1, E_DEI(275): 0x0, Ingress_E_CID_base(276): 0x2f, Reserved(288): 0x0, GRP(290): 0x2, E_CID_base(292): 0xd62, Ingress_E_CID_ext(304): 0x28, E_CID_ext(312): 0xd3,
data_structure vlan: TPID(320): 0x788, up(336): 0x0, cfi(339): 0x1, vid(340): 0x75d,
*/
    GT_U32  hdrWords[]={
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x22222222,
        0x22223333,
        0x33333333,
        0xe31d202f,
        0x2d6228d3,
        0x0788175d,
        0x86dd67c4,
        0xe2a8e1c8,
        0xcf3a65ca,
        0x582c2de2,
        0x0c60dc2c,
        0x62053c62,
        0xfac599b0,
        0x274068c3,
        0xabba2d24,
        0xc1109bc4,
        0x61f1fcd8,
        0xbf4ad3e6,
        0x1502eeee,
        0x0030a5f2,   /*0x0030 replace 0xa2e8 */
        0xf2ea7777,
        0x66665555,
        0x4444b1d7,
        0xe5e94431,
        0xe11d7382,
        0x8d739cc6,
        0xced4573d,
        0xa1069d37,
        };
/*****************start ********************/
/*
cfg: 00000000000000000000000000000000349e23f608882c86a9caaaaa00000000
data_structure HA_Table_reserved_space: reserved_0(0): 0x0, reserved_1(32): 0x0, reserved_2(64): 0x0, reserved_3(96): 0x0,
data_structure srcPortEntry: PCID(128): 0x349, src_port_num(140): 0xe, reserved(144): 0x23f6,
data_structure targetPortEntry: TPID(160): 0x888, PVID(176): 0x2c8, VlanEgrTagState(188): 0x0, reserved(189): 0x6, PCID(192): 0xa9c, Egress_Pipe_Delay(204): 0xaaaaa,
data_structure cfgReservedSpace: reserved_0(224): 0x0,
*/
    GT_U32 cfgWords[] = {
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x349e23f6,
        0x08882c86,
        0xa9caaaaa,
        0x00000000};
/*****************start ********************/
/*
desc: 0000000080022c18f00000fb01c0ee2800000000000000000000000000000000
timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x1, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0,
ptp_u_field(45): 0x0, ptp_pkt_type_idx(46): 0x11, ptp_offset(51): 0x30, outer_l3_offset(58): 0x18, up0(64): 0x7, dei(67): 0x1, dp_1(68): 0x0,
dp_0(69): 0x0, packet_hash_6_5(70): 0x0, cnc_address(72): 0x0, txq_queue_id(88): 0xfb, ingress_byte_count(96): 0x70, table_hw_error_detected(110): 0x0,
header_hw_error_detected(111): 0x0, tc(112): 0x7, packet_is_cut_through(115): 0x0, cut_through_id(116): 0x714, need_crc_removal(127): 0x0,
txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0,
desc_reserved(140): 0x0, ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0,
ptp_timestamp_queue_entry_id(150): 0x0, egress_pipe_delay(160): 0x0, ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0,
fw_bc_modification(200): 0x0, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0, ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0,
udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 descWords[] = {
        0x00000000,
        0x80022C98,  /*use ptp_offset = 0x32 instead of 0x30*/
        0xf00000fb,
        0x01c0ee28,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000};
/*****************start ********************/
/*
outgoing header: 2222222222223333333333330788175d86dd67c4e2a8e1c8cf3a65ca582c2de20c60dc2c62053c62fac599b0274068c3abba2d24c1109bc461f1fcd8bf4ad3e61502eeeea2e8a5f2f2ea7777666655554444b1d7e5e94431e11d73828d739cc6ced4573da1069d37
data_structure mac_header: mac_da_47_32(224): 0x2222, mac_da_31_16(240): 0x2222, mac_da_15_0(256): 0x2222, mac_sa_47_32(272): 0x3333, mac_sa_31_16(288): 0x3333, mac_sa_15_0(304): 0x3333,
data_structure vlan: TPID(320): 0x788, up(336): 0x0, cfi(339): 0x1, vid(340): 0x75d,
*/
    GT_U32 outgoing_headerWords[] = {
        0x00000000,/*old info*/
        0x00000000,/*old info*/
        0x00000000,/*old info*/
        0x00000000,/*old info*/
        0x00000000,/*old info*/
        0x22222222,/*old info*/
        0x22223333,/*old info*/
        0x22222222,/* new start for packet */
        0x22223333,
        0x33333333,
        0x0788175d,
        0x86dd67c4,
        0xe2a8e1c8,
        0xcf3a65ca,
        0x582c2de2,
        0x0c60dc2c,
        0x62053c62,
        0xfac599b0,
        0x274068c3,
        0xabba2d24,
        0xc1109bc4,
        0x61f1fcd8,
        0xbf4ad3e6,
        0x1502eeee,
        0x0030a5f2,
        0xf2ea7777,
        0x66665555,
        0x4444b1d7,
        0xe5e94431,
        0xe11d7382,
        0x8d739cc6,
        0xced4573d,
        0xa1069d37,
        };

/*
out ppa desc: 0000000080022c18f00000fb01c0ee2800008800002aaaa848f850c500000000
timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x1, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0,
ptp_u_field(45): 0x0, ptp_pkt_type_idx(46): 0x11, ptp_offset(51): 0x30, outer_l3_offset(58): 0x18, up0(64): 0x7, dei(67): 0x1, dp_1(68): 0x0, dp_0(69): 0x0,
packet_hash_6_5(70): 0x0, cnc_address(72): 0x0, txq_queue_id(88): 0xfb, ingress_byte_count(96): 0x70, table_hw_error_detected(110): 0x0,
header_hw_error_detected(111): 0x0, tc(112): 0x7, packet_is_cut_through(115): 0x0, cut_through_id(116): 0x714, need_crc_removal(127): 0x0,
txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0, qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0,
desc_reserved(140): 0x0, ptp_dispatching_en(144): 0x1, ptp_packet_format(145): 0x0, mac_timestamping_en(148): 0x1, ptp_cf_wraparound_check_en(149): 0x0,
ptp_timestamp_queue_entry_id(150): 0x0, egress_pipe_delay(160): 0xaaaaa, ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x48,
fw_bc_modification(200): 0xf8, ptp_action(208): 0x5, ptp_egress_tai_sel(212): 0x0, ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0,
udp_checksum_offset(215): 0x62, udp_checksum_update_en(223): 0x1, reserved_0(224): 0x0,
*/
    GT_U32 outgoing_descWords[] = {
        0x00000000,
        0x80022C98,  /*use ptp_offset = 0x32 instead of 0x30*/
        0xf00000fb,
        0x01c0ee28,
        0x00008800,
        0x002aaaa8,
        0x4AF850E5, /*was 0x48f850c5, [udp_checksum_offset] I got [0x72] expected [0x62] , [timestamp_offset] I got [0x4a] expected [0x48] */
        0x00000000};


    GT_U32  pktNumWords = sizeof(hdrWords)/4;
    GT_U32  outgoing_pktNumWords = sizeof(outgoing_headerWords)/4;

    return test_firmware_case(enum_THR2_U2E,"PTP",hdrWords,pktNumWords,cfgWords,descWords,outgoing_headerWords,outgoing_pktNumWords,outgoing_descWords);

}


/* init values for testing of 'test_THR3_U2C' */
/* return is passed : GT_TRUE --> passed , GT_FALSE --> failed */
static GT_BOOL test_THR3_U2C_with_ptp(void)
{
/*****************start ********************/
/*
hdr: 0000000000000000000000000000000000000000333333333333444444444444ca9e7b055dfe7609
data_structure expansion_space: reserved_0(0): 0x0, reserved_1(32): 0x0, reserved_2(64): 0x0, reserved_3(96): 0x0, reserved_4(128): 0x0,
data_structure mac_header: mac_da_47_32(160): 0x3333, mac_da_31_16(176): 0x3333, mac_da_15_0(192): 0x3333, mac_sa_47_32(208): 0x4444, mac_sa_31_16(224): 0x4444, mac_sa_15_0(240): 0x4444,
data_structure etag: TPID(256): 0xca9e, E_PCP(272): 0x3, E_DEI(275): 0x1, Ingress_E_CID_base(276): 0xb05, Reserved(288): 0x1, GRP(290): 0x1, E_CID_base(292): 0xdfe, Ingress_E_CID_ext(304): 0x76, E_CID_ext(312): 0x9,
*/
    GT_U32  hdrWords[]={
          0x00000000,
          0x00000000,
          0x00000000,
          0x00000000,
          0x00000000,
          0x33333333,
          0x33334444,
          0x44444444,
          0xca9e7b05,
          0x5dfe7609,
          0x86dd6769,
          0xd17eb874,
          0x719ab0ec,
          0x350f4e59,
          0x50d6f8a3,
          0xde2755e0,
          0x6cebf76b,
          0xbb743d36,
          0xc16c7761,
          0xf10b4c0b,
          0xbc8d5968,
          0x58e41b13,
          0xc0aaeeee,
          0x00301789, /*0x0030 replace 0xfc59 */
          0xa2be7777,
          0x66665555,
          0x4444a7b9,
          0x2f7d702f,
          0x9ffbd4ae,
          0x5b574be5,
          0x29d26a1e,
          0x4f1273ff,
          0x3f000000,
        };
/*****************start ********************/
/*
cfg: 0000000000000000000000000000000095687d4757192536dfeaaaaa00000000
data_structure HA_Table_reserved_space: reserved_0(0): 0x0, reserved_1(32): 0x0, reserved_2(64): 0x0, reserved_3(96): 0x0,
data_structure srcPortEntry: PCID(128): 0x956, src_port_num(140): 0x8, reserved(144): 0x7d47,
data_structure targetPortEntry: TPID(160): 0x5719, PVID(176): 0x253, VlanEgrTagState(188): 0x0, reserved(189): 0x6, PCID(192): 0xdfe, Egress_Pipe_Delay(204): 0xaaaaa,
data_structure cfgReservedSpace: reserved_0(224): 0x0,
*/
    GT_U32 cfgWords[] = {
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x95687d47,
        0x57192536,
        0xdfeaaaaa,
        0x00000000};
/*****************start ********************/
/*
desc: 0000000080050c145000001b01b46d8e00040000000000000000000000000000
timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x1, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0, ptp_u_field(45): 0x1,
ptp_pkt_type_idx(46): 0x8, ptp_offset(51): 0x30, outer_l3_offset(58): 0x14, up0(64): 0x2, dei(67): 0x1, dp_1(68): 0x0, dp_0(69): 0x0, packet_hash_6_5(70): 0x0,
cnc_address(72): 0x0, txq_queue_id(88): 0x1b, ingress_byte_count(96): 0x6d, table_hw_error_detected(110): 0x0, header_hw_error_detected(111): 0x0, tc(112): 0x3,
packet_is_cut_through(115): 0x0, cut_through_id(116): 0x6c7, need_crc_removal(127): 0x0, txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0,
qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0, desc_reserved(140): 0x4, ptp_dispatching_en(144): 0x0, ptp_packet_format(145): 0x0,
 mac_timestamping_en(148): 0x0, ptp_cf_wraparound_check_en(149): 0x0, ptp_timestamp_queue_entry_id(150): 0x0, egress_pipe_delay(160): 0x0,
 ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x0, fw_bc_modification(200): 0x0, ptp_action(208): 0x0, ptp_egress_tai_sel(212): 0x0,
 ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0, udp_checksum_offset(215): 0x0, udp_checksum_update_en(223): 0x0, reserved_0(224): 0x0,
*/
    GT_U32 descWords[] = {
        0x00000000,
        0x80050C94,   /*ptp_offset = 0x32 instead of 0x30*/
        0x5000001b,
        0x01b46d8e,
        0x00040000,
        0x00000000,
        0x00000000,
        0x00000000};
/*****************start ********************/
/*
outgoing header: 333333333333444444444444ca9e7b055dfe760986dd6769d17eb874719ab0ec350f4e5950d6f8a3de2755e06cebf76bbb743d36c16c7761f10b4c0bbc8d596858e41b13c0aaeeeefc591789a2be7777666655554444a7b92f7d702f9ffbd4ae5b574be529d26a1e4f1273ff3f
data_structure mac_header: mac_da_47_32(160): 0x3333, mac_da_31_16(176): 0x3333, mac_da_15_0(192): 0x3333, mac_sa_47_32(208): 0x4444, mac_sa_31_16(224): 0x4444, mac_sa_15_0(240): 0x4444,
data_structure etag: TPID(256): 0xca9e, E_PCP(272): 0x3, E_DEI(275): 0x1, Ingress_E_CID_base(276): 0xb05, Reserved(288): 0x1, GRP(290): 0x1, E_CID_base(292): 0xdfe, Ingress_E_CID_ext(304): 0x76, E_CID_ext(312): 0x9,
*/
    GT_U32 outgoing_headerWords[] = {
          0x00000000,
          0x00000000,
          0x00000000,
          0x00000000,
          0x00000000,
          0x33333333,
          0x33334444,
          0x44444444,
          0xca9e7b05,
          0x5dfe7609,
          0x86dd6769,
          0xd17eb874,
          0x719ab0ec,
          0x350f4e59,
          0x50d6f8a3,
          0xde2755e0,
          0x6cebf76b,
          0xbb743d36,
          0xc16c7761,
          0xf10b4c0b,
          0xbc8d5968,
          0x58e41b13,
          0xc0aaeeee,
          0x00301789,
          0xa2be7777,
          0x66665555,
          0x4444a7b9,
          0x2f7d702f,
          0x9ffbd4ae,
          0x5b574be5,
          0x29d26a1e,
          0x4f1273ff,
          0x3f000000,
        };

/*
Out ppa desc: 0000000080050c145000001b01b46d8e00048800002aaaa84c0050cd00000000
timestamp_Sec(0): 0x0, timestamp_NanoSec(2): 0x0, is_ptp(32): 0x1, mark_ecn(33): 0x0, reserved_flags(34): 0x0, packet_hash_4_0(40): 0x0, ptp_u_field(45): 0x1,
ptp_pkt_type_idx(46): 0x8, ptp_offset(51): 0x30, outer_l3_offset(58): 0x14, up0(64): 0x2, dei(67): 0x1, dp_1(68): 0x0, dp_0(69): 0x0, packet_hash_6_5(70): 0x0,
cnc_address(72): 0x0, txq_queue_id(88): 0x1b, ingress_byte_count(96): 0x6d, table_hw_error_detected(110): 0x0, header_hw_error_detected(111): 0x0, tc(112): 0x3,
packet_is_cut_through(115): 0x0, cut_through_id(116): 0x6c7, need_crc_removal(127): 0x0, txdma2ha_header_ctrl_bus(128): 0x0, generate_cnc(136): 0x0,
qcn_generated_by_local_dev(137): 0x0, truncate_packet(138): 0x0, fw_drop(139): 0x0, desc_reserved(140): 0x4, ptp_dispatching_en(144): 0x1, ptp_packet_format(145): 0x0,
mac_timestamping_en(148): 0x1, ptp_cf_wraparound_check_en(149): 0x0, ptp_timestamp_queue_entry_id(150): 0x0, egress_pipe_delay(160): 0xaaaaa,
ingress_timestamp_seconds(190): 0x0, timestamp_offset(192): 0x4c, fw_bc_modification(200): 0x0, ptp_action(208): 0x5, ptp_egress_tai_sel(212): 0x0,
ptp_timestamp_queue_select(213): 0x0, add_crc(214): 0x0, udp_checksum_offset(215): 0x66, udp_checksum_update_en(223): 0x1, reserved_0(224): 0x0,
*/
    GT_U32 outgoing_descWords[] = {
        0x00000000,
        0x80050C94,   /*ptp_offset = 0x32 instead of 0x30*/
        0x5000001b,
        0x01b46d8e,
        0x00048800,
        0x002aaaa8,
        0x4E0050ED, /*was 4c0050cd, [udp_checksum_offset] I got [0x76] expected [0x66] , [timestamp_offset] I got [0x4e] expected [0x4c] */
        0x00000000};


    GT_U32  pktNumWords = sizeof(hdrWords)/4;
    GT_U32  outgoing_pktNumWords = sizeof(outgoing_headerWords)/4;

    return test_firmware_case(enum_THR3_U2C,"PTP",hdrWords,pktNumWords,cfgWords,descWords,outgoing_headerWords,outgoing_pktNumWords,outgoing_descWords);
}

static void clearFlags(void)
{
    isIpv4Valid = 0;
    isIpv6Valid = 0;
    isPtpValid = 0;
    isInHdrThr17Valid = 0;
    copyMem_nonQcn_desc__isOrigSaved = 0;
}

static void applyAllChanges(void)
{
    /* apply ALL the changes that the descriptor/header got */
    PIPE_APPLY_CHANGES(PIPE_DESC_REGs_lo,nonQcn_desc);

    /* __LOG which fields changed in the descriptor */
    pipeMemCompare_nonQcn_desc(GT_FALSE);

    if(isIpv4Valid)
    {
        /* apply the changes ipv4 header got */
        PIPE_APPLY_CHANGES(savedAddr_IPv4_Header,IPv4_Header);
    }

    if(isIpv6Valid)
    {
        /* apply the changes ipv6 header got */
        PIPE_APPLY_CHANGES(savedAddr_IPv6_Header,IPv6_Header);
    }

    if(isPtpValid)
    {
        /* apply the changes PTP header got */
        PIPE_APPLY_CHANGES(savedAddr_ptp_header,ptp_header);
    }

    if(isInHdrThr17Valid)
    {
        /* apply the changes IN header got */
        PIPE_APPLY_CHANGES(savedAddr_inhdr_thr17,thr17_u2ipl_in_hdr);
    }

}

/**
* @internal convertSimulationQcnDescToPHaDesc function
* @endinternal
*
* @brief   convert Simulation Descriptor To PHA Descriptor.
*         converted descriptor is written to PPN 'desc memory'
*/
static void convertSimulationQcnDescToPHaDesc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    #define TBD 0
    #define OUT_PARAM 0

    copyMem_Qcn_desc.reserved_0                          = 0;
    copyMem_Qcn_desc.UDP_checksum_update_En              = OUT_PARAM;
    copyMem_Qcn_desc.UDP_checksum_offset                 = OUT_PARAM;
    copyMem_Qcn_desc.add_CRC                             = TBD;
    copyMem_Qcn_desc.PTP_timestamp_Queue_select          = TBD;
    copyMem_Qcn_desc.PTP_Egress_TAI_sel                  = TBD;
    copyMem_Qcn_desc.PTP_action                          = TBD;
    copyMem_Qcn_desc.fw_bc_modification                  = 0;
    copyMem_Qcn_desc.timestamp_offset                    = OUT_PARAM;
    copyMem_Qcn_desc.ingress_timestamp_seconds           = OUT_PARAM;
    copyMem_Qcn_desc.egress_pipe_delay                   = 0;
    copyMem_Qcn_desc.PTP_timestamp_Queue_Entry_ID        = OUT_PARAM;
    copyMem_Qcn_desc.PTP_CF_wraparound_check_en          = OUT_PARAM;
    copyMem_Qcn_desc.MAC_timestamping_en                 = OUT_PARAM;
    copyMem_Qcn_desc.PTP_packet_format                   = OUT_PARAM;
    copyMem_Qcn_desc.PTP_dispatching_En                  = OUT_PARAM;
    copyMem_Qcn_desc.desc_reserved                       = 0;
    copyMem_Qcn_desc.fw_drop                             = pipe_descrPtr->pipe_egressPacketCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E ? 1 : 0;
    copyMem_Qcn_desc.Truncate_packet                     = TBD;
    copyMem_Qcn_desc.QCN_generated_by_local_Dev          = 0;
    copyMem_Qcn_desc.Generate_CNC                        = pipe_descrPtr->pipe_egressCncGenerate;
    copyMem_Qcn_desc.txdma2ha_header_ctrl_bus            = 0;
    copyMem_Qcn_desc.need_CRC_removal                    = 0;
    copyMem_Qcn_desc.cut_through_id                      = 0;
    copyMem_Qcn_desc.packet_is_cut_through               = 0;
    copyMem_Qcn_desc.TC                                  = pipe_descrPtr->pipe_TC;
    copyMem_Qcn_desc.header_HW_error_detected            = 0;
    copyMem_Qcn_desc.table_HW_error_detected             = 0;
    copyMem_Qcn_desc.ingress_byte_count                  = pipe_descrPtr->cheetah_descrPtr->byteCount;
    copyMem_Qcn_desc.txq_queue_id                        = 0;
    copyMem_Qcn_desc.CNC_address                         = pipe_descrPtr->pipe_egressCncIndex;
    copyMem_Qcn_desc.packet_hash_bits_6_5                = SMEM_U32_GET_FIELD(pipe_descrPtr->pipe_PacketHash,5,2);
    copyMem_Qcn_desc.dp                                  = SMEM_U32_GET_FIELD(pipe_descrPtr->pipe_DP,0,2);
    copyMem_Qcn_desc.dei                                 = pipe_descrPtr->pipe_DEI;
    copyMem_Qcn_desc.up0                                 = pipe_descrPtr->pipe_UP;
    copyMem_Qcn_desc.Outer_l3_offset                     = pipe_descrPtr->pipe_L3Offset;
    /* QCN specific fields */
    copyMem_Qcn_desc.congested_port_number               = pipe_descrPtr->pipe_currentEgressPort;
    copyMem_Qcn_desc.qcn_q_fb                            = TBD;
    copyMem_Qcn_desc.congested_queue_number              = TBD;
    copyMem_Qcn_desc.packet_hash_4qcn_bits_4_0           = TBD;
    copyMem_Qcn_desc.reserved_flags_4qcn                 = TBD;
    copyMem_Qcn_desc.mark_ecn_4qcn                       = TBD;
    copyMem_Qcn_desc.qcn_q_delta                         = TBD;
    copyMem_Qcn_desc.reserved                            = TBD;
    copyMem_Qcn_desc.orig_generated_QCN_prio             = TBD;
    copyMem_Qcn_desc.qcn_q_offset                        = TBD;

    isQcnDescriptor = 1;
    /********************************/
    /* download the value in 'copyMem_nonQcn_desc' into the memory behind address PIPE_DESC_REGs_lo */
    /********************************/
    PIPE_APPLY_CHANGES(PIPE_DESC_REGs_lo,Qcn_desc);
}

/**
* @internal convertSimulationDescToPHaDesc function
* @endinternal
*
* @brief   convert Simulation Descriptor To PHA Descriptor.
*         converted descriptor is written to PPN 'desc memory'
*/
static void convertSimulationDescToPHaDesc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    #define TBD 0
    #define OUT_PARAM 0

    if (pipe_descrPtr->pipe_emulated_threadId == enum_THR25_EVB_QCN)
    {
        convertSimulationQcnDescToPHaDesc(devObjPtr, pipe_descrPtr);
        return;
    }

    copyMem_nonQcn_desc.reserved_0                          = 0;
    copyMem_nonQcn_desc.udp_checksum_update_en              = OUT_PARAM;
    copyMem_nonQcn_desc.udp_checksum_offset                 = OUT_PARAM;
    copyMem_nonQcn_desc.add_crc                             = TBD;
    copyMem_nonQcn_desc.ptp_timestamp_queue_select          = TBD;
    copyMem_nonQcn_desc.ptp_egress_tai_sel                  = TBD;
    copyMem_nonQcn_desc.ptp_action                          = TBD;
    copyMem_nonQcn_desc.fw_bc_modification                  = 0;
    copyMem_nonQcn_desc.timestamp_offset                    = OUT_PARAM;
    copyMem_nonQcn_desc.ingress_timestamp_seconds           = OUT_PARAM;
    copyMem_nonQcn_desc.egress_pipe_delay                   = 0;
    copyMem_nonQcn_desc.ptp_timestamp_queue_entry_id        = OUT_PARAM;
    copyMem_nonQcn_desc.ptp_cf_wraparound_check_en          = OUT_PARAM;
    copyMem_nonQcn_desc.mac_timestamping_en                 = OUT_PARAM;
    copyMem_nonQcn_desc.ptp_packet_format                   = OUT_PARAM;
    copyMem_nonQcn_desc.ptp_dispatching_en                  = OUT_PARAM;
    copyMem_nonQcn_desc.desc_reserved                       = 0;
    copyMem_nonQcn_desc.fw_drop                             = pipe_descrPtr->pipe_egressPacketCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E ? 1 : 0;
    copyMem_nonQcn_desc.truncate_packet                     = TBD;
    copyMem_nonQcn_desc.qcn_generated_by_local_dev          = 0;
    copyMem_nonQcn_desc.generate_cnc                        = pipe_descrPtr->pipe_egressCncGenerate;
    copyMem_nonQcn_desc.txdma2ha_header_ctrl_bus            = 0;
    copyMem_nonQcn_desc.need_crc_removal                    = 0;
    copyMem_nonQcn_desc.cut_through_id                      = 0;
    copyMem_nonQcn_desc.packet_is_cut_through               = 0;
    copyMem_nonQcn_desc.tc                                  = pipe_descrPtr->pipe_TC;
    copyMem_nonQcn_desc.header_hw_error_detected            = 0;
    copyMem_nonQcn_desc.table_hw_error_detected             = 0;
    copyMem_nonQcn_desc.ingress_byte_count                  = pipe_descrPtr->cheetah_descrPtr->byteCount;
    copyMem_nonQcn_desc.txq_queue_id                        = 0;
    copyMem_nonQcn_desc.cnc_address                         = pipe_descrPtr->pipe_egressCncIndex;
    copyMem_nonQcn_desc.packet_hash_6_5                     = SMEM_U32_GET_FIELD(pipe_descrPtr->pipe_PacketHash,5,2);
    copyMem_nonQcn_desc.dp_1                                = SMEM_U32_GET_FIELD(pipe_descrPtr->pipe_DP,1,1);
    copyMem_nonQcn_desc.dp_0                                = SMEM_U32_GET_FIELD(pipe_descrPtr->pipe_DP,0,1);
    copyMem_nonQcn_desc.dei                                 = pipe_descrPtr->pipe_DEI;
    copyMem_nonQcn_desc.up0                                 = pipe_descrPtr->pipe_UP;
    copyMem_nonQcn_desc.outer_l3_offset                     = pipe_descrPtr->pipe_L3Offset;
    copyMem_nonQcn_desc.ptp_offset                          = pipe_descrPtr->pipe_PTPOffset;
    copyMem_nonQcn_desc.ptp_pkt_type_idx                    = pipe_descrPtr->pipe_PTPTypeIdx;
    copyMem_nonQcn_desc.ptp_u_field                         = TBD;
    copyMem_nonQcn_desc.packet_hash_4_0                     = SMEM_U32_GET_FIELD(pipe_descrPtr->pipe_PacketHash,0,5);
    copyMem_nonQcn_desc.reserved_flags                      = 0;
    copyMem_nonQcn_desc.mark_ecn                            = pipe_descrPtr->pipe_ECNCapable;
    copyMem_nonQcn_desc.is_ptp                              = pipe_descrPtr->pipe_IsPTP;
    copyMem_nonQcn_desc.timestamp_NanoSec                   = SMEM_U32_GET_FIELD(pipe_descrPtr->cheetah_descrPtr->packetTimestamp,0,30);
    copyMem_nonQcn_desc.timestamp_Sec                       = SMEM_U32_GET_FIELD(pipe_descrPtr->cheetah_descrPtr->packetTimestamp,30,2);

    /********************************/
    /* download the value in 'copyMem_nonQcn_desc' into the memory behind address PIPE_DESC_REGs_lo */
    /********************************/
    PIPE_APPLY_CHANGES(PIPE_DESC_REGs_lo,nonQcn_desc);
}

/**
* @internal copyPHaConfigToMemory function
* @endinternal
*
* @brief   put the 'PHA config'   into the PPN 'config memory'.
*/
static void copyPHaConfigToMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    PPA_FW_SP_WRITE(PIPE_CFG_REGs_lo + 0x00,pipe_descrPtr->pipe_haTemplate[3]);
    PPA_FW_SP_WRITE(PIPE_CFG_REGs_lo + 0x04,pipe_descrPtr->pipe_haTemplate[2]);
    PPA_FW_SP_WRITE(PIPE_CFG_REGs_lo + 0x08,pipe_descrPtr->pipe_haTemplate[1]);
    PPA_FW_SP_WRITE(PIPE_CFG_REGs_lo + 0x0c,pipe_descrPtr->pipe_haTemplate[0]);

    PPA_FW_SP_WRITE(PIPE_CFG_REGs_lo + 0x10,pipe_descrPtr->pipe_haSrcPortData);

    PPA_FW_SP_WRITE(PIPE_CFG_REGs_lo + 0x14,pipe_descrPtr->pipe_haTrgPortData[1]);
    PPA_FW_SP_WRITE(PIPE_CFG_REGs_lo + 0x18,pipe_descrPtr->pipe_haTrgPortData[0]);

    PPA_FW_SP_WRITE(PIPE_CFG_REGs_lo + 0x1c,0);
}

/**
* @internal copyPacketHeaderToMemory function
* @endinternal
*
* @brief   put first 128 bytes of ingress packet into 'header memory' (clear to ZERO first 20 bytes)
*/
static void copyPacketHeaderToMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    GT_U32  ii,iiMax;
    GT_U32  address;

    address = PIPE_PKT_REGs_lo;

    for(ii = 0 ; ii < (EXPANSION_SPACE_SIZE/4) ; ii++ , address+=4)
    {
        PPA_FW_SP_WRITE(address, 0);
    }

    iiMax = pipe_descrPtr->cheetah_descrPtr->byteCount;
    if(iiMax > 128)
    {
        iiMax = 128;
    }

    /* here address is : (PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE) */
    for(ii = 0 ; ii < iiMax ; ii++ , address++)
    {
        PPA_FW_SP_BYTE_WRITE(address,
            pipe_descrPtr->cheetah_descrPtr->startFramePtr[ii]);
    }

}

/**
* @internal ppnProcessing function
* @endinternal
*
* @brief   let the PPN firmware to process the info
*/
static void ppnProcessing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    GT_U32  threadType;

    threadType = pipe_descrPtr->pipe_emulated_threadId;

    clearFlags();

    /* run the thread to process the data , and then we can check egress packet */
    __LOG_NO_LOCATION_META_DATA(("Firmware Start : %s (for egress port[%d])\n",
        threadNames[threadType].threadDescription,
        pipe_descrPtr->pipe_currentEgressPort));

    threadNames[threadType].threadType();

    __LOG_NO_LOCATION_META_DATA(("Firmware Ended : %s (for egress port[%d])\n",
        threadNames[threadType].threadDescription,
        pipe_descrPtr->pipe_currentEgressPort));

    /* apply ALL the changes that the descriptor/header got */
    applyAllChanges();
}

/**
* @internal takeEgressPacketHeaderFromMemory function
* @endinternal
*
* @brief   get up to 148 bytes of egress packet from 'header memory'.
*/
static void takeEgressPacketHeaderFromMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    GT_U32  ii,iiMax;
    GT_U32  numBytesTakenFromIngress;
    GT_U32  address;
    GT_U32  numBytesOver128;

    address = PIPE_PKT_REGs_lo;

    numBytesTakenFromIngress = pipe_descrPtr->cheetah_descrPtr->byteCount;
    if(numBytesTakenFromIngress > 128)
    {
        numBytesTakenFromIngress = 128;
        numBytesOver128 = pipe_descrPtr->cheetah_descrPtr->byteCount - numBytesTakenFromIngress;
    }
    else
    {
        numBytesOver128 = 0;
    }

    address = PIPE_PKT_REGs_lo;

    if(copyMem_nonQcn_desc.fw_bc_modification < 0)
    {
        GT_U32  reducedNum = 0 - copyMem_nonQcn_desc.fw_bc_modification;

        /* means negative value --> so number of bytes 'reduced' */
        address += EXPANSION_SPACE_SIZE + reducedNum;

        iiMax = numBytesTakenFromIngress - reducedNum;
    }
    else
    {
        GT_U32  enlargedNum = copyMem_nonQcn_desc.fw_bc_modification;

        if(copyMem_nonQcn_desc.fw_bc_modification > EXPANSION_SPACE_SIZE)
        {
            skernelFatalError("takeEgressPacketHeaderFromMemory : can not move start of packet beyond [%d] bytes , but got [%d] \n",
                EXPANSION_SPACE_SIZE , copyMem_nonQcn_desc.fw_bc_modification);
        }

        /* means positive value --> so number of bytes 'enlarged' */
        address += EXPANSION_SPACE_SIZE  - copyMem_nonQcn_desc.fw_bc_modification;
        iiMax = numBytesTakenFromIngress + enlargedNum;
    }

    for(ii = 0 ; ii < iiMax ; ii++ , address++)
    {
        devObjPtr->egressBuffer[ii] = PPA_FW_SP_BYTE_READ(address);
    }

    /* copy the rest of the bytes */
    for(ii = 0 ; ii < numBytesOver128 ; ii++ , address++)
    {
        devObjPtr->egressBuffer[ii+iiMax] =
            pipe_descrPtr->cheetah_descrPtr->startFramePtr[ii+128];
    }

    pipe_descrPtr->cheetah_descrPtr->egressByteCount = iiMax + numBytesOver128;
}

/**
* @internal convertPHaDescToSimulationDesc function
* @endinternal
*
* @brief   convert the 'PHA descriptor' (from memory) back to fields in simulation 'pipe_descrPtr'
*/
static void convertPHaDescToSimulationDesc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(convertPHaDescToSimulationDesc);
/*  suspected fields that simulation may need to use ... but currently don't know how.

    copyMem_nonQcn_desc.udp_checksum_update_en
    copyMem_nonQcn_desc.udp_checksum_offset
    copyMem_nonQcn_desc.timestamp_offset
    copyMem_nonQcn_desc.ingress_timestamp_seconds
    copyMem_nonQcn_desc.ptp_timestamp_queue_entry_id
    copyMem_nonQcn_desc.ptp_cf_wraparound_check_en
    copyMem_nonQcn_desc.mac_timestamping_en
    copyMem_nonQcn_desc.ptp_packet_format
    copyMem_nonQcn_desc.ptp_dispatching_en
    copyMem_nonQcn_desc.truncate_packet
*/

    /* NOTE: we already used copyMem_nonQcn_desc.fw_bc_modification to build the egress packet */

    if(copyMem_nonQcn_desc.fw_drop)
    {
        __LOG(("Firmware DROP : WARNING : Packet is dropped !!! \n"));
    }

    pipe_descrPtr->pipe_egressPacketCmd   = copyMem_nonQcn_desc.fw_drop ?
                                    SKERNEL_EXT_PKT_CMD_HARD_DROP_E :
                                    SKERNEL_EXT_PKT_CMD_FORWARD_E;

    if(pipe_descrPtr->pipe_egressCncIndex != copyMem_nonQcn_desc.cnc_address)
    {
        __LOG(("Firmware modified : pipe_egressCncIndex from [%d] to [%d] \n",
            pipe_descrPtr->pipe_egressCncIndex,
            copyMem_nonQcn_desc.cnc_address));

        pipe_descrPtr->pipe_egressCncIndex    = copyMem_nonQcn_desc.cnc_address;
    }


    if(pipe_descrPtr->pipe_egressCncGenerate != copyMem_nonQcn_desc.generate_cnc)
    {
        __LOG(("Firmware modified : pipe_egressCncGenerate from [%d] to [%d] \n",
            pipe_descrPtr->pipe_egressCncGenerate,
            copyMem_nonQcn_desc.generate_cnc));

        pipe_descrPtr->pipe_egressCncGenerate = copyMem_nonQcn_desc.generate_cnc;
    }

}


/**
* @internal copySingletonMemoryToSimulationPpnMemory function
* @endinternal
*
* @brief   copy from this singleton memory manager , the memories of : descriptor,config,header
*         to the memory of the PPN in the proper device in the proper memory space
*         this to allow the CPU to be able to look at the processing that was done.
*/
static void copySingletonMemoryToSimulationPpnMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    GT_U32  srcAddress,trgAddress,memNumWords,value,ii;
    GT_U32  simulationRelativeAddress;

    simulationRelativeAddress =
        UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_PHA) +
        (PPG_OFFSET * pipe_descrPtr->pipe_ppgId) +
        (PPN_OFFSET * pipe_descrPtr->pipe_ppnId);

    /********copy config***************/
    srcAddress = PIPE_CFG_REGs_lo;
    trgAddress = srcAddress + simulationRelativeAddress;
    memNumWords = CONFIG_MEM_SIZE_IN_WORDS_CNS;

    for(ii = 0 ; ii < memNumWords; ii++ , srcAddress+=4)
    {
        value = PPA_FW_SP_READ(srcAddress);
        smemRegSet(devObjPtr, trgAddress , value);
    }
    /********copy descriptor***************/
    srcAddress = PIPE_DESC_REGs_lo;
    trgAddress = srcAddress + simulationRelativeAddress;
    memNumWords = DESC_MEM_SIZE_IN_WORDS_CNS;

    for(ii = 0 ; ii < memNumWords; ii++ , srcAddress+=4)
    {
        value = PPA_FW_SP_READ(srcAddress);
        smemRegSet(devObjPtr, trgAddress , value);
    }
    /********copy packet header***************/
    srcAddress = PIPE_PKT_REGs_lo;
    trgAddress = srcAddress + simulationRelativeAddress;
    memNumWords = (128 + EXPANSION_SPACE_SIZE) / 4;

    for(ii = 0 ; ii < memNumWords; ii++ , srcAddress+=4)
    {
        value = PPA_FW_SP_READ(srcAddress);
        smemRegSet(devObjPtr, trgAddress , value);
    }
    /***********************/

}

/* debug option to force specific firmware and PPN and PPG */
static GT_BIT             debug_force_FirmwareThread;
static ENUM_TREADS_ENT    debug_force_threadId;
static GT_U32             debug_force_PPG_id;
static GT_U32             debug_force_PPN_id;

void debug_force_FirmwareHandler
(
    IN  ENUM_TREADS_ENT threadId,
    IN  GT_U32  PPG_id,
    IN GT_U32   PPN_id
)
{
    if(threadId > enum_THR49_VariableCyclesLength)
    {
        debug_force_FirmwareThread = 0;

        return;
    }

    debug_force_FirmwareThread = 1;

    debug_force_threadId = threadId;
    debug_force_PPG_id = PPG_id;
    debug_force_PPN_id = PPN_id;

}

/**
* @internal pipe_actual_firmware_processing function
* @endinternal
*
* @brief   actual 'firmware' processing.
*/
void pipe_actual_firmware_processing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipe_actual_firmware_processing);

    /************************************************/
    /* the code is written as 'single instance' !!! */
    /* so we must lock it from other simulation tasks*/
    /************************************************/
    SCIB_SEM_TAKE;

    current_fw_devObjPtr = devObjPtr;

    if(debug_force_FirmwareThread)
    {
        __LOG(("NOTE: --- DEBUG MODE --- using specific PPG[%d] PPN[%d] thread[%d]\n",
            debug_force_PPG_id,
            debug_force_PPN_id,
            debug_force_threadId));

        pipe_descrPtr->pipe_emulated_threadId = debug_force_threadId;
        pipe_descrPtr->pipe_ppgId = debug_force_PPG_id;
        pipe_descrPtr->pipe_ppnId = debug_force_PPN_id;
    }

    __LOG(("The PPN[%d] in PPG[%d] starts processing (for egress port[%d]) \n",
        pipe_descrPtr->pipe_ppgId,
        pipe_descrPtr->pipe_ppnId,
        pipe_descrPtr->pipe_currentEgressPort));

    /********* PRE-processing *************/
    /* 1. convert simulation 'pipe_descrPtr' to 'PHA descriptor' */
    /* 2. put the 'PHA descriptor' into the PPN 'desc memory' */
    convertSimulationDescToPHaDesc(devObjPtr,pipe_descrPtr);
    /* 3. put the 'PHA config'     into the PPN 'config memory' */
    copyPHaConfigToMemory(devObjPtr,pipe_descrPtr);
    /* 4. put first 128 bytes of ingress packet into 'header memory' (clear to ZERO first 20 bytes) */
    copyPacketHeaderToMemory(devObjPtr,pipe_descrPtr);

    /********* The processing *************/
    /* 5. let the PPN firmware to process the info */
    ppnProcessing(devObjPtr,pipe_descrPtr);

    /********* POST-processing *************/
    /* 6. put the packet in the device egress buffer */
    takeEgressPacketHeaderFromMemory(devObjPtr,pipe_descrPtr);
    /* 7. convert the 'PHA descriptor' (from memory) back to fields in simulation 'pipe_descrPtr' */
    convertPHaDescToSimulationDesc(devObjPtr,pipe_descrPtr);

    /* 8. copy from this singleton memory manager , the memories of : descriptor,config,header
       to the memory of the PPN in the proper device in the proper memory space

       this to allow the CPU to be able to look at the processing that was done.
    */
    copySingletonMemoryToSimulationPpnMemory(devObjPtr,pipe_descrPtr);

    __LOG(("The PPN[%d] in PPG[%d] ended processing (for egress port[%d]) \n",
        pipe_descrPtr->pipe_ppgId,
        pipe_descrPtr->pipe_ppnId,
        pipe_descrPtr->pipe_currentEgressPort));

    SCIB_SEM_SIGNAL;

}


#define RUN_TEST(testName) \
    clearFlags();          \
    numTests++;            \
    passed_##testName = testName()

#define CHECK_PASS(testName) \
    if(passed_##testName == GT_FALSE) \
    {                                 \
        printf("%s - failed \n",#testName); \
        numFails++;                         \
    }

void simulationTestPipeCases(void)
{
    GT_U32  numFails = 0;
    GT_U32  numTests = 0;
    GT_BOOL passed_test_THR1_E2U;
    GT_BOOL passed_test_THR1_E2U_with_ECN_ipv4;
    GT_BOOL passed_test_THR1_E2U_with_ECN_ipv6;
    GT_BOOL passed_test_THR2_U2E;
    GT_BOOL passed_test_THR2_U2E_with_ptp;
    GT_BOOL passed_test_THR3_U2C_with_ptp;

    /* run the tests */
    RUN_TEST(test_THR1_E2U);
    RUN_TEST(test_THR1_E2U_with_ECN_ipv4);
    RUN_TEST(test_THR1_E2U_with_ECN_ipv6);
    RUN_TEST(test_THR2_U2E);
    RUN_TEST(test_THR2_U2E_with_ptp);
    RUN_TEST(test_THR3_U2C_with_ptp);

    printf("\n ******* summary of pass/fail ******* \n\n");

    /* check which test failed */
    CHECK_PASS(test_THR1_E2U);
    CHECK_PASS(test_THR1_E2U_with_ECN_ipv4);
    CHECK_PASS(test_THR1_E2U_with_ECN_ipv6);

    /*THR2_U2E*/
    CHECK_PASS(test_THR2_U2E);
    CHECK_PASS(test_THR2_U2E_with_ptp);

    /*THR3_U2C*/
    CHECK_PASS(test_THR3_U2C_with_ptp);

    if(numFails == 0)
    {
        printf("GOOD : All [%d] tests PASSED \n",numTests);
    }
    else
    {
        printf("ERROR : [%d] tests failed (out of [%d] tests)\n",
            numFails,numTests);
    }
}

static void pipe_phaFwApplyChangesInDescriptor(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN GT_BOOL                     reload
)
{
    if(reload == GT_TRUE)
    {
        /* we must reload the changes into the descriptor object */
        PIPE_MEM_CAST(devObjPtr->PHA_FW_support.addr_DESC_REGs_lo,nonQcn_desc);
    }
    else
    {
        /* apply ALL the changes that the descriptor/header got */
        PIPE_APPLY_CHANGES(devObjPtr->PHA_FW_support.addr_DESC_REGs_lo,nonQcn_desc);
    }
}


void simulationPipeFirmwareInit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    devObjPtr->PHA_FW_support.pha_acceleratorInfoPtr = pipe_accelInfoArr;
    devObjPtr->PHA_FW_support.pha_memoryMap        = pipe_fw_memoryMap;
    devObjPtr->PHA_FW_support.pha_activeMemoryMap  = pipe_fw_activeMemoryMap;
    devObjPtr->PHA_FW_support.addr_ACCEL_CMDS_TRIG_BASE_ADDR = ACCEL_CMDS_TRIG_BASE_ADDR;
    devObjPtr->PHA_FW_support.addr_ACCEL_CMDS_CFG_BASE_ADDR   = ACCEL_CMDS_CFG_BASE_ADDR;
    devObjPtr->PHA_FW_support.addr_PKT_REGs_lo               = PIPE_PKT_REGs_lo;
    devObjPtr->PHA_FW_support.addr_DESC_REGs_lo              = PIPE_DESC_REGs_lo;
    devObjPtr->PHA_FW_support.addr_CFG_REGs_lo               = PIPE_CFG_REGs_lo;
    devObjPtr->PHA_FW_support.phaFwApplyChangesInDescriptor  = pipe_phaFwApplyChangesInDescriptor;

    /************************************************/
    /* the code is written as 'single instance' !!! */
    /* so we must lock it from other simulation tasks*/
    /************************************************/
    SCIB_SEM_TAKE;

    current_fw_devObjPtr = devObjPtr;
    pipe_init();

    SCIB_SEM_SIGNAL;
}

