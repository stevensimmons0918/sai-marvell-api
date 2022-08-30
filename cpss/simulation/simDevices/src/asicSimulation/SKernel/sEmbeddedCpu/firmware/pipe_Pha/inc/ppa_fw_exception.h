/*
 *  ppa_fw_exception.h
 *
 *  PPA exception definitions
 *
 */

#ifndef PPA_FW_EXCEPTION_H
#define PPA_FW_EXCEPTION_H


/*************************************************************************
 * Compilation flag definition
 *************************************************************************/
#define RECOVER_FROM_EXCEPTION (0)

/*************************************************************************
 * Constants & macros definitions
 *************************************************************************/
#define NO_EXCEPTION_TILL_NOW   (0x900d900d)
#define EXCEPTION_WAS_TRIGGERED (0xbadbad)
#define EXCEPTION_STATUS     (0)
#define A0_REG               (1)
#define A1_REG               (2)
#define A2_REG               (3)
#define A3_REG               (4)
#define A4_REG               (5)
#define A5_REG               (6)
#define A6_REG               (7)
#define A7_REG               (8)
#define A8_REG               (9)
#define A9_REG               (10)
#define A10_REG              (11)
#define A11_REG              (12)
#define A12_REG              (13)
#define A13_REG              (14)
#define EPC1_REG             (15)
#define EXCCAUSE_REG         (16)
#define EXCSAVE1_REG         (17)
#define EXCVADDR_REG         (18)
#define PRID_REG             (19)
#define PS_REG               (20)
#define SAR_REG              (21)
#define XTENSA_REG_NUM       (22)
#define OFFSET(n)	         (n*4)



#endif /* PPA_FW_EXCEPTION_H */
