#include "MainComponent.h"

DesignerMainComponent::DesignerToolbarFactory::DesignerToolbarFactory(DesignerMainComponent& ownerToUse)
    : owner(ownerToUse)
{
}

void DesignerMainComponent::DesignerToolbarFactory::getAllToolbarItemIds(juce::Array<int>& ids)
{
    ids.add(cmdNewLayout);
    ids.add(cmdLoadXml);
    ids.add(cmdSaveXml);
    ids.add(cmdExportForPlugin);
    ids.add(separatorBarId);
    ids.add(cmdToggleEditMode);
    ids.add(cmdToggleVectorPanel);
    ids.add(separatorBarId);
    ids.add(cmdLoadVectorUrl);
    ids.add(cmdLoadVectorFile);
    ids.add(cmdDiagnoseVector);
    ids.add(cmdOpenExternal);
    ids.add(separatorBarId);
    ids.add(cmdLoadSvgEditPreset);
    ids.add(cmdLoadMiniPaintPreset);
    ids.add(cmdLoadMethodDrawPreset);
    ids.add(flexibleSpacerId);
    ids.add(cmdCustomizeToolbar);
    ids.add(cmdResetToolbar);
}

void DesignerMainComponent::DesignerToolbarFactory::getDefaultItemSet(juce::Array<int>& ids)
{
    ids.add(cmdNewLayout);
    ids.add(cmdLoadXml);
    ids.add(cmdSaveXml);
    ids.add(cmdExportForPlugin);
    ids.add(separatorBarId);
    ids.add(cmdToggleEditMode);
    ids.add(cmdToggleVectorPanel);
    ids.add(separatorBarId);
    ids.add(cmdLoadVectorUrl);
    ids.add(cmdLoadVectorFile);
    ids.add(cmdDiagnoseVector);
    ids.add(cmdOpenExternal);
    ids.add(flexibleSpacerId);
    ids.add(cmdCustomizeToolbar);
    ids.add(cmdResetToolbar);
}

juce::ToolbarItemComponent* DesignerMainComponent::DesignerToolbarFactory::createItem(int itemId)
{
    juce::String label;
    switch (itemId)
    {
        case cmdNewLayout: label = "New"; break;
        case cmdLoadXml: label = "Load"; break;
        case cmdSaveXml: label = "Save"; break;
        case cmdExportForPlugin: label = "Export"; break;
        case cmdToggleEditMode: label = "Edit"; break;
        case cmdToggleVectorPanel: label = "Vector"; break;
        case cmdLoadVectorUrl: label = "URL"; break;
        case cmdLoadVectorFile: label = "HTML"; break;
        case cmdDiagnoseVector: label = "Diagnose"; break;
        case cmdOpenExternal: label = "External"; break;
        case cmdLoadSvgEditPreset: label = "SVG-Edit"; break;
        case cmdLoadMiniPaintPreset: label = "miniPaint"; break;
        case cmdLoadMethodDrawPreset: label = "MethodDraw"; break;
        case cmdCustomizeToolbar: label = "Customize"; break;
        case cmdResetToolbar: label = "Reset"; break;
        default: label = "Action"; break;
    }

    auto* button = new juce::ToolbarButton(
        itemId,
        label,
        owner.createToolbarIcon(itemId),
        owner.createToolbarIcon(itemId));
    button->setCommandToTrigger(&owner.commandManager, itemId, false);
    return button;
}

