REM Delete all tempoary files 

erase /A H *.suo
erase *.ncb
erase *.opt
erase *.ilk

erase bin\*.pdb
erase bin\*.ilk
erase bin\*d.exe
erase bin\*d.dll
erase bin\*LIB.exe

erase docs\Warnings.txt

erase lib\*d.lib
erase lib\*.exp
erase lib\*LIB.lib

rmdir /S /Q crashcon\src\Debug
rmdir /S /Q crashcon\src\Release
rmdir /S /Q "crashcon\src\Release LIB"
rmdir /S /Q crashcon\src\x64
erase /A H crashcon\src\*.user
erase crashcon\src\*.aps

rmdir /S /Q crashrpt\src\Debug
rmdir /S /Q crashrpt\src\Release
rmdir /S /Q crashrpt\src\x64
erase /A H crashrpt\src\*.user
erase crashrpt\src\*.aps

rmdir /S /Q CrashRptTest\src\Debug
rmdir /S /Q CrashRptTest\src\Release
rmdir /S /Q "CrashRptTest\src\Release LIB"
rmdir /S /Q CrashRptTest\src\x64
erase /A H CrashRptTest\src\*.user
erase CrashRptTest\src\*.aps

rmdir /S /Q CrashSender\src\Debug
rmdir /S /Q CrashSender\src\Release
rmdir /S /Q "CrashRptTest\src\Release LIB"
rmdir /S /Q CrashSender\src\x64
erase /A H CrashSender\src\*.user
erase CrashSender\src\*.aps
erase CrashSender\src\*.aps

rmdir /S /Q zlib\src\Debug
rmdir /S /Q zlib\src\Release
rmdir /S /Q "CrashRptTest\src\Release LIB"
rmdir /S /Q zlib\src\x64
rmdir /S /Q zlib\lib
erase /A H zlib\src\*.user
erase zlib\src\*.aps

rmdir /S /Q libpng\src\Debug
rmdir /S /Q libpng\src\Release
rmdir /S /Q "libpng\src\Release LIB"
rmdir /S /Q libpng\src\x64
rmdir /S /Q libpng\lib
erase /A H libpng\src\*.user
erase libpng\src\*.aps

rmdir /S /Q minizip\src\Debug
rmdir /S /Q minizip\src\Release
rmdir /S /Q "minizip\src\Release LIB"
rmdir /S /Q minizip\src\x64
rmdir /S /Q minizip\lib
erase /A H minizip\src\*.user
erase minizip\src\*.aps

rmdir /S /Q CrashRptProbe\src\Debug
rmdir /S /Q CrashRptProbe\src\Release
rmdir /S /Q "CrashRptProbe\src\Release LIB"
rmdir /S /Q CrashRptProbe\src\x64
erase /A H CrashRptProbe\src\*.user
erase CrashRptProbe\src\*.aps

rmdir /S /Q crprober\src\Debug
rmdir /S /Q crprober\src\Release
rmdir /S /Q "crprober\src\Release LIB"
rmdir /S /Q crprober\src\x64
erase /A H crprober\src\*.user
erase crprober\src\*.aps
