del /q /s html
mkdir html
python wiki.py
copy source\*.* html\
htmlhelp\hhc.exe "html\OpenMPT Manual.hhp"
copy "html\OpenMPT Manual.chm" "..\..\packageTemplate\"
@pause