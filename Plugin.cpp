#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <cmath>

namespace
{
struct Control
{
    const char* id;
    const char* name;
    float minimum, maximum, initial;
};

constexpr float gateOff = -81.0f;
constexpr size_t gateIndex = 6;

constexpr std::array<Control, 7> controls {{
    {"gain", "Gain (dB)", 12, 60, 42},
    {"tight", "Tight (%)", 0, 100, 62},
    {"bass", "Bass (dB)", -12, 12, 2},
    {"mid", "Mid (dB)", -12, 12, -2},
    {"treble", "Treble (dB)", -12, 12, 1},
    {"level", "Level (dB)", -30, 6, -10},
    {"gate", "Gate (dB)", gateOff, -20, gateOff}
}};

using State = juce::AudioProcessorValueTreeState;

State::ParameterLayout makeParameters()
{
    State::ParameterLayout layout;
    for (const auto& control : controls)
    {
        const juce::ParameterID id {control.id, 1};
        juce::AudioParameterFloatAttributes attributes;
        if (juce::StringRef(control.id) == juce::StringRef(controls[gateIndex].id))
        {
            attributes = attributes
                .withStringFromValueFunction([](float value, int)
                {
                    return value <= gateOff ? juce::String("Off") : juce::String(value, 0);
                })
                .withValueFromStringFunction([](const juce::String& text)
                {
                    return text.trim().equalsIgnoreCase("Off") ? gateOff : text.getFloatValue();
                });
        }
        layout.add(std::make_unique<juce::AudioParameterFloat>(id, control.name,
            juce::NormalisableRange<float>{control.minimum, control.maximum, 1.0f},
            control.initial, attributes));
    }
    return layout;
}

class LindsayEditor final : public juce::AudioProcessorEditor
{
public:
    LindsayEditor(juce::AudioProcessor& processor, State& state) : AudioProcessorEditor(processor)
    {
        for (size_t i = 0; i < knobs.size(); ++i)
        {
            const auto& control = controls[i];
            auto& knob = knobs[i];
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 23);
            knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd9ff6b));
            knob.setName(control.name);
            addAndMakeVisible(knob);
            knobLinks[i] = std::make_unique<State::SliderAttachment>(state, control.id, knob);
        }
        setSize(920, 264);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff171324));
        g.setColour(juce::Colours::white);
        g.setFont(26.0f);
        g.drawText("Lindsay Low-Hands", 20, 12, 580, 38, juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff302541));
        g.fillRoundedRectangle(20, 60, 880, 184, 12);
        g.setColour(juce::Colours::white);
        g.setFont(17.0f);
        g.drawText("Metal distortion", 34, 67, 120, 25, juce::Justification::centredLeft);
        g.saveState();
        g.addTransform(juce::AffineTransform::scale(0.58f).translated(855.0f, 5.0f));
        juce::Path sole;
        sole.startNewSubPath(44, 16);
        sole.cubicTo(60, 12, 70, 27, 64, 40);
        sole.cubicTo(59, 51, 49, 57, 48, 73);
        sole.cubicTo(47, 92, 18, 95, 15, 76);
        sole.cubicTo(12, 64, 25, 55, 23, 44);
        sole.cubicTo(21, 30, 30, 18, 44, 16);
        sole.closeSubPath();
        g.setColour(juce::Colour(0xffff9b91));
        g.fillPath(sole);
        for (int toe = 0; toe < 5; ++toe)
        {
            const auto size = 16.0f - 1.8f * static_cast<float>(toe);
            g.fillEllipse(52.0f - 10.3f * static_cast<float>(toe),
                          1.0f + 3.4f * static_cast<float>(toe), size, size);
        }
        g.setColour(juce::Colour(0xff541e42));
        g.fillEllipse(36, 39, 5, 7);
        g.fillEllipse(48, 36, 5, 7);
        juce::Path grin;
        grin.startNewSubPath(37, 51);
        grin.quadraticTo(46, 59, 54, 48);
        g.strokePath(grin, juce::PathStrokeType(2.6f));
        g.restoreState();
        g.setColour(juce::Colours::white);
        g.setFont(13.0f);
        for (size_t i = 0; i < knobs.size(); ++i)
            g.drawText(controls[i].name, knobX[i], 208, 106, 23, juce::Justification::centred);
    }

    void resized() override
    {
        for (size_t i = 0; i < knobs.size(); ++i)
            knobs[i].setBounds(knobX[i], 102, 106, 103);
    }

private:
    static constexpr std::array<int, 7> knobX {34, 158, 282, 406, 530, 654, 778};
    std::array<juce::Slider, controls.size()> knobs;
    std::array<std::unique_ptr<State::SliderAttachment>, controls.size()> knobLinks;
};

class LindsayDistortion final : public juce::AudioProcessor
{
public:
    LindsayDistortion() : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
        state(*this, nullptr, "LindsayDistortion", makeParameters())
    {
        for (size_t i = 0; i < controls.size(); ++i)
            values[i] = state.getRawParameterValue(controls[i].id);
    }

