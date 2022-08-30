#ifndef _PDQ_IRONMAN_REGISTERS_DESCRIPTION_H_
#define _PDQ_IRONMAN_REGISTERS_DESCRIPTION_H_

#define FOO(...) __VA_ARGS__
#define foo(x) x


#define		PDQ_SIP_6_30_PPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 8 , 0x50, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_Port_Pizza_Last_Slice(operation, ...) \
    TM_REG_ROW_##operation( pzlast_slice, 0, 9 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_APerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_APerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_APerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_CPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0x71c, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_CPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_CPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_Identity_Addr(operation, ...) \
    TM_REG_ROW_##operation( SUID, 8, 8 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( UID, 0, 8 , 0x33, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ScrubSlots_Addr(operation, ...) \
    TM_REG_ROW_##operation( PortSlots, 32, 6 , 0x4, __VA_ARGS__)\
    TM_REG_ROW_##operation( ClvlSlots, 24, 6 , 0x4, __VA_ARGS__)\
    TM_REG_ROW_##operation( BlvlSlots, 16, 6 , 0x8, __VA_ARGS__)\
    TM_REG_ROW_##operation( AlvlSlots, 8, 6 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( QueueSlots, 0, 6 , 0x20, __VA_ARGS__)\

#define		PDQ_SIP_6_30_TreeDeqEn_Addr(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 1 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PDWRREnReg(operation, ...) \
    TM_REG_ROW_##operation( PrioEn, 0, 1 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_Global_Cfg(operation, ...) \
    TM_REG_ROW_##operation( scrube_dis, 0, 1 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ErrorStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( FIFOOvrflowErr, 5, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( TBNegSat, 4, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPBSat, 3, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncECCErr, 2, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CorrECCErr, 1, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_FirstExcp_Addr(operation, ...) \
    TM_REG_ROW_##operation( FIFOOvrflowErr, 5, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( TBNegSat, 4, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPBSat, 3, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncECCErr, 2, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CorrECCErr, 1, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ErrCnt_Addr(operation, ...) \
    TM_REG_ROW_##operation( Cnt, 0, 16 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ExcpCnt_Addr(operation, ...) \
    TM_REG_ROW_##operation( Cnt, 0, 16 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ExcpMask_Addr(operation, ...) \
    TM_REG_ROW_##operation( ECOReg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( FIFOOvrflowErr, 5, 1 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( TBNegSat, 4, 1 , 0x3, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPBSat, 3, 1 , 0x2, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncECCErr, 2, 1 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( CorrECCErr, 1, 1 , 0x3, __VA_ARGS__)\
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x3, __VA_ARGS__)\

#define		PDQ_SIP_6_30_Plast_Addr(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 6 , 0x3f, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PPerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 0, 6 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_CPerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 10 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BPerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 10 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_APerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 10 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 6 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 4 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( Periods, 32, 13 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 17 , 0x1ffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 17 , 0x1ffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortDWRRPrioEn(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 8 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortQuantumsPriosLo(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x1010, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortRangeMap(operation, ...) \
    TM_REG_ROW_##operation( Hi, 24, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Lo, 0, 6 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueEligPrioFunc(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 6 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( AbvEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxDivExp, 40, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueQuantum(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x1040, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueAMap(operation, ...) \
    TM_REG_ROW_##operation( Alvl, 0, 6 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 4 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( MaxDivExp, 40, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlDWRRPrioEn(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 8 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlQuantum(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x1040, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ALvltoBlvlAndQueueRangeMap(operation, ...) \
    TM_REG_ROW_##operation( Blvl, 48, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QueueHi, 24, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( QueueLo, 0, 9 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 4 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( MaxDivExp, 40, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlDWRRPrioEn(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 8 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlQuantum(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x1040, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BLvltoClvlAndAlvlRangeMap(operation, ...) \
    TM_REG_ROW_##operation( Clvl, 48, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( AlvlHi, 24, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( AlvlLo, 0, 6 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlEligPrioFuncPtr(operation, ...) \
    TM_REG_ROW_##operation( Ptr, 0, 4 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( MaxDivExp, 40, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 28, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 16, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 12, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlTokenBucketBurstSize(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 32, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlDWRRPrioEn(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 8 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlQuantum(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x1040, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvltoPortAndBlvlRangeMap(operation, ...) \
    TM_REG_ROW_##operation( Port, 48, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BlvlHi, 24, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BlvlLo, 0, 6 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 28 , 0x7ffffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 28 , 0x7ffffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_CLevelShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BLevelShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ALevelShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 33 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_PortBPFromQMgr(operation, ...) \
    TM_REG_ROW_##operation( BP, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueL1ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueL1ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueL2ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueueL2ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_QueuePerStatus(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 10 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 27 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_ClvlL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 21 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 12 , 0xfff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlL1ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_BlvlL1ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 15 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 18 , 0x3ffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 18 , 0x3ffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 18 , 0x3ffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 18 , 0x3ffff, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlL1ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlL1ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlL2ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define		PDQ_SIP_6_30_AlvlL2ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#endif
