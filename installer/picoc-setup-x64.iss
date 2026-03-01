; ============================================================
; picoC Installer Script  –  64-bit (x64)
; Inno Setup 6
; ============================================================
; Build with:  ISCC.exe picoc-setup-x64.iss
;   or use:    .\build-installers.ps1
; ============================================================

#define MyAppName      "picoC"
#define MyAppVersion   "1.0.0"
#define MyAppPublisher "picoC Team"
#define MyAppURL       "https://github.com/picoC"
#define MyAppGUI       "picoc-gui.exe"
#define MyAppCLI       "picoc.exe"
#define DistDir        "dist-x64"

; ---- General settings ----
[Setup]
AppId={{B3F7A2E1-8C4D-4F6A-9E1B-2D3C4E5F6A7B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=output
OutputBaseFilename=picoC-setup-{#MyAppVersion}-x64
SetupIconFile=logo.ico
UninstallDisplayIcon={app}\{#MyAppGUI}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
DisableWelcomePage=no

; ---- Languages ----
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

; ---- User-selectable tasks ----
[Tasks]
Name: "desktopicon";  Description: "Create a &desktop shortcut for picoC GUI";  GroupDescription: "Additional shortcuts:"
Name: "addtopath";    Description: "Add picoC to the system &PATH (use picoc from any terminal)"; GroupDescription: "System integration:"; Flags: unchecked

; ---- Files to install ----
[Files]
; Main executables
Source: "{#DistDir}\picoc-gui.exe";  DestDir: "{app}"; Flags: ignoreversion
Source: "{#DistDir}\picoc.exe";      DestDir: "{app}"; Flags: ignoreversion

; Qt runtime DLLs
Source: "{#DistDir}\*.dll";          DestDir: "{app}"; Flags: ignoreversion

; Qt plugins
Source: "{#DistDir}\platforms\*";          DestDir: "{app}\platforms";          Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\styles\*";             DestDir: "{app}\styles";             Flags: ignoreversion recursesubdirs createallsubdirs; Check: DirExists(ExpandConstant('{#DistDir}\styles'))
Source: "{#DistDir}\imageformats\*";       DestDir: "{app}\imageformats";       Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\iconengines\*";        DestDir: "{app}\iconengines";        Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\generic\*";            DestDir: "{app}\generic";            Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\tls\*";                DestDir: "{app}\tls";                Flags: ignoreversion recursesubdirs createallsubdirs

; Bundled tools (NASM assembler + GoLink linker)
Source: "{#DistDir}\tools\nasm\*";   DestDir: "{app}\tools\nasm";   Flags: ignoreversion
Source: "{#DistDir}\tools\golink\*"; DestDir: "{app}\tools\golink"; Flags: ignoreversion

; ---- Start-Menu & Desktop shortcuts ----
[Icons]
Name: "{group}\picoC GUI";        Filename: "{app}\{#MyAppGUI}"; Comment: "Launch picoC Compiler GUI"; IconFilename: "{app}\{#MyAppGUI}"
Name: "{group}\Uninstall picoC";  Filename: "{uninstallexe}"
Name: "{autodesktop}\picoC";      Filename: "{app}\{#MyAppGUI}"; Comment: "picoC Compiler"; Tasks: desktopicon

; ---- Post-install: offer to launch ----
[Run]
Filename: "{app}\{#MyAppGUI}"; Description: "Launch picoC now"; Flags: nowait postinstall skipifsilent

; ---- Pascal script for PATH management ----
[Code]

{ -- Add to PATH on install -- }
procedure CurStepChanged(CurStep: TSetupStep);
var
  Path: string;
begin
  if (CurStep = ssPostInstall) and WizardIsTaskSelected('addtopath') then
  begin
    if RegQueryStringValue(HKLM,
        'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
        'Path', Path) then
    begin
      if Pos(ExpandConstant('{app}'), Path) = 0 then
      begin
        Path := Path + ';' + ExpandConstant('{app}');
        RegWriteStringValue(HKLM,
          'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
          'Path', Path);
      end;
    end;
  end;
end;

{ -- Remove from PATH on uninstall -- }
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Path, AppDir: string;
  P: Integer;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    AppDir := ExpandConstant('{app}');
    if RegQueryStringValue(HKLM,
        'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
        'Path', Path) then
    begin
      { Try ";C:\...\picoC" first }
      P := Pos(';' + AppDir, Path);
      if P > 0 then
        Delete(Path, P, Length(AppDir) + 1)
      else begin
        { Maybe it is the very first entry }
        P := Pos(AppDir + ';', Path);
        if P = 1 then
          Delete(Path, 1, Length(AppDir) + 1)
        else begin
          { Only entry }
          P := Pos(AppDir, Path);
          if P > 0 then
            Delete(Path, P, Length(AppDir));
        end;
      end;
      RegWriteStringValue(HKLM,
        'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
        'Path', Path);
    end;
  end;
end;

