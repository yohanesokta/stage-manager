; Inno Setup Script for Stage Manager
; Supports GUI and Silent Install (/SILENT, /VERYSILENT)
; Supports User-selectable Startup Mode (via GUI Task or /TASKS="startup")

#define MyAppName "Stage Manager"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Stage Manager"
#define MyAppURL "https://github.com/stage-manager"
#define MyAppExeName "stagemanager.exe"
#define MyAppIcon "icons.ico"

[Setup]
AppId={{5C7DE473-A332-4752-9445-56B9B3411BE5}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=installer_output
OutputBaseFilename=StageManager_Setup
SetupIconFile={#MyAppIcon}
UninstallDisplayIcon={app}\{#MyAppIcon}
UninstallDisplayName={#MyAppName}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible

; Privilege settings: allows installation per-user without admin or machine-wide with admin
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog commandline

; Auto-close running instances during install/uninstall (essential for silent install)
CloseApplications=force
RestartApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "startup"; Description: "Run Stage Manager automatically when Windows starts"; GroupDescription: "Startup Mode:"; Flags: unchecked

[Files]
Source: "build\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "*.pdb,*.obj,*.log"
Source: "{#MyAppIcon}"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppIcon}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppIcon}"; Tasks: desktopicon

[Registry]
; Adds autorun to registry if the startup task is selected (works in GUI or silent via /TASKS="startup")
Root: HKA; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "StageManager"; ValueData: """{app}\{#MyAppExeName}"""; Flags: uninsdeletevalue; Tasks: startup

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}"
