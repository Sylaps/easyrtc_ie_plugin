
@echo off
echo.Current User is '%USERNAME%'
echo.'%USERNAME%' must have administrative privilege to uninstall EasyWebRTC Plugin

cd %~dp0

regsvr32 /u WebRTC_ATL.dll