DesignerMainComponent::DesignerMainComponent()
{
    magicBuilder.registerJUCELookAndFeels();
    magicBuilder.registerJUCEFactories();

    magicState.setApplicationSettingsFile(
        juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("TwoNistLabs")
            .getChildFile("DesignerApp.settings"));

    toolbarLayoutFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                            .getChildFile("TwoNistLabs")
                            .getChildFile("DesignerToolbar.layout");
    toolbarLayoutFile.getParentDirectory().createDirectory();

    magicState.setGuiValueTree(juce::ValueTree("magic"));
    addAndMakeVisible(layoutCanvas);
    magicBuilder.createGUI(layoutCanvas);

    commandManager.registerAllCommandsForTarget(this);
    addKeyListener(commandManager.getKeyMappings());
    setWantsKeyboardFocus(true);
    initialiseToolbar();

    status.setText("Designer ready. Use toolbar + palette to build UI.", juce::dontSendNotification);
    status.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(status);

    vectorHeader.setText("Vector Editor (SVG)", juce::dontSendNotification);
    vectorHeader.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(vectorHeader);

    vectorAddress.setText("https://svg-edit.github.io/svgedit/releases/latest/svg-editor.html", juce::dontSendNotification);
    addAndMakeVisible(vectorAddress);
    addAndMakeVisible(svgEditPresetButton);
    addAndMakeVisible(miniPaintPresetButton);
    addAndMakeVisible(methodDrawPresetButton);
    svgEditPresetButton.onClick = [this]
    {
        loadVectorPreset("https://svg-edit.github.io/svgedit/releases/latest/svg-editor.html", "SVG-Edit");
    };
    miniPaintPresetButton.onClick = [this]
    {
        loadVectorPreset("https://viliusle.github.io/miniPaint/", "miniPaint");
    };
    methodDrawPresetButton.onClick = [this]
    {
        loadVectorPreset("https://editor.method.ac/", "Method Draw");
    };

    diagnostics.setText("Diagnostics: waiting for first load.", juce::dontSendNotification);
    diagnostics.setJustificationType(juce::Justification::topLeft);
    diagnostics.setMinimumHorizontalScale(0.8f);
    addAndMakeVisible(diagnostics);

    const auto webView2Data = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                  .getChildFile("TwoNistLabs")
                                  .getChildFile("DesignerApp.WebView2");
    webView2Data.createDirectory();

    auto browserOptions = juce::WebBrowserComponent::Options()
                              .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                              .withWinWebView2Options(
                                  juce::WebBrowserComponent::Options::WinWebView2()
                                      .withUserDataFolder(webView2Data)
                                      .withStatusBarDisabled());

    vectorBrowser = std::make_unique<TrackingBrowser>(
        browserOptions,
        [this](const juce::String& url)
        {
            lastVectorUrl = url;
            lastVectorError.clear();
            lastVectorLoadSucceeded = true;
            lastVectorLoadFailed = false;
            runVectorDiagnostics();
        },
        [this](const juce::String& errorInfo)
        {
            lastVectorError = errorInfo;
            lastVectorLoadSucceeded = false;
            lastVectorLoadFailed = true;
            runVectorDiagnostics();
        });

    addAndMakeVisible(*vectorBrowser);
    vectorBrowser->goToURL(vectorAddress.getText());
    runVectorDiagnostics();

#if FOLEYS_SHOW_GUI_EDITOR_PALLETTE
    magicBuilder.attachToolboxToWindow(layoutCanvas);
#endif

    setSize(1360, 860);
}

DesignerMainComponent::~DesignerMainComponent()
{
    saveToolbarLayout();
}

void DesignerMainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withBrightness(0.15f));
}

void DesignerMainComponent::resized()
{
    auto area = getLocalBounds();
    auto topArea = area.removeFromTop(86).reduced(8, 6);
    toolbar.setBounds(topArea.removeFromTop(54));
    status.setBounds(topArea.reduced(4, 0));

    auto content = area.reduced(8);
    auto layoutArea = content;
    if (vectorPanelVisible)
    {
        auto vectorPanel = content.removeFromRight(460);
        vectorHeader.setVisible(true);
        vectorAddress.setVisible(true);
        svgEditPresetButton.setVisible(true);
        miniPaintPresetButton.setVisible(true);
        methodDrawPresetButton.setVisible(true);
        diagnostics.setVisible(true);
        vectorBrowser->setVisible(true);

        auto vectorTop = vectorPanel.removeFromTop(28);
        vectorHeader.setBounds(vectorTop);
        auto presetRow = vectorPanel.removeFromTop(28);
        svgEditPresetButton.setBounds(presetRow.removeFromLeft(96));
        miniPaintPresetButton.setBounds(presetRow.removeFromLeft(96));
        methodDrawPresetButton.setBounds(presetRow.removeFromLeft(110));
        auto vectorControls = vectorPanel.removeFromTop(32);
        vectorAddress.setBounds(vectorControls.removeFromLeft(360));
        auto diagnosticsArea = vectorPanel.removeFromTop(84);
        diagnostics.setBounds(diagnosticsArea);
        vectorPanel.removeFromTop(6);
        vectorBrowser->setBounds(vectorPanel);
    }
    else
    {
        vectorHeader.setVisible(false);
        vectorAddress.setVisible(false);
        svgEditPresetButton.setVisible(false);
        miniPaintPresetButton.setVisible(false);
        methodDrawPresetButton.setVisible(false);
        diagnostics.setVisible(false);
        vectorBrowser->setVisible(false);
    }

    layoutCanvas.setBounds(layoutArea);
    magicBuilder.updateLayout(layoutCanvas.getLocalBounds());
}

