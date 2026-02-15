!include "MUI2.nsh"

Name "PeerChat"
OutFile "peerchat-setup.exe"
InstallDir "$PROGRAMFILES64\PeerChat"
InstallDirRegKey HKLM "Software\PeerChat" "InstallDir"
RequestExecutionLevel admin

!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Install"
    SetOutPath "$INSTDIR"
    File "peerchat.exe"

    ; Write registry keys
    WriteRegStr HKLM "Software\PeerChat" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PeerChat" \
        "DisplayName" "PeerChat"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PeerChat" \
        "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PeerChat" \
        "Publisher" "rapoyrazoglu"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PeerChat" \
        "DisplayVersion" "0.0.1"

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; Add to PATH
    EnVar::AddValue "PATH" "$INSTDIR"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\peerchat.exe"
    Delete "$INSTDIR\uninstall.exe"
    RMDir "$INSTDIR"

    ; Remove from PATH
    EnVar::DeleteValue "PATH" "$INSTDIR"

    ; Remove registry keys
    DeleteRegKey HKLM "Software\PeerChat"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PeerChat"
SectionEnd
