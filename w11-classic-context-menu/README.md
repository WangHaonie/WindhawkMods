# w11-classic-context-menu

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