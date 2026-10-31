// ==WindhawkMod==
// @id              w11-classic-context-menu
// @name            Windows 11 Classic Context Menu
// @description     Classic context menu for Windows 11 Explorer
// @version         1.0.0
// @author          WangHaonie
// @github          https://github.com/WangHaonie
// @include         explorer.exe
// @architecture    x86-64
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
Classic context menu for Windows 11 Explorer.

This mod is based on `explorer-context-menu-classic` by m417z
(https://github.com/m417z), extended to work on the latest
Windows 11 Beta (`26220.9836`), where the context menu was reworked and the
original symbol-based hook no longer works.

On `26220.9836`, the navigation pane creates the modern menu
through `Windows.UI.FileExplorer.dll`, bypassing the presenter. That
class is also refused.

The older `CNscTree::ShouldShowMiniMenu` hook is kept for older Windows
versions. It relies on PDB symbols and is skipped when they are unavailable,
without preventing the rest of the mod from working.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- overrideWithCtrl: true
  $name: Show the new menu when Ctrl is held
  $description: When enabled, holding Ctrl shows the Windows 11 new context menu.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <shlwapi.h>

struct {
    bool overrideWithCtrl;
} g_settings;

using IUnknown_QueryService_t = decltype(&IUnknown_QueryService);
IUnknown_QueryService_t IUnknown_QueryService_Original;
HRESULT WINAPI IUnknown_QueryService_Hook(IUnknown* punk,
                                          const GUID& guidService,
                                          const IID& riid,
                                          void** ppvOut) {
    // GUIDs from shell32.dll, CDefView::TryGetContextMenuPresenter.

    // {B306C5B1-B4F2-473C-B6FF-701B246CE2D2}
    constexpr GUID guidServiceTarget = {
        0xb306c5b1,
        0xb4f2,
        0x473c,
        {0xb6, 0xff, 0x70, 0x1b, 0x24, 0x6c, 0xe2, 0xd2}};

    // {706461D1-AC5F-4730-BFE3-CAC6CAD5EF5E}
    constexpr GUID riidTarget = {
        0x706461d1,
        0xac5f,
        0x4730,
        {0xbf, 0xe3, 0xca, 0xc6, 0xca, 0xd5, 0xef, 0x5e}};

    // {37A472F7-63CF-4CCF-A88B-5231A3C7D8B6}
    // Changed to this version in update KB5052093 of Windows 11 version 24H2.
    constexpr GUID riidTarget2 = {
        0x37a472f7,
        0x63cf,
        0x4ccf,
        {0xa8, 0x8b, 0x52, 0x31, 0xa3, 0xc7, 0xd8, 0xb6}};

    // Navigation pane on Windows 11 version 25H2 build 26220, which doesn't
    // use the CNscTree::ShouldShowMiniMenu symbol anymore.

    // {7E3A2466-C585-4CBB-9A89-F32D5AF94584}
    constexpr GUID guidServiceTarget2 = {
        0x7e3a2466,
        0xc585,
        0x4cbb,
        {0x9a, 0x89, 0xf3, 0x2d, 0x5a, 0xf9, 0x45, 0x84}};

    // {26B79130-4C9F-4424-AEFB-52CC63F4D3C6}
    constexpr GUID riidTarget3 = {
        0x26b79130,
        0x4c9f,
        0x4424,
        {0xae, 0xfb, 0x52, 0xcc, 0x63, 0xf4, 0xd3, 0xc6}};

    const bool isPresenterRiid =
        IsEqualGUID(riid, riidTarget) || IsEqualGUID(riid, riidTarget2) ||
        IsEqualGUID(riid, riidTarget3);
    const bool isKnownService = IsEqualGUID(guidService, guidServiceTarget) ||
                                IsEqualGUID(guidService, guidServiceTarget2);

    if (isPresenterRiid && isKnownService) {
        Wh_Log(L">");

        if (g_settings.overrideWithCtrl && GetKeyState(VK_CONTROL) < 0) {
            // Temporarily off.
        } else {
            Wh_Log(L"Disallowing new menu");
            return E_FAIL;
        }
    }

    return IUnknown_QueryService_Original(punk, guidService, riid, ppvOut);
}

using CoCreateInstance_t = decltype(&CoCreateInstance);
CoCreateInstance_t CoCreateInstance_Original;
HRESULT WINAPI CoCreateInstance_Hook(const CLSID& rclsid,
                                     IUnknown* pUnkOuter,
                                     DWORD dwClsContext,
                                     const IID& riid,
                                     void** ppv) {
    // {51668040-5E31-4023-BBE3-149163BD80F1} is an undocumented class from
    // Windows.UI.FileExplorer.dll, created when the modern context menu is
    // shown. Refusing it mimics a disabled class registration and makes
    // Explorer fall back to the classic menu. This covers the navigation
    // pane, which creates the new menu without asking for the presenter.
    constexpr GUID fileExplorerMenuClsid = {
        0x51668040,
        0x5e31,
        0x4023,
        {0xbb, 0xe3, 0x14, 0x91, 0x63, 0xbd, 0x80, 0xf1}};

    if (IsEqualGUID(rclsid, fileExplorerMenuClsid) &&
        !(g_settings.overrideWithCtrl && GetKeyState(VK_CONTROL) < 0)) {
        Wh_Log(L"Blocking Windows.UI.FileExplorer menu class");
        return REGDB_E_CLASSNOTREG;
    }

    return CoCreateInstance_Original(rclsid, pUnkOuter, dwClsContext, riid,
                                     ppv);
}

using CNscTree_ShouldShowMiniMenu_t = bool(WINAPI*)(void* pThis, void* param1);
CNscTree_ShouldShowMiniMenu_t CNscTree_ShouldShowMiniMenu_Original;
bool WINAPI CNscTree_ShouldShowMiniMenu_Hook(void* pThis, void* param1) {
    Wh_Log(L">");

    if (g_settings.overrideWithCtrl && GetKeyState(VK_CONTROL) < 0) {
        // Temporarily off.
    } else {
        Wh_Log(L"Disallowing new menu");
        return false;
    }

    return CNscTree_ShouldShowMiniMenu_Original(pThis, param1);
}

bool HookExplorerFrameSymbols() {
    HMODULE module = LoadLibrary(L"explorerframe.dll");
    if (!module) {
        Wh_Log(L"Couldn't load explorerframe.dll");
        return false;
    }

    WindhawkUtils::SYMBOL_HOOK explorerFrameDllHooks[] = {
        {
            {LR"(private: bool __cdecl CNscTree::ShouldShowMiniMenu(struct _TREEITEM *))"},
            &CNscTree_ShouldShowMiniMenu_Original,
            CNscTree_ShouldShowMiniMenu_Hook,
            true,
        },
    };

    return HookSymbols(module, explorerFrameDllHooks,
                       ARRAYSIZE(explorerFrameDllHooks));
}

void LoadSettings() {
    g_settings.overrideWithCtrl = Wh_GetIntSetting(L"overrideWithCtrl");
}

BOOL Wh_ModInit() {
    Wh_Log(L">");

    LoadSettings();

    if (!HookExplorerFrameSymbols()) {
        // The symbol isn't necessarily required. Keep going, so that the
        // hooks below still get installed when symbols are unavailable (for
        // example, on Windows Insider builds without public PDB files).
        Wh_Log(L"Explorer frame symbols not available, skipping hook");
    }

    HMODULE shcoreModule = LoadLibrary(L"shcore.dll");
    if (shcoreModule) {
        IUnknown_QueryService_t pIUnknown_QueryService =
            (IUnknown_QueryService_t)GetProcAddress(shcoreModule,
                                                    "IUnknown_QueryService");
        if (pIUnknown_QueryService) {
            WindhawkUtils::Wh_SetFunctionHookT(pIUnknown_QueryService,
                                               IUnknown_QueryService_Hook,
                                               &IUnknown_QueryService_Original);
        } else {
            Wh_Log(L"Error getting IUnknown_QueryService");
        }
    } else {
        Wh_Log(L"Error loading shcore.dll");
    }

    HMODULE combaseModule = GetModuleHandle(L"combase.dll");
    if (!combaseModule) {
        combaseModule = LoadLibrary(L"combase.dll");
    }

    if (combaseModule) {
        CoCreateInstance_t pCoCreateInstance =
            (CoCreateInstance_t)GetProcAddress(combaseModule,
                                               "CoCreateInstance");
        if (pCoCreateInstance) {
            WindhawkUtils::Wh_SetFunctionHookT(pCoCreateInstance,
                                               CoCreateInstance_Hook,
                                               &CoCreateInstance_Original);
        } else {
            Wh_Log(L"Error getting CoCreateInstance");
        }
    } else {
        Wh_Log(L"Error loading combase.dll");
    }

    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L">");
}

void Wh_ModSettingsChanged() {
    Wh_Log(L">");

    LoadSettings();
}
