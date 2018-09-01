:begin
rem if %1==SN93700_VBM_PU_HS_A7130 goto sn93700_pu_hs
rem if %1==SN93700_VBM_PU_HS_A7130 goto sn93700_bu_hs
rem if %1==SN93700_VBM_PU_LP goto sn93700_pu_lp
rem if %1==SN93700_VBM_BU_LP goto sn93700_bu_lp
goto resume

rem -----------------------------------------------------------------------------------------
rem                             		SN93700_PU_HS target
rem -----------------------------------------------------------------------------------------
:sn93700_pu_hs
IF EXIST .\BIN\SN93700_VBM_PU_LP.hex (
copy .\BIN\SN93700_VBM_PU_LP_Exec.hex .\BIN\SN93700temp.hex
)
goto resume

rem -----------------------------------------------------------------------------------------
rem                             		SN93700_BU_HS target
rem -----------------------------------------------------------------------------------------
:sn93700_bu_hs
IF EXIST .\BIN\SN93700_VBM_BU_LP.hex (
copy .\BIN\SN93700_VBM_BU_LP_Exec.hex .\BIN\SN93700temp.hex
)
goto resume

rem -----------------------------------------------------------------------------------------
rem                             		SN93700_PU_LP target
rem -----------------------------------------------------------------------------------------
:sn93700_pu_lp
IF EXIST .\BIN\SN93700_VBM_PU_HS.hex (
copy .\BIN\SN93700_VBM_PU_HS_Exec.hex .\BIN\SN93700temp.hex
)
goto resume

rem -----------------------------------------------------------------------------------------
rem                             		SN93700_BU_LP target
rem -----------------------------------------------------------------------------------------
:sn93700_bu_lp
IF EXIST .\BIN\SN93700_VBM_BU_HS.hex (
copy .\BIN\SN93700_VBM_BU_HS_Exec.hex .\BIN\SN93700temp.hex
)
goto resume

rem -----------------------------------------------------------------------------------------
rem                         Generate bin/hex for current target
rem -----------------------------------------------------------------------------------------
:resume

.\UTILITY\binConverter -swp .\BIN\%1 39920
.\UTILITY\binModify .\BIN\ISPMemDrive 39920 %errorlevel%
.\UTILITY\binConverter -sds .\BIN\%1 39928
.\UTILITY\binModify .\BIN\ISPMemDrive 39928 %errorlevel%
.\UTILITY\binConverter -re .\BIN\%1 %2
.\UTILITY\binModify .\CONFIG\Header %5 %errorlevel%
.\UTILITY\binModify .\CONFIG\Header %4 %3h
.\UTILITY\binVerify ..\..\ ..\COMMON_SRC\CMSIS\ .\CONFIG\Header
.\UTILITY\bin2hex .\CONFIG\Header.bin .\CONFIG\Header.hex

del .\BIN\%1.bin
ren .\BIN\%1_Rep.bin %1.bin
del .\BIN\%1_Exec.hex
.\UTILITY\bin2hex_ext 0x%3 .\BIN\%1.bin .\BIN\%1_Exec.hex

rem Book Hook Hex
.\UTILITY\bin2hex_ext 0x2000 .\BIN\Boot_Hook.bin .\BIN\Boot_Hook.hex

rem -----------------------------------------------------------------------------------------
rem                            Merge all hex to SN93700 hex
rem -----------------------------------------------------------------------------------------
.\UTILITY\HexMerger .\CONFIG\Header.hex .\BIN\Boot_Hook.hex .\BIN\temp1.hex
IF EXIST .\BIN\SN93700temp.hex (
.\UTILITY\HexMerger .\BIN\temp1.hex .\BIN\SN93700temp.hex .\BIN\temp2.hex
.\UTILITY\HexMerger .\BIN\temp2.hex .\BIN\%1_Exec.hex .\BIN\SN93700IMD.hex
del .\BIN\temp2.hex
del .\BIN\SN93700temp.hex
) ELSE (
.\UTILITY\HexMerger .\BIN\temp1.hex .\BIN\%1_Exec.hex .\BIN\SN93700IMD.hex
)
.\UTILITY\hex2bin .\BIN\SN93700IMD.hex
del .\BIN\temp1.hex

.\UTILITY\binConverter -gs .\BIN\SN93700IMD 192
.\UTILITY\binModify .\BIN\SN93700IMD 192 %errorlevel%

