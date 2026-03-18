; ============================================================
; picoC Installer Script  –  32-bit (x86)
; Inno Setup 6
; ============================================================
; Build with:  ISCC.exe picoc-setup-x86.iss
;   or use:    .\build-installers.ps1
; ============================================================

#define MyAppName      "picoC (32-bit)"
#define MyAppVersion   "1.0.0"
#define MyAppPublisher "picoC Team"
#define MyAppURL       "https://github.com/picoC"
#define MyAppCLI       "picoc.exe"
#define DistDir        "dist-x86"

; ---- General settings ----
[Setup]
; Different AppId so both 32-bit and 64-bit can coexist side-by-side
AppId={{A1E2B3C4-5D6F-7A8B-9C0D-1E2F3A4B5C6D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
DefaultDirName={autopf32}\picoC
DefaultGroupName={#MyAppName}
OutputDir=output
OutputBaseFilename=picoC-setup-{#MyAppVersion}-x86
SetupIconFile=logo.ico
UninstallDisplayIcon={app}\{#MyAppCLI}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
; No ArchitecturesAllowed restriction — runs on both 32-bit and 64-bit Windows
DisableWelcomePage=no

; ---- Languages ----
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

; ---- User-selectable tasks ----
[Tasks]
Name: "addtopath";  Description: "Add picoC to the system &PATH (use picoc from any terminal)"; GroupDescription: "System integration:"; Flags: unchecked

; ---- Files to install ----
[Files]
; Main executable (CLI only — no GUI in the 32-bit build)
Source: "{#DistDir}\picoc.exe";  DestDir: "{app}"; Flags: ignoreversion

; Bundled tools (NASM assembler + GoLink linker)
Source: "{#DistDir}\tools\nasm\*";   DestDir: "{app}\tools\nasm";   Flags: ignoreversion
Source: "{#DistDir}\tools\golink\*"; DestDir: "{app}\tools\golink"; Flags: ignoreversion

; ---- Start-Menu shortcuts ----
[Icons]
Name: "{group}\picoC Command Prompt"; Filename: "{cmd}"; Parameters: "/K cd /d ""{app}"""; Comment: "Open a terminal in the picoC directory"; WorkingDir: "{app}"
Name: "{group}\Uninstall picoC";      Filename: "{uninstallexe}"

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

