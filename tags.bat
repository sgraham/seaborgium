@echo off
ctags -R --extra=+f *
del csearchindex
set CSEARCHINDEX=csearchindex
cindex .

