#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(){
    levelSlider.setRange(0.0, 0.25);
    levelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    levelLabel.setText("Noise Level", juce::dontSendNotification);

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
    juce::String message;
    message << "Preparing to play some audio...\n";
    message << "samplesPerBlockExpected = " << samplesPerBlockExpected << "\n";
    message << "samplerRate = " << sampleRate;
    juce::Logger::getCurrentLogger()->writeToLog(message);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) {
    auto level = (float)levelSlider.getValue();
    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel) {
        // get a pointer to the start sample in the buffer for this audio output channel
        auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

        // fill the required number of samples with noise between -0.125 and +0.125
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
            *buffer++ = random.nextFloat() * level;
        }
    }
}

void MainComponent::releaseResources() {
    juce::Logger::getCurrentLogger()->writeToLog("Releasing audio resources");
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);

    // You can add your drawing code here!
}

void MainComponent::resized() {
    levelLabel.setBounds(10, 10, 90, 20);
    levelSlider.setBounds(100, 10, getWidth() - 110, 20);
}
