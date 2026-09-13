#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include <functional>

class VoidLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VoidLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff080a0d));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff12151b));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff3f1016));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff12151b));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff7d0b16));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff161a22));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xd0080a0e));
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff3f1016));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        juce::ignoreUnused(slider);

        // Reserve bottom area for textbox cleanly
        const float tbHeight = 18.0f;
        const float availHeight = static_cast<float>(height) - tbHeight;
        const float diameter = std::min(static_cast<float>(width), availHeight) - 8.0f;
        const float radius = diameter * 0.5f;
        const float cx = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        const float cy = static_cast<float>(y) + radius + 3.0f;

        // Outer chassis drop shadow
        g.setColour(juce::Colour(0xff020304));
        g.fillEllipse(cx - radius - 3.0f, cy - radius - 1.0f, (radius + 3.0f) * 2.0f, (radius + 3.0f) * 2.0f);

        // Heavy-duty knurled outer rim
        const int numGrips = 28;
        g.setColour(juce::Colour(0xff181a21));
        for (int i = 0; i < numGrips; ++i)
        {
            const float angle = static_cast<float>(i) * (juce::MathConstants<float>::twoPi / static_cast<float>(numGrips));
            const float gx = cx + (radius + 1.2f) * std::cos(angle);
            const float gy = cy + (radius + 1.2f) * std::sin(angle);
            g.fillEllipse(gx - 1.8f, gy - 1.8f, 3.6f, 3.6f);
        }

        // Outer bezel ring
        juce::ColourGradient bezelGrad(juce::Colour(0xff4a5160), cx, cy - radius,
                                       juce::Colour(0xff161820), cx, cy + radius, false);
        g.setGradientFill(bezelGrad);
        g.fillEllipse(cx - radius, cy - radius, diameter, diameter);

        g.setColour(juce::Colour(0xff687384));
        g.drawEllipse(cx - radius, cy - radius, diameter, diameter, 1.2f);

        // Metallic dish cap
        const float capRadius = radius - 5.0f;
        juce::ColourGradient capGrad(juce::Colour(0xff22262f), cx, cy - capRadius,
                                     juce::Colour(0xff0b0d11), cx, cy + capRadius, false);
        g.setGradientFill(capGrad);
        g.fillEllipse(cx - capRadius, cy - capRadius, capRadius * 2.0f, capRadius * 2.0f);

        // Circular machined micro-grooves
        g.setColour(juce::Colour(0x1fffffff));
        g.drawEllipse(cx - capRadius + 3.0f, cy - capRadius + 3.0f, (capRadius - 3.0f) * 2.0f, (capRadius - 3.0f) * 2.0f, 0.8f);
        g.drawEllipse(cx - capRadius + 7.0f, cy - capRadius + 7.0f, (capRadius - 7.0f) * 2.0f, (capRadius - 7.0f) * 2.0f, 0.8f);

        // Crimson value arc track
        const float trackRadius = radius - 2.5f;
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(cx, cy, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff180508));
        g.strokePath(backgroundArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        juce::Path valueArc;
        valueArc.addCentredArc(cx, cy, trackRadius, trackRadius, 0.0f, rotaryStartAngle, currentAngle, true);
        g.setColour(juce::Colour(0xffff1e30));
        g.strokePath(valueArc, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // High-contrast pointer needle with red tip
        const float needleLength = capRadius - 2.0f;
        const float pX = cx + needleLength * std::sin(currentAngle);
        const float pY = cy - needleLength * std::cos(currentAngle);
        const float inX = cx + (needleLength * 0.35f) * std::sin(currentAngle);
        const float inY = cy - (needleLength * 0.35f) * std::cos(currentAngle);

        juce::Path needle;
        needle.startNewSubPath(inX, inY);
        needle.lineTo(pX, pY);
        g.setColour(juce::Colours::white);
        g.strokePath(needle, juce::PathStrokeType(2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(juce::Colour(0xffff1e30));
        g.fillEllipse(pX - 2.0f, pY - 2.0f, 4.0f, 4.0f);
    }
};

// Rotary Slider supporting right-click for MIDI Learn and double-click reset
class VoidKnob : public juce::Slider
{
public:
    explicit VoidKnob(double defaultVal = 0.5)
        : defaultValue(defaultVal)
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 16);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xd0080a0e));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff3f1016));
    }

    void mouseDoubleClick(const juce::MouseEvent& e) override
    {
        juce::ignoreUnused(e);
        setValue(defaultValue, juce::sendNotification);
    }

    void setDefaultResetValue(double val) { defaultValue = val; }

    std::function<void(const juce::MouseEvent&)> onRightClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }
        juce::Slider::mouseDown(e);
    }

