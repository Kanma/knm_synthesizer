/** knm::synthesizer - v1.0.0 - MIT License

DESCRIPTION

    A SoundFont MIDI synthesizer written in C++.

    knm::synthesizer is a single-file, header-only C++ library, with only one dependency:
    knm::soundfont, another single-file, header-only C++ library, used to parse a
    SoundFont file.

    Everything is defined in the `knm::synth` namespace (`knm` for my username, *Kanma*).

    The implementation of the synthesis is an adaptation of `Py-MeltySynth`
    (https://github.com/sinshu/py-meltysynth).

USAGE

    1. Copy this file and `knm_soundfont.hpp` in a convenient place for your project

    2. In *one* C++ file, add the following code to create the implementation:

        #define KNM_SYNTHESIZER_IMPLEMENTATION
        #include <knm_synthesizer.hpp>

    In other files, just use #include <knm_synthesizer.hpp>

    Here is an example using the library to render a C4 note at velocity 100 during 0.5
    second using channel 0, in 1 second left & right buffers:

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


LICENSE

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

*/


#pragma once

#ifdef KNM_SYNTHESIZER_IMPLEMENTATION
    #define KNM_SOUNDFONT_IMPLEMENTATION
    #define _USE_MATH_DEFINES
#endif

#include <knm_soundfont.hpp>


#ifdef KNM_SYNTHESIZER_IMPLEMENTATION
    #include <cmath>
#endif


#define KNM_SYNTHESIZER_VERSION_MAJOR 1
#define KNM_SYNTHESIZER_VERSION_MINOR 0
#define KNM_SYNTHESIZER_VERSION_PATCH 0


namespace knm {
namespace synth {

    class Sampler;
    class VolumeEnvelope;
    class ModulationEnvelope;
    class Lfo;
    class Voice;
    class VoiceCollection;


    //------------------------------------------------------------------------------------
    /// @brief  Holds the settings for a synthesizer
    ///
    /// Note that settings are assigned at synthesizer creation and can't be changed
    /// afterwards.
    //------------------------------------------------------------------------------------
    class SynthesizerSettings
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param sample_rate  The sample rate of the synthesized signal
        //--------------------------------------------------------------------------------
        SynthesizerSettings(uint32_t sample_rate);

        //--------------------------------------------------------------------------------
        /// @brief  Set the sample rate
        ///
        /// @param sample_rate  The sample rate of the synthesized signal
        //--------------------------------------------------------------------------------
        void setSampleRate(uint32_t sample_rate);

        //--------------------------------------------------------------------------------
        /// @brief  Set the block size used internally during synthesis
        ///
        /// @param block_size   The block size
        //--------------------------------------------------------------------------------
        void setBlockSize(uint16_t block_size);

        //--------------------------------------------------------------------------------
        /// @brief  Set the maximum number of notes that can be played at any single time
        ///
        /// @param maximum_polyphony    The maximum number of concurrent notes
        //--------------------------------------------------------------------------------
        void setMaximumPolyphony(uint16_t maximum_polyphony); 

        //--------------------------------------------------------------------------------
        /// @brief  Enable/disable reverb and chorus
        ///
        /// @param enable   Whether to enable or disable
        //--------------------------------------------------------------------------------
        void enableReverbAndChorus(bool enable);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the sample rate of the synthesized signal
        //--------------------------------------------------------------------------------
        inline uint32_t sampleRate() const
        {
            return _sample_rate;
        }
    
        //--------------------------------------------------------------------------------
        /// @brief  Returns the block size used internally during synthesis
        //--------------------------------------------------------------------------------
        inline uint16_t blockSize() const
        {
            return _block_size;
        }
    
