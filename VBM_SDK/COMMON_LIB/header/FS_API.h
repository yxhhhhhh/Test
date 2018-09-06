/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		FS_API.h
	\brief		File system API header file
	\author		Chinwei Hsu
	\version	1.3
	\date		2018/08/02
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _FS_API_H_
#define _FS_API_H_

#include "_510PF.h"

#define FS_MAJORVER    1		// Major version
#define FS_MINORVER    3        // Minor version

#define FS_FLD_NAME_MAX_LENGTH		8
#define FS_FILE_NAME_MAX_LENGTH		22

#define FS_FLD_MAX_NUM				100
#define FS_FILE_MAX_NUM				1000

//-----------------------------------------------------------------------------
// fs source num, dont modify
#pragma pack(push)
#pragma pack(1)
typedef enum
{
	FS_VDO_SRC_0 = 0,
	FS_VDO_SRC_1 = 1,
	FS_VDO_SRC_2 = 2,
	FS_VDO_SRC_3 = 3,
	
	FS_JPG_SRC_0 = 4,
	FS_JPG_SRC_1 = 5,
	FS_JPG_SRC_2 = 6,
	FS_JPG_SRC_3 = 7,
	
	FS_USERDEF_SRC_0 = 8
}FS_SRC_NUM;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// fs create queue call status
typedef enum
{
	Q_CREATE_FAIL = 0,					//!< queue create fail
	Q_CREATE_OK,						//!< queue create ok
}FS_Q_CREATE_STATUS;
//-----------------------------------------------------------------------------
// fs write queue call status
typedef enum
{
	Q_WRITE_FAIL = 0,			//!< queue write fail 
	Q_WRITE_OK,					//!< queue write ok 
}FS_Q_WRITE_STATUS;
//-----------------------------------------------------------------------------
// fs close queue call status
typedef enum
{
	Q_CLOSE_FAIL = 0,			//!< queue close fail
	Q_CLOSE_OK,					//!< queue close ok
}FS_Q_CLOSE_STATUS;
//-----------------------------------------------------------------------------
// fs update header queue call status
typedef enum
{
	Q_UPDATE_VDO_HEADER_FAIL = 0,			//!< queue update video header fail
	Q_UPDATE_VDO_HEADER_OK,					//!< queue update video header ok
}FS_Q_UPDATEHEADER_STATUS;
//-----------------------------------------------------------------------------
// fs update hidden file info queue call status
typedef enum
{
	Q_UPDATEHIDDEN_FAIL = 0,		//!< queue update hidden file info fail
	Q_UPDATEHIDDEN_OK,				//!< queue update hidden file info ok
}FS_Q_UPDATEHIDDEN_STATUS;
//-----------------------------------------------------------------------------
// fs thumbnail data queue call status
typedef enum
{
	Q_THUMB_DATA_READ_FAIL = 0,			//!< queue thumbnail data read fail
	Q_THUMB_DATA_READ_OK,				//!< queue thumbnail data read ok
}FS_Q_THUMB_STATUS;
//-----------------------------------------------------------------------------
// fs open file queue call status
typedef enum
{
	Q_OPEN_FAIL = 0,		//!< queue open file fail
	Q_OPEN_OK,				//!< queue open file ok
}FS_Q_OPEN_STATUS;
//-----------------------------------------------------------------------------
// fs read file queue call status
typedef enum
{
	Q_READ_FAIL = 0,		//!< queue read file fail
	Q_READ_OK,				//!< queue read file ok
}FS_Q_READ_STATUS;
//-----------------------------------------------------------------------------
// fs get info(folder/file) queue call status
typedef enum
{
	Q_GET_INFO_FAIL = 0,		//!< queue get info(folder/file) fail
	Q_GET_INFO_OK,				//!< queue get info(folder/file) ok
}FS_Q_GET_INFO_STATUS;
//-----------------------------------------------------------------------------
// fs manual delete file queue call status
typedef enum
{
	Q_DEL_FAIL = 0,			//!< queue manual delete file fail
	Q_DEL_OK,				//!< queue manual delete file ok
}FS_Q_DEL_STATUS;
//-----------------------------------------------------------------------------
// fs manual lock file queue call status
typedef enum
{
	Q_LOCK_FAIL = 0,			//!< queue manual lock file fail
	Q_LOCK_OK,					//!< queue manual lock file ok
}FS_Q_LOCK_STATUS;
//-----------------------------------------------------------------------------
// lock file switch(when recording)
typedef enum
{
	LOCK_FILE_OFF = 0,	//!< lock file off
	LOCK_FILE_ON		//!< lock file on
}FS_LOCK_FILE_SWITCH;
//-----------------------------------------------------------------------------
// fs queue status
typedef enum
{
	FS_NULL            = 0,		//!< fs queue status: initial->null
	FS_REC_CREATE_INI  = 1,		//!< fs queue status: create file initial
	FS_REC_CREATE_OK   = 2,		//!< fs queue status: create file ok
	FS_REC_CLOSED_INI  = 3,		//!< fs queue status: close file initial
	FS_REC_CLOSED_OK   = 4,		//!< fs queue status: close file ok
	FS_PLY_OPEN_INI    = 5,		//!< fs queue status: open file initial
	FS_PLY_OPEN_OK     = 6,		//!< fs queue status: open file ok
	FS_PLY_READ_INI    = 7,		//!< fs queue status: read file initial
	FS_PLY_READ_OK     = 8,		//!< fs queue status: read file ok
	FS_THUMB_READ_INI  = 9,		//!< fs queue status: thumbnail read initial
	FS_THUMB_READ_OK   = 10,	//!< fs queue status: thumbnail read ok
	FS_GET_INFO_INI    = 11,	//!< fs queue status: get info(folder/file) initial
	FS_GET_INFO_OK     = 12,	//!< fs queue status: get info(folder/file) ok
	FS_MANUAL_DEL_INI  = 13,	//!< fs queue status: manual delete file initial
	FS_MANUAL_DEL_OK   = 14,	//!< fs queue status: manual delete file ok
	FS_LOOP_DEL_INI    = 15,	//!< fs queue status: loop delete file initial
	FS_LOOP_DEL_OK     = 16,	//!< fs queue status: loop delete file ok
	FS_MANUAL_LOCK_INI = 17,	//!< fs queue status: manual lock file initial
	FS_MANUAL_LOCK_OK  = 18,	//!< fs queue status: manual lock file ok
	FS_UPDATE_HIDDEN_INFO_INI = 19,	//!< fs queue status: update hidden info initial
	FS_UPDATE_HIDDEN_INFO_OK  = 20	//!< fs queue status: update hidden info ok
}FS_Q_STATUS;
//-----------------------------------------------------------------------------
// resolution access mode
typedef enum
{
	FS_RES_MODE_FHDx1,	//!< resolution mode: 1T FHD
	FS_RES_MODE_HDx1,	//!< resolution mode: 1T HD
	FS_RES_MODE_VGAx1,	//!< resolution mode: 1T VGA
	FS_RES_MODE_HDx2,	//!< resolution mode: 2T HD
	FS_RES_MODE_HDx4,	//!< resolution mode: 4T HD
	FS_RES_MODE_VGAx4,	//!< resolution mode: 4T VGA
}FS_RESOLUTION_MODE;
//-----------------------------------------------------------------------------
// sd card format status
typedef enum
{
	FORMAT_FAIL = 0,	//!< sd card format fail
	FORMAT_OK			//!< sd card format ok
}FS_FMT_STATUS;
//-----------------------------------------------------------------------------
// sd card in/out state
typedef enum 
{
	FS_SD_CARD_OUT = 0,	//!< sd card out
	FS_SD_CARD_IN		//!< sd card in
}FS_SD_CARD_INOUT_STATUS;
//------------------------------------------------------------------------------
// sd card ready access status
typedef enum
{
	FS_SD_NOT_RDY = 0,	//!< sd card not ready to access
	FS_SD_GET_INFO_FAIL,//!< sd card info fail -> need format
	FS_SD_RDY			//!< sd card ready to access
}FS_SD_RDY_ACC;
//-----------------------------------------------------------------------------
// file attribute
typedef enum
{
	FILE_ATTR_READ_ONLY = 0x01,		//!< file is read-only
	FILE_ATTR_HIDDEN    = 0x02,		//!< file is hidden
	FILE_ATTR_ARCHIVE	= 0x20,		//!< file is archive
}FS_FILE_ATTR;
//-----------------------------------------------------------------------------
// file create path
typedef enum
{
	FILE_PATH1 = 0,		//!< user define file path1 -> ex: E:\DCIM\USERDEF1
	FILE_PATH2,			//!< user define file path2 -> ex: E:\USERDEF2
	FILE_PATH3,			//!< user define file path3 -> ex: E:
	FILE_PATH_DEFAULT	//!< default record path -> ex: E:\DCIM\100SONIX
}FS_FILE_PATH;
//-----------------------------------------------------------------------------
// manual lock file mode
typedef enum
{
	FILE_UNLOCK = 0x00,		//!< manual unlock file
	FILE_LOCK   = 0x01		//!< manual lock file
}FS_FILE_LOCK_MODE;
//-----------------------------------------------------------------------------
// manual lock file mode
typedef enum
{
	FS_SD_CARD_FAIL,		//!< sd card test fail
	FS_SD_CARD_SLOW,		//!< sd card is too slow
	FS_SD_CARD_OK			//!< sd card is ok
}FS_SD_CARD_SPD_RPT;
//-----------------------------------------------------------------------------
// for kernal call -> create process information
#pragma pack(push)
#pragma pack(1)
typedef struct FS_KNL_CREATE_PROCESS
{
	FS_SRC_NUM SrcNum;		//!< source number
	
	uint8_t ubRecGroupFileNum;			//!< how many files in this record group
	
	char chFileName[FS_FILE_NAME_MAX_LENGTH+1];	//!< file name
	uint8_t ubFileNameLen;	//!< file name length
	char chFileExtName[3+1];	//!< file extension name
	FS_FILE_ATTR FileAttr;	//!< file attribute
	FS_FILE_PATH FilePath;	//!< file store path
	
	uint16_t uwGroupIdx;	//!< group index
}FS_KNL_CRE_PROCESS_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// for kernal call -> hidden info process information
#pragma pack(push)
#pragma pack(1)
typedef struct FS_KNL_HIDDEN_PROCESS
{
	FS_SRC_NUM SrcNum;	//!< source number	
	
	char chFileName[FS_FILE_NAME_MAX_LENGTH+1];	//!< file name
	uint8_t ubFileNameLen;	//!< file name length
	char chFileExtName[3+1];	//!< file extension name
	
	uint16_t uwDateInfo;	//!< date information
	uint16_t uwTimeInfo;	//!< time information
	uint8_t ubSecondOfs;	//!< second offset(0 or 1)
	
	uint64_t ullFileSize;	//!< file size
	
	uint8_t ubEvent;		//!< event number
	uint16_t uwGroupIdx;	//!< group index
	uint8_t ubPreviewMode;	//!< preview mode
	
	uint16_t uwMovieLen;	//!< movie length
	
	uint32_t ulThumbnailSrcDramAddr;	//!< thumbnail source dram address
	uint32_t ulThumbnailSize;			//!< thumbnail data size
}FS_KNL_HIDDEN_PROCESS_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// for kernal call -> open process information
#pragma pack(push)
#pragma pack(1)
typedef struct FS_KNL_OPEN_PROCESS
{
	char chFolderName[FS_FLD_NAME_MAX_LENGTH+1];	//!< folder name
	uint8_t ubFolderNameLen;	//!< folder name length
	
	char chFileName[FS_FILE_NAME_MAX_LENGTH+1];	//!< file name
	uint8_t ubFileNameLen;	//!< file name length	
	char chFileExtName[3+1];	//!< file extension name
	
	FS_SRC_NUM SrcNum;	//!< source number
	
	FS_FILE_PATH FilePath;	//!< file open path
}FS_KNL_OPEN_PROCESS_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// file hidden information
#pragma pack(push)
#pragma pack(1)
typedef struct FS_FILE_HIDDEN_INFO
{
	uint8_t ubDummyData[16];		//!< for avi/mp4/jpg compatibility
	
	char chFolderName[FS_FLD_NAME_MAX_LENGTH+1];	//!< folder name
	uint8_t ubFolderNameLen;	//!< folder name length
	
	char chFileName[FS_FILE_NAME_MAX_LENGTH+1];	//!< file name
	uint8_t ubFileNameLen;	//!< file name length
	char chFileExtName[3+1];	//!< file name length
	
	FS_SRC_NUM SrcNum;	//!< sorce number
	
	uint16_t uwCreateYear;	//!< create year
	uint8_t ubCreateMonth;	//!< create month
	uint8_t ubCreateDay;	//!< create day	
	uint8_t uwCreateHour;	//!< create hour
	uint8_t ubCreateMin;	//!< create minute
	uint8_t ubCreateSec;	//!< create second	
	uint8_t ubSecondOfs;	//!< second offset(0 or 1)
	
	uint64_t ullFileSize;	//!< file size
	
	uint8_t ubEvent;		//!< event number
	uint16_t uwGroupIdx;	//!< group index
	uint8_t ubPreviewMode;	//!< preview mode
	
	uint16_t uwMovieLen;	//!< movie length
	
	uint8_t ubFileLockBit;	//!< file lock bit; 1:lock, 0:unlock
}FS_FILE_HIDDEN_INFO_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// file information
#pragma pack(push)
#pragma pack(1)
typedef struct FS_FILEINFO
{
	char chFileName[FS_FILE_NAME_MAX_LENGTH+1];	//!< file name
	uint8_t ubFileNameLen;	//!< file name length
	char chFileExtName[3+1];	//!< file name length
	
	uint8_t ubFileAttr;
	
	uint16_t uwCreateYear;	//!< create year
	uint8_t ubCreateMonth;	//!< create month
	uint8_t ubCreateDay;	//!< create day
	
	uint8_t uwCreateHour;	//!< create hour
	uint8_t ubCreateMin;	//!< create minute
	uint8_t ubCreateSec;	//!< create second
	
	uint32_t ulFirstClus;	//!< create second
	
	uint64_t ullFileSize;	//!< file size
}FS_FILE_INFO_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// folder information
#pragma pack(push)
#pragma pack(1)
typedef struct FS_FLD_INFO
{
	char chFolderName[FS_FLD_NAME_MAX_LENGTH+1];	//!< folder name
	uint8_t ubFolderNameLen;	//!< folder name length
	
	uint8_t ubFileAttr;
	
	uint16_t uwCreateYear;	//!< create year
	uint8_t ubCreateMonth;	//!< create month
	uint8_t ubCreateDay;	//!< create day
	
	uint8_t uwCreateHour;	//!< create hour
	uint8_t ubCreateMin;	//!< create minute
	uint8_t ubCreateSec;	//!< create second
	
	uint32_t ulFirstClus;	//!< create second
}FS_FLD_INFO_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// for kernal call -> manual delete file process information
#pragma pack(push)
#pragma pack(1)
typedef struct FS_KNL_MANUAL_DEL_PROCESS
{	
	char chFolderName[FS_FLD_NAME_MAX_LENGTH+1];	//!< folder name
	uint8_t ubFolderNameLen;					//!< folder name length
	
	char chFileName[FS_FILE_NAME_MAX_LENGTH+1];	//!< file name
	uint8_t ubFileNameLen;						//!< file name length
	char chFileExtName[3+1];						//!< file extension name
	
	FS_FILE_PATH FilePath;							//!< file delete path
}FS_KNL_MANUAL_DEL_PROCESS_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// for kernal call -> manual lock file process information
#pragma pack(push)
#pragma pack(1)
typedef struct FS_KNL_MANUAL_LOCK_PROCESS
{
	char chFolderName[FS_FLD_NAME_MAX_LENGTH+1];	//!< folder name
	uint8_t ubFolderNameLen;					//!< folder name length
	
	char chFileName[FS_FILE_NAME_MAX_LENGTH+1];	//!< file name
	uint8_t ubFileNameLen;						//!< file name length
	char chFileExtName[3+1];						//!< file extension name
	
	FS_FILE_LOCK_MODE FileLockMode;				//!< file lock mode
}FS_KNL_MANUAL_LOCK_PROCESS_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// for kernal call -> create time info
#pragma pack(push)
#pragma pack(1)
typedef struct FS_CREATE_TIME_INFO
{
	uint16_t uwDateInfo;	//!< date information
	uint16_t uwTimeInfo;	//!< time information
	uint8_t ubSecondOfs;	//!< second offset(0 or 1)
}FS_CRE_TIME_INFO_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
// FS parameter for kernal setting
#pragma pack(push)
#pragma pack(1)
typedef struct KNL_FS_PARAMETER
{
	uint32_t ulFS_BufStartAddr;
	FS_RESOLUTION_MODE Mode;
}FS_KNL_PARA_t;
#pragma pack(pop)
//------------------------------------------------------------------------------
// Sorting related
#pragma pack(push)
#pragma pack(1)
typedef enum
{	
	SORT_BY_NAME_DESCENDING,
    SORT_BY_NAME_ASCENDING,
	SORT_BY_TIME_DESCENDING,
    SORT_BY_TIME_ASCENDING,
    SORT_BY_FILE_SIZE_DESCENDING,
    SORT_BY_FILE_SIZE_ASCENDING,
	SORT_INVALILD = 0xFF
}FS_SortMode_t;
#pragma pack(pop)
#pragma pack(push)
#pragma pack(1)
typedef struct
{
	char cFolderName[15];		//!< folder name
	uint8_t ubFolderNameLen;	//!< folder name length
}FS_FoldersInfo_t;
#pragma pack(pop)
#pragma pack(push)
#pragma pack(1)
typedef struct
{
	char cFileName[FS_FILE_NAME_MAX_LENGTH];
	uint8_t ubFileNameLen;		//!< file name length
    char cFileExtName[3];
	uint16_t uwCreateYear;		//!< create year
	uint8_t ubCreateMonth;		//!< create month
	uint8_t ubCreateDay;		//!< create day	
	uint8_t uwCreateHour;		//!< create hour
	uint8_t ubCreateMin;		//!< create minute
	uint8_t ubCreateSec;		//!< create second	
	uint32_t ullFileSize;
    uint16_t uwGroupIndex;
	FS_SRC_NUM SrcNum;
}FS_FilesInfo_t;
#pragma pack(pop)
//------------------------------------------------------------------------------

