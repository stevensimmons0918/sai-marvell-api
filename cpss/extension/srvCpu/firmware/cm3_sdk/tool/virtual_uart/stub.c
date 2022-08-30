
/* This file contains stubs, which enable the use of existing CPSS extDrv code for mapping PEX resources,
   without any modifications.
   In these stubs, irrelavnt code that handles DMA & interrupts - was removed.
*/

#include <gtExtDrv/drivers/gtPciDrv.h>

off_t mvDmaDrvOffset = 0;
uint64_t prvExtDrvDmaPhys64 = 0;
GT_STATUS extDrvGetDmaBase(GT_UINTPTR * dmaBase)
{
	*dmaBase = (GT_UINTPTR)0;
	return GT_OK;
}
GT_STATUS extDrvGetDmaSize(GT_U32 * dmaSize)
{
	*dmaSize = 0;
	return GT_OK;
}
GT_BOOL prvExtDrvMvIntDrvConnected(void)
{
	return GT_FALSE;
}
GT_STATUS prvExtDrvMvIntDrvEnableMsi(
    IN  GT_U32  pciBus GT_UNUSED,
    IN  GT_U32  pciDev GT_UNUSED,
    IN  GT_U32  pciFunc GT_UNUSED
)
{
	return GT_OK;
}