        //--------------------------------------------------------------------------------
        /// @brief  Returns the maximum number of notes that can be played at any single
        ///         time
        //--------------------------------------------------------------------------------
        inline uint16_t maximumPolyphony() const
        {
            return _maximum_polyphony;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Indicates if reverb and chorus are enabled
        //--------------------------------------------------------------------------------
        inline bool reverbAndChorusEnabled() const
        {
            return _reverb_and_chorus_enabled;
        }


        //_____ Constants __________
    private:
        const uint32_t DEFAULT_BLOCK_SIZE = 64;
        const uint16_t DEFAULT_MAXIMUM_POLYPHONY = 64;
        const bool DEFAULT_REVERB_AND_CHORUS_ENABLED = true;


        //_____ Attributes __________
    private:
        uint32_t _sample_rate;
        uint16_t _block_size;
        uint16_t _maximum_polyphony;
        bool _reverb_and_chorus_enabled;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a MIDI channel
    ///
    /// A MIDI Synthesizer has several channels. Each one can be assigned to a specific
    /// "bank:preset" pair in the SoundFont file.
    ///
    /// Various parameters (affecting the synthesis) of a channel can be modified via
    /// MIDI events. Only a few are supported by this class.
    //------------------------------------------------------------------------------------
    class Channel
    {
    public:
    /// @name Construction
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Initializes a Channel instance
        ///
        /// @param percussion   (optional): Indicates if the channel is a percussion
        ///                     channel. Percussion channels are handled differently.
        ///                     Defaults to false.
        //--------------------------------------------------------------------------------
        Channel(bool percussion = false);
    /// @}

    /// @name Reset methods
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Reset the channel to the default values
        //--------------------------------------------------------------------------------
        void reset();

        //--------------------------------------------------------------------------------
        /// @brief  Reset the controllers of the channel to the default values
        //--------------------------------------------------------------------------------
        void resetControllers();
    /// @}

    /// @name Setters for general parameters
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Sets the bank number for the channel
        //--------------------------------------------------------------------------------
        inline void setBank(uint8_t value)
        {
            _bank = value + (_percussion ? 128 : 0);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the preset number for the channel
        //--------------------------------------------------------------------------------
        inline void setPreset(uint8_t value)
        {
            _preset = value;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the pitch bend value for the channel
        ///
        /// @param value1   The least significant byte of the pitch bend value
        /// @param value2   The most significant byte of the pitch bend value
        //--------------------------------------------------------------------------------
        inline void setPitchBend(uint8_t value1, uint8_t value2)
        {
            _pitch_bend = (1.0f / 8192.0f) * (int16_t(value1 | (value2 << 7)) - 8192);
        }
    /// @}

    /// @name Setters for high resolution continuous controllers
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Sets the coarse modulation value
        //--------------------------------------------------------------------------------
        inline void setModulationCoarse(uint8_t value)
        {
            _modulation = (value << 7) | (_modulation & 0x7F);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the fine modulation value
        //--------------------------------------------------------------------------------
        inline void setModulationFine(uint8_t value)
        {
            _modulation = (_modulation & 0xFF80) | value;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the coarse volume value
        //--------------------------------------------------------------------------------
        inline void setVolumeCoarse(uint8_t value)
        {
            _volume = (value << 7) | (_volume & 0x7F);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the fine volume value
        //--------------------------------------------------------------------------------
        inline void setVolumeFine(uint8_t value)
        {
            _volume = (_volume & 0xFF80) | value;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the coarse pan value
        //--------------------------------------------------------------------------------
        inline void setPanCoarse(uint8_t value)
        {
            _pan = (value << 7) | (_pan & 0x7F);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the fine pan value
        //--------------------------------------------------------------------------------
        inline void setPanFine(uint8_t value)
        {
            _pan = (_pan & 0xFF80) | value;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the coarse expression value
        //--------------------------------------------------------------------------------
        inline void setExpressionCoarse(uint8_t value)
        {
            _expression = (value << 7) | (_expression & 0x7F);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the fine expression value
        //--------------------------------------------------------------------------------
        inline void setExpressionFine(uint8_t value)
        {
            _expression = (_expression & 0xFF80) | value;
        }
    /// @}

    /// @name Setters for switches
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Sets the sustain switch
        //--------------------------------------------------------------------------------
        inline void setSustain(uint8_t value)
        {
            _sustain = (value >= 64);
        }
    /// @}

    /// @name Setters for low resolution continuous controllers
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Sets the reverb send level
        //--------------------------------------------------------------------------------
        inline void setReverbSend(uint8_t value)
        {
            _reverb_send = value;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the chorus send level
        //--------------------------------------------------------------------------------
        inline void setChorusSend(uint8_t value)
        {
            _chorus_send = value;
        }
    /// @}

    /// @name Setters for Registered Parameters (RPN)
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Sets the coarse RPN value
        //--------------------------------------------------------------------------------
        inline void setRPNCoarse(uint8_t value)
        {
            _rpn = (value << 7) | (_rpn & 0x7F);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the fine RPN value
        //--------------------------------------------------------------------------------
        inline void setRPNFine(uint8_t value)
        {
            _rpn = (_rpn & 0xFF80) | value;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the coarse data entry value based on the current RPN
        //--------------------------------------------------------------------------------
        inline void setDataEntryCoarse(uint8_t value)
        {
            switch (_rpn)
            {
                case 0:
                    _pitch_bend_range = (value << 7) | (_pitch_bend_range & 0x7F);
                    break;

                case 1:
                    _fine_tune = (value << 7) | (_fine_tune & 0x7F);
                    break;

                case 2:
                    _coarse_tune = int8_t(value) - 64;
                    break;

                default:
                    break;
            }
        }

        //--------------------------------------------------------------------------------
        /// @brief  Sets the fine data entry value based on the current RPN
        //--------------------------------------------------------------------------------
        inline void setDataEntryFine(uint8_t value)
        {
            switch (_rpn)
            {
                case 0:
                    _pitch_bend_range = (_pitch_bend_range & 0xFF80) | value;
                    break;

                case 1:
                    _fine_tune =  (_fine_tune & 0xFF80) | value;
                    break;

                default:
                    break;
            }
        }
    /// @}

    /// @name Getters for the parameters
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Returns whether the channel is a percussion channel
        //--------------------------------------------------------------------------------
        inline bool percussion() const
        {
            return _percussion;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the bank number of the channel
        //--------------------------------------------------------------------------------
        inline uint8_t bank() const
        {
            return _bank;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the preset number of the channel
        //--------------------------------------------------------------------------------
        inline uint8_t preset() const
        {
            return _preset;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the modulation value
        //--------------------------------------------------------------------------------
        inline float modulation() const
        {
            return (50.0f / 16383.0f) * _modulation;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the volume value in decibels
        //--------------------------------------------------------------------------------
        inline float volume() const
        {
            return 40.0f * log10(float(_volume) / 16383.0f);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the pan value
        //--------------------------------------------------------------------------------
        inline float pan() const
        {
            return (100.0f / 16383.0f) * _pan - 50.0f;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the expression value
        //--------------------------------------------------------------------------------
        inline float expression() const
        {
            return (1.0f / 16383.0f) * _expression;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns whether sustain is enabled
        //--------------------------------------------------------------------------------
        inline bool sustain() const
        {
            return _sustain;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the reverb send level
        //--------------------------------------------------------------------------------
        inline float reverbSend() const
        {
            return (1.0f / 127.0f) * _reverb_send;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the chorus send level
        //--------------------------------------------------------------------------------
        inline float chorusSend() const
        {
            return (1.0f / 127.0f) * _chorus_send;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the pitch bend range
        //--------------------------------------------------------------------------------
        inline float pitchBendRange() const
        {
            return (_pitch_bend_range >> 7) + 0.01f * (_pitch_bend_range & 0x7F);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the pitch bend value
        //--------------------------------------------------------------------------------
        inline float pitchBend() const
        {
            return pitchBendRange() * _pitch_bend;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the tuning adjustment
        //--------------------------------------------------------------------------------
        inline float tune() const
        {
            return float(_coarse_tune) + (1.0f / 8192.0f) * (int16_t(_fine_tune) - 8192);
        }
    /// @}

        //_____ Attributes __________
    private:
        // General parameters
        bool _percussion;
        uint8_t _bank;
        uint8_t _preset;
        float _pitch_bend;

        // High resolution continuous controllers (14 bits)
        uint16_t _modulation;
        uint16_t _volume;
        uint16_t _pan;
        uint16_t _expression;

        // Switches
        bool _sustain;

        // Low resolution continuous controllers (7 bits)
        uint8_t _reverb_send;
        uint8_t _chorus_send;

        // Registered Parameters
        int16_t _rpn;
        uint16_t _pitch_bend_range;
        int8_t _coarse_tune;
        uint16_t _fine_tune;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a MIDI synthesizer
    ///
    /// A SoundFont file must be loaded into the synthesizer before any synthesis can
    /// happen.
    ///
    /// The synthesizer can be controlled either via MIDI messages, or directly by calling
    /// dedicated methods.
    //------------------------------------------------------------------------------------
    class Synthesizer
    {
    public:
    /// @name Construction / Destruction
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param settings     The settings of the synthetiser
        //--------------------------------------------------------------------------------
        Synthesizer(const SynthesizerSettings& settings);

        //--------------------------------------------------------------------------------
        /// @brief  Destructor
        //--------------------------------------------------------------------------------
        ~Synthesizer();
    /// @}

    /// @name SoundFont file loading methods
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Load a SoundFont file
        ///
        /// @param  path    Path to the SoundFont file
        /// @return True if the file was loaded successfully, false otherwise
        //--------------------------------------------------------------------------------
        bool loadSoundFont(const std::filesystem::path& path);

        //--------------------------------------------------------------------------------
        /// @brief  Load a SoundFont file from a buffer
        ///
        /// @param  buffer  The buffer
        /// @param  size    Size of the buffer
        /// @return True if the file was loaded successfully, false otherwise
        //--------------------------------------------------------------------------------
        bool loadSoundFont(const char* buffer, size_t size);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the loaded SoundFont file representation
        //--------------------------------------------------------------------------------
        inline const sf::SoundFont& soundfont() const
        {
            return _soundfont;
        }
    /// @}

    /// @name MIDI & notes
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Process a MIDI message
        ///
        /// @param channel  The channel affected by the message
        /// @param command  The command to process
        /// @param data1    Data associated to the command
        /// @param data2    Secondary data associated to the command
        //--------------------------------------------------------------------------------
        bool processMidiMessage(
            uint8_t channel, uint8_t command, uint8_t data1, uint8_t data2
        );

        //--------------------------------------------------------------------------------
        /// @brief  Start to press a key
        ///
        /// The key will be pressed until `noteOff()` is called.
        ///
        /// @param channel  The channel
        /// @param key      The key to press
        /// @param velocity The velocity of the key press
        //--------------------------------------------------------------------------------
        void noteOn(uint8_t channel, uint8_t key, uint8_t velocity);

        //--------------------------------------------------------------------------------
        /// @brief  Release a key
        ///
        /// It is expected that `noteOn()` has been called on that channel with that key
        /// before.
        ///
        /// When a key is released, its sound gradually falls off.
        ///
        /// @param channel  The channel
        /// @param key      The key to release
        //--------------------------------------------------------------------------------
        void noteOff(uint8_t channel, uint8_t key);

        //--------------------------------------------------------------------------------
        /// @brief  Release all the keys
        ///
        /// @param immediate    If false, the sound of the notes gradually falls off like
        ///                     with `noteOff()`.
        //--------------------------------------------------------------------------------
        void allNotesOff(bool immediate);

        //--------------------------------------------------------------------------------
        /// @brief  Release all the keys of a specific channel
        ///
        /// @param channel      The channel
        /// @param immediate    If false, the sound of the notes gradually falls off like
        ///                     with `noteOff()`.
        //--------------------------------------------------------------------------------
        void allNotesOff(uint8_t channel, bool immediate);

        //--------------------------------------------------------------------------------
        /// @brief  Reset the value of all the MIDI controllers
        //--------------------------------------------------------------------------------
        void resetAllControllers();

        //--------------------------------------------------------------------------------
        /// @brief  Reset the value of all the MIDI controllers of a specific channel
        ///
        /// @param channel  The channel
        //--------------------------------------------------------------------------------
        void resetControllers(uint8_t channel);
    /// @}

    /// @name Audio synthesis
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Render the audio into stereo buffers (left and right)
        ///
        /// It is expected that this method is called with buffers of appropriate size
        /// each time a MIDI message is processed by `processMidiMessage()` or each time
        /// a key (or a group of keys) is pressed or released.
        ///
        /// @param left     The left buffer (will be filled)
        /// @param right    The right buffer (will be filled)
        /// @param size     Size of the buffers
        //--------------------------------------------------------------------------------
        void render(float* left, float* right, size_t size);

        //--------------------------------------------------------------------------------
        /// @brief  Render the audio into a mono buffer
        ///
        /// It is expected that this method is called with a buffer of appropriate size
        /// each time a MIDI message is processed by `processMidiMessage()` or each time
        /// a key (or a group of keys) is pressed or released.
        ///
        /// @param buffer   The buffer (will be filled)
        /// @param size     Size of the buffer
        //--------------------------------------------------------------------------------
        void render(float* buffer, size_t size);

        //--------------------------------------------------------------------------------
        /// @brief  Sets the master volume, in dB
        //--------------------------------------------------------------------------------
        void setMasterVolume(float volume);

        //--------------------------------------------------------------------------------
        /// @brief  Returns the master volume, in dB
        //--------------------------------------------------------------------------------
        float masterVolume() const;
        
        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of samples rendered so far
        //--------------------------------------------------------------------------------
        inline uint32_t nbRenderedSamples() const
        {
            return _nb_rendered_samples;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of active voices
        //--------------------------------------------------------------------------------
        uint16_t nbActiveVoices() const;
    /// @}

    /// @name Channels
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of channels
        //--------------------------------------------------------------------------------
        inline size_t nbChannels() const
        {
            return _channels.size();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Assign a preset from the SoundFont file to a channel
        ///
        /// @param channel  The number of the channel
        /// @param bank     The bank of the preset
        /// @param preset   The number of the preset
        //--------------------------------------------------------------------------------
        bool configureChannel(uint8_t channel, uint8_t bank, uint8_t preset);

        //--------------------------------------------------------------------------------
        /// @brief  Assign a preset from the SoundFont file to a channel
        ///
        /// @param channel  The number of the channel
        /// @param id       The id of the preset
        //--------------------------------------------------------------------------------
        inline bool configureChannel(uint8_t channel, sf::preset_id_t id)
        {
            return configureChannel(channel, id.bank, id.number);
        }

        //--------------------------------------------------------------------------------
        /// @brief  Retrieves a list of the names of all the presets in thr SoundFont file
        //--------------------------------------------------------------------------------
        std::map<sf::preset_id_t, std::string> presetNames();

        //--------------------------------------------------------------------------------
        /// @brief  Returns a channel
        ///
        /// @param channel  The number of the channel
        //--------------------------------------------------------------------------------
        inline Channel& getChannel(uint8_t channel)
        {
            return _channels[channel];
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns a channel
        ///
        /// @param channel  The number of the channel
        //--------------------------------------------------------------------------------
        inline const Channel& getChannel(uint8_t channel) const
        {
            return _channels[channel];
        }
    /// @}

    /// @name Other methods
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Returns the settings of the synthesizer
        //--------------------------------------------------------------------------------
        inline const SynthesizerSettings& settings() const
        {
            return _settings;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Reset the synthesizer (like if no rendering happened, no key was
        ///         pressed)
        //--------------------------------------------------------------------------------
        void reset();
    /// @}

    private:
        void renderBlockStereo();
        void renderBlockMono();

        void writeBlock(
            float previous_gain, float current_gain, float* source, float* destination
        );


        //_____ Constants __________
    private:
        const uint32_t CHANNEL_COUNT = 16;
        const uint16_t PERCUSSION_CHANNEL = 9;


        //_____ Attributes __________
    private:
        sf::SoundFont _soundfont;
        SynthesizerSettings _settings;

        sf::preset_id_t _default_preset;

        std::vector<Channel> _channels;
        VoiceCollection* _voices = nullptr;

        float* _block_left = nullptr;
        float* _block_right = nullptr;
        uint32_t _blocks_offset;
        float _inverse_block_size;

        uint32_t _nb_rendered_samples = 0;
        float _master_volume = 1.0f;
    };


#ifdef KNM_SYNTHESIZER_IMPLEMENTATION

    /********************************* INTERNAL TYPES ***********************************/

    enum loop_mode_t
    {
        LOOP_MODE_NONE = 0,
        LOOP_MODE_CONTINUOUS = 1,
        LOOP_MODE_UNTIL_RELEASE = 3,
    };

    //-----------------------------------------------------------------------

    enum envelope_stage_t
    {
        ENV_STAGE_DELAY = 0,
        ENV_STAGE_ATTACK = 1,
        ENV_STAGE_HOLD = 2,
        ENV_STAGE_DECAY = 3,
        ENV_STAGE_RELEASE = 4,
    };
 
    //-----------------------------------------------------------------------

    enum voice_state_t
    {
        VOICE_STATE_PLAYING,
        VOICE_STATE_RELEASE_REQUESTED,
        VOICE_STATE_RELEASED,
    };


    /************************************ CONSTANTS *************************************/

    const float NON_AUDIBLE = 0.001f;
    const float LOG_NON_AUDIBLE = log(NON_AUDIBLE);


    /******************************** HELPER FUNCTIONS **********************************/

    inline float clamp(float value, float min, float max)
    {
        if (value < min)
            return min;
        else if (value > max)
            return max;
        else
            return value;
    }

    //-----------------------------------------------------------------------

    inline float exp_cutoff(float x)
    {
        if (x < LOG_NON_AUDIBLE)
            return 0.0f;

        return exp(x);
    }

    //-----------------------------------------------------------------------

    inline float timecents_to_seconds(float x)
    {
        return pow(2.0f, (1.0f / 1200.0f) * x);
    }

    //-----------------------------------------------------------------------

    inline float decibels_to_linear(float x)
    {
        return pow(10.0f, 0.05f * x);
    };

    //-----------------------------------------------------------------------

    inline float linear_to_decibels(float x)
    {
        return 20.0f * log10(x);
    }

    //-----------------------------------------------------------------------

    inline float cents_to_hertz(float x)
    {
        return 8.176f * pow(2.0f, (1.0f / 1200.0f) * x);
    }

    //-----------------------------------------------------------------------

    inline float cents_to_multiplying_factor(float x)
    {
        return pow(2.0f, (1.0f / 1200.0f) * x);
    }

    //-----------------------------------------------------------------------

    inline float key_number_to_multiplying_factor(int16_t cents, uint8_t key)
    {
        return timecents_to_seconds(cents * (60 - key));
    }


    /************************************* CHANNEL **************************************/

    Channel::Channel(bool percussion)
    : _percussion(percussion)
    {
        reset();
    }

    //-----------------------------------------------------------------------

    void Channel::reset()
    {
        _bank = (_percussion ? 128 : 0);
        _preset = 0;

        _modulation = 0;
        _volume = 100 << 7;
        _pan = 64 << 7;
        _expression = 127 << 7;
        _sustain = false;

        _reverb_send = 40;
        _chorus_send = 0;

        _rpn = -1;
        _pitch_bend_range = 2 << 7;
        _coarse_tune = 0;
        _fine_tune = 8192;

        _pitch_bend = 0.0f;
    }
    
    //-----------------------------------------------------------------------

    void Channel::resetControllers()
    {
        _modulation = 0;
        _expression = 127 << 7;
        _sustain = false;

        _rpn = -1;

        _pitch_bend = 0.0f;
    }


    /************************************ SAMPLER ***************************************/

    //------------------------------------------------------------------------------------
    /// @brief  
    //------------------------------------------------------------------------------------
    class Sampler
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        //--------------------------------------------------------------------------------
        Sampler(uint16_t sample_rate);

        //--------------------------------------------------------------------------------
        /// @brief  
        ///
        /// Start index of the sample in the buffer
        /// End index of the sample in the buffer
        /// Starting point of the loop
        /// Ending point of the loop
        /// Ending point of the loop
        /// Samnple rate, in hertz, at which this sample was acquired
        /// The MIDI key number of the recorded pitch of the sample
        //--------------------------------------------------------------------------------
        void start(
            const float* buffer,
            uint32_t start,
            uint32_t end,
            loop_mode_t loop_mode,
            uint32_t loop_start,
            uint32_t loop_end,
            uint32_t sample_rate,
            uint8_t root_key,
            int8_t coarse_tune,
            int8_t fine_tune,
            uint16_t scale_tuning
        );

        void release();

        bool process(float* dest, size_t size, float pitch);


        //_____ Attributes __________
    private:
        // Information about the audio sample
        const float* _buffer = nullptr;
        uint32_t _start;
        uint32_t _end;
        loop_mode_t _loop_mode;
        uint32_t _loop_start;
        uint32_t _loop_end;
        uint32_t _sample_rate;
        uint8_t _root_key;
    
        // Internal state
        uint16_t _dest_sample_rate;
        double _current_index;
        bool _looping;
        float _tune;
        float _pitch_change_scale;
        float _sample_rate_ratio;
    };

    //-----------------------------------------------------------------------

    Sampler::Sampler(uint16_t sample_rate)
    : _dest_sample_rate(sample_rate)
    {
    }

    //-----------------------------------------------------------------------

    void Sampler::start(
        const float* buffer,
        uint32_t start,
        uint32_t end,
        loop_mode_t loop_mode,
        uint32_t loop_start,
        uint32_t loop_end,
        uint32_t sample_rate,
        uint8_t original_pitch,
        int8_t coarse_tune,
        int8_t fine_tune,
        uint16_t scale_tuning
    )
    {
        _buffer = buffer;
        _start = start;
        _end = end;
        _loop_mode = loop_mode;
        _loop_start = loop_start;
        _loop_end = loop_end;
        _sample_rate = sample_rate;
        _root_key = original_pitch;

        _tune = float(coarse_tune) + 0.01f * float(fine_tune);
        _pitch_change_scale = 0.01f * float(scale_tuning);
        _sample_rate_ratio = float(sample_rate) / float(_dest_sample_rate);

        _looping = (loop_mode != LOOP_MODE_NONE);
        _current_index = (double) start;
    }

    //-----------------------------------------------------------------------

    void Sampler::release()
    {
        if (_loop_mode == LOOP_MODE_UNTIL_RELEASE)
            _looping = false;
    }

    //-----------------------------------------------------------------------

    bool Sampler::process(float* dest, size_t size, float pitch)
    {
        const float pitch_change = _pitch_change_scale * (pitch - _root_key) + _tune;
        const float pitch_ratio = _sample_rate_ratio * pow(2.0f, pitch_change / 12.0f);

        const uint32_t loop_length = _loop_end - _loop_start;

        for (int i = 0; i < size; ++i)
        {
            uint32_t index = (uint32_t) floor(_current_index);
            uint32_t index2 = index + 1;

            if (!_looping)
            {
                if (index >= _end)
                {
                    if (i == 0)
                        return false;

                    for (int j = i; j < size; ++j)
                        dest[j] = 0.0f;

                    return true;
                }
            }
            else
            {
                if (index2 >= _loop_end)
                    index2 -= loop_length;
            }

            float x1 = (float) _buffer[index];
            float x2 = (float) _buffer[index2];
            float a = _current_index - float(index);
            dest[i] = (x1 + a * (x2 - x1));

            _current_index += pitch_ratio;

            if (_looping && (_current_index >= _loop_end))
                _current_index -= (float) loop_length;
        }

        return true;
    }


    /********************************* VOLUME ENVELOPE **********************************/

    //------------------------------------------------------------------------------------
    /// @brief  Allows to generate a volume envelope
    //------------------------------------------------------------------------------------
    class VolumeEnvelope
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param sample_rate  The sample rate of the synthesized signal
        //--------------------------------------------------------------------------------
        VolumeEnvelope(uint16_t sample_rate);

        //--------------------------------------------------------------------------------
        /// @brief  Starts a new envelope
        ///
        /// Must be called when the key is pressed, before 'process()', to define the
        /// shape of the envelope.
        //--------------------------------------------------------------------------------
        void start(
            float delay, float attack, float hold, float decay, float sustain,
            float release
        );

        //--------------------------------------------------------------------------------
        /// @brief  Must be called when the key is released
        //--------------------------------------------------------------------------------
        void release();

        //--------------------------------------------------------------------------------
        /// @brief  Update the value of the envelope
        ///
        /// @param nb_samples   The number of samples to process during this update
        /// @return True if the envelope still allows the signal to be heard
        //--------------------------------------------------------------------------------
        bool process(uint32_t nb_samples);

        inline float getValue() const
        {
            return value;
        }

        inline float getPriority() const
        {
            return priority;
        }


        //_____ Attributes __________
    private:
        uint16_t sample_rate;

        float attack_slope;
        float decay_slope;
        float release_slope;
    
        float attack_start_time;
        float hold_start_time;
        float decay_start_time;
        float release_start_time;
    
        float sustain_level;
        float release_level;
    
        uint32_t nb_processed_samples;
        envelope_stage_t stage;

        float value;
        float priority;
    };

    //-----------------------------------------------------------------------

    VolumeEnvelope::VolumeEnvelope(uint16_t sample_rate)
    : sample_rate(sample_rate)
    {
    }

    //-----------------------------------------------------------------------

    void VolumeEnvelope::start(
        float delay, float attack, float hold, float decay, float sustain,
        float release
    )
    {
        attack_slope = 1.0f / attack;
        decay_slope = -9.226f / decay;
        release_slope = -9.226f / release;

        attack_start_time = delay;
        hold_start_time = attack_start_time + attack;
        decay_start_time = hold_start_time + hold;
        release_start_time = 0;

        sustain_level = clamp(sustain, 0.0f, 1.0f);
        release_level = 0.0f;

        nb_processed_samples = 0;
        stage = ENV_STAGE_DELAY;
        value = 0.0f;

        process(0);
    }

    //-----------------------------------------------------------------------

    void VolumeEnvelope::release()
    {
        stage = ENV_STAGE_RELEASE;
        release_start_time = float(nb_processed_samples) / sample_rate;
        release_level = value;
    }

    //-----------------------------------------------------------------------

    bool VolumeEnvelope::process(uint32_t nb_samples)
    {
        nb_processed_samples += nb_samples;

        float current_time = float(nb_processed_samples) / sample_rate;

        // Change stage if necessary
        while (stage <= ENV_STAGE_HOLD)
        {
            float end;

            switch (stage)
            {
                case ENV_STAGE_DELAY: end = attack_start_time; break;
                case ENV_STAGE_ATTACK: end = hold_start_time; break;
                case ENV_STAGE_HOLD: end = decay_start_time; break;
                default: return false;
            }

            if (current_time < end)
                break;

            stage = envelope_stage_t(int(stage) + 1);
        }

        // Compute the envelope value at current stage
        switch (stage)
        {
            case ENV_STAGE_DELAY:
                value = 0.0f;
                priority = 3.0;
                return true;
            
            case ENV_STAGE_ATTACK:
                value = attack_slope * (current_time - attack_start_time);
                priority = 3.0 - value;
                return true;
            
            case ENV_STAGE_HOLD:
                value = 1.0;
                priority = 2.0;
                return true;

            case ENV_STAGE_DECAY:
                value = fmax(
                    exp_cutoff(decay_slope * (current_time - decay_start_time)),
                    sustain_level
                );
                priority = 1.0 + value;
                return (value > NON_AUDIBLE);

            case ENV_STAGE_RELEASE:
                value = release_level *
                            exp_cutoff(release_slope * (current_time - release_start_time));
                priority = value;
                return (value > NON_AUDIBLE);
        }

        return false;
    }


    /******************************* MODULATION ENVELOPE ********************************/

    //------------------------------------------------------------------------------------
    /// @brief  Allows to generate a modulation envelope
    //------------------------------------------------------------------------------------
    class ModulationEnvelope
    {
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param sample_rate  The sample rate of the synthesized signal
        //--------------------------------------------------------------------------------
        ModulationEnvelope(uint16_t sample_rate);

        //--------------------------------------------------------------------------------
        /// @brief  Starts a new envelope
        ///
        /// Must be called when the key is pressed, before 'process()', to define the
        /// shape of the envelope.
        //--------------------------------------------------------------------------------
        void start(
            float delay, float attack, float hold, float decay, float sustain,
            float release
        );

        //--------------------------------------------------------------------------------
        /// @brief  Must be called when the key is released
        //--------------------------------------------------------------------------------
        void release();

        //--------------------------------------------------------------------------------
        /// @brief  Update the value of the envelope
        ///
        /// @param nb_samples   The number of samples to process during this update
        /// @return True if the envelope still allows the signal to be heard
        //--------------------------------------------------------------------------------
        bool process(uint32_t nb_samples);

        inline float getValue() const
        {
            return value;
        }


        //_____ Attributes __________
    private:
        uint16_t sample_rate;

        float attack_slope;
        float decay_slope;
        float release_slope;
    
        float attack_start_time;
        float hold_start_time;
        float decay_start_time;

        float decay_end_time;
        float release_end_time;
    
        float sustain_level;
        float release_level;
    
        uint32_t nb_processed_samples;
        envelope_stage_t stage;

        float value;
    };

    //-----------------------------------------------------------------------

    ModulationEnvelope::ModulationEnvelope(uint16_t sample_rate)
    : sample_rate(sample_rate)
    {
    }

    //-----------------------------------------------------------------------

    void ModulationEnvelope::start(
        float delay, float attack, float hold, float decay, float sustain,
        float release
    )
    {
        attack_slope = 1.0f / attack;
        decay_slope = 1.0f / decay;
        release_slope = 1.0f / release;

        attack_start_time = delay;
        hold_start_time = attack_start_time + attack;
        decay_start_time = hold_start_time + hold;

        decay_end_time = decay_start_time + decay;
        release_end_time = release;

        sustain_level = clamp(sustain, 0.0f, 1.0f);
        release_level = 0.0f;

        nb_processed_samples = 0;
        stage = ENV_STAGE_DELAY;
        value = 0.0f;

        process(0);
    }

    //-----------------------------------------------------------------------

    void ModulationEnvelope::release()
    {
        stage = ENV_STAGE_RELEASE;
        release_end_time += float(nb_processed_samples) / sample_rate;
        release_level = value;
    }

    //-----------------------------------------------------------------------

    bool ModulationEnvelope::process(uint32_t nb_samples)
    {
        nb_processed_samples += nb_samples;

        float current_time = float(nb_processed_samples) / sample_rate;

        // Change stage if necessary
        while (stage <= ENV_STAGE_HOLD)
        {
            float end;

            switch (stage)
            {
                case ENV_STAGE_DELAY: end = attack_start_time; break;
                case ENV_STAGE_ATTACK: end = hold_start_time; break;
                case ENV_STAGE_HOLD: end = decay_start_time; break;
                default: return false;
            }

            if (current_time < end)
                break;

            stage = envelope_stage_t(int(stage) + 1);
        }

        // Compute the envelope value at current stage
        switch (stage)
        {
            case ENV_STAGE_DELAY:
                value = 0.0f;
                return true;
            
            case ENV_STAGE_ATTACK:
                value = attack_slope * (current_time - attack_start_time);
                return true;
            
            case ENV_STAGE_HOLD:
                value = 1.0;
                return true;

            case ENV_STAGE_DECAY:
                value = fmax(
                    decay_slope * (decay_end_time - current_time),
                    sustain_level
                );
                return (value > NON_AUDIBLE);

            case ENV_STAGE_RELEASE:
                value = fmax(
                    release_level * release_slope * (release_end_time - current_time),
                    0.0f
                );
                return (value > NON_AUDIBLE);
        }

        return false;
    }


    /*************************************** LFO ****************************************/

    //------------------------------------------------------------------------------------
    /// @brief  Low frequency oscillator
    //------------------------------------------------------------------------------------
    class Lfo
    {
    public:
        Lfo(const SynthesizerSettings& settings);

        void start(float delay, float frequency);

        void process();

        inline float value()
        {
            return _value;
        }
    

        //_____ Attributes __________
    private:
        const SynthesizerSettings& _settings;

        bool _active;
        float _delay;
        float _period;

        uint32_t _nb_processed_samples;
        float _value;
    };

    //-----------------------------------------------------------------------

    Lfo::Lfo(const SynthesizerSettings& settings)
    : _settings(settings)
    {
    }

    //-----------------------------------------------------------------------

    void Lfo::start(float delay, float frequency)
    {
        if (frequency > 0.001f)
        {
            _active = true;

            _delay = delay;
            _period = 1.0f / frequency;

            _nb_processed_samples = 0;
            _value = 0.0f;
        }
        else
        {
            _active = false;
            _value = 0.0f;
        }
    }

    //-----------------------------------------------------------------------

    void Lfo::process()
    {
        if (!_active)
            return;

        _nb_processed_samples += _settings.blockSize();

        float current_time = float(_nb_processed_samples) / _settings.sampleRate();

        if (current_time < _delay)
        {
            _value = 0.0f;
        }
        else
        {
            float phase = fmod(current_time - _delay, _period) / _period;

            if (phase < 0.25f)
                _value = 4.0f * phase;
            else if (phase < 0.75f)
                _value = 4.0f * (0.5f - phase);
            else
                _value = 4.0f * (phase - 1.0f);
        }
    }


    /*********************************** BIQUADFILTER ***********************************/

    //------------------------------------------------------------------------------------
    /// @brief  Bi-quad filter
    //------------------------------------------------------------------------------------
    class BiQuadFilter
    {
    public:
        BiQuadFilter(const SynthesizerSettings& settings);

        void clearBuffer();

        void setLowPassFilter(float cutoff_frequency, float resonance);

        void process(float* block, size_t size);

    private:
        void setCoefficients(float a0, float a1, float a2, float b0, float b1, float b2);


        //_____ Constants __________
    private:
        const float RESONANCE_PEAK_OFFSET = 1.0f - 1.0f / sqrt(2.0f);


        //_____ Attributes __________
    private:
        const SynthesizerSettings& _settings;
        
        bool _active;

        float _a0;
        float _a1;
        float _a2;
        float _a3;
        float _a4;
    
        float _x1;
        float _x2;
        float _y1;
        float _y2;
    };

    //-----------------------------------------------------------------------

    BiQuadFilter::BiQuadFilter(const SynthesizerSettings& settings)
    : _settings(settings)
    {
    }

    //-----------------------------------------------------------------------

    void BiQuadFilter::clearBuffer()
    {
        _x1 = 0.0f;
        _x2 = 0.0f;
        _y1 = 0.0f;
        _y2 = 0.0f;
    }

    //-----------------------------------------------------------------------

    void BiQuadFilter::setLowPassFilter(float cutoff_frequency, float resonance)
    {
        if (cutoff_frequency < 0.499f * _settings.sampleRate())
        {
            _active = true;

            // This equation gives the Q value which makes the desired resonance peak.
            // The error of the resultant peak height is less than 3%.
            float q = resonance - RESONANCE_PEAK_OFFSET / (1.0f + 6.0f * (resonance - 1.0f));

            float w = 2.0f * M_PI * cutoff_frequency / _settings.sampleRate();
            float cosw = cos(w);
            float alpha = sin(w) / (2.0f * q);

            float b0 = (1.0f - cosw) / 2.0f;
            float b1 = 1.0f - cosw;
            float b2 = (1.0f - cosw) / 2.0f;
            float a0 = 1.0f + alpha;
            float a1 = -2.0f * cosw;
            float a2 = 1.0f - alpha;

            setCoefficients(a0, a1, a2, b0, b1, b2);
        }
        else
        {
            _active = false;
        }
    }

    //-----------------------------------------------------------------------

    void BiQuadFilter::process(float* block, size_t size)
    {
        if (_active)
        {
            for (int t = 0; t < size; ++t)
            {
                float input = block[t];
                float output = _a0 * input + _a1 * _x1 + _a2 * _x2 - _a3 * _y1 - _a4 * _y2;

                _x2 = _x1;
                _x1 = input;
                _y2 = _y1;
                _y1 = output;

                block[t] = output;
            }
        }
        else
        {
            _x2 = block[size - 2];
            _x1 = block[size - 1];
            _y2 = _x2;
            _y1 = _x1;
        }
    }

    //-----------------------------------------------------------------------

    void BiQuadFilter::setCoefficients(
        float a0, float a1, float a2, float b0, float b1, float b2
    )
    {
        _a0 = b0 / a0;
        _a1 = b1 / a0;
        _a2 = b2 / a0;
        _a3 = a1 / a0;
        _a4 = a2 / a0;
    }


    /************************************** VOICE ***************************************/

    //------------------------------------------------------------------------------------
    /// @brief  Each voice is responsible to play one note
    //------------------------------------------------------------------------------------
    class Voice
    {
        //_____ Internal types __________
    private:
        struct track_t
        {
            track_t(const SynthesizerSettings& settings)
            : volume_envelope(settings.sampleRate()),
              modulation_envelope(settings.sampleRate()),
              vibrato_lfo(settings),
              modulation_lfo(settings),
              sampler(settings.sampleRate()),
              filter(settings)
            {}

            VolumeEnvelope volume_envelope;
            ModulationEnvelope modulation_envelope;

            Lfo vibrato_lfo;
            Lfo modulation_lfo;

            Sampler sampler;
            BiQuadFilter filter;

            float note_gain;

            float cutoff;
            float resonance;

            float vib_lfo_to_pitch;
            float mod_lfo_to_pitch;
            float mod_env_to_pitch;

            int mod_lfo_to_cutoff;
            int mod_env_to_cutoff;
            bool dynamic_cutoff;

            float mod_lfo_to_volume;
            bool dynamic_volume;

            float instrument_pan;
            float instrument_reverb;
            float instrument_chorus;

            // Some instruments require fast cutoff change, which can cause pop noise.
            // This is used to smooth out the cutoff frequency.
            float smoothed_cutoff;

            float* block = nullptr;

            float previous_mix_gain = 0.0f;
            float current_mix_gain = 0.0f;
        };


        //_____ Methods __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        ///
        /// @param settings The settings of the synthesizer
        //--------------------------------------------------------------------------------
        Voice(const Synthesizer* synthesizer);

        ~Voice();

        void start(
            const sf::key_info_t key_info, const float* buffer, uint8_t channel,
            uint8_t key, uint8_t velocity
        );

        void end();
        void kill();

        bool process();

        inline float priority() const
        {
            if (_stereo)
            {
                if ((_left.note_gain < NON_AUDIBLE) && (_right.note_gain < NON_AUDIBLE))
                    return 0.0f;

                return std::max(_left.volume_envelope.getPriority(), _right.volume_envelope.getPriority());
            }
            else
            {
                if (_left.note_gain < NON_AUDIBLE)
                    return 0.0f;

                return _left.volume_envelope.getPriority();
            }
        }

        inline bool stereo() const
        {
            return _stereo;
        }

        inline float* block_left() const
        {
            return _left.block;
        }

        inline float* block_right() const
        {
            return _right.block;
        }

        inline float previousMixGainLeft() const
        {
            return _left.previous_mix_gain;
        }

        inline float previousMixGainRight() const
        {
            return _right.previous_mix_gain;
        }

        inline float currentMixGainLeft() const
        {
            return _left.current_mix_gain;
        }

        inline float currentMixGainRight() const
        {
            return _right.current_mix_gain;
        }

        inline float previousReverbSend() const
        {
            return _previous_reverb_send;
        }

        inline float previousChorusSend() const
        {
            return _previous_chorus_send;
        }

        inline float currentReverbSend() const
        {
            return _current_reverb_send;
        }

        inline float currentChorusSend() const
        {
            return _current_chorus_send;
        }

        inline uint8_t exclusiveClass() const
        {
            return _exclusive_class;
        }

        inline uint8_t channel() const
        {
            return _channel;
        }

        inline uint8_t key() const
        {
            return _key;
        }

        inline uint8_t velocity() const
        {
            return _velocity;
        }

        inline uint32_t voiceLength() const
        {
            return _voice_length;
        }

    private:
        void start(const sf::sample_info_t& key_info, const float* buffer, track_t& track);
        bool process(const Channel& channel_info, track_t& track);


        //_____ Attributes __________
    private:
        const Synthesizer* _synthesizer;

        bool _stereo = false;
        track_t _left;
        track_t _right;

        float _previous_reverb_send = 0.0f;
        float _previous_chorus_send = 0.0f;
        float _current_reverb_send = 0.0f;
        float _current_chorus_send = 0.0f;

        uint8_t _exclusive_class;
        uint8_t _channel;
        uint8_t _key;
        uint8_t _velocity;

        voice_state_t _voice_state;
        uint32_t _voice_length;
    };

    //-----------------------------------------------------------------------

    Voice::Voice(const Synthesizer* synthesizer)
    : _synthesizer(synthesizer), _left(synthesizer->settings()), _right(synthesizer->settings())
    {
        _left.block = new float[synthesizer->settings().blockSize()];
        _right.block = new float[synthesizer->settings().blockSize()];
    }

    //-----------------------------------------------------------------------

    Voice::~Voice()
    {
        delete[] _left.block;
        delete[] _right.block;
    }

    //-----------------------------------------------------------------------

    void Voice::start(
        const sf::key_info_t key_info, const float* buffer, uint8_t channel, uint8_t key,
        uint8_t velocity
    )
    {
        _stereo = key_info.stereo;

        _exclusive_class = key_info.left.generator(sf::GEN_TYPE_EXCLUSIVE_CLASS, { 0 }).uvalue;
        _channel = channel;
        _key = key;
        _velocity = velocity;

        start(key_info.left, buffer, _left);

        if (_stereo)
            start(key_info.right, buffer, _right);

        _voice_state = VOICE_STATE_PLAYING;
        _voice_length = 0;
    }

    //-----------------------------------------------------------------------

    void Voice::end()
    {
        if (_voice_state == VOICE_STATE_PLAYING)
            _voice_state = VOICE_STATE_RELEASE_REQUESTED;
    }

    //-----------------------------------------------------------------------

    void Voice::kill()
    {
        _left.note_gain = 0.0f;
        _right.note_gain = 0.0f;
    }
    
    //-----------------------------------------------------------------------

    bool Voice::process()
    {
        if ((_left.note_gain < NON_AUDIBLE) && (!_stereo || (_right.note_gain < NON_AUDIBLE)))
            return false;

        auto channel_info = _synthesizer->getChannel(_channel);

        if ((_voice_length >= _synthesizer->settings().sampleRate() / 500) &&
            (_voice_state == VOICE_STATE_RELEASE_REQUESTED) &&
            !channel_info.sustain())
        {
            _left.volume_envelope.release();
            _left.modulation_envelope.release();
            _left.sampler.release();

            if (_stereo)
            {
                _right.volume_envelope.release();
                _right.modulation_envelope.release();
                _right.sampler.release();
            }

            _voice_state = VOICE_STATE_RELEASED;
        }

        _left.previous_mix_gain = _left.current_mix_gain;
        _right.previous_mix_gain = _right.current_mix_gain;

        bool success = process(channel_info, _left);

        if (_stereo)
            success = process(channel_info, _right) || success;

        if (!success)
            return false;

        if (!_stereo)
        {
            float pan = channel_info.pan() + _left.instrument_pan;
            if ((pan > -50.0f) &&  (pan < 50.0f))
            {
                float angle = (M_PI_2 / 50.0f) * pan;
                float factor = 1.0 + (sqrtf(2.0f) - 1.0) * cosf(angle);

                float gain = _left.current_mix_gain;

                _left.current_mix_gain = gain * (50.0f - pan) / 100.0f * factor;
                _right.current_mix_gain = gain * (50.0f + pan) / 100.0f * factor;
            }
        }
        else
        {
            float pan = channel_info.pan() + _left.instrument_pan;
            if ((pan > -50.0f) &&  (pan < 50.0f))
            {
                float angle = (M_PI_2 / 50.0f) * pan;
                float factor = 1.0 + (sqrtf(2.0f) - 1.0) * cosf(angle);

                _left.current_mix_gain *= (50.0f - pan) / 100.0f * factor;
            }

            pan = channel_info.pan() + _right.instrument_pan;
            if ((pan > -50.0f) &&  (pan < 50.0f))
            {
                float angle = (M_PI_2 / 50.0f) * pan;
                float factor = 1.0 + (sqrtf(2.0f) - 1.0) * cosf(angle);

                _right.current_mix_gain *= (50.0f - pan) / 100.0f * factor;
            }
        }

        _previous_reverb_send = _current_reverb_send;
        _previous_chorus_send = _current_chorus_send;

        if (_stereo)
        {
            _current_reverb_send = clamp(
                channel_info.reverbSend() + (_left.instrument_reverb + _right.instrument_reverb) * 0.5f,
                0, 1
            );
            _current_chorus_send = clamp(
                channel_info.chorusSend() + (_left.instrument_chorus + _right.instrument_chorus) * 0.5f,
                0, 1
            );
        }
        else
        {
            _current_reverb_send = clamp(channel_info.reverbSend() + _left.instrument_reverb, 0, 1);
            _current_chorus_send = clamp(channel_info.chorusSend() + _left.instrument_chorus, 0, 1);
        }

        if (_voice_length == 0)
        {
            _left.previous_mix_gain = _left.current_mix_gain;
            _right.previous_mix_gain = _right.current_mix_gain;
            _previous_reverb_send = _current_reverb_send;
            _previous_chorus_send = _current_chorus_send;
        }

        _voice_length += _synthesizer->settings().blockSize();

        return true;
    }

    //-----------------------------------------------------------------------

    void Voice::start(const sf::sample_info_t& sample_info, const float* buffer, track_t& track)
    {
        if (_velocity > 0)
        {
            // According to the Polyphone's implementation, the initial attenuation should be reduced to 40%.
            float sample_attenuation =
                0.1f * 0.1f * float(sample_info.generator(sf::GEN_TYPE_INITIAL_ATTENUATION, { 0 }).uvalue);

            float filter_attenuation =
                0.5f * 0.1f * float(sample_info.generator(sf::GEN_TYPE_INITIAL_FILTER_Q, { 0 }).uvalue);

            float decibels = -linear_to_decibels(127.0f / _velocity) -
                             sample_attenuation - filter_attenuation;

            track.note_gain = decibels_to_linear(decibels);
        }
        else
        {
            track.note_gain = 0.0f;
        }

        track.cutoff = cents_to_hertz(sample_info.generator(sf::GEN_TYPE_INITIAL_FILTER_CUTOFF_FREQUENCY, { .uvalue=13500 }).uvalue);
        track.resonance = decibels_to_linear(0.1f * float(sample_info.generator(sf::GEN_TYPE_INITIAL_FILTER_Q, { 0 }).uvalue));

        track.vib_lfo_to_pitch = 0.01f * float(sample_info.generator(sf::GEN_TYPE_VIBRATO_LFO_TO_PITCH, { 0 }).ivalue);
        track.mod_lfo_to_pitch = 0.01f * float(sample_info.generator(sf::GEN_TYPE_MODULATION_LFO_TO_PITCH, { 0 }).ivalue);
        track.mod_env_to_pitch = 0.01f * float(sample_info.generator(sf::GEN_TYPE_MODULATION_ENVELOPE_TO_PITCH, { 0 }).ivalue);

        track.mod_lfo_to_cutoff = sample_info.generator(sf::GEN_TYPE_MODULATION_LFO_TO_FILTER_CUTOFF_FREQUENCY, { 0 }).ivalue;
        track.mod_env_to_cutoff = sample_info.generator(sf::GEN_TYPE_MODULATION_ENVELOPE_TO_FILTER_CUTOFF_FREQUENCY, { 0 }).ivalue;
        track.dynamic_cutoff = (track.mod_lfo_to_cutoff != 0) || (track.mod_env_to_cutoff != 0);

        track.mod_lfo_to_volume = 0.1f * float(sample_info.generator(sf::GEN_TYPE_MODULATION_LFO_TO_VOLUME, { 0 }).ivalue);
        track.dynamic_volume = (track.mod_lfo_to_volume > 0.05f);

        track.instrument_pan = clamp(0.1f * float(sample_info.generator(sf::GEN_TYPE_PAN, { 0 }).ivalue), -50.0f, 50.0f);
        track.instrument_reverb = 0.01f * 0.1f * float(sample_info.generator(sf::GEN_TYPE_REVERB_EFFECTS_SEND, { 0 }).uvalue);
        track.instrument_chorus = 0.01f * 0.1f * float(sample_info.generator(sf::GEN_TYPE_CHORUS_EFFECTS_SEND, { 0 }).uvalue);

        {
            float delay = timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_DELAY_VOLUME_ENVELOPE, { .ivalue=-12000 }).ivalue);

            float attack = timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_ATTACK_VOLUME_ENVELOPE, { .ivalue=-12000 }).ivalue);

            float hold = timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_HOLD_VOLUME_ENVELOPE, { .ivalue=-12000 }).ivalue) *
                key_number_to_multiplying_factor(sample_info.generator(sf::GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_HOLD, { 0 }).ivalue, _key);

            float decay = timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_DECAY_VOLUME_ENVELOPE, { .ivalue=-12000 }).ivalue) *
                key_number_to_multiplying_factor(sample_info.generator(sf::GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_DECAY, { 0 }).ivalue, _key);

            float sustain = decibels_to_linear(-0.1f * float(sample_info.generator(sf::GEN_TYPE_SUSTAIN_VOLUME_ENVELOPE, { 0 }).uvalue));

            float release = fmax(timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_RELEASE_VOLUME_ENVELOPE, { .ivalue=-12000 }).ivalue), 0.01f);

            track.volume_envelope.start(delay, attack, hold, decay, sustain, release);
        }

        {
            float delay = timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_DELAY_MODULATION_ENVELOPE, { .ivalue=-12000 }).ivalue);

            float attack = timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_ATTACK_MODULATION_ENVELOPE, { .ivalue=-12000 }).ivalue) *
                           ((145.0f - _velocity) / 144.0f);

            float hold = timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_HOLD_MODULATION_ENVELOPE, { .ivalue=-12000 }).ivalue) *
                key_number_to_multiplying_factor(sample_info.generator(sf::GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_HOLD, { 0 }).ivalue, _key);

            float decay = timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_DECAY_MODULATION_ENVELOPE, { .ivalue=-12000 }).ivalue) *
                key_number_to_multiplying_factor(sample_info.generator(sf::GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_DECAY, { 0 }).ivalue, _key);

            float sustain = 1.0f - float(sample_info.generator(sf::GEN_TYPE_SUSTAIN_MODULATION_ENVELOPE, { 0 }).uvalue) / 100.0f;

            float release = timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_RELEASE_MODULATION_ENVELOPE, { .ivalue=-12000 }).ivalue);

            track.modulation_envelope.start(delay, attack, hold, decay, sustain, release);
        }

        track.vibrato_lfo.start(
            timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_DELAY_VIBRATO_LFO, { .ivalue=-12000 }).ivalue),
            cents_to_hertz(sample_info.generator(sf::GEN_TYPE_FREQUENCY_VIBRATO_LFO, { 0 }).ivalue)
        );

        track.modulation_lfo.start(
            timecents_to_seconds(sample_info.generator(sf::GEN_TYPE_DELAY_MODULATION_LFO, { .ivalue=-12000 }).ivalue),
            cents_to_hertz(sample_info.generator(sf::GEN_TYPE_FREQUENCY_MODULATION_LFO, { 0 }).ivalue)
        );

        int overriding_root_key = sample_info.generator(sf::GEN_TYPE_OVERRIDING_ROOT_KEY, { .ivalue=-1 }).ivalue;
        loop_mode_t loop_mode = static_cast<loop_mode_t>(sample_info.generator(sf::GEN_TYPE_SAMPLE_MODES, { 0 }).uvalue);
        int8_t coarse_tune = sample_info.generator(sf::GEN_TYPE_COARSE_TUNE, { 0 }).ivalue;
        int8_t fine_tune = sample_info.generator(sf::GEN_TYPE_FINE_TUNE, { 0 }).ivalue;
        int16_t scale_tuning = sample_info.generator(sf::GEN_TYPE_SCALE_TUNING, { 100 }).uvalue;

        uint8_t root_key = (overriding_root_key != -1 ? uint8_t(overriding_root_key) : sample_info.sample->original_pitch);

        track.sampler.start(
            buffer,
            sample_info.sample->start, sample_info.sample->end,
            loop_mode,
            sample_info.sample->loop_start, sample_info.sample->loop_end,
            sample_info.sample->sample_rate, root_key,
            coarse_tune,
            fine_tune + sample_info.sample->pitch_correction,
            scale_tuning
        );

        track.filter.clearBuffer();
        track.filter.setLowPassFilter(track.cutoff, track.resonance);

        track.smoothed_cutoff = track.cutoff;
    }

    //-----------------------------------------------------------------------

    bool Voice::process(const Channel& channel_info, track_t& track)
    {
        if (!track.volume_envelope.process(_synthesizer->settings().blockSize()))
            return false;

        track.modulation_envelope.process(_synthesizer->settings().blockSize());
        track.vibrato_lfo.process();
        track.modulation_lfo.process();

        float vib_pitch_change = (0.01f * channel_info.modulation() + track.vib_lfo_to_pitch) * track.vibrato_lfo.value();
        float mod_pitch_change = track.mod_lfo_to_pitch * track.modulation_lfo.value() +
                                 track.mod_env_to_pitch * track.modulation_envelope.getValue();

        float channel_pitch_change = channel_info.tune() + channel_info.pitchBend();
        float pitch = _key + vib_pitch_change + mod_pitch_change + channel_pitch_change;

        if (!track.sampler.process(track.block, _synthesizer->settings().blockSize(), pitch))
            return false;

        if (track.dynamic_cutoff)
        {
            float cents = float(track.mod_lfo_to_cutoff) * track.modulation_lfo.value() +
                          float(track.mod_env_to_cutoff) * track.modulation_envelope.getValue();

            float factor = cents_to_multiplying_factor(cents);
            float new_cutoff = factor * track.cutoff;

            // The cutoff change is limited within x0.5 and x2 to reduce pop noise
            float lower_limit = 0.5 * track.smoothed_cutoff;
            float upper_limit = 2.0 * track.smoothed_cutoff;
            if (new_cutoff < lower_limit)
                track.smoothed_cutoff = lower_limit;
            else if (new_cutoff > upper_limit)
                track.smoothed_cutoff = upper_limit;
            else
                track.smoothed_cutoff = new_cutoff;

            track.filter.setLowPassFilter(track.smoothed_cutoff, track.resonance);
        }

        track.filter.process(track.block, _synthesizer->settings().blockSize());

        float channel_gain = decibels_to_linear(channel_info.volume()) * channel_info.expression();

        float mix_gain = track.note_gain * channel_gain * track.volume_envelope.getValue();
        if (track.dynamic_volume)
        {
            float decibels = track.mod_lfo_to_volume * track.modulation_lfo.value();
            mix_gain *= decibels_to_linear(decibels);
        }

        track.current_mix_gain = mix_gain;

        return true;
    }


    /******************************** VOICE COLLECTION **********************************/

    class VoiceCollection
    {
    public:
        VoiceCollection(const Synthesizer* synthesizer);
        ~VoiceCollection();

        Voice* request(uint8_t channel, uint8_t exclusive_class);
        void process();
        void clear();
    
        inline size_t nbActiveVoices() const
        {
            return _nb_active_voices;            
        }

        inline std::vector<Voice*>& voices()
        {
            return _voices;            
        }


        //_____ Attributes __________
    private:
        std::vector<Voice*> _voices;
        size_t _nb_active_voices = 0;
    };

    //-----------------------------------------------------------------------

    VoiceCollection::VoiceCollection(const Synthesizer* synthesizer)
    {
        for (int i = 0; i < synthesizer->settings().maximumPolyphony(); ++i)
            _voices.push_back(new Voice(synthesizer));
    }

    //-----------------------------------------------------------------------

    VoiceCollection::~VoiceCollection()
    {
        for (auto voice : _voices)
            delete voice;
    }

    //-----------------------------------------------------------------------

    Voice* VoiceCollection::request(uint8_t channel, uint8_t exclusive_class)
    {
        // If an exclusive class is assigned to the region, find a voice with the same class.
        // If found, reuse it to avoid playing multiple voices with the same class at a time.
        if (exclusive_class != 0)
        {
            for (int i = 0; i < _nb_active_voices; ++i)
            {
                Voice* voice = _voices[i];
                if ((voice->exclusiveClass() == exclusive_class) && (voice->channel() == channel))
                    return voice;
            }
        }

        // If the number of active voices is less than the limit, use a free one
        if (_nb_active_voices < _voices.size())
        {
            Voice* voice = _voices[_nb_active_voices];
            ++_nb_active_voices;
            return voice;
        }

        // Too many active voices: find the one which has the lowest priority
        Voice* candidate = nullptr;
        float lowest_priority = 1000000.0f;

        for (auto voice : _voices)
        {
            float priority = voice->priority();

            if (priority < lowest_priority)
            {
                lowest_priority = priority;
                candidate = voice;
            }
            else if (priority == lowest_priority)
            {
                // Same priority : the older one should be more suitable for reuse
                if (voice->voiceLength() > candidate->voiceLength())
                    candidate = voice;
            }
        }

        return candidate;
    }

    //-----------------------------------------------------------------------

    void VoiceCollection::process()
    {
        int i = 0;

        while (i != _nb_active_voices)
        {
            if (_voices[i]->process())
            {
                ++i;
            }
            else
            {
                --_nb_active_voices;

                auto tmp = _voices[i];
                _voices[i] = _voices[_nb_active_voices];
                _voices[_nb_active_voices] = tmp;
            }
        }
    }

    //-----------------------------------------------------------------------

    void VoiceCollection::clear()
    {
        _nb_active_voices = 0;
    }


    /****************************** SYNTHESIZER SETTINGS ********************************/

    SynthesizerSettings::SynthesizerSettings(uint32_t sample_rate)
    {
        setSampleRate(sample_rate);
        _block_size = DEFAULT_BLOCK_SIZE;
        _maximum_polyphony = DEFAULT_MAXIMUM_POLYPHONY;
        _reverb_and_chorus_enabled = DEFAULT_REVERB_AND_CHORUS_ENABLED;
    }

    //-----------------------------------------------------------------------

    void SynthesizerSettings::setSampleRate(uint32_t sample_rate)
    {
        if ((sample_rate < 16000) || (sample_rate > 192000))
            throw std::runtime_error(std::string("The sample rate must be between 16000 and 192000."));

        _sample_rate = sample_rate;
    }

    //-----------------------------------------------------------------------

    void SynthesizerSettings::setBlockSize(uint16_t block_size)
    {
        if ((block_size < 8) || (block_size > 1024))
            throw std::runtime_error(std::string("The block size must be between 8 and 1024."));

        _block_size = block_size;
    }

    //-----------------------------------------------------------------------

    void SynthesizerSettings::setMaximumPolyphony(uint16_t maximum_polyphony)
    {
        if ((maximum_polyphony < 8) || (maximum_polyphony > 256))
            throw std::runtime_error(std::string("The block size must be between 8 and 256."));

            _maximum_polyphony = maximum_polyphony;
    }

    //-----------------------------------------------------------------------

    void SynthesizerSettings::enableReverbAndChorus(bool enable)
    {
        _reverb_and_chorus_enabled = enable;
    }


    /*********************************** SYNTHESIZER ************************************/

    Synthesizer::Synthesizer(const SynthesizerSettings& settings)
    : _settings(settings)
    {
        for (int i = 0; i < CHANNEL_COUNT; ++i)
            _channels.emplace_back(Channel(i == PERCUSSION_CHANNEL));

        _voices = new VoiceCollection(this);

        _block_left = new float[_settings.blockSize()];
        _block_right = new float[_settings.blockSize()];
        _blocks_offset = _settings.blockSize();
        _inverse_block_size = 1.0f / float(_settings.blockSize());
    }

    //-----------------------------------------------------------------------

    Synthesizer::~Synthesizer()
    {
        delete[] _block_left;
        delete[] _block_right;
        delete _voices;
    }

    //-----------------------------------------------------------------------

    bool Synthesizer::loadSoundFont(const std::filesystem::path& path)
    {
        if (!_soundfont.load(path))
            return false;

        _default_preset = _soundfont.getPresets().begin()->first;

        return true;
    }

    //-----------------------------------------------------------------------

    bool Synthesizer::loadSoundFont(const char* buffer, size_t size)
    {
        if (!_soundfont.load(buffer, size))
            return false;

        _default_preset = _soundfont.getPresets().begin()->first;

        return true;
    }

    //-----------------------------------------------------------------------

    bool Synthesizer::processMidiMessage(
        uint8_t channel, uint8_t command, uint8_t data1, uint8_t data2
    )
    {
        if (channel >= _channels.size())
            return false;

        auto& channel_info = _channels[channel];

        switch (command)
        {
            // Note Off
            case 0x80: 
                noteOff(channel, data1);
                break;

            // Note On
            case 0x90:
                noteOn(channel, data1, data2);
                break;

            // Controller
            case 0xB0:
            {
                switch (data1)
                {
                    // Bank Selection
                    case 0x00:
                        channel_info.setBank(data2);
                        break;

                    // Modulation Coarse
                    case 0x01:
                        channel_info.setModulationCoarse(data2);
                        break;

                    // Modulation Fine
                    case 0x21:
                        channel_info.setModulationFine(data2);
                        break;

                    // Data Entry Coarse
                    case 0x06:
                        channel_info.setDataEntryCoarse(data2);
                        break;

                    // Data Entry Fine
                    case 0x26:
                        channel_info.setDataEntryFine(data2);
                        break;

                    // Channel Volume Coarse
                    case 0x07:
                        channel_info.setVolumeCoarse(data2);
                        break;

                    // Channel Volume Fine
                    case 0x27:
                        channel_info.setVolumeFine(data2);
                        break;

                    // Pan Coarse
                    case 0x0A:
                        channel_info.setPanCoarse(data2);
                        break;

                    // Pan Fine
                    case 0x2A:
                        channel_info.setPanFine(data2);
                        break;

                    // Expression Coarse
                    case 0x0B:
                        channel_info.setExpressionCoarse(data2);
                        break;

                    // Expression Fine
                    case 0x2B:
                        channel_info.setExpressionFine(data2);
                        break;

                    // Sustain
                    case 0x40:
                        channel_info.setSustain(data2);
                        break;

                    // Reverb Send
                    case 0x5B:
                        channel_info.setReverbSend(data2);
                        break;

                    // Chorus Send
                    case 0x5D:
                        channel_info.setChorusSend(data2);
                        break;

                    // RPN Coarse
                    case 0x65:
                        channel_info.setRPNCoarse(data2);
                        break;

                    // RPN Fine
                    case 0x64:
                        channel_info.setRPNFine(data2);
                        break;

                    // All Sound Off
                    case 0x78:
                        allNotesOff(channel, true);
                        break;

                    // Reset All Controllers
                    case 0x79:
                        resetControllers(channel);
                        break;

                    // All Note Off
                    case 0x7B:
                        allNotesOff(channel, false);
                        break;

                    default:
                        break;
                }
           
                break;
            }

            // Program Change
            case 0xC0:
                channel_info.setPreset(data1);
                break;

            // Pitch Bend
            case 0xE0:
                channel_info.setPitchBend(data1, data2);

            default:
                break;
        }

        return true;
    }

    //-----------------------------------------------------------------------

    void Synthesizer::noteOff(uint8_t channel, uint8_t key)
    {
        if (channel >= _channels.size())
            return;

        auto& voices = _voices->voices();
        for (int i = 0; i < _voices->nbActiveVoices(); ++i)
        {
            Voice* voice = voices[i];
            if ((voice->channel() == channel) && (voice->key() == key))
                voice->end();
        }
    }

    //-----------------------------------------------------------------------

    void Synthesizer::noteOn(uint8_t channel, uint8_t key, uint8_t velocity)
    {
        if (velocity == 0)
        {
            noteOff(channel, key);
            return;
        }

        if (channel >= _channels.size())
            return;

        auto& channel_info = _channels[channel];

        sf::preset_id_t preset_id = { channel_info.bank(), channel_info.preset() };

        sf::key_info_t key_info;
        if (!_soundfont.getKeyInfo(preset_id.bank, preset_id.number, key, velocity, key_info))
        {
            // Try fallback to the GM sound set.
            // Normally, the given preset number + the bank number 0 will work.
            // For drums (bank number >= 128), it seems to be better to select the standard set (128:0).
            if (channel_info.bank() < 128)
            {
                preset_id.bank = 0;
            }
            else
            {
                preset_id.bank = 128;
                preset_id.number = 0;
            }

            if (!_soundfont.getKeyInfo(preset_id.bank, preset_id.number, key, velocity, key_info))
            {
                // No corresponding preset was found. Use the default one.
                _soundfont.getKeyInfo(_default_preset.bank, _default_preset.number, key, velocity, key_info);
            }
        }

        Voice* voice = _voices->request(channel, key_info.left.generator(sf::GEN_TYPE_EXCLUSIVE_CLASS, { 0 }).uvalue);
        voice->start(key_info, _soundfont.getBuffer(), channel, key, velocity);
    }

    //-----------------------------------------------------------------------

    void Synthesizer::allNotesOff(bool immediate)
    {
        if (immediate)
        {
            _voices->clear();
        }
        else
        {
            auto& voices = _voices->voices();
            for (int i = 0; i < _voices->nbActiveVoices(); ++i)
                voices[i]->end();
        }
    }

    //-----------------------------------------------------------------------

    void Synthesizer::allNotesOff(uint8_t channel, bool immediate)
    {
        if (immediate)
        {
            auto& voices = _voices->voices();
            for (int i = 0; i < _voices->nbActiveVoices(); ++i)
            {
                Voice* voice = voices[i];
                if (voice->channel() == channel)
                    voice->kill();
            }
        }
        else
        {
            auto& voices = _voices->voices();
            for (int i = 0; i < _voices->nbActiveVoices(); ++i)
            {
                Voice* voice = voices[i];
                if (voice->channel() == channel)
                    voice->end();
            }
        }
    }

    //-----------------------------------------------------------------------

    void Synthesizer::resetAllControllers()
    {
        for (auto& channel : _channels)
            channel.resetControllers();
    }

    //-----------------------------------------------------------------------

    void Synthesizer::resetControllers(uint8_t channel)
    {
        if (channel >= _channels.size())
            return;

        _channels[channel].resetControllers();
    }

    //-----------------------------------------------------------------------

    void Synthesizer::reset()
    {
        _voices->clear();

        for (auto& channel : _channels)
            channel.reset();
    
        _blocks_offset = _settings.blockSize();
        _nb_rendered_samples = 0;
    }

    //-----------------------------------------------------------------------

    void Synthesizer::render(float* left, float* right, size_t size)
    {
        size_t nb_written = 0;

        while (nb_written < size)
        {
            if (_blocks_offset == _settings.blockSize())
            {
                renderBlockStereo();
                _blocks_offset = 0;
            }

            size_t src_remainder = _settings.blockSize() - _blocks_offset;
            size_t dst_remainder = size - nb_written;
            size_t remainder = fmin(src_remainder, dst_remainder);

            for (int t = 0; t < remainder; ++t)
            {
                left[nb_written + t] = _block_left[_blocks_offset + t];
                right[nb_written + t] = _block_right[_blocks_offset + t];
            }

            _blocks_offset += remainder;
            nb_written += remainder;
        }

        _nb_rendered_samples += nb_written;
    }

    //-----------------------------------------------------------------------

    void Synthesizer::render(float* buffer, size_t size)
    {
        size_t nb_written = 0;

        while (nb_written < size)
        {
            if (_blocks_offset == _settings.blockSize())
            {
                renderBlockMono();
                _blocks_offset = 0;
            }

            size_t src_remainder = _settings.blockSize() - _blocks_offset;
            size_t dst_remainder = size - nb_written;
            size_t remainder = fmin(src_remainder, dst_remainder);

            for (int t = 0; t < remainder; ++t)
                buffer[nb_written + t] = _block_left[_blocks_offset + t];

            _blocks_offset += remainder;
            nb_written += remainder;
        }

        _nb_rendered_samples += nb_written;
    }

    //-----------------------------------------------------------------------

    uint16_t Synthesizer::nbActiveVoices() const
    {
        return _voices->nbActiveVoices();
    }

    //-----------------------------------------------------------------------

    float Synthesizer::masterVolume() const
    {
        return linear_to_decibels(_master_volume);
    }

    //-----------------------------------------------------------------------

    void Synthesizer::setMasterVolume(float volume)
    {
        _master_volume = decibels_to_linear(volume);
    }

    //-----------------------------------------------------------------------

    bool Synthesizer::configureChannel(uint8_t channel, uint8_t bank, uint8_t preset)
    {
        if (channel >= _channels.size())
            return false;

        const sf::preset_t* p = _soundfont.getPreset(bank, preset);
        if (!p)
            return false;

        Channel& c = _channels[channel];
        c.setBank(bank);
        c.setPreset(preset);

        return true;
    }

    //-----------------------------------------------------------------------

    std::map<sf::preset_id_t, std::string> Synthesizer::presetNames()
    {
        std::map<sf::preset_id_t, std::string> result;

        for (const auto& preset : _soundfont.getPresets())
            result[preset.first] = preset.second.name;

        return result;
    }

    //-----------------------------------------------------------------------

    void Synthesizer::renderBlockStereo()
    {
        _voices->process();

       memset((char*) _block_left, 0, _settings.blockSize() * sizeof(float));
       memset((char*) _block_right, 0, _settings.blockSize() * sizeof(float));

        auto& voices = _voices->voices();
        for (int i = 0; i < _voices->nbActiveVoices(); ++i)
        {
            Voice* voice = voices[i];

            float previous_gain_left = _master_volume * voice->previousMixGainLeft();
            float current_gain_left = _master_volume * voice->currentMixGainLeft();
            writeBlock(
                previous_gain_left, current_gain_left, voice->block_left(), _block_left
            );

            float previous_gain_right = _master_volume * voice->previousMixGainRight();
            float current_gain_right = _master_volume * voice->currentMixGainRight();
            writeBlock(
                previous_gain_right, current_gain_right,
                voice->stereo() ? voice->block_right() : voice->block_left(),
                _block_right
            );
        }
    }

    //-----------------------------------------------------------------------

    void Synthesizer::renderBlockMono()
    {
        _voices->process();

       memset((char*) _block_left, 0, _settings.blockSize() * sizeof(float));

        auto& voices = _voices->voices();
        for (int i = 0; i < _voices->nbActiveVoices(); ++i)
        {
            Voice* voice = voices[i];

            if (voice->stereo())
            {
                float previous_gain_left = _master_volume * voice->previousMixGainLeft();
                float current_gain_left = _master_volume * voice->currentMixGainLeft();
                writeBlock(
                    previous_gain_left, current_gain_left, voice->block_left(), _block_left
                );
    
                float previous_gain_right = _master_volume * voice->previousMixGainRight();
                float current_gain_right = _master_volume * voice->currentMixGainRight();
                writeBlock(
                    previous_gain_right, current_gain_right, voice->block_right(), _block_left
                );
            }
            else
            {
                float previous_gain = _master_volume * voice->previousMixGainLeft();
                float current_gain = _master_volume * voice->currentMixGainLeft();
                writeBlock(
                    previous_gain, current_gain, voice->block_left(), _block_left
                );
            }
        }
    }

    //-----------------------------------------------------------------------

    void Synthesizer::writeBlock(
        float previous_gain, float current_gain, float* source, float* destination
    )
    {
        if (fmax(previous_gain, current_gain) < NON_AUDIBLE)
            return;

        if (fabs(current_gain - previous_gain) < 1.0e-3)
        {
            for (int i = 0; i < _settings.blockSize(); ++i)
                destination[i] += current_gain * source[i];
        }
        else
        {
            float step = _inverse_block_size * (current_gain - previous_gain);
            float gain = previous_gain;

            for (int i = 0; i < _settings.blockSize(); ++i)
            {
                destination[i] += gain * source[i];
                gain += step;
            }
        }
    }


#endif // KNM_SYNTHESIZER_IMPLEMENTATION

}
}
