//#define SETUP_FILE "Xstart-redist-"+GetDateTimeString("dddd_dd/mm_(hh:nn)", '.', '-');
#define SETUP_FILE "xstart-dev-setup"

[Setup]
AppName=xstart runtime
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
Compression=zip/1

[Files]
Source: *; Excludes: ".git,.gitignore,build,src,extras,build*.bat,make.bat,CMakeLists.txt,SETUP.ISS,_SETUP,_OLD,Debug,Release,*.bak,video.m*,MinSizeRel,AStyle.exe,editor,examples,scripts,arialuni.ttf,watchman-linux,*.iss,format.bat"; DestDir: {app}; Flags: recursesubdirs;
;Source: "bin\vcredist_x86.exe"; DestDir: "{app}"; Flags: deleteafterinstall

;[Run]
;Filename: "{app}\vcredist_x86.exe"; Parameters: "/q:a /c:""VCREDI~3.EXE /q:a /c:""""msiexec /i vcredist.msi /qn"""" """; WorkingDir: {app}; StatusMsg: "Installing Micosoft's C/C++ runtime ...";

;[Tasks]
;Name: startup; Description: "Run Udoo-Connect on startup";

[Registry]
Root: HKCR; Subkey: ".gm"; ValueType: string; ValueName: ""; ValueData: "GM-Script"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "GM-Script"; ValueType: string; ValueName: ""; ValueData: "GM-Script"; Flags: uninsdeletekey
Root: HKCR; Subkey: "GM-Script\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\xstart.exe,0"
Root: HKCR; Subkey: "GM-Script\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\xstart.exe"" ""%1"""
