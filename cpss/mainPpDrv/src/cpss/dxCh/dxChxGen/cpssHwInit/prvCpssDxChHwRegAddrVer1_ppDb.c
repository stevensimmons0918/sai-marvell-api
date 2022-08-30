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
* @file prvCpssDxChHwRegAddrVer1_ppDb.c
*
* @brief This file includes definitions of DB of units base addresses.
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/*--------------------------------------------------------------*
 * Unit Data Base for various  PP                               *
 *--------------------------------------------------------------*/
static const PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChBobcat2UnitsIdUnitBaseAddrArr[] =
{
     { PRV_CPSS_DXCH_UNIT_TTI_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TTI_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_PCL_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_PCL_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_L2I_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_L2I_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_FDB_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_FDB_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_EQ_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EQ_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_LPM_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LPM_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_EFT_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_EFT_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_QAG_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_QAG_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_SHT_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_SHT_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_HA_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_HA_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_ETS_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ETS_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_MLL_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MLL_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPLR_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_EPLR_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EPLR_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_IPVX_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPVX_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_IOAM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IOAM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_EOAM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EOAM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_TCAM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCAM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_RXDMA_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_EPCL_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EPCL_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_TM_FCU_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_FCU_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TM_DROP_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_DROP_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TM_QMAP_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_QMAP_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TM_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_GOP_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_GOP_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_MIB_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MIB_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_SERDES_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SERDES_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_ERMRK_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ERMRK_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_BM_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_BM_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TM_INGRESS_GLUE_E,    PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_INGRESS_GLUE_CNS            }
    ,{ PRV_CPSS_DXCH_UNIT_TM_EGRESS_GLUE_E,     PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_EGRESS_GLUE_CNS             }
    ,{ PRV_CPSS_DXCH_UNIT_ETH_TXFIFO_E,         PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ETH_TXFIFO_CNS                 }
    ,{ PRV_CPSS_DXCH_UNIT_ILKN_TXFIFO_E,        PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ILKN_TXFIFO_CNS                }
    ,{ PRV_CPSS_DXCH_UNIT_ILKN_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ILKN_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_0_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_0_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_2_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_2_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_3_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_3_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TX_FIFO_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_BMA_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_BMA_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CNC_0_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CNC_1_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,          PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_QUEUE_CNS                  }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_LL_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_LL_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_PFC_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QCN_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_QCN_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_DQ_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_DFX_SERVER_E,         PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_DFX_SERVER_CNS                 }
    ,{ PRV_CPSS_DXCH_UNIT_MPPM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MPPM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_0_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_0_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_2_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_2_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_3_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_3_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_0_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_0_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_2_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_2_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_3_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_3_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_MPPM_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MPPM_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_CTU_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CTU_0_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_CTU_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CTU_1_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_SHT_E ,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_SHT_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_EGR0_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_EGR0_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_EGR1_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_EGR1_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DIST_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_DIST_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPLR_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXDMA_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_MG_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MG_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TCC_IPCL_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCC_PCL_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TCC_IPVX_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCC_IPVX_CNS                   }
     /* B0 subunits */
    ,{ PRV_CPSS_DXCH_UNIT_SMI_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_0_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_1_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_2_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_2_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_3_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_3_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_0_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_1_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_2_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_2_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_3_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_3_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_4_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_4_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LAST_E,               NON_VALID_ADDR_CNS                                           }
};