rem copy .\BIN\SN93700IMD.hex .\Download
rem copy .\BIN\%1.hex .\Download
rem copy .\BIN\%1.AXF .\Download

.\UTILITY\bin2hex_ext 0x0 .\BIN\ISPMemDrive.bin .\BIN\temp1.hex
.\UTILITY\bin2hex_ext 0xA000 .\BIN\SN93700IMD.bin .\BIN\temp2.hex
.\UTILITY\HexMerger .\BIN\temp1.hex .\BIN\temp2.hex .\BIN\FW_SN93700.hex
.\UTILITY\hex2bin .\BIN\FW_SN93700.hex
.\UTILITY\binConverter -gs .\BIN\FW_SN93700 41156
.\UTILITY\binModify .\BIN\FW_SN93700 41156 %errorlevel%
.\UTILITY\binModify .\BIN\FW_SN93700 41160 %6
del .\BIN\%1.bin
copy .\BIN\FW_SN93700.bin .\BIN\%1.bin
copy .\BIN\FW_SN93700.hex .\BIN\%1.hex
del .\BIN\temp1.hex
del .\BIN\temp2.hex

rem -----------------------------------------------------------------------------------------
rem                            Merge IQ or OSD
rem -----------------------------------------------------------------------------------------
SET PROFADDR=%6%
SET /a PROFADDR*=1024, PROFADDR+=40960

if %1==SN93700_VBM_PU_HS_A7130 goto MergeOSD

:MergeIQ
.\UTILITY\bin2hex_ext  %PROFADDR% .\SYSTEM\Profile\Profile.dat .\BIN\temp1.hex
.\UTILITY\HexMerger  .\BIN\FW_SN93700.hex .\BIN\temp1.hex .\BIN\temp2.hex
.\UTILITY\hex2bin  .\BIN\temp2.hex
IF EXIST .\BIN\FW_SN93700_IQ.bin (
del .\BIN\FW_SN93700_IQ.bin
)
IF EXIST .\BIN\FW_SN93700_OSD.bin (
del .\BIN\FW_SN93700_OSD.bin
)
del .\BIN\temp1.hex
copy /b .\BIN\temp2.bin+.\SYSTEM\IQ\H62\H62.bin .\BIN\FW_SN93700_IQ.bin
.\UTILITY\binConverter -gs .\BIN\FW_SN93700_IQ 41156
.\UTILITY\binModify .\BIN\FW_SN93700_IQ 41156 %errorlevel%
.\UTILITY\binModify .\BIN\FW_SN93700_IQ 41160 %6
del .\BIN\temp2.hex
del .\BIN\temp2.bin
goto end_merge

:MergeOSD
.\UTILITY\bin2hex_ext %PROFADDR% .\SYSTEM\Profile\Profile.dat .\BIN\temp1.hex
.\UTILITY\HexMerger .\BIN\FW_SN93700.hex .\BIN\temp1.hex .\BIN\temp2.hex
.\UTILITY\hex2bin .\BIN\temp2.hex
IF EXIST .\BIN\FW_SN93700_OSD.bin (
del .\BIN\FW_SN93700_OSD.bin
)
IF EXIST .\BIN\FW_SN93700_IQ.bin (
del .\BIN\FW_SN93700_IQ.bin
)
del .\BIN\temp1.hex
copy /b .\BIN\temp2.bin+.\SYSTEM\OSD\OSD_Font_HD_90¢X.pat+.\SYSTEM\OSD\OSD_Image_HD_90¢X.dat .\BIN\temp1.bin
SET /a LOGOADDR=PROFADDR+12
.\UTILITY\binConverter -gs .\BIN\temp1 %LOGOADDR%
SET FILESZ=0
SET /A FILESZ=%errorlevel%-40960
.\UTILITY\binModify .\BIN\temp1 %LOGOADDR% %FILESZ%
copy /b .\BIN\temp1.bin+.\SYSTEM\OSD\OSD_Logo_HD_90¢X.dat .\BIN\FW_SN93700_OSD.bin
.\UTILITY\binConverter -gs .\BIN\FW_SN93700_OSD 41156
.\UTILITY\binModify .\BIN\FW_SN93700_OSD 41156 %errorlevel%
.\UTILITY\binModify .\BIN\FW_SN93700_OSD 41160 %6
del .\BIN\temp1.bin
del .\BIN\temp2.hex
del .\BIN\temp2.bin

:end_merge
del .\BIN\FW_SN93700.bin
del .\BIN\FW_SN93700.hex
