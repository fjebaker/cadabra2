; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Cadabra2"
#define MyAppVersion "2.4.2"
#define MyAppPublisher "Kasper Peeters"
#define MyAppURL "https://www.cadabra.science/"
#define MyAppExeName "cadabra2-gtk.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{1ADE9581-27A2-43EA-9A22-CE05177883B4}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE
; File showing the prerequesties etc...
InfoBeforeFile=pre_install.rtf 
; File shown after installation, linking to docs etc...
InfoAfterFile=post_install.rtf
OutputDir=build
OutputBaseFilename=cadabra2-installer
; Icon of the installer. If this causes a compilation error, just 
; comment it out
SetupIconFile=cadabra2.ico
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Cadabra\bin\cadabra2-gtk.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "C:\Cadabra\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; Source: "C:\Users\kasper\Anaconda3\DLLs\sqlite3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
; VC++ redistributable runtime. Extracted by VC2019RedistNeedsInstall(), if needed.
Source: "vcredist_x64.exe"; DestDir: {tmp}; Flags: dontcopy

[Run]
Filename: "{tmp}\vcredist_x64.exe"; StatusMsg: "Installing VC++ redistributables..."; Parameters: "/quiet"; Check: VC2019RedistNeedsInstall ; Flags: waituntilterminated

; https://stackoverflow.com/questions/24574035/how-to-install-microsoft-vc-redistributables-silently-in-inno-setup
[Code]
function VC2019RedistNeedsInstall: Boolean;
var 
  Version: String;
begin
  if (RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version', Version)) then
  begin
    // Is the installed version at least 14.23 ? 
    Log('VC Redist Version check : found ' + Version);
    Result := (CompareStr(Version, 'v14.23.27820.00')<0);
  end
  else 
  begin
    // Not even an old version installed
    Result := True;
  end;
  if (Result) then
  begin
    ExtractTemporaryFile('vcredist_x64.exe');
  end;
end;


[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\bin\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\bin\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

