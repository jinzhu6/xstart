#define SETUP_FILE "xstart-"+GetDateTimeString("dddd_dd/mm_(hh:nn)", '.', '-');

[Setup]
AppName=xstart
PrivilegesRequired=none
DefaultDirName={pf}\Huettinger\xstart\
AllowRootDirectory=true
DirExistsWarning=no
ShowLanguageDialog=no
LanguageDetectionMethod=none
Uninstallable=true
VersionInfoCompany=Huettinger
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
SourceDir=.
ChangesAssociations=yes
Compression=zip/1

[Files]
Source: *; Excludes: ".git,.gitignore,build,src,build*.bat,make.bat,CMakeLists.txt,SETUP.ISS,_SETUP,_OLD,Debug,Release,*.bak,video.m*"; DestDir: {app}; Flags: recursesubdirs;

;Source: "bin\vcredist_x86.exe"; DestDir: "{app}"; Flags: deleteafterinstall

;[Run]
;Filename: "{app}\vcredist_x86.exe"; Parameters: "/q:a /c:""VCREDI~3.EXE /q:a /c:""""msiexec /i vcredist.msi /qn"""" """; WorkingDir: {app}; StatusMsg: "Installing Micosoft's C/C++ runtime ...";

[Tasks]
Name: startup; Description: "Run Udoo-Connect on startup";

[Icons]
Name: "{userstartmenu}\xstart\Editor"; Filename: "{app}\editor\scite.exe";
Name: "{userstartmenu}\xstart\Udoo Connect"; Filename: "{app}\scripts\udoo-connect.gm";
Name: "{userstartmenu}\xstart\Script Reference"; Filename: "{app}\editor\help.htm";
Name: "{userstartmenu}\xstart\Console"; Filename: "{app}\bin\xstart.exe";
Name: "{userstartmenu}\xstart\Examples"; Filename: "{app}\examples\";
Name: "{userdesktop}\Script Editor (xstart)"; Filename: "{app}\editor\scite.exe";
Name: "{userdesktop}\Udoo Connect"; Filename: "{app}\scripts\udoo-connect.gm";
Name: "{userstartup}\Udoo Connect"; Filename: "{app}\scripts\udoo-connect.gm"; IconFilename: "{app}\bin\xstart.exe"; Tasks: startup;

[Registry]
Root: HKCR; Subkey: ".gm"; ValueType: string; ValueName: ""; ValueData: "GM-Script"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "GM-Script"; ValueType: string; ValueName: ""; ValueData: "GM-Script"; Flags: uninsdeletekey
Root: HKCR; Subkey: "GM-Script\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\xstart.exe,0"
Root: HKCR; Subkey: "GM-Script\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\xstart.exe"" ""%1"""
Root: HKCR; Subkey: "GM-Script\shell\edit\command"; ValueType: string; ValueName: ""; ValueData: """{app}\editor\scite.exe"" ""%1"""
Root: HKCR; Subkey: ".js"; ValueType: string; ValueName: ""; ValueData: "JSFile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "JSFile\shell\edit\command"; ValueType: string; ValueName: ""; ValueData: """{app}\editor\scite.exe"" ""%1"""