private:
    double defaultValue = 0.5;
};

// Multi-Segment Selector Switch (e.g. 3-way, 4-way discrete positions)
class VoidSegmentSwitch : public juce::Component
{
public:
    VoidSegmentSwitch(const juce::StringArray& optionLabels)
        : labels(optionLabels)
    {
    }

    int getSelectedIndex() const { return selectedIndex; }

    void setSelectedIndex(int idx, juce::NotificationType notify = juce::sendNotification)
    {
        idx = juce::jlimit(0, labels.size() - 1, idx);
        if (selectedIndex != idx)
        {
            selectedIndex = idx;
            repaint();
            if (notify == juce::sendNotification && onSelectionChange)
                onSelectionChange(selectedIndex);
        }
    }

    std::function<void(int newIndex)> onSelectionChange;
    std::function<void(const juce::MouseEvent&)> onRightClick;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const int numSegments = labels.size();
        if (numSegments == 0) return;

        // Recessed base slot plate
        g.setColour(juce::Colour(0xff090b0e));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xff3f1016));
        g.drawRoundedRectangle(bounds, 4.0f, 1.2f);

        const float segW = bounds.getWidth() / static_cast<float>(numSegments);
        const float segH = bounds.getHeight();

        for (int i = 0; i < numSegments; ++i)
        {
            auto segRect = juce::Rectangle<float>(bounds.getX() + static_cast<float>(i) * segW, bounds.getY(), segW, segH);
            const bool isSelected = (i == selectedIndex);

            if (isSelected)
            {
                // Glowing crimson active segment
                juce::ColourGradient selGrad(juce::Colour(0xffff2233), 0, segRect.getY(),
                                             juce::Colour(0xff880a14), 0, segRect.getBottom(), false);
                g.setGradientFill(selGrad);
                g.fillRoundedRectangle(segRect.reduced(1.5f), 3.0f);
                g.setColour(juce::Colour(0xffff8899));
                g.drawRoundedRectangle(segRect.reduced(1.5f), 3.0f, 1.0f);
            }
            else
            {
                // Subtle divider lines between unselected items
                if (i > 0 && (i - 1) != selectedIndex)
                {
                    g.setColour(juce::Colour(0xff222834));
                    g.drawVerticalLine(static_cast<int>(segRect.getX()), segRect.getY() + 3.0f, segRect.getBottom() - 3.0f);
                }
            }

            g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
            g.setColour(isSelected ? juce::Colours::white : juce::Colour(0xff8a94a6));
            g.drawText(labels[i], segRect, juce::Justification::centred);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }

        const int numSegments = labels.size();
        if (numSegments <= 0) return;

        const float segW = static_cast<float>(getWidth()) / static_cast<float>(numSegments);
        int clickedIdx = juce::jlimit(0, numSegments - 1, static_cast<int>(static_cast<float>(e.x) / segW));
        setSelectedIndex(clickedIdx, juce::sendNotification);
    }

private:
    juce::StringArray labels;
    int selectedIndex = 0;
};

// Industrial Toggle Button with Illuminated Jewel LED
class VoidToggleButton : public juce::Component
{
public:
    VoidToggleButton(const juce::String& buttonText)
        : text(buttonText)
    {
    }

    void setToggleState(bool state, juce::NotificationType notify = juce::sendNotification)
    {
        if (isToggled != state)
        {
            isToggled = state;
            repaint();
            if (notify == juce::sendNotification && onStateChange)
                onStateChange(isToggled);
        }
    }

    bool getToggleState() const { return isToggled; }

