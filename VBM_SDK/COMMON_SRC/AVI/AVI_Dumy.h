// ------------------------------------------------------------------
// SN93560A_Free_RTOS
// SVN $Rev: 4302 $
// SVN $Date: 2014-07-30 10:27:48 +0800 (©P¤T, 2014-07-30) $
// ------------------------------------------------------------------

//----------------------------------------------------------


#ifndef _AVI_DUMY_H_
#define _AVI_DUMY_H_

#if 1
// H264
uint32_t ulH264_GetStreamOffset(void);

// CRC
uint32_t ulCrc16_Compute(uint32_t ulDramAddr,uint32_t ulNumOfByte, uint8_t ubWaitTime);
uint16_t uwCrc16_Get(void);
#endif

#endif
