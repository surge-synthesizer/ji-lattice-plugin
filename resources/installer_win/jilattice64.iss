#define MyAppPublisher "Surge Synth Team"
#define MyAppURL "https://www.surge-synth-team.org/"
#define MyAppName "JI Lattice Plugin"
#define MyAppNameCondensed "JILattice"
#define MyID "AAA29DB2-70CC-4FF0-8806-36A5C6C9E95C"

#ifndef MyAppVersion
#define MyAppVersion "0.0.0"
#endif

;uncomment these two lines if building the installer locally!
;#define JI_LATTICE_SRC "..\..\"
;#define JI_LATTICE_BIN "..\..\build\"

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppId={#MyID}
AppName={#MyAppName}
AppVerName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppPublisher}\{#MyAppName}\
DefaultGroupName={#MyAppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
AlwaysShowDirOnReadyPage=yes
LicenseFile={#JI_LATTICE_SRC}\resources\LICENSE.distribution
OutputBaseFilename={#MyAppNameCondensed}-{#MyAppVersion}-Windows-64bit-setup
SetupIconFile={#JI_LATTICE_SRC}\resources\installer_win\bstep.ico
UninstallDisplayIcon={uninstallexe}
UsePreviousAppDir=yes
Compression=lzma
SolidCompression=yes
UninstallFilesDir={autoappdata}\{#MyAppName}\uninstall
CloseApplicationsFilter=*.exe,*.vst3
WizardStyle=modern
WizardSizePercent=100
WizardImageFile={#JI_LATTICE_SRC}\resources\installer_win\empty.bmp
WizardSmallImageFile={#JI_LATTICE_SRC}\resources\installer_win\empty.bmp
WizardImageAlphaFormat=defined

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "Full installation"
Name: "clap"; Description: "CLAP installation"
Name: "vst3"; Description: "VST3 installation"
Name: "standalone"; Description: "Standalone installation"
Name: "custom"; Description: "Custom"; Flags: iscustom

[Components]
Name: "CLAP"; Description: "{#MyAppName} CLAP (64-bit)"; Types: full clap custom
Name: "VST3"; Description: "{#MyAppName} VST3 (64-bit)"; Types: full vst3 custom
Name: "SA"; Description: "{#MyAppName} Standalone (64-bit)"; Types: full standalone custom

[Files]
Source: "{#JI_LATTICE_BIN}\ji-lattice-products\{#MyAppName}.clap"; DestDir: "{autocf}\CLAP\{#MyAppPublisher}\"; Components: CLAP; Flags: ignoreversion
Source: "{#JI_LATTICE_BIN}\ji-lattice-products\{#MyAppName}.vst3\*"; DestDir: "{autocf}\VST3\{#MyAppPublisher}\{#MyAppName}.vst3\"; Components: VST3; Flags: ignoreversion recursesubdirs
Source: "{#JI_LATTICE_BIN}\ji-lattice-products\{#MyAppName}.exe"; DestDir: "{app}"; Components: SA; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppName}.exe"; Flags: createonlyiffileexists