static const PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChBobKUnitsIdUnitBaseAddrArr[] =
{
     { PRV_CPSS_DXCH_UNIT_TTI_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TTI_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_PCL_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_PCL_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_L2I_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_L2I_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_FDB_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_FDB_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_EQ_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EQ_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_LPM_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LPM_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_EFT_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_EFT_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_QAG_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_QAG_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_SHT_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_SHT_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_HA_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_HA_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_ETS_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ETS_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_MLL_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MLL_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPLR_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_EPLR_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EPLR_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_IPVX_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPVX_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_IOAM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IOAM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_EOAM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EOAM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_TCAM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCAM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_RXDMA_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_EPCL_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EPCL_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_TM_FCU_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_FCU_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TM_DROP_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_DROP_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TM_QMAP_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_QMAP_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TM_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_GOP_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_GOP_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_MIB_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MIB_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_SERDES_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SERDES_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_ERMRK_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ERMRK_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_BM_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_BM_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TM_INGRESS_GLUE_E,    PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_INGRESS_GLUE_CNS            }
    ,{ PRV_CPSS_DXCH_UNIT_TM_EGRESS_GLUE_E,     PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TM_EGRESS_GLUE_CNS             }
    ,{ PRV_CPSS_DXCH_UNIT_ETH_TXFIFO_E,         PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ETH_TXFIFO_CNS                 }
    ,{ PRV_CPSS_DXCH_UNIT_ILKN_TXFIFO_E,        PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ILKN_TXFIFO_CNS                }
    ,{ PRV_CPSS_DXCH_UNIT_ILKN_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ILKN_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_0_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_0_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_2_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_2_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_3_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_3_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TX_FIFO_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_BMA_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_BMA_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CNC_0_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CNC_1_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,          PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_QUEUE_CNS                  }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_LL_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_LL_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_PFC_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QCN_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_QCN_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_DQ_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_DFX_SERVER_E,         PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_DFX_SERVER_CNS                 }
    ,{ PRV_CPSS_DXCH_UNIT_MPPM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MPPM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_0_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_0_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_2_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_2_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_3_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_3_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_0_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_0_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_2_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_2_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_3_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_3_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_MPPM_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MPPM_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_CTU_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CTU_0_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_CTU_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CTU_1_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_SHT_E ,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_SHT_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_EGR0_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_EGR0_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_EGR1_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_EGR1_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DIST_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_DIST_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPLR_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXDMA_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_MG_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MG_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TCC_IPCL_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCC_PCL_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TCC_IPVX_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCC_IPVX_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_0_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_1_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_2_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_2_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_3_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_3_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_0_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_1_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_2_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_2_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_3_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_3_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_4_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_4_E                        }
     /*  bobk units */
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA1_E,             PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_RXDMA1_E                          }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA1_E,             PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TXDMA1_E                          }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,           PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TX_FIFO1_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_ETH_TXFIFO1_E,        PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_ETH_TX_FIFO1_CNS                  }

    ,{ PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E,         PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_RXDMA_GLUE_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E,         PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TXDMA_GLUE_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TAI_E,                PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TAI_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_LAST_E,               NON_VALID_ADDR_CNS                                                }
};

static const PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChAldrinUnitsIdUnitBaseAddrArr[] =
{
     { PRV_CPSS_DXCH_UNIT_TTI_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TTI_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_PCL_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_PCL_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_L2I_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_L2I_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_FDB_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_FDB_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_EQ_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EQ_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_LPM_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LPM_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_EFT_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_EFT_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_QAG_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_QAG_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_SHT_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EGF_SHT_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_HA_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_HA_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_ETS_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ETS_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_MLL_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MLL_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPLR_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_EPLR_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EPLR_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_IPVX_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPVX_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_IOAM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IOAM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_EOAM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EOAM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_TCAM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCAM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_RXDMA_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_EPCL_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_EPCL_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_GOP_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_GOP_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_MIB_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MIB_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_SERDES_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SERDES_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_ERMRK_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ERMRK_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_BM_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_BM_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_ETH_TXFIFO_E,         PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_ETH_TXFIFO_CNS                 }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_0_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_0_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS0_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS0_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TX_FIFO_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_BMA_E,                PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_BMA_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CNC_0_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CNC_1_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,          PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_QUEUE_CNS                  }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_LL_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_LL_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_PFC_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QCN_E,            PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_QCN_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_DQ_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_DFX_SERVER_E,         PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_DFX_SERVER_CNS                 }
    ,{ PRV_CPSS_DXCH_UNIT_MPPM_E,               PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MPPM_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_0_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_0_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_2_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_2_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS1_3_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS1_3_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_0_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_0_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_2_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_2_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_LMS2_3_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LMS2_3_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_MPPM_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MPPM_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_CTU_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CTU_0_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_CTU_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_CTU_1_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_SHT_E ,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_SHT_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_EGR0_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_EGR0_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_EGR1_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_EGR1_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DIST_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXQ_DIST_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_1_E,             PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_IPLR_1_CNS                     }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TXDMA_CNS                      }
    ,{ PRV_CPSS_DXCH_UNIT_MG_E,                 PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_MG_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TCC_IPCL_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCC_PCL_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TCC_IPVX_E,           PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_TCC_IPVX_CNS                   }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_0_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_1_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_2_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_2_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_3_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_SMI_3_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_0_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_0_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_1_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_1_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_2_E,              PRV_CPSS_DXCH_BOBCAT2_BASE_ADDRESS_LED_2_E                        }
     /*  bobk units */
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA1_E,             PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_RXDMA1_E                          }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA1_E,             PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TXDMA1_E                          }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,           PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TX_FIFO1_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA_GLUE_E,         PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_RXDMA_GLUE_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA_GLUE_E,         PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TXDMA_GLUE_CNS                    }
    ,{ PRV_CPSS_DXCH_UNIT_TAI_E,                PRV_CPSS_DXCH_BOBK_BASE_ADDRESS_TAI_CNS                           }
     /* aldrin units */
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA2_E,             PRV_CPSS_DXCH_ALDRIN_BASE_ADDRESS_RXDMA2_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA2_E,             PRV_CPSS_DXCH_ALDRIN_BASE_ADDRESS_TXDMA2_E                        }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,           PRV_CPSS_DXCH_ALDRIN_BASE_ADDRESS_TX_FIFO2_E                      }

    ,{ PRV_CPSS_DXCH_UNIT_LAST_E,               NON_VALID_ADDR_CNS                                                }
};

