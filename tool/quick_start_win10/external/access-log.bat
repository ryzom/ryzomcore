@echo off
title Access Log
powershell Get-Content -Path ".\nginx\logs\access.log" -Wait
