#define SETUP_FILE "xstart-setup"

[Setup]
AppName=XStart Development Kit
DefaultGroupName=XStart Development Kit
UninstallDisplayIcon={app}\icon.ico
PrivilegesRequired=none
DefaultDirName={pf}\xstart\
AllowRootDirectory=true
DirExistsWarning=no
ShowLanguageDialog=no
LanguageDetectionMethod=none
Uninstallable=true
VersionInfoProductName=xstart
AppVerName=xstart
AlwaysRestart=false
DisableProgramGroupPage=yes
DisableReadyPage=true
DisableReadyMemo=false
DisableWelcomePage=false
UsePreviousSetupType=true
UsePreviousTasks=true
OutputDir=_SETUP
OutputBaseFilename={#SETUP_FILE}
SourceDir=..
ChangesAssociations=yes
Compression=lzma2/normal
LicenseFile=COPYING

[Files]
Source: *; Excludes: ".git,.gitignore,build,src,extras,_extras,scripts,build*.bat,make.bat,CMakeLists.txt,_OLD,_SETUP,Debug,Release,MinSizeRel,*.bak,video.m*,fonts"; DestDir: {app}; Flags: recursesubdirs;
;Source: "bin\vcredist_x86.exe"; DestDir: "{app}"; Flags: deleteafterinstall

[Run]
Filename: "{app}\editor\quickstart.htm"; Description: "Read Quickstart"; Flags: postinstall shellexec skipifsilent
;Filename: "{app}\vcredist_x86.exe"; Parameters: "/q:a /c:""VCREDI~3.EXE /q:a /c:""""msiexec /i vcredist.msi /qn"""" """; WorkingDir: {app}; StatusMsg: "Installing Micosoft's C/C++ runtime ...";

;[Tasks]
;Name: startup; Description: "Run SBC-Connect on startup";

[Icons]
Name: "{group}\XStart Editor"; Filename: "{app}\editor\scite.exe";
Name: "{group}\XStart Quickstart"; Filename: "{app}\editor\quickstart.htm";
Name: "{group}\XStart Reference"; Filename: "{app}\editor\help.htm";
Name: "{group}\XStart Console"; Filename: "{app}\bin\xstart.exe";
Name: "{group}\XStart Examples"; Filename: "{app}\examples\";
Name: "{userdesktop}\XStart Editor"; Filename: "{app}\editor\scite.exe";

[Registry]
Root: HKCR; Subkey: ".gm"; ValueType: string; ValueName: ""; ValueData: "GM-Script"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "GM-Script"; ValueType: string; ValueName: ""; ValueData: "GM-Script"; Flags: uninsdeletekey
Root: HKCR; Subkey: "GM-Script\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\icon.ico,0"
Root: HKCR; Subkey: "GM-Script\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\xstart.exe"" ""%1"""
Root: HKCR; Subkey: "GM-Script\shell\edit\command"; ValueType: string; ValueName: ""; ValueData: """{app}\editor\scite.exe"" ""%1"""