static const PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChBobcat3UnitsIdUnitBaseAddrArr[] =
{
     { PRV_CPSS_DXCH_UNIT_TTI_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TTI_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_PCL_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPCL_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_L2I_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_L2I_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_FDB_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_FDB_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_EQ_E,                 PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EQ_CNS                               }
    ,{ PRV_CPSS_DXCH_UNIT_LPM_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_LPM_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_EFT_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_EFT_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_QAG_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_QAG_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_SHT_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_SHT_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_HA_E,                 PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_HA_CNS                               }
    ,{ PRV_CPSS_DXCH_UNIT_MLL_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MLL_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_EPLR_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPLR_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_IPVX_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPVX_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_IOAM_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IOAM_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_EOAM_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EOAM_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_TCAM_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TCAM_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_EPCL_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPCL_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_GOP_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_MIB_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MIB_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_SERDES_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_SERDES_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_ERMRK_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_ERMRK_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_BM_E,                 PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_BM_CNS                               }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_BMA_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_BMA_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_0_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_0_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_1_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_1_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,          PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_QUEUE_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_LL_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_LL_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CPFC_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QCN_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_QCN_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_BMX_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_BMX_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_MPPM_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MPPM_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_1_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR1_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_MG_E,                 PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MG_CNS                               }

    ,{ PRV_CPSS_DXCH_UNIT_SMI_0_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_0_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_1_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_1_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_2_E,              BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_0_CNS)     }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_3_E,              BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_1_CNS)     }
    ,{ PRV_CPSS_DXCH_UNIT_LED_0_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_0_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_1_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_1_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_2_E,              BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_0_CNS)     }
    ,{ PRV_CPSS_DXCH_UNIT_LED_3_E,              BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_1_CNS)     }

    ,{ PRV_CPSS_DXCH_UNIT_RXDMA1_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_1_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA1_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_1_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,           PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_1_CNS                        }

    ,{ PRV_CPSS_DXCH_UNIT_RXDMA2_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_2_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA3_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_3_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA4_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_4_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA5_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_5_CNS                         }

    ,{ PRV_CPSS_DXCH_UNIT_TXDMA2_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_2_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA3_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_3_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA4_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_4_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA5_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_5_CNS                         }

    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,           PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_2_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO3_E,           PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_3_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO4_E,           PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_4_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO5_E,           PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_5_CNS                        }

    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_1_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ2_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_2_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ3_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_3_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ4_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_4_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ5_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_5_CNS                         }

    ,{ PRV_CPSS_DXCH_UNIT_MIB1_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MIB1_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_GOP1_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP1_CNS                             }

    ,{ PRV_CPSS_DXCH_UNIT_TAI_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TAI_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_TAI1_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TAI1_CNS                             }

    ,{ PRV_CPSS_DXCH_UNIT_EGF_SHT_1_E,          PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_SHT_1_CNS                        }

    ,{ PRV_CPSS_DXCH_UNIT_TTI_1_E               ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TTI_1_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_IPCL_1_E              ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPCL_1_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_L2I_1_E               ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_L2I_1_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_IPVX_1_E              ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPVX_1_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_0_1_E            ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR_0_1_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_1_1_E            ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR_1_1_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_IOAM_1_E              ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IOAM_1_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_MLL_1_E               ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MLL_1_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_EQ_1_E                ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EQ_1_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_EFT_1_E           ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_EFT_1_CNS                       }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_0_1_E             ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_0_1_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_1_1_E             ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_1_1_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_SERDES_1_E            ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_SERDES_1_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_HA_1_E                ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_HA_1_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_ERMRK_1_E             ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_ERMRK_1_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_EPCL_1_E              ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPCL_1_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_EPLR_1_E              ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPLR_1_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_EOAM_1_E              ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EOAM_1_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_LPM_1_E               ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_LPM_2_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_IA_E                  ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IA_0_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_IA_1_E                ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IA_1_CNS                            }

    ,{ PRV_CPSS_DXCH_UNIT_LAST_E,               NON_VALID_ADDR_CNS                                                 }
};

