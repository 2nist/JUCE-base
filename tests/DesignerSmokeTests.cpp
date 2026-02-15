#include <juce_gui_extra/juce_gui_extra.h>

#include "apps/designer/MainComponent.h"

#include <iostream>

int main()
{
    juce::ScopedJuceInitialiser_GUI guiInit;

    try
    {
        DesignerMainComponent component;
        component.setSize(1280, 820);
        component.resized();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Designer smoke test failed: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Designer smoke test failed with unknown exception." << std::endl;
        return 1;
    }

    std::cout << "Designer smoke tests passed." << std::endl;
    return 0;
}
