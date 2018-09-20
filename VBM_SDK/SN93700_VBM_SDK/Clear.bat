del /q .\LST\*.*
rd .\LST
del .\BIN\*.crf
del .\BIN\*.d
del .\BIN\*.o
del .\BIN\*.htm
del .\BIN\*.lnp
del .\BIN\*.plg
del .\BIN\*.tra
del .\BIN\*.__i
del .\BIN\*.iex
del .\BIN\*.bak
del .\BIN\*._ia
del .\BIN\*.axf
if exist .\BIN\Boot_Hook.bin ren .\BIN\Boot_Hook.bin Boot_Hook.a
if exist .\BIN\ISPMemDrive.bin ren .\BIN\ISPMemDrive.bin ISPMemDrive.a
del .\BIN\*.bin
del .\BIN\*.hex
del *.uvgui.*
rem del *.uvopt
del *.dep
del *.bak
del JLinkLog.txt
del *.plg
rem del JLinkArm*.*
if exist .\BIN\Boot_Hook.a ren .\BIN\Boot_Hook.a Boot_Hook.bin
if exist .\BIN\ISPMemDrive.a ren .\BIN\ISPMemDrive.a ISPMemDrive.bin