knm::synthesizer
================

A SoundFont MIDI synthesizer written in C++.

**knm::synthesizer** is a single-file, header-only C++ library, with only one dependency:
**knm::soundfont**, another single-file, header-only C++ library, used to parse a
SoundFont file.

Everything is defined in the ``knm::synth`` namespace (``knm`` for my username, *Kanma*).

The implementation of the synthesis is an adaptation of ``Py-MeltySynth``
(https://github.com/sinshu/py-meltysynth).


.. toctree::
   :maxdepth: 2
   :caption: Documentation
   
   usage
   license


.. toctree::
   :maxdepth: 2
   :caption: API
   
   api_synthesizer
   api_settings
   api_channel