    const juce::String getName() const override { return "Lindsay Low-Hands"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override { return new LindsayEditor(*this, state); }
    void releaseResources() override {}

    bool isBusesLayoutSupported(const BusesLayout& layout) const override
    {
        const auto output = layout.getMainOutputChannelSet();
        return layout.getMainInputChannelSet() == output
            && (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo());
    }

    void prepareToPlay(double sampleRate, int) override
    {
        rate = static_cast<float>(sampleRate);
        const auto response = [this](float seconds)
        {
            return 1.0f - std::exp(-1.0f / (seconds * rate));
        };
        gateAttack = response(0.001f);
        gateRelease = response(0.035f);
        peakDecay = 1.0f - response(0.020f);
        gateHoldSamples = juce::jmax(1, juce::roundToInt(rate * 0.030f));
        reset();
        for (size_t i = 0; i < parameters.size(); ++i)
        {
            parameters[i].reset(sampleRate, 0.025);
            parameters[i].setCurrentAndTargetValue(values[i]->load());
        }
    }

    void reset() override
    {
        bass.fill(0.0f);
        speaker.fill(0.0f);
        dc.fill(0.0f);
        low.fill(0.0f);
        crossover.fill(0.0f);
        gateGain = values[gateIndex]->load() <= gateOff ? 1.0f : 0.0f;
        gatePeak = 0.0f;
        gateHold = 0;
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals noDenormals;
        const auto coefficient = [this](float hz)
        {
            return 1.0f - std::exp(-juce::MathConstants<float>::twoPi * hz / rate);
        };
        const auto dcCoefficient = coefficient(18.0f);
        const auto lowSplit = coefficient(230.0f);
        const auto highSplit = coefficient(1900.0f);
        const auto speakerCoefficient = coefficient(6200.0f);
        const auto gateDb = values[gateIndex]->load();
        const auto gateEnabled = gateDb > gateOff;
        const auto openThreshold = juce::Decibels::decibelsToGain(gateDb);
        const auto closeThreshold = openThreshold * 0.708f; // 3 dB hysteresis
        for (size_t i = 0; i < parameters.size(); ++i)
            parameters[i].setTargetValue(values[i]->load());

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto drive = juce::Decibels::decibelsToGain(parameters[0].getNextValue());
            const auto tight = parameters[1].getNextValue() * 0.01f;
            const auto lowGain = juce::Decibels::decibelsToGain(parameters[2].getNextValue());
            const auto midGain = juce::Decibels::decibelsToGain(parameters[3].getNextValue());
            const auto highGain = juce::Decibels::decibelsToGain(parameters[4].getNextValue());
            const auto level = juce::Decibels::decibelsToGain(parameters[5].getNextValue());
            const auto bassCoefficient = coefficient(45.0f + 195.0f * tight);

            // A single detector keeps left and right gain aligned for stereo signals.
            if (gateEnabled)
            {
                gatePeak *= peakDecay;
                for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                    gatePeak = juce::jmax(gatePeak, std::abs(buffer.getSample(channel, sample)));

                if (gatePeak >= openThreshold || (gateHold > 0 && gatePeak >= closeThreshold))
                    gateHold = gateHoldSamples;
                else if (gateHold > 0)
                    --gateHold;
            }
            else
            {
                gatePeak = 0.0f;
                gateHold = 0;
            }

            const auto gateTarget = !gateEnabled || gateHold > 0 ? 1.0f : 0.0f;
            const auto smoothing = gateTarget > gateGain ? gateAttack : gateRelease;
            gateGain += smoothing * (gateTarget - gateGain);
            if (std::abs(gateGain - gateTarget) < 1.0e-6f)
                gateGain = gateTarget;

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                const auto input = buffer.getSample(channel, sample) * gateGain;
                bass[channel] += bassCoefficient * (input - bass[channel]);
                const auto driven = (input - bass[channel]) * drive;
                const auto preamp = std::tanh(driven);
                const auto secondStage = std::tanh(3.6f * preamp + 0.23f) - std::tanh(0.23f);
                const auto clipped = juce::jlimit(-0.83f, 0.83f, 1.7f * secondStage);
                speaker[channel] += speakerCoefficient * (clipped - speaker[channel]);
                dc[channel] += dcCoefficient * (speaker[channel] - dc[channel]);
                const auto signal = speaker[channel] - dc[channel];
                low[channel] += lowSplit * (signal - low[channel]);
                crossover[channel] += highSplit * (signal - crossover[channel]);
                const auto equalized = lowGain * low[channel]
                                     + midGain * (crossover[channel] - low[channel])
                                     + highGain * (signal - crossover[channel]);
                buffer.setSample(channel, sample, level * equalized);
            }
        }
    }
    void getStateInformation(juce::MemoryBlock& destination) override
    {
        if (auto xml = state.copyState().createXml())
            copyXmlToBinary(*xml, destination);
    }

    void setStateInformation(const void* data, int bytes) override
    {
        if (auto xml = getXmlFromBinary(data, bytes))
            if (xml->hasTagName(state.state.getType()))
            {
                auto restored = juce::ValueTree::fromXml(*xml);
                const auto gateId = controls[gateIndex].id;
                if (!restored.getChildWithProperty("id", gateId).isValid())
                {
                    // Older presets did not store a gate setting.
                    juce::ValueTree gateParameter("PARAM");
                    gateParameter.setProperty("id", gateId, nullptr);
                    gateParameter.setProperty("value", gateOff, nullptr);
                    restored.appendChild(gateParameter, nullptr);
                }
                state.replaceState(restored);
            }
    }

private:
    State state;
    std::array<std::atomic<float>*, controls.size()> values {};
    std::array<juce::SmoothedValue<float>, gateIndex> parameters;
    std::array<float, 2> bass {}, speaker {}, dc {}, low {}, crossover {};
    float rate = 44100.0f;
    float gateAttack = 0.0f, gateRelease = 0.0f, peakDecay = 0.0f;
    float gateGain = 1.0f, gatePeak = 0.0f;
    int gateHoldSamples = 0, gateHold = 0;
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LindsayDistortion();
}