juce::ApplicationCommandTarget* DesignerMainComponent::getNextCommandTarget()
{
    return nullptr;
}

void DesignerMainComponent::getAllCommands(juce::Array<juce::CommandID>& commands)
{
    commands.addArray({
        cmdNewLayout,
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
    });
}

void DesignerMainComponent::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case cmdNewLayout:
            result.setInfo("New Layout", "Create a new empty GUI layout", "Layout", 0);
            result.addDefaultKeypress('n', juce::ModifierKeys::commandModifier);
            break;
        case cmdLoadXml:
            result.setInfo("Load XML", "Load GUI layout XML", "Layout", 0);
            result.addDefaultKeypress('o', juce::ModifierKeys::commandModifier);
            break;
        case cmdSaveXml:
            result.setInfo("Save XML", "Save GUI layout XML", "Layout", 0);
            result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier);
            break;
        case cmdExportForPlugin:
            result.setInfo("Export Layout", "Export GUI layout for plugin runtime", "Layout", 0);
            result.addDefaultKeypress('e', juce::ModifierKeys::commandModifier);
            break;
        case cmdToggleEditMode:
            result.setInfo("Edit Mode", "Toggle interactive GUI edit mode", "Layout", 0);
            result.setTicked(editMode);
            break;
        case cmdToggleVectorPanel:
            result.setInfo("Vector Panel", "Show or hide vector editor panel", "View", 0);
            result.setTicked(vectorPanelVisible);
            break;
        case cmdLoadVectorUrl:
            result.setInfo("Load URL", "Load vector tool from address bar", "Vector", 0);
            break;
        case cmdLoadVectorFile:
            result.setInfo("Load Local HTML", "Load local HTML vector tool", "Vector", 0);
            break;
        case cmdDiagnoseVector:
            result.setInfo("Diagnose", "Run vector embed diagnostics", "Vector", 0);
            break;
        case cmdOpenExternal:
            result.setInfo("Open External", "Open current vector URL in system browser", "Vector", 0);
            break;
        case cmdLoadSvgEditPreset:
            result.setInfo("SVG-Edit", "Load SVG-Edit preset", "Vector Preset", 0);
            break;
        case cmdLoadMiniPaintPreset:
            result.setInfo("miniPaint", "Load miniPaint preset", "Vector Preset", 0);
            break;
        case cmdLoadMethodDrawPreset:
            result.setInfo("Method Draw", "Load Method Draw preset", "Vector Preset", 0);
            break;
        case cmdCustomizeToolbar:
            result.setInfo("Customize", "Customize toolbar items and style", "Toolbar", 0);
            break;
        case cmdResetToolbar:
            result.setInfo("Reset Toolbar", "Reset toolbar to default layout", "Toolbar", 0);
            break;
        default:
            break;
    }
}

bool DesignerMainComponent::perform(const juce::ApplicationCommandTarget::InvocationInfo& info)
{
    switch (info.commandID)
    {
        case cmdNewLayout:
            createNewLayout();
            return true;
        case cmdLoadXml:
            loadLayoutFromFile();
            return true;
        case cmdSaveXml:
            saveLayoutToFile();
            return true;
        case cmdExportForPlugin:
            exportLayoutForPlugin();
            return true;
        case cmdToggleEditMode:
            toggleEditMode();
            return true;
        case cmdToggleVectorPanel:
            toggleVectorPanel();
            return true;
        case cmdLoadVectorUrl:
            loadVectorEditorFromAddress();
            return true;
        case cmdLoadVectorFile:
            loadVectorEditorFromFile();
            return true;
        case cmdDiagnoseVector:
            runVectorDiagnostics();
            updateStatus("Vector diagnostics updated.");
            return true;
        case cmdOpenExternal:
        {
            const auto url = vectorAddress.getText().trim();
            if (url.isEmpty())
            {
                updateStatus("Vector URL is empty.");
                return true;
            }
            juce::URL(url).launchInDefaultBrowser();
            updateStatus("Opened external browser.");
            return true;
        }
        case cmdLoadSvgEditPreset:
            loadVectorPreset("https://svg-edit.github.io/svgedit/releases/latest/svg-editor.html", "SVG-Edit");
            return true;
        case cmdLoadMiniPaintPreset:
            loadVectorPreset("https://viliusle.github.io/miniPaint/", "miniPaint");
            return true;
        case cmdLoadMethodDrawPreset:
            loadVectorPreset("https://editor.method.ac/", "Method Draw");
            return true;
        case cmdCustomizeToolbar:
            toolbar.showCustomisationDialog(*toolbarFactory, juce::Toolbar::allCustomisationOptionsEnabled);
            updateStatus("Toolbar customization dialog opened.");
            return true;
        case cmdResetToolbar:
            toolbar.addDefaultItems(*toolbarFactory);
            saveToolbarLayout();
            updateStatus("Toolbar reset to defaults.");
            return true;
        default:
            break;
    }

    return false;
}