/*
    NOTE: the addresses of Aldrin2 are identical to those in BC3 !
    (even addresses from 'pipe 1' of BC3 ,like RxDMA,TxDma,TxFifo,DQ of unit[3])
        so we use same 'defines'
*/
static const PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChAldrin2UnitsIdUnitBaseAddrArr[] =
{
     { PRV_CPSS_DXCH_UNIT_TTI_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TTI_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_PCL_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPCL_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_L2I_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_L2I_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_FDB_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_FDB_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_EQ_E,                 PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EQ_CNS                               }
    ,{ PRV_CPSS_DXCH_UNIT_LPM_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_LPM_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_EFT_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_EFT_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_QAG_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_QAG_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_EGF_SHT_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EGF_SHT_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_HA_E,                 PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_HA_CNS                               }
    ,{ PRV_CPSS_DXCH_UNIT_MLL_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MLL_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_EPLR_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPLR_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_IPVX_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPVX_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_IOAM_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IOAM_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_EOAM_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EOAM_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_TCAM_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TCAM_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_EPCL_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_EPCL_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_GOP_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_MIB_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MIB_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_SERDES_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_SERDES_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_ERMRK_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_ERMRK_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_BM_E,                 PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_BM_CNS                               }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_BMA_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_BMA_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_0_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_0_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_CNC_1_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CNC_1_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,          PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_QUEUE_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_LL_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_LL_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_CPFC_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QCN_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_QCN_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_BMX_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_BMX_CNS                          }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_MPPM_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MPPM_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_IPLR_1_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IPLR1_CNS                            }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_CNS                           }
    ,{ PRV_CPSS_DXCH_UNIT_MG_E,                 PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MG_CNS                               }

    ,{ PRV_CPSS_DXCH_UNIT_SMI_0_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_0_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_1_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_1_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_2_E,              BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_0_CNS)     }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_3_E,              BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_SMI_1_CNS)     }
    ,{ PRV_CPSS_DXCH_UNIT_LED_0_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_0_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_1_E,              PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_1_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_LED_2_E,              BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_0_CNS)     }
    ,{ PRV_CPSS_DXCH_UNIT_LED_3_E,              BOBCAT3_PIPE1_MAC(PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP_LED_1_CNS)     }

    ,{ PRV_CPSS_DXCH_UNIT_RXDMA1_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_1_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA1_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_1_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,           PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_1_CNS                        }

    ,{ PRV_CPSS_DXCH_UNIT_RXDMA2_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_2_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA3_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_RX_DMA_3_CNS                         }

    ,{ PRV_CPSS_DXCH_UNIT_TXDMA2_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_2_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA3_E,             PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_DMA_3_CNS                         }

    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,           PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_2_CNS                        }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO3_E,           PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TX_FIFO_3_CNS                        }

    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_1_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ2_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_2_CNS                         }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ3_E,            PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TXQ_DQ_3_CNS                         }

    ,{ PRV_CPSS_DXCH_UNIT_MIB1_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_MIB1_CNS                             }
    ,{ PRV_CPSS_DXCH_UNIT_GOP1_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_GOP1_CNS                             }

    ,{ PRV_CPSS_DXCH_UNIT_TAI_E,                PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TAI_CNS                              }
    ,{ PRV_CPSS_DXCH_UNIT_TAI1_E,               PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_TAI1_CNS                             }

    ,{ PRV_CPSS_DXCH_UNIT_SERDES_1_E            ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_SERDES_1_CNS                        }

    ,{ PRV_CPSS_DXCH_UNIT_IA_E                  ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_IA_0_CNS                            }

    ,{ PRV_CPSS_DXCH_UNIT_LPM_1_E               ,PRV_CPSS_DXCH_BOBCAT3_BASE_ADDRESS_LPM_1_CNS                           }

    ,{ PRV_CPSS_DXCH_UNIT_LAST_E,               NON_VALID_ADDR_CNS                                                 }
};


