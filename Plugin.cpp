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

constexpr std::array<Control, 6> controls {{
    {"gain", "Gain (dB)", 12, 60, 42},
    {"tight", "Tight (%)", 0, 100, 62},
    {"bass", "Bass (dB)", -12, 12, 2},
    {"mid", "Mid (dB)", -12, 12, -2},
    {"treble", "Treble (dB)", -12, 12, 1},
    {"level", "Level (dB)", -30, 6, -10}
}};

using State = juce::AudioProcessorValueTreeState;

State::ParameterLayout makeParameters()
{
    State::ParameterLayout layout;
    for (const auto& control : controls)
    {
        const juce::ParameterID id {control.id, 1};
        layout.add(std::make_unique<juce::AudioParameterFloat>(id, control.name,
            juce::NormalisableRange<float>{control.minimum, control.maximum, 1.0f},
            control.initial));
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
            const auto& control = controls[knobIndices[i]];
            auto& knob = knobs[i];
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 23);
            knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd9ff6b));
            knob.setName(control.name);
            addAndMakeVisible(knob);
            knobLinks[i] = std::make_unique<State::SliderAttachment>(state, control.id, knob);
        }
        setSize(820, 264);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff171324));
        g.setColour(juce::Colours::white);
        g.setFont(26.0f);
        g.drawText("Lindsay Low-Hands", 20, 12, 580, 38, juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff302541));
        g.fillRoundedRectangle(20, 60, 780, 184, 12);
        g.setColour(juce::Colours::white);
        g.setFont(17.0f);
        g.drawText("Metal distortion", 34, 67, 120, 25, juce::Justification::centredLeft);
        g.saveState();
        g.addTransform(juce::AffineTransform::scale(0.58f).translated(755.0f, 5.0f));
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
            g.drawText(controls[knobIndices[i]].name, knobX[i], 208, 106, 23, juce::Justification::centred);
    }

    void resized() override
    {
        for (size_t i = 0; i < knobs.size(); ++i)
            knobs[i].setBounds(knobX[i], 102, 106, 103);
    }

private:
    static constexpr std::array<size_t, 6> knobIndices {0, 1, 2, 3, 4, 5};
    static constexpr std::array<int, 6> knobX {34, 163, 292, 421, 550, 679};
    std::array<juce::Slider, 6> knobs;
    std::array<std::unique_ptr<State::SliderAttachment>, 6> knobLinks;
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

            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                const auto input = buffer.getSample(channel, sample);
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
                state.replaceState(juce::ValueTree::fromXml(*xml));
    }

private:
    State state;
    std::array<std::atomic<float>*, 6> values {};
    std::array<juce::SmoothedValue<float>, 6> parameters;
    std::array<float, 2> bass {}, speaker {}, dc {}, low {}, crossover {};
    float rate = 44100.0f;
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LindsayDistortion();
}








