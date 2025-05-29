Usage
=====

1. Copy the ``knm_synthesizer.hpp`` and ``knm_soundfont.hpp`` files in a convenient place for
   your project

2. In *one* C++ file, add the following code to create the implementation:

.. code:: cpp

    #define KNM_SYNTHESIZER_IMPLEMENTATION
    #include <knm_synthesizer.hpp>

In other files, just use ``#include <knm_synthesizer.hpp>``


Here is an example using the library to retrieve the data about key 60 (C4)
at velocity 20 in bank 0, preset 0:

.. code:: cpp

    #define KNM_SYNTHESIZER_IMPLEMENTATION
    #include <knm_synthesizer.hpp>

    #include <iostream>

    using namespace knm::synth;


    int main(int argc, char** argv)
    {
        // Create the synthesizer
        SynthesizerSettings settings(22050);
        Synthesizer synthesizer(settings);

        // Load the SoundFont file
        if (!synthesizer.loadSoundFont("/path/to/sounfont/file.sf2"))
        {
            std::cerr << "Failed to load SoundFont file" << std::endl;
            return 1;
        }

        synthesizer.setMasterVolume(6.0f);

        // Create the output buffers
        size_t duration = 1.0 * settings.sampleRate();

        float* left = new float[duration];
        float* right = new float[duration];

        memset((char*) left, 0, duration * sizeof(float));
        memset((char*) right, 0, duration * sizeof(float));

        // Play the note for 0.5 second and then release the key
        size_t note_duration = duration / 2;
        
        synthesizer.noteOn(0, 60, 100);
        synthesizer.render(left, right, note_duration);

        synthesizer.noteOff(0, 60);
        synthesizer.render(left + note_duration, right + note_duration, note_duration);

        // Do something with the buffers

        delete[] left;
        delete[] right;

        return 0;
    }
