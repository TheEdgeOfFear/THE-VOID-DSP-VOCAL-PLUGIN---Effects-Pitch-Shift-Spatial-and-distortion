#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiManager.h"

class MidiMappingModal : public juce::Component,
                         public juce::TableListBoxModel
{
public:
    MidiMappingModal(MidiManager& mm)
        : midiManager(mm)
    {
        mappings = midiManager.getMappings();

        table.setModel(this);
        table.getHeader().addColumn("Parameter", 1, 240, 150, 400, juce::TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("CC #", 2, 90, 60, 120, juce::TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("Channel", 3, 90, 60, 120, juce::TableHeaderComponent::defaultFlags);
        table.getHeader().addColumn("Action", 4, 100, 80, 150, juce::TableHeaderComponent::defaultFlags);
        addAndMakeVisible(table);

        closeButton.setButtonText("CLOSE");
        closeButton.onClick = [this]() {
            if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                dw->exitModalState(0);
        };
        addAndMakeVisible(closeButton);

        setSize(560, 420);
    }

    int getNumRows() override { return static_cast<int>(mappings.size()); }

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        juce::ignoreUnused(width, height, rowIsSelected);
        if (rowNumber % 2 == 0)
            g.fillAll(juce::Colour(0xff12151b));
        else
            g.fillAll(juce::Colour(0xff181c24));
    }


    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        juce::ignoreUnused(rowIsSelected);
        if (rowNumber < 0 || rowNumber >= static_cast<int>(mappings.size()))
            return;

        const auto& m = mappings[static_cast<size_t>(rowNumber)];
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(13.0f));

        if (columnId == 1)
        {
            g.drawText("  " + juce::String(m.paramName), 0, 0, width, height, juce::Justification::centredLeft, true);
        }
        else if (columnId == 2)
        {
            const juce::String ccStr = (m.ccNumber >= 0) ? juce::String(m.ccNumber) : "---";
            g.drawText(ccStr, 0, 0, width, height, juce::Justification::centred, true);
        }
        else if (columnId == 3)
        {
            const juce::String chStr = (m.channel == 0) ? "Omni" : juce::String(m.channel);
            g.drawText(chStr, 0, 0, width, height, juce::Justification::centred, true);
        }
    }

    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override
    {
        juce::ignoreUnused(isRowSelected);
        if (columnId == 4 && rowNumber >= 0 && rowNumber < static_cast<int>(mappings.size()))
        {
            auto* btn = dynamic_cast<juce::TextButton*>(existingComponentToUpdate);
            if (btn == nullptr)
                btn = new juce::TextButton("CLEAR");

            const auto paramId = mappings[static_cast<size_t>(rowNumber)].paramId;
            btn->onClick = [this, paramId]() {
                midiManager.clearMapping(paramId);
                mappings = midiManager.getMappings();
                table.updateContent();
                table.repaint();
            };
            return btn;
        }

        delete existingComponentToUpdate;
        return nullptr;
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(12);
        closeButton.setBounds(bounds.removeFromBottom(34).withSizeKeepingCentre(120, 30));
        bounds.removeFromBottom(10);
        table.setBounds(bounds);
    }

private:
    MidiManager& midiManager;
    std::vector<MidiMapping> mappings;
    juce::TableListBox table;
    juce::TextButton closeButton;
};