/* MACRO to fill in DB entry for specific family */
#define PRV_UNIT_ID_DB_INIT_MAC(_dbIdx, _ppFamily, _ppSubFamily, _toBaseAddrArr, _baseAddArr) \
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl)[PRV_CPSS_DXCH_UNITID_DB_FAMILY_##_dbIdx##_E].devFamily = CPSS_PP_FAMILY_DXCH_##_ppFamily##_E;      \
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl)[PRV_CPSS_DXCH_UNITID_DB_FAMILY_##_dbIdx##_E].devSubfamily = CPSS_PP_SUB_FAMILY_##_ppSubFamily##_E; \
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl)[PRV_CPSS_DXCH_UNITID_DB_FAMILY_##_dbIdx##_E].unitId2UnitBaseAdrrList = _toBaseAddrArr;             \
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl)[PRV_CPSS_DXCH_UNITID_DB_FAMILY_##_dbIdx##_E].devUnitsBaseAddr = PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(_baseAddArr);

/* MACRO to fill in DB entry for specific SIP_6 family */
#define PRV_UNIT_ID_DB_SIP_6_INIT_MAC(_dbIdx, _ppFamily, _ppSubFamily, _toBaseAddrArr, _baseAddArr) \
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl)[PRV_CPSS_DXCH_UNITID_DB_FAMILY_##_dbIdx##_E].devFamily = CPSS_PP_FAMILY_DXCH_##_ppFamily##_E;      \
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl)[PRV_CPSS_DXCH_UNITID_DB_FAMILY_##_dbIdx##_E].devSubfamily = CPSS_PP_SUB_FAMILY_##_ppSubFamily##_E; \
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl)[PRV_CPSS_DXCH_UNITID_DB_FAMILY_##_dbIdx##_E].unitId2UnitBaseAdrrList = PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(_toBaseAddrArr); \
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl)[PRV_CPSS_DXCH_UNITID_DB_FAMILY_##_dbIdx##_E].devUnitsBaseAddr = PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(_baseAddArr);

/**
* @internal prvCpssDxChUnitBaseTableInit function
* @endinternal
*
* @brief   init based address lists for all ASICs
*
* @note   APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   None.
*
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChUnitBaseTableInit
(
    IN CPSS_PP_FAMILY_TYPE_ENT devFamily
)
{
    GT_STATUS   rc;
    GT_U32 ppIdx;
    GT_U32 unitIdx;
    PRV_CPSS_DXCH_UNIT_ENT unitType;
    const PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC  * unitTypeBaseAdrrPtr;
    GT_U32                                 * currDevUnitsBaseAddr;
    GT_BOOL rebuildTable; /* rebuild unit addrees table after device specific adjusting initial data */

    if (PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(unitBaseDbInitDone) == GT_FALSE)
    {
        PRV_UNIT_ID_DB_INIT_MAC(BOBCAT2,   BOBCAT2, NONE,         prvDxChBobcat2UnitsIdUnitBaseAddrArr,  prvDxChBobcat2UnitsBaseAddr );
        PRV_UNIT_ID_DB_INIT_MAC(BOBK,      BOBCAT2, BOBCAT2_BOBK, prvDxChBobKUnitsIdUnitBaseAddrArr,     prvDxChBobkUnitsBaseAddr    );
        PRV_UNIT_ID_DB_INIT_MAC(BOBCAT3,   BOBCAT3, NONE,         prvDxChBobcat3UnitsIdUnitBaseAddrArr,  prvDxChBobcat3UnitsBaseAddr );
        PRV_UNIT_ID_DB_INIT_MAC(ALDRIN,    ALDRIN , NONE,         prvDxChAldrinUnitsIdUnitBaseAddrArr,   prvDxChAldrinUnitsBaseAddr  );
        PRV_UNIT_ID_DB_INIT_MAC(AC3X,      AC3X   , NONE,         prvDxChAldrinUnitsIdUnitBaseAddrArr,   prvDxChAldrinUnitsBaseAddr  );
        PRV_UNIT_ID_DB_INIT_MAC(ALDRIN2,   ALDRIN2, NONE,         prvDxChAldrin2UnitsIdUnitBaseAddrArr,  prvDxChAldrin2UnitsBaseAddr );
        PRV_UNIT_ID_DB_SIP_6_INIT_MAC(FALCON,    FALCON , NONE,         prvDxChFalconUnitsIdUnitBaseAddrArr,   prvDxChFalconUnitsBaseAddr  );
        PRV_UNIT_ID_DB_SIP_6_INIT_MAC(AC5P,      AC5P   , NONE,         prvDxChHawkUnitsIdUnitBaseAddrArr,     prvDxChHawkUnitsBaseAddr    );
        PRV_UNIT_ID_DB_SIP_6_INIT_MAC(AC5X,      AC5X   , NONE,         prvDxChPhoenixUnitsIdUnitBaseAddrArr,  prvDxChPhoenixUnitsBaseAddr );
        PRV_UNIT_ID_DB_SIP_6_INIT_MAC(HARRIER,   HARRIER, NONE,         prvDxChHarrierUnitsIdUnitBaseAddrArr,  prvDxChHarrierUnitsBaseAddr );
        PRV_UNIT_ID_DB_SIP_6_INIT_MAC(IRONMAN_L, IRONMAN, NONE,         prvDxChIronmanLUnitsIdUnitBaseAddrArr, prvDxChIronmanLUnitsBaseAddr);
    }

    rebuildTable = GT_FALSE;
    for (ppIdx = 0 ; ppIdx < PRV_CPSS_DXCH_UNITID_DB_FAMILY__LAST__E; ppIdx++)
    {
        if ((PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl[ppIdx].devFamily) == devFamily) &&
            (PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl[ppIdx].isUnitAddrInit) == GT_FALSE))
        {
            switch (devFamily)
            {
                case CPSS_PP_FAMILY_DXCH_FALCON_E:
                    rc = prvDxChHwRegAddrFalconDbInit();
                    rebuildTable = GT_TRUE;
                    break;
                case CPSS_PP_FAMILY_DXCH_AC5P_E:
                    rc = prvDxChHwRegAddrHawkDbInit();
                    rebuildTable = GT_TRUE;
                    break;
                case CPSS_PP_FAMILY_DXCH_AC5X_E:
                    rc = prvDxChHwRegAddrPhoenixDbInit();
                    rebuildTable = GT_TRUE;
                    break;
                case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                    rc = prvDxChHwRegAddrHarrierDbInit();
                    rebuildTable = GT_TRUE;
                    break;
                case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
                    rc = prvDxChHwRegAddrIronmanDbInit();
                    rebuildTable = GT_TRUE;
                    break;
                default:
                    rc = GT_OK;
                    break;
            }
            if (rc != GT_OK)
            {
                return rc;
            }
            PRV_SHARED_HW_INIT_REG_V1_DB_VAR_SET(prv_UnitBasedArrTbl[ppIdx].isUnitAddrInit, GT_TRUE);
            if (rebuildTable != GT_FALSE)
            {
                /* cause rebuild in loop below */
                PRV_SHARED_HW_INIT_REG_V1_DB_VAR_SET(prv_UnitBasedArrTbl[ppIdx].isInit, GT_FALSE);
            }
        }
    }

    for (ppIdx = 0 ; ppIdx < PRV_CPSS_DXCH_UNITID_DB_FAMILY__LAST__E; ppIdx++)
    {
        if (PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl[ppIdx].isInit) != GT_FALSE) continue;

        unitTypeBaseAdrrPtr       = PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl[ppIdx].unitId2UnitBaseAdrrList);
        currDevUnitsBaseAddr = PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl[ppIdx].devUnitsBaseAddr);

        /*---------------------------------------------------------------------------------*
         *init default value                                                               *
         *  all but last (0:PRV_CPSS_DXCH_UNIT_LAST_E-1) --    NON_VALID_BASE_ADDR_CNS     *
         *  last(idx = PRV_CPSS_DXCH_UNIT_LAST_E)        --    END_OF_TABLE_INDICATOR_CNS  *
         *---------------------------------------------------------------------------------*/
        for (unitIdx = 0 ; unitIdx < PRV_CPSS_DXCH_UNIT_LAST_E + 1 ; unitIdx++)
        {
            currDevUnitsBaseAddr[unitIdx] = NON_VALID_BASE_ADDR_CNS;
        }
        currDevUnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E] = END_OF_TABLE_INDICATOR_CNS;

        /*--------------------------------------------------*
         *  init spectfic values : from init list           *
         *--------------------------------------------------*/
        for (unitIdx = 0 ; unitTypeBaseAdrrPtr[unitIdx].unitId != PRV_CPSS_DXCH_UNIT_LAST_E ; unitIdx++)
        {
            unitType = unitTypeBaseAdrrPtr[unitIdx].unitId;
            currDevUnitsBaseAddr[unitType] = unitTypeBaseAdrrPtr[unitIdx].unitBaseAdrr;
        }

        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_SET(prv_UnitBasedArrTbl[ppIdx].isInit, GT_TRUE);
    }

    if (PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(unitBaseDbInitDone) == GT_FALSE)
    {
        /* bind once : HWS with function that calc base addresses of units for : Falcon,Hawk,Phoenix */
        hwsFalconUnitBaseAddrCalcBind(prvCpssSip6HwsUnitBaseAddrCalc);
        PRV_SHARED_HW_INIT_REG_V1_DB_VAR_SET(unitBaseDbInitDone, GT_TRUE);
    }

    return GT_OK;
}




