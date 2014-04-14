
@echo off
echo.Current User is '%USERNAME%'
echo.'%USERNAME%' must have administrative privilege to install EasyWebRTC Plugin

cd %~dp0

regsvr32 WebRTC_ATL.dll