void DesignerMainComponent::initialiseToolbar()
{
    toolbarFactory = std::make_unique<DesignerToolbarFactory>(*this);
    toolbar.setStyle(juce::Toolbar::iconsWithText);
    addAndMakeVisible(toolbar);
    loadToolbarLayout();
}

void DesignerMainComponent::saveToolbarLayout() const
{
    toolbarLayoutFile.replaceWithText(toolbar.toString());
}

void DesignerMainComponent::loadToolbarLayout()
{
    const auto layout = toolbarLayoutFile.existsAsFile() ? toolbarLayoutFile.loadFileAsString() : juce::String();
    if (layout.isNotEmpty() && toolbar.restoreFromString(*toolbarFactory, layout))
        return;

    toolbar.addDefaultItems(*toolbarFactory);
}

std::unique_ptr<juce::Drawable> DesignerMainComponent::createToolbarIcon(int itemId) const
{
    auto drawable = std::make_unique<juce::DrawablePath>();
    juce::Path path;
    const auto s = 48.0f;

    switch (itemId)
    {
        case cmdNewLayout:
            path.addRectangle(6.0f, 6.0f, 36.0f, 36.0f);
            path.addLineSegment({ 24.0f, 12.0f, 24.0f, 36.0f }, 5.0f);
            path.addLineSegment({ 12.0f, 24.0f, 36.0f, 24.0f }, 5.0f);
            break;
        case cmdLoadXml:
            path.addRectangle(8.0f, 8.0f, 32.0f, 32.0f);
            path.addLineSegment({ 12.0f, 20.0f, 24.0f, 32.0f }, 4.0f);
            path.addLineSegment({ 24.0f, 32.0f, 36.0f, 20.0f }, 4.0f);
            break;
        case cmdSaveXml:
            path.addRectangle(8.0f, 8.0f, 32.0f, 32.0f);
            path.addRectangle(14.0f, 10.0f, 20.0f, 12.0f);
            path.addRectangle(14.0f, 28.0f, 20.0f, 10.0f);
            break;
        case cmdExportForPlugin:
            path.addEllipse(8.0f, 8.0f, 32.0f, 32.0f);
            path.addLineSegment({ 24.0f, 12.0f, 24.0f, 30.0f }, 4.0f);
            path.addLineSegment({ 18.0f, 24.0f, 24.0f, 30.0f }, 4.0f);
            path.addLineSegment({ 30.0f, 24.0f, 24.0f, 30.0f }, 4.0f);
            break;
        case cmdToggleEditMode:
            path.addRectangle(9.0f, 32.0f, 30.0f, 8.0f);
            path.addLineSegment({ 10.0f, 30.0f, 30.0f, 10.0f }, 5.0f);
            break;
        case cmdToggleVectorPanel:
            path.addRectangle(6.0f, 10.0f, 36.0f, 28.0f);
            path.addLineSegment({ 20.0f, 10.0f, 20.0f, 38.0f }, 2.0f);
            break;
        case cmdLoadVectorUrl:
            path.addEllipse(6.0f, 6.0f, 36.0f, 36.0f);
            path.addLineSegment({ 6.0f, 24.0f, 42.0f, 24.0f }, 2.5f);
            path.addLineSegment({ 24.0f, 6.0f, 24.0f, 42.0f }, 2.5f);
            break;
        case cmdLoadVectorFile:
            path.addRectangle(10.0f, 8.0f, 28.0f, 34.0f);
            path.addTriangle(30.0f, 8.0f, 38.0f, 16.0f, 30.0f, 16.0f);
            break;
        case cmdDiagnoseVector:
            path.addEllipse(10.0f, 10.0f, 28.0f, 28.0f);
            path.addLineSegment({ 24.0f, 16.0f, 24.0f, 26.0f }, 4.0f);
            path.addEllipse(22.0f, 30.0f, 4.0f, 4.0f);
            break;
        case cmdOpenExternal:
            path.addRectangle(8.0f, 12.0f, 28.0f, 28.0f);
            path.addLineSegment({ 20.0f, 28.0f, 36.0f, 12.0f }, 4.0f);
            path.addLineSegment({ 28.0f, 12.0f, 36.0f, 12.0f }, 4.0f);
            path.addLineSegment({ 36.0f, 12.0f, 36.0f, 20.0f }, 4.0f);
            break;
        case cmdLoadSvgEditPreset:
        case cmdLoadMiniPaintPreset:
        case cmdLoadMethodDrawPreset:
            path.addStar({ 24.0f, 24.0f }, 5, 8.0f, 16.0f, 0.0f);
            break;
        case cmdCustomizeToolbar:
            path.addEllipse(10.0f, 10.0f, 28.0f, 28.0f);
            path.addLineSegment({ 24.0f, 6.0f, 24.0f, 42.0f }, 3.0f);
            path.addLineSegment({ 6.0f, 24.0f, 42.0f, 24.0f }, 3.0f);
            break;
        case cmdResetToolbar:
            path.startNewSubPath(14.0f, 14.0f);
            path.lineTo(34.0f, 14.0f);
            path.lineTo(34.0f, 34.0f);
            path.lineTo(14.0f, 34.0f);
            path.closeSubPath();
            path.addLineSegment({ 14.0f, 24.0f, 8.0f, 24.0f }, 4.0f);
            path.addLineSegment({ 8.0f, 24.0f, 12.0f, 20.0f }, 4.0f);
            path.addLineSegment({ 8.0f, 24.0f, 12.0f, 28.0f }, 4.0f);
            break;
        default:
            path.addRectangle(10.0f, 10.0f, 28.0f, 28.0f);
            break;
    }

    auto transform = path.getTransformToScaleToFit(0.0f, 0.0f, s, s, true);
    path.applyTransform(transform);
    drawable->setPath(path);
    drawable->setFill(juce::Colours::whitesmoke);
    drawable->setStrokeFill(juce::Colours::whitesmoke);
    drawable->setStrokeType(juce::PathStrokeType(1.0f));
    return drawable;
}

