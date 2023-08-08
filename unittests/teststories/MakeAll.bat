@rem ***************************************************************************
@rem * This file compiles all the test stories used for testing the dap
@rem * implementation
@rem * ---------------------------------------------------------------
@rem * It assumes that t3make is in PATH
@rem ***************************************************************************
@echo off

t3make -version >NUL 2>&1
if "%ERRORLEVEL%" <> "0" (
  echo ERROR: Cannot find the t3make compiler. Make sure that PATH is setup correctly
  goto end
)

@rem Create the obj folder
setlocal
if not exist obj\ (
  echo Yes 
  md obj
  set CleanObj=1
)

@rem Build all story files
t3make -Fy obj -Fo obj -d -q -lib system -source BreakpointTest





@rem If we had to create the obj folder, remove it
if defined CleanObj (
  echo Yes 
  del /q obj\*.* 
  rd obj
)

:end
endlocal

