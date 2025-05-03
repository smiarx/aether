#define PluginName "##PROJECT##"
#define Publisher "##COMPANY##"
#define Version "##VERSION##"

[Setup]
AppName={#PluginName}
AppPublisher={#Publisher}
AppVersion={#Version}
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
DefaultDirName="{commoncf64}\VST3\{#PluginName}.vst3\"
DisableDirPage=yes
OutputBaseFilename={#PluginName}-windows
UninstallFilesDir={commonappdata}\{#PluginName}\uninstall

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#PluginName}Data"

[Files]
Source: "{#PluginName}_artefacts\Release\VST3\{#PluginName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#PluginName}.vst3\"; Flags: ignoreversion recursesubdirs;

[Run]
Filename: "{cmd}"; \
    WorkingDir: "{commoncf64}\VST3"; \
    Parameters: "/C mklink /D ""{commoncf64}\VST3\{#PluginName}Data"" ""{commonappdata}\{#PluginName}"""; \
    Flags: runascurrentuser;