    std::function<void(bool state)> onStateChange;
    std::function<void(const juce::MouseEvent&)> onRightClick;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Button background plate
        juce::ColourGradient bgGrad(isToggled ? juce::Colour(0xff221014) : juce::Colour(0xff14171f), 0, bounds.getY(),
                                    isToggled ? juce::Colour(0xff12080a) : juce::Colour(0xff0b0d12), 0, bounds.getBottom(), false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(isToggled ? juce::Colour(0xffff2233) : juce::Colour(0xff3f1016));
        g.drawRoundedRectangle(bounds, 4.0f, 1.2f);

        // LED Indicator on the left
        const float ledX = bounds.getX() + 10.0f;
        const float ledY = bounds.getCentreY();
        const float ledR = 4.5f;

        // LED Bezel
        g.setColour(juce::Colour(0xff1a1d24));
        g.fillEllipse(ledX - ledR - 1.5f, ledY - ledR - 1.5f, (ledR + 1.5f) * 2.0f, (ledR + 1.5f) * 2.0f);
        g.setColour(juce::Colour(0xff464d5c));
        g.drawEllipse(ledX - ledR - 1.5f, ledY - ledR - 1.5f, (ledR + 1.5f) * 2.0f, (ledR + 1.5f) * 2.0f, 1.0f);

        if (isToggled)
        {
            // Glowing LED
            g.setColour(juce::Colour(0x66ff1e2e));
            g.fillEllipse(ledX - ledR - 4.0f, ledY - ledR - 4.0f, (ledR + 4.0f) * 2.0f, (ledR + 4.0f) * 2.0f);

            juce::ColourGradient ledGrad(juce::Colour(0xffff6677), ledX - 1.5f, ledY - 1.5f,
                                         juce::Colour(0xffcc1122), ledX + ledR, ledY + ledR, true);
            g.setGradientFill(ledGrad);
            g.fillEllipse(ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);
        }
        else
        {
            // Unlit dark LED
            g.setColour(juce::Colour(0xff2d060a));
            g.fillEllipse(ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);
        }

        // Button label text
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(isToggled ? juce::Colours::white : juce::Colour(0xff8a94a6));
        g.drawText(text, juce::Rectangle<float>(ledX + ledR + 8.0f, bounds.getY(), bounds.getWidth() - ledX - ledR - 12.0f, bounds.getHeight()),
                   juce::Justification::centredLeft);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }
        setToggleState(!isToggled, juce::sendNotification);
    }

private:
    juce::String text;
    bool isToggled = false;
};

// Heavy-Duty Stomp Footswitch with animated depression, radiant Jewel LED, and smart tap/hold
class VoidFootswitchComponent : public juce::Component
{
public:
    VoidFootswitchComponent(const juce::String& labelText)
        : name(labelText)
    {
    }

    void setLedActive(bool active)
    {
        if (isLedOn != active)
        {
            isLedOn = active;
            repaint();
        }
    }

    bool getLedActive() const { return isLedOn; }

    std::function<void(bool isDown)> onPointerState;
    std::function<void()> onClick;
    std::function<void(const juce::MouseEvent&)> onRightClick;

    void paint(juce::Graphics& g) override
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f - 14.0f;
        const float r = 32.0f;

        // Outer chassis shadow
        g.setColour(juce::Colour(0xff020305));
        g.fillEllipse(cx - r - 4.0f, cy - r - 2.0f, (r + 4.0f) * 2.0f, (r + 4.0f) * 2.0f);

        // Machined stainless steel hex nut bezel
        const int numSides = 6;
        juce::Path hexPath;
        const float hexRadius = r + 2.0f;
        for (int i = 0; i < numSides; ++i)
        {
            const float angle = static_cast<float>(i) * (juce::MathConstants<float>::twoPi / 6.0f);
            const float px = cx + hexRadius * std::cos(angle);
            const float py = cy + hexRadius * std::sin(angle);
            if (i == 0) hexPath.startNewSubPath(px, py);
            else        hexPath.lineTo(px, py);
        }
        hexPath.closeSubPath();

        juce::ColourGradient nutGrad(juce::Colour(0xff656d7d), cx - hexRadius, cy - hexRadius,
                                     juce::Colour(0xff1f2228), cx + hexRadius, cy + hexRadius, false);
        g.setGradientFill(nutGrad);
        g.fillPath(hexPath);
        g.setColour(juce::Colour(0xff838c9c));
        g.strokePath(hexPath, juce::PathStrokeType(1.5f));

