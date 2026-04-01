@echo off
cl /nologo a1hex.cxx /I. /GS- /MT /Ot /Ox /Ob2 /Oi /Qpar /O2 /EHac /Zi /DDEBUG /D_AMD64_ /link ntdll.lib user32.lib /OPT:REF

