
@echo off
echo.Current User is '%USERNAME%'
echo.'%USERNAME%' must have administrative privilege to install EasyWebRTC Plugin

cd %~dp0

set "filemask=WebRTC_ATL.dll"
for %%A in (%filemask%) do regsvr32 %%A || GOTO:EOF

ECHO.&PAUSE&GOTO:EOF