//==============================================================================
// FS External API
//==============================================================================
/*!
\brief FS version
\return ((FS_MAJORVER << 8) + FS_MINORVER)
*/
uint16_t uwFS_GetVersion(void);
//-----------------------------------------------------------------------------
/*!
\brief create file
\param KnlCreProc 	parameter of create information
\return FS_Q_CREATE_STATUS
*/
FS_Q_CREATE_STATUS FS_CreateFile(FS_KNL_CRE_PROCESS_t KnlCreProc);
//-----------------------------------------------------------------------------
/*!
\brief write file
\param SrcNum 			source number
\param ulSrcDramAddr 	source dram address
\param ulSize 			data size
\return FS_Q_WRITE_STATUS
*/
FS_Q_WRITE_STATUS FS_WriteFile(FS_SRC_NUM SrcNum, uint32_t ulSrcDramAddr, uint32_t ulSize);
//-----------------------------------------------------------------------------
/*!
\brief close file
\param SrcNum		source number
\return FS_Q_CLOSE_STATUS
*/
FS_Q_CLOSE_STATUS FS_CloseFile(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief update avi part-1 header(avi size and hdrl header); data range:0~499bytes
\param SrcNum 			source number
\param ulSrcDramAddr 	source dram address
\param ulSize 			data size
\return FS_Q_UPDATEHEADER_STATUS
*/
FS_Q_UPDATEHEADER_STATUS FS_UpdateAviHeader1(FS_SRC_NUM SrcNum, uint32_t ulSrcDramAddr, uint32_t ulSize);
//-----------------------------------------------------------------------------
/*!
\brief update avi part-2 header(movi size); data range:66548(avi header1 size+hidden info size+thumbnail size)~66559bytes
\param SrcNum 			source number
\param ulSrcDramAddr 	source dram address
\param ulSize 			data size
\return FS_Q_UPDATEHEADER_STATUS
*/
FS_Q_UPDATEHEADER_STATUS FS_UpdateAviHeader2(FS_SRC_NUM SrcNum, uint32_t ulSrcDramAddr, uint32_t ulSize);
//-----------------------------------------------------------------------------
/*!
\brief update mp4 header
\param SrcNum 			source number
\param ulSrcDramAddr	source dram address
\param ulSize			data size
\return FS_Q_UPDATEHEADER_STATUS
*/
FS_Q_UPDATEHEADER_STATUS FS_UpdateMp4Header(FS_SRC_NUM SrcNum, uint32_t ulSrcDramAddr, uint32_t ulSize);
//-----------------------------------------------------------------------------
/*!
\brief update hidden information
\param KnlHidProc		parameter of update information
\return FS_Q_UPDATEHIDDEN_STATUS
*/
FS_Q_UPDATEHIDDEN_STATUS FS_UpdateHiddenInfo(FS_KNL_HIDDEN_PROCESS_t KnlHidProc);
//-----------------------------------------------------------------------------
/*!
\brief read thumbnail data
\param ulDestDramAddr		destination dram address
\param chFldName 			folder name
\param ubFldNameLen			folder name length
\param ulOffset 			thumbnail offset: 0,1,2,...
\param ulNum 				read number
\return FS_Q_THUMB_STATUS
*/
FS_Q_THUMB_STATUS FS_ThumbDataRead(uint32_t ulDestDramAddr, char *chFldName, uint8_t ubFldNameLen, uint32_t ulOffset, uint32_t ulNum);
//-----------------------------------------------------------------------------
/*!
\brief open file
\param KnlOpenProc		parameter of open information
\return FS_Q_OPEN_STATUS
*/
FS_Q_OPEN_STATUS FS_OpenFile(FS_KNL_OPEN_PROCESS_t KnlOpenProc);
//-----------------------------------------------------------------------------
/*!
\brief read file data
\param ulDestDramAddr		destination dram address
\param SrcNum 				source number
\param ullReadAddr			read data address->the actual data offset
\param ulSize 				read size(the maximum limit is 16MB)
\return FS_Q_READ_STATUS
*/
FS_Q_READ_STATUS FS_ReadFile(uint32_t ulDestDramAddr, FS_SRC_NUM SrcNum, uint64_t ullReadAddr, uint32_t ulSize);
//-----------------------------------------------------------------------------
/*!
\brief get file hidden information
\param OutputFileInfo		output file information
\param OutputValidNum		output valid number
\param chFldName				folder name
\param ubFldNameLen			folder name length
\param ulOffset					read file offset: 0,1,2,...
\param ulNum						read file number
\return FS_Q_GET_INFO_STATUS
*/
FS_Q_GET_INFO_STATUS FS_GetFileHiddenInfo(FS_FILE_HIDDEN_INFO_t *OutputFileInfo, uint32_t *OutputValidNum, char *chFldName, uint8_t ubFldNameLen, uint32_t ulOffset, uint32_t ulNum);
//-----------------------------------------------------------------------------
/*!
\brief get file information
\param OutputFileInfo		output file information
\param OutputValidNum		output valid number
\param chFldName			folder name
\param ubFldNameLen			folder name length
\param ulOffset				read file offset: 0,1,2,...
\param ulNum				read file number
\return FS_Q_GET_INFO_STATUS
*/
FS_Q_GET_INFO_STATUS FS_GetFileInfo(FS_FILE_INFO_t *OutputFileInfo, uint32_t *OutputValidNum, char *chFldName, uint8_t ubFldNameLen, uint32_t ulOffset, uint32_t ulNum);
//-----------------------------------------------------------------------------
/*!
\brief get folder information
\param OutputFldInfo		output folder information
\param OutputValidNum		valid output number
\param ulOffset				read folder offset: 0,1,2,...
\param ulNum				read folder number
\return FS_Q_GET_INFO_STATUS
*/
FS_Q_GET_INFO_STATUS FS_GetFldInfo(FS_FLD_INFO_t *OutputFldInfo, uint32_t *OutputValidNum, uint32_t ulOffset, uint32_t ulNum);
//-----------------------------------------------------------------------------
/*!
\brief manual delete file
\param KnlDelProc		parameter of delete information
\return FS_Q_DEL_STATUS
*/
FS_Q_DEL_STATUS FS_ManualDeleteFile(FS_KNL_MANUAL_DEL_PROCESS_t KnlDelProc);
//-----------------------------------------------------------------------------
/*!
\brief manual lock file
\param KnlLockProc		parameter of lock information
\return FS_Q_LOCK_STATUS
*/
FS_Q_LOCK_STATUS FS_ManualLockFile(FS_KNL_MANUAL_LOCK_PROCESS_t KnlLockProc);
//-----------------------------------------------------------------------------
/*!
\brief lock file(when recording)
\param LockSwitch		lock switch:LOCK_FILE_OFF/LOCK_FILE_ON
\return(no)
*/
void FS_LockFileForRecording(FS_LOCK_FILE_SWITCH LockSwitch);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer create file process
\param SrcNum		source number
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkCreateStatus(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer close file process
\param SrcNum		source number
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkCloseStatus(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer open file process
\param SrcNum		source number
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkOpenStatus(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer read file process
\param SrcNum		source number
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkReadStatus(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer read thumbnail data process
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkThumbDataReadStatus(void);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer get (normal/hidden) file information process
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkGetFileInfoStatus(void);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer get (normal/hidden) folder information process
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkGetFldInfoStatus(void);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer delete file process
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkManualDelStatus(void);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer looping delete process
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkLoopDelStatus(void);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer manual lock file process
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkManualLockStatus(void);
//-----------------------------------------------------------------------------
/*!
\brief get status of lower layer update hidden information process
\param SrcNum		source number
\return FS_Q_STATUS
*/
FS_Q_STATUS FS_ChkUpdateHiddenInfoStatus(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief file system initial function
\param FS_KNL_PARA_t
\return(no)
*/
void FS_Init(FS_KNL_PARA_t *FsPara);
//-----------------------------------------------------------------------------
/*!
\brief file system uninitial function
\return(no)
*/
void FS_UnInit(void);
//-----------------------------------------------------------------------------
/*!
\brief get file system total buffer size
\param FS_RESOLUTION_MODE
\return total buffer size(unit:bytes)
*/
uint32_t ulFS_GetTotalBufSize(FS_RESOLUTION_MODE Mode);
//-----------------------------------------------------------------------------
/*!
\brief fat32 format
\return FS_Q_DEL_STATUS
*/
FS_FMT_STATUS FS_Fat32Format(void);
//-----------------------------------------------------------------------------
/*!
\brief exfat format
\return FS_Q_DEL_STATUS
*/
FS_FMT_STATUS FS_ExFatFormat(void);
//-----------------------------------------------------------------------------
/*!
\brief fat format(fat32 for 8~32GB and exfat for 64GB~128GB)
\return FS_Q_DEL_STATUS
*/
FS_FMT_STATUS FS_FatFormat(void);
//-----------------------------------------------------------------------------
/*!
\brief get create file time of a source number(for hidden info)
\param SrcNum		source number
\return FS_CRE_TIME_INFO_t: time information
*/
FS_CRE_TIME_INFO_t FS_GetCreateTimeInfo(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief get close file size(for hidden info)
\param SrcNum		source number
\return file size(unit:bytes)
*/
uint64_t ullFS_GetCloseFileSize(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief check sd card exist
\return SD_CARD_INOUT_STATE
*/
FS_SD_CARD_INOUT_STATUS FS_ChkSdExist(void);
//-----------------------------------------------------------------------------
/*!
\brief check sd card ready access status
\return FS_SD_RDY_ACC
*/
FS_SD_RDY_ACC FS_ChkSdRdy(void);
//-----------------------------------------------------------------------------
/*!
\brief get sd card free space
\return free space; unit:Mbytes
*/
uint32_t ulFS_GetFreeSpace(void);
//-----------------------------------------------------------------------------
/*!
\brief get sd card total space
\return total space; unit:Mbytes
*/
uint32_t ulFS_GetTotalSpace(void);
//-----------------------------------------------------------------------------
/*!
\brief turn on the looping
\param ulLoopingFreeSpaceTh		uint:MB; loop delete file will enable when free space is smaller than this value
\return(no)
*/
void FS_LoopingOn(uint32_t ulFreeSpaceTh);
//-----------------------------------------------------------------------------
/*!
\brief turn off the looping
\return(no)
*/
void FS_LoopingOff(void);
//-----------------------------------------------------------------------------
/*!
\brief get latest file group index
\param SrcNum				source number
\return group index
*/
uint16_t uwFS_GetLatestGroupIdx(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief set max rolling value of group index, max value is 65536
\param SrcNum				source number
\return group index
*/
void FS_SetMaxRollingValOfGrpIdx(uint32_t ulMaxNum);
//-----------------------------------------------------------------------------
/*!
\brief get latest file name
\param Output		output file name
\return(no)
*/
void FS_GetLatestFileName(char *Output);
//-----------------------------------------------------------------------------
/*!
\brief tempory file name handle
\param Output			output file name
\param Input			input file name
\param ulNameLen		file name length
\return(no)
*/
void FS_FileNameHandle(char *Output, char *Input, uint32_t ulNameLen);
//-----------------------------------------------------------------------------
/*!
\brief get the open file size
\param SrcNum		source number
\return file size
*/
uint64_t ullFS_GetOpenFileSize(FS_SRC_NUM SrcNum);
//-----------------------------------------------------------------------------
/*!
\brief sd card speed test
\param OutputRdThrput		read output throughput, unit:KBytes/s
\param OutputWrtThrput		write output throughput, unit:KBytes/s
\return FS_SD_CARD_SPD_RPT
*/
FS_SD_CARD_SPD_RPT FS_SdCardRWTest(uint32_t *OutputRdThrput, uint32_t *OutputWrtThrput);
//-----------------------------------------------------------------------------
/*!
\brief Sorting folder infomation
\param pFoldersInfo	  Folder information
\return Folder Number
*/
uint32_t ulFS_GetSortingFolders(FS_FoldersInfo_t *pFoldersInfo);
//-----------------------------------------------------------------------------
/*!
\brief Sorting files infomation
\param uwFolderIndex  Folder index
\param tSortMode	  Soring mode
\param pFilesInfo	  Files information
\return File Number
*/
uint16_t uwFS_GetSortingFiles(uint16_t uwFolderIndex, FS_SortMode_t tSortMode, FS_FilesInfo_t *pFilesInfo);
//-----------------------------------------------------------------------------
/*!
\brief Reset sorting result
\return no
*/
void FS_ResetSortingResult(void);
//-----------------------------------------------------------------------------
#endif