        // Circular actuator plunger (depresses visually when pressed)
        const float plungerR = isPressed ? (r - 7.0f) : (r - 5.0f);
        juce::ColourGradient plungerGrad(isPressed ? juce::Colour(0xff1e2229) : juce::Colour(0xff454c59), cx, cy - plungerR,
                                         isPressed ? juce::Colour(0xff0e1014) : juce::Colour(0xff242830), cx, cy + plungerR, false);
        g.setGradientFill(plungerGrad);
        g.fillEllipse(cx - plungerR, cy - plungerR, plungerR * 2.0f, plungerR * 2.0f);

        g.setColour(juce::Colour(0xff6e7888));
        g.drawEllipse(cx - plungerR, cy - plungerR, plungerR * 2.0f, plungerR * 2.0f, 1.5f);

        // Center machined dimple
        g.setColour(juce::Colour(0xff101216));
        g.fillEllipse(cx - 8.0f, cy - 8.0f, 16.0f, 16.0f);

        // Radiant Jewel LED Indicator above footswitch
        const float ledY = cy - r - 16.0f;
        const float ledR = 7.0f;

        // LED Bezel
        g.setColour(juce::Colour(0xff1a1d24));
        g.fillEllipse(cx - ledR - 2.0f, ledY - ledR - 2.0f, (ledR + 2.0f) * 2.0f, (ledR + 2.0f) * 2.0f);
        g.setColour(juce::Colour(0xff464d5c));
        g.drawEllipse(cx - ledR - 2.0f, ledY - ledR - 2.0f, (ledR + 2.0f) * 2.0f, (ledR + 2.0f) * 2.0f, 1.0f);

        if (isLedOn)
        {
            // Outer radiant aura
            g.setColour(juce::Colour(0x66ff1e2e));
            g.fillEllipse(cx - ledR - 6.0f, ledY - ledR - 6.0f, (ledR + 6.0f) * 2.0f, (ledR + 6.0f) * 2.0f);

            // Glowing lens
            juce::ColourGradient ledGrad(juce::Colour(0xffff5566), cx - 2.0f, ledY - 2.0f,
                                         juce::Colour(0xffcc1122), cx + ledR, ledY + ledR, true);
            g.setGradientFill(ledGrad);
            g.fillEllipse(cx - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);

            // Specular reflection glint
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            g.fillEllipse(cx - 3.0f, ledY - 4.0f, 4.0f, 3.0f);
        }
        else
        {
            // Inactive unlit dark jewel
            g.setColour(juce::Colour(0xff2d060a));
            g.fillEllipse(cx - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);
        }

        // Sleek badge pill label below footswitch to protect against background graphic clashes
        const float badgeW = 100.0f;
        const float badgeH = 22.0f;
        const float badgeX = (static_cast<float>(getWidth()) - badgeW) * 0.5f;
        const float badgeY = static_cast<float>(getHeight()) - badgeH - 2.0f;
        juce::Rectangle<float> badgeRect(badgeX, badgeY, badgeW, badgeH);

        g.setColour(juce::Colour(0xe6080a0e));
        g.fillRoundedRectangle(badgeRect, 4.0f);
        g.setColour(isLedOn ? juce::Colour(0xffff1e30) : juce::Colour(0xff3f1016));
        g.drawRoundedRectangle(badgeRect, 4.0f, 1.2f);

        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(isLedOn ? juce::Colours::white : juce::Colour(0xff9ca3b0));
        g.drawText(name, badgeRect, juce::Justification::centred);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            if (onRightClick)
            {
                onRightClick(e);
                return;
            }
        }

        isPressed = true;
        pressTimestamp = juce::Time::currentTimeMillis();
        repaint();
        if (onPointerState)
            onPointerState(true);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        juce::ignoreUnused(e);
        if (isPressed)
        {
            isPressed = false;
            const juce::int64 duration = juce::Time::currentTimeMillis() - pressTimestamp;
            repaint();
            if (onPointerState)
                onPointerState(false);
            if (onClick && duration < 280)
                onClick();
        }
    }

private:
    juce::String name;
    bool isPressed = false;
    bool isLedOn = false;
    juce::int64 pressTimestamp = 0;
};