void DesignerMainComponent::createNewLayout()
{
    magicBuilder.clearGUI();
    magicBuilder.setEditMode(true);
    editMode = true;
    commandManager.commandStatusChanged();
    updateStatus("New empty layout created.");
}

void DesignerMainComponent::loadLayoutFromFile()
{
    fileChooser = std::make_unique<juce::FileChooser>("Load GUI XML", lastFile, "*.xml");
    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            fileChooser.reset();
            if (file == juce::File())
                return;

            auto xmlText = file.loadFileAsString();
            auto tree = juce::ValueTree::fromXml(xmlText);
            if (!tree.isValid())
            {
                updateStatus("Failed to load XML.");
                return;
            }

            lastFile = file;
            magicState.setGuiValueTree(tree);
            magicBuilder.updateComponents();
            updateStatus("Loaded: " + file.getFullPathName());
        });
}

void DesignerMainComponent::saveLayoutToFile()
{
    fileChooser = std::make_unique<juce::FileChooser>("Save GUI XML", lastFile, "*.xml");
    fileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
        [this](const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            fileChooser.reset();
            if (file == juce::File())
                return;

            if (!file.hasFileExtension(".xml"))
                file = file.withFileExtension(".xml");

            if (auto xml = magicState.getGuiTree().createXml())
            {
                file.replaceWithText(xml->toString());
                lastFile = file;
                updateStatus("Saved: " + file.getFullPathName());
            }
        });
}

void DesignerMainComponent::exportLayoutForPlugin()
{
    auto exportFile = juce::File::getCurrentWorkingDirectory()
                          .getChildFile("resources")
                          .getChildFile("designer_layout.xml");
    exportFile.getParentDirectory().createDirectory();

    if (auto xml = magicState.getGuiTree().createXml())
    {
        exportFile.replaceWithText(xml->toString());
        updateStatus("Exported plugin layout: " + exportFile.getFullPathName());
    }
}

