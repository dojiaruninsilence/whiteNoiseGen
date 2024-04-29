#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(){
    // set initial value for target level
    targetLevel = 0.125f;

    // set properties for the level slider
    levelSlider.setRange(0.0, 0.25);
    levelSlider.setValue(targetLevel, juce::dontSendNotification);
    levelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    levelSlider.onValueChange = [this] {
        targetLevel = (float)levelSlider.getValue();
        samplesToTarget = rampLengthSamples;
    };

    levelLabel.setText("Noise Level", juce::dontSendNotification);

    // diplay the ui stuffs
    addAndMakeVisible(&levelSlider);
    addAndMakeVisible(&levelLabel);

    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio)) {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    } else {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }
}

MainComponent::~MainComponent() {
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
    resetParameters();

    // just messaging stuff, for anal retentive purposes
    juce::String message;
    message << "Preparing to play some audio...\n";
    message << "samplesPerBlockExpected = " << samplesPerBlockExpected << "\n";
    message << "samplerRate = " << sampleRate;
    juce::Logger::getCurrentLogger()->writeToLog(message);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) {
    auto numSamplesRemaining = bufferToFill.numSamples;
    auto offset = 0;

    // check that there are actually samples to target
    if (samplesToTarget > 0) {
        auto levelIncrement = (targetLevel - currentLevel) / (float)samplesToTarget;
        auto numSamplesThisTime = juce::jmin(numSamplesRemaining, samplesToTarget);

        // iterate all of the samples for this time
        for (auto sample = 0; sample < numSamplesThisTime; ++sample) {
            // iterate all of the channels
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel) {
                // set the current sample in the current channel to a random value multiplied by a user input val
                bufferToFill.buffer->setSample(channel, sample, random.nextFloat() * currentLevel);
            }
            // add the level increment to the current level
            currentLevel += levelIncrement;
            // decrement the samples to target
            samplesToTarget--;
        }

        // upadate values
        offset = numSamplesThisTime;
        numSamplesRemaining -= numSamplesThisTime;

        // if this was the last sample current level becomes the target level value
        if (samplesToTarget == 0) {
            currentLevel = targetLevel;
        }
    }
    
    // old remove when done
    if (numSamplesRemaining > 0) {
        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel) {
            // get a pointer to the start sample in the buffer for this audio output channel
            auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample + offset);

            // fill the required number of samples with noise between -0.125 and +0.125
            for (auto sample = 0; sample < numSamplesRemaining; ++sample) {
                *buffer++ = random.nextFloat() * currentLevel;
            }
        }
    }
}

void MainComponent::releaseResources() {
    // just messaging stuff, for anal retentive purposes
    juce::Logger::getCurrentLogger()->writeToLog("Releasing audio resources");
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g) {
    // fill black
    g.fillAll(juce::Colours::black);
}

void MainComponent::resized() {
    // set the bounds of ui
    levelLabel.setBounds(10, 10, 90, 20);
    levelSlider.setBounds(100, 10, getWidth() - 110, 20);
}

void MainComponent::resetParameters() {
    currentLevel = targetLevel;
    samplesToTarget = 0;
}