/**
* @internal prvCpssDxChUnitBaseTableGet function
* @endinternal
*
* @brief   get based address lists for all ASICs
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @param[out] devUnitsBaseAddrPtrPtr   - ptr to point to table
*                                       GT_OK on success
*/
GT_STATUS prvCpssDxChUnitBaseTableGet
(
    IN  GT_U8    devNum,
    OUT GT_U32 **devUnitsBaseAddrPtrPtr
)
{
    GT_STATUS rc;
    GT_U32 ppIdx;
    CPSS_PP_FAMILY_TYPE_ENT                devFamily;
    CPSS_PP_SUB_FAMILY_TYPE_ENT            devSubFamily;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(devUnitsBaseAddrPtrPtr);

    rc = prvCpssDxChUnitBaseTableInit(PRV_CPSS_PP_MAC(devNum)->devFamily);
    if (rc != GT_OK)
    {
        return rc;
    }

    devFamily    = PRV_CPSS_PP_MAC(devNum)->devFamily;
    devSubFamily = PRV_CPSS_PP_MAC(devNum)->devSubFamily;

    for (ppIdx = 0 ; ppIdx < PRV_CPSS_DXCH_UNITID_DB_FAMILY__LAST__E; ppIdx++)
    {
        if (PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl[ppIdx].devFamily)    == devFamily &&
            PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl[ppIdx].devSubfamily) == devSubFamily)
        {
            *devUnitsBaseAddrPtrPtr = PRV_SHARED_HW_INIT_REG_V1_DB_VAR_GET(prv_UnitBasedArrTbl[ppIdx].devUnitsBaseAddr);
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
}



