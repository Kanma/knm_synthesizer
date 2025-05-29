# knm::synthesizer

[![Documentation Status](https://readthedocs.org/projects/knm-synthesizer/badge/?version=latest)](https://knm-synthesizer.readthedocs.io/en/latest/)

A SoundFont MIDI synthesizer written in C++.

**knm::synthesizer** is a single-file, header-only C++ library, with only one dependency:
**knm::soundfont**, another single-file, header-only C++ library, used to parse a
SoundFont file.

Everything is defined in the `knm::synth` namespace (`knm` for my username, *Kanma*).

The implementation of the synthesis is an adaptation of
[Py-MeltySynth](https://github.com/sinshu/py-meltysynth).


## Usage

1. Copy the ```knm_synthesizer.hpp``` and ```knm_soundfont.hpp``` files in a convenient place
   for your project

2. In *one* C++ file, add the following code to create the implementation:

```cpp
    #define KNM_SYNTHESIZER_IMPLEMENTATION
    #include <knm_synthesizer.hpp>
```

In other files, just use ```#include <knm_synthesizer.hpp>```


Here is an example using the library to render a C4 note at velocity 100 during 0.5
second using channel 0, in 1 second left & right buffers:

```cpp
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
```


## License

knm::synthesizer is made available under the MIT License.

Copyright (c) 2025 Philip Abbet

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
