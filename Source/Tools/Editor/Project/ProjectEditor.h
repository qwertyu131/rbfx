//
// Copyright (c) 2017-2020 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Project/CloseDialog.h"
#include "../Project/EditorTab.h"

#include <Urho3D/Core/Object.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Plugins/PluginManager.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/XMLFile.h>

#include <EASTL/functional.h>
#include <EASTL/set.h>

#include <regex>

namespace Urho3D
{

class AssetManager;
class HotkeyManager;
class SettingsManager;
class UndoManager;

/// Request to open resource in Editor.
struct OpenResourceRequest
{
    ea::string fileName_;
    ea::string resourceName_;

    SharedPtr<File> file_;
    SharedPtr<XMLFile> xmlFile_;
    SharedPtr<JSONFile> jsonFile_;

    bool IsValid() const { return !fileName_.empty(); }
    explicit operator bool() const { return IsValid(); }

    static OpenResourceRequest FromResourceName(Context* context, const ea::string& resourceName);
};

/// Result of the graceful project close.
enum class CloseProjectResult
{
    Undefined,
    Closed,
    Canceled
};

/// Helper class to keep and restore state of ResourceCache.
class ResourceCacheGuard
{
public:
    explicit ResourceCacheGuard(Context* context);
    ~ResourceCacheGuard();

    const ea::string& GetCoreData() const { return oldCoreData_; }
    const ea::string& GetEditorData() const { return oldEditorData_; }

private:
    Context* context_{};
    StringVector oldResourceDirs_;
    ea::string oldCoreData_;
    ea::string oldEditorData_;
};

/// Main class for all Editor logic related to the project folder.
class ProjectEditor : public Object
{
    URHO3D_OBJECT(ProjectEditor, Object);

public:
    Signal<void()> OnInitialized;
    Signal<void()> OnRenderProjectMenu;
    Signal<void()> OnRenderProjectToolbar;

    ProjectEditor(Context* context, const ea::string& projectPath);
    ~ProjectEditor() override;
    void SerializeInBlock(Archive& archive) override;

    /// Request graceful close of the project. Called multiple times during close sequence.
    CloseProjectResult CloseGracefully();
    /// Request graceful close of the resource.
    void CloseResourceGracefully(const CloseResourceRequest& request);

    /// Update and render main window with tabs.
    void Render();
    /// Update and render toolbar.
    void RenderToolbar();
    /// Update and render project menu.
    void RenderProjectMenu();
    /// Update and render the rest of menu bar.
    void RenderMainMenu();

    /// Mark files with specified name pattern as internal and ignore them in UI.
    void IgnoreFileNamePattern(const ea::string& pattern);
    /// Return whether the file name is ignored.
    bool IsFileNameIgnored(const ea::string& fileName) const;
    /// Add new tab. Avoid calling it in realtime.
    void AddTab(SharedPtr<EditorTab> tab);
    /// Find first tab of matching type.
    template <class T> T* FindTab() const;
    /// Open resource in appropriate resource tab.
    void OpenResource(const OpenResourceRequest& request);
    /// Set whether the global hotkeys are enabled.
    void SetGlobalHotkeysEnabled(bool enabled) { areGlobalHotkeysEnabled_ = enabled; }
    /// Set whether the UI highlight is enabled.
    void SetHighlightEnabled(bool enabled) { isHighlightEnabled_ = enabled; }
    /// Return name of random temporary directory.
    ea::string GetRandomTemporaryPath() const;
    /// Create temporary directory that will be automatically deleted when the handler is destroyed.
    TemporaryDir CreateTemporaryDir();

    /// Commands
    /// @{
    void SaveShallowOnly();
    void SaveProjectOnly();
    void SaveResourcesOnly();
    void Save();
    /// @}

    void ReadIniSettings(const char* entry, const char* line);
    void WriteIniSettings(ImGuiTextBuffer& output);

    /// Return global properties
    /// @{
    const ea::string& GetCoreDataPath() const { return coreDataPath_; }
    const ea::string& GetDataPath() const { return dataPath_; }
    const ea::string& GetCachePath() const { return cachePath_; }
    /// @}

    /// Return singletons
    /// @{
    AssetManager* GetAssetManager() const { return assetManager_; }
    HotkeyManager* GetHotkeyManager() const { return hotkeyManager_; }
    SettingsManager* GetSettingsManager() const { return settingsManager_; }
    UndoManager* GetUndoManager() const { return undoManager_; }
    PluginManager* GetPluginManager() const { return pluginManager_; }
    /// @}

    /// Internal
    /// @{
    void SetFocusedTab(EditorTab* tab);
    EditorTab* GetRootFocusedTab() { return focusedRootTab_; }
    /// @}

private:
    void InitializeHotkeys();
    void EnsureDirectoryInitialized();
    void InitializeDefaultProject();
    void InitializeResourceCache();
    void ResetLayout();
    void ApplyPlugins();
    void SaveGitIgnore();

    /// Project properties
    /// @{
    const ea::string projectPath_;

    const ea::string coreDataPath_;
    const ea::string cachePath_;
    const ea::string tempPath_;

    const ea::string projectJsonPath_;
    const ea::string settingsJsonPath_;
    const ea::string cacheJsonPath_;
    const ea::string uiIniPath_;
    const ea::string gitIgnorePath_;

    ea::string dataPath_;

    const ResourceCacheGuard oldCacheState_;
    /// @}

    SharedPtr<AssetManager> assetManager_;
    SharedPtr<HotkeyManager> hotkeyManager_;
    SharedPtr<SettingsManager> settingsManager_;
    SharedPtr<UndoManager> undoManager_;
    SharedPtr<PluginManager> pluginManager_;

    ea::weak_ptr<void> initializationGuard_;
    bool initialized_{};
    ea::vector<SharedPtr<EditorTab>> tabs_;
    ea::map<ea::string, SharedPtr<EditorTab>> sortedTabs_;
    ea::set<ea::string> ignoredFileNames_;
    ea::vector<std::regex> ignoredFileNameRegexes_;

    /// Close popup handling
    /// @{
    SharedPtr<CloseDialog> closeDialog_;
    CloseProjectResult closeProjectResult_{};
    /// @}

    /// UI state
    /// @{
    bool pendingResetLayout_{};
    ImGuiID dockspaceId_{};
    WeakPtr<EditorTab> focusedTab_;
    WeakPtr<EditorTab> focusedRootTab_;
    bool areGlobalHotkeysEnabled_{true};
    bool isHighlightEnabled_{};
    /// @}
};

template <class T>
T* ProjectEditor::FindTab() const
{
    for (EditorTab* tab : tabs_)
    {
        if (auto derivedTab = dynamic_cast<T*>(tab))
            return derivedTab;
    }
    return nullptr;
}

}