#include "sdy_drv.h"

SDY_INTERFACE void sdy3dListener(const SDY_LISTENER* pListener)
{
	sdyDrvSetListener(pListener);
}

SDY_INTERFACE void sdy3dSetPos(SDY_SOUND* pSound, sdyFloat x, sdyFloat y, sdyFloat z)
{
	pSound->ctrl3d.vPos.x = x;
	pSound->ctrl3d.vPos.y = y;
	pSound->ctrl3d.vPos.z = z;
	sdyDrvSet3DBuffer(pSound->pBuffer, &pSound->ctrl3d);
}

SDY_INTERFACE void sdy3dSetVel(SDY_SOUND* pSound, sdyFloat x, sdyFloat y, sdyFloat z)
{
	pSound->ctrl3d.vVel.x = x;
	pSound->ctrl3d.vVel.x = y;
	pSound->ctrl3d.vVel.x = z;
	sdyDrvSet3DBuffer(pSound->pBuffer, &pSound->ctrl3d);
}

SDY_INTERFACE void sdy3dSetDist(SDY_SOUND* pSound, sdyFloat min, sdyFloat max)
{
	pSound->ctrl3d.fMinDist = min;
	pSound->ctrl3d.fMaxDist = max;
	sdyDrvSet3DBuffer(pSound->pBuffer, &pSound->ctrl3d);
}