void DesignerMainComponent::toggleEditMode()
{
    editMode = !editMode;
    magicBuilder.setEditMode(editMode);
    commandManager.commandStatusChanged();
    updateStatus(editMode ? "Edit mode enabled." : "Edit mode disabled.");
}

void DesignerMainComponent::updateStatus(const juce::String& text)
{
    status.setText(text, juce::dontSendNotification);
}

void DesignerMainComponent::loadVectorEditorFromAddress()
{
    const auto url = vectorAddress.getText().trim();
    if (url.isEmpty())
    {
        updateStatus("Vector URL is empty.");
        return;
    }

    vectorBrowser->goToURL(url);
    lastVectorLoadSucceeded = false;
    lastVectorLoadFailed = false;
    lastVectorError.clear();
    runVectorDiagnostics();
    updateStatus("Loaded vector editor URL (use Open External if embed fails).");
}

void DesignerMainComponent::loadVectorEditorFromFile()
{
    fileChooser = std::make_unique<juce::FileChooser>("Load Vector Editor HTML", juce::File(), "*.html;*.htm");
    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            fileChooser.reset();
            if (file == juce::File())
                return;

            vectorAddress.setText(file.getFullPathName(), juce::dontSendNotification);
            vectorBrowser->goToURL(juce::URL(file).toString(true));
            lastVectorLoadSucceeded = false;
            lastVectorLoadFailed = false;
            lastVectorError.clear();
            runVectorDiagnostics();
            updateStatus("Loaded local vector editor: " + file.getFileName());
        });
}

void DesignerMainComponent::toggleVectorPanel()
{
    vectorPanelVisible = !vectorPanelVisible;
    commandManager.commandStatusChanged();
    resized();
    updateStatus(vectorPanelVisible ? "Vector panel enabled." : "Vector panel hidden.");
}

void DesignerMainComponent::loadVectorPreset(const juce::String& url, const juce::String& label)
{
    vectorAddress.setText(url, juce::dontSendNotification);
    vectorBrowser->goToURL(url);
    lastVectorLoadSucceeded = false;
    lastVectorLoadFailed = false;
    lastVectorError.clear();
    runVectorDiagnostics();
    updateStatus("Loaded vector preset: " + label + " (use Open External if embed fails).");
}

void DesignerMainComponent::runVectorDiagnostics()
{
    const auto rawAddress = vectorAddress.getText().trim();
    const auto addressLooksLikeWeb = juce::URL::isProbablyAWebsiteURL(rawAddress);
    const auto url = juce::URL(rawAddress);
    const auto urlWellFormed = url.isWellFormed();
    const auto localFile = juce::File(rawAddress);
    const auto localHtmlExists = localFile.existsAsFile();

    const auto defaultSupported = juce::WebBrowserComponent::areOptionsSupported(
        juce::WebBrowserComponent::Options());
    const auto webView2Supported = juce::WebBrowserComponent::areOptionsSupported(
        juce::WebBrowserComponent::Options().withBackend(juce::WebBrowserComponent::Options::Backend::webview2));

    juce::StringArray lines;
    lines.add("Diagnostics");
    lines.add("Default browser backend: " + juce::String(defaultSupported ? "supported" : "unavailable"));
    lines.add("WebView2 backend: " + juce::String(webView2Supported ? "supported" : "not supported"));
    lines.add("Address looks like URL: " + juce::String(addressLooksLikeWeb ? "yes" : "no"));
    lines.add("Address well formed: " + juce::String(urlWellFormed ? "yes" : "no"));
    lines.add("Local file exists: " + juce::String(localHtmlExists ? "yes" : "no"));

    if (lastVectorLoadSucceeded)
        lines.add("Last page loaded: " + (lastVectorUrl.isNotEmpty() ? lastVectorUrl : rawAddress));
    else if (lastVectorLoadFailed)
        lines.add("Last load error: " + (lastVectorError.isNotEmpty() ? lastVectorError : "unknown error"));
    else
        lines.add("Last load state: pending");

    if (!webView2Supported)
        lines.add("Action: install Microsoft Edge WebView2 Runtime or use Open External.");
    else if (!urlWellFormed && !localHtmlExists)
        lines.add("Action: enter a valid https:// URL or load a local .html file.");
    else if (lastVectorLoadFailed)
        lines.add("Action: click Open External to verify if the site blocks embedding.");
    else
        lines.add("Action: embedded tool should be ready.");

    diagnostics.setText(lines.joinIntoString("\n"), juce::dontSendNotification);
}
