#pragma once

#include <functional>

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <foleys_gui_magic/foleys_gui_magic.h>

class DesignerMainComponent : public juce::Component,
                              private juce::ApplicationCommandTarget
{
public:
    DesignerMainComponent();
    ~DesignerMainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    enum CommandIDs
    {
        cmdNewLayout = 0x2200,
        cmdLoadXml,
        cmdSaveXml,
        cmdExportForPlugin,
        cmdToggleEditMode,
        cmdToggleVectorPanel,
        cmdLoadVectorUrl,
        cmdLoadVectorFile,
        cmdDiagnoseVector,
        cmdOpenExternal,
        cmdLoadSvgEditPreset,
        cmdLoadMiniPaintPreset,
        cmdLoadMethodDrawPreset,
        cmdCustomizeToolbar,
        cmdResetToolbar
    };

    class DesignerToolbarFactory final : public juce::ToolbarItemFactory
    {
    public:
        explicit DesignerToolbarFactory(DesignerMainComponent& ownerToUse);

        void getAllToolbarItemIds(juce::Array<int>& ids) override;
        void getDefaultItemSet(juce::Array<int>& ids) override;
        juce::ToolbarItemComponent* createItem(int itemId) override;

    private:
        DesignerMainComponent& owner;
    };
    friend class DesignerToolbarFactory;

    juce::ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(juce::Array<juce::CommandID>& commands) override;
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
    bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info) override;

    void initialiseToolbar();
    void saveToolbarLayout() const;
    void loadToolbarLayout();
    std::unique_ptr<juce::Drawable> createToolbarIcon(int itemId) const;

    void createNewLayout();
    void loadLayoutFromFile();
    void saveLayoutToFile();
    void exportLayoutForPlugin();
    void toggleEditMode();
    void updateStatus(const juce::String& text);
    void loadVectorEditorFromAddress();
    void loadVectorEditorFromFile();
    void toggleVectorPanel();
    void loadVectorPreset(const juce::String& url, const juce::String& label);
    void runVectorDiagnostics();

    class TrackingBrowser final : public juce::WebBrowserComponent
    {
    public:
        using LoadCallback = std::function<void(const juce::String&)>;

        TrackingBrowser(const Options& options, LoadCallback onFinished, LoadCallback onError)
            : juce::WebBrowserComponent(options),
              finishedCallback(std::move(onFinished)),
              errorCallback(std::move(onError))
        {
        }

    private:
        void pageFinishedLoading(const juce::String& url) override
        {
            if (finishedCallback != nullptr)
                finishedCallback(url);
        }

        bool pageLoadHadNetworkError(const juce::String& errorInfo) override
        {
            if (errorCallback != nullptr)
                errorCallback(errorInfo);
            return true;
        }

        LoadCallback finishedCallback;
        LoadCallback errorCallback;
    };

    foleys::MagicGUIState magicState;
    foleys::MagicGUIBuilder magicBuilder { magicState };

    juce::ApplicationCommandManager commandManager;
    juce::Toolbar toolbar;
    std::unique_ptr<DesignerToolbarFactory> toolbarFactory;
    juce::File toolbarLayoutFile;

    juce::Component layoutCanvas;
    juce::Label status;

    juce::Label vectorHeader;
    juce::TextEditor vectorAddress;
    juce::TextButton svgEditPresetButton { "SVG-Edit" };
    juce::TextButton miniPaintPresetButton { "miniPaint" };
    juce::TextButton methodDrawPresetButton { "Method Draw" };
    juce::Label diagnostics;
    std::unique_ptr<TrackingBrowser> vectorBrowser;
    juce::String lastVectorUrl;
    juce::String lastVectorError;
    bool lastVectorLoadSucceeded = false;
    bool lastVectorLoadFailed = false;

    juce::File lastFile;
    std::unique_ptr<juce::FileChooser> fileChooser;
    bool vectorPanelVisible = true;
    bool editMode = true;
};
