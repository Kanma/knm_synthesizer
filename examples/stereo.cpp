#define KNM_SYNTHESIZER_IMPLEMENTATION
#include <knm_synthesizer.hpp>

#include <iostream>
#include <fstream>

using namespace knm::synth;


int main(int argc, char** argv)
{
    // Simple command-line parameters handling
    if (((argc < 2) || (argc > 3)) || (strncmp(argv[1], "--help", 6) == 0) || (strncmp(argv[1], "-h", 2) == 0))
    {
        std::cout << "Usage: stereo <soundfont> <output>" << std::endl;
        std::cout << std::endl;
        std::cout << "Create an raw stereo audio file" << std::endl;
        return 0;
    }

    // Create the synthesizer
    SynthesizerSettings settings(22050);
    Synthesizer synthesizer(settings);

    // Load the SoundFont file
    if (!synthesizer.loadSoundFont(argv[1]))
    {
        std::cerr << "Failed to load SoundFont file: " << argv[1] << std::endl;
        return 1;
    }

    // Configure the synthesizer
    synthesizer.setMasterVolume(6.0f);
    synthesizer.configureChannel(0, 0, 0);

    // Allocate the buffers (for a duration of 4 seconds)
    size_t size = 4.0f * settings.sampleRate();

    float* left = new float[size];
    float* right = new float[size];

    memset((char*) left, 0, size * sizeof(float));
    memset((char*) right, 0, size * sizeof(float));

    // Play some notes, each during 0.5 second
    uint8_t notes[] = {60, 62, 64, 65, 67, 69, 71, 72};
    size_t note_duration = 0.4f * settings.sampleRate();
    size_t nb_notes = 8;

    size_t offset = 0;
    for (int i = 0; i < nb_notes + 1; ++i)
    {
        if (i > 0)
            synthesizer.noteOff(0, notes[i-1]);

        if (i < nb_notes)
            synthesizer.noteOn(0, notes[i], 100);

        synthesizer.render(left + offset, right + offset, note_duration);

        offset += note_duration;
    }

    // Write the result in a file (can be imported in Audacity with settings:
    // 32bits floats, little endian, 2 channels, 22050Hz)
    std::ofstream stream;
    stream.open(argv[2], std::ios_base::binary);

    for (int i = 0; i < size; ++i)
    {
        stream.write((const char*) (&left[i]), sizeof(float));
        stream.write((const char*) (&right[i]), sizeof(float));
    }

    // Cleanup
    delete[] left;
    delete[] right;

    return 0;
}
