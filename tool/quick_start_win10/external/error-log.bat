@echo off
title Error Log
powershell Get-Content -Path ".\nginx\logs\error.log" -Wait
