/** knm::soundfont - v1.0.0 - MIT License

DESCRIPTION

    A C++ library to parse SoundFont (2.x) files.

    knm::soundfont is a single-file, header-only C++ library, without any dependency.

    To know how to use the information in a SoundFont file, please refer to the
    SoundFont v2.01 specification at: https://www.synthfont.com/SFSPEC21.PDF

    Notes:
        1) No synthesis is performed, the objective is only to parse a SoundFont file
        and provide a simple interface to retrieve informations from it (to maybe
        implement a synthetizer).

        2) Some sanity checks aren't implemented yet (there are numerous conditions
        that should lead to the SoundFont file to be declared invalid, but for now
        it is assumed that the file is valid).

    Everything is defined in the `knm::sf` namespace (`knm` for my username, *Kanma*,
    and `sf` for *SoundFont*).


USAGE

    1. Copy this file in a convenient place for your project

    2. In *one* C++ file, add the following code to create the implementation:

        #define KNM_SOUNDFONT_IMPLEMENTATION
        #include <knm_soundfont.hpp>

    In other files, just use #include <knm_soundfont.hpp>

    Here is an example using the library to retrieve the data about key 60 (C4)
    at velocity 20 in bank 0, preset 0:

        #define KNM_SOUNDFONT_IMPLEMENTATION
        #include <knm_soundfont.hpp>

        #include <iostream>

        using namespace knm::sf;

        int main(int argc, char** argv)
        {
            uint16_t bank = 0;
            uint16_t preset = 0;
            uint16_t key = 60;
            uint16_t velocity = 20;

            // Load the SoundFont file
            SoundFont soundfont;

            if (!soundfont.load("/path/to/sounfont/file.sf2"))
            {
                std::cerr << "Failed to load SoundFont file" << std::endl;
                return 1;
            }

            // Retrieve the data about the key
            key_info_t info;
            if (!soundfont.getKeyInfo(bank, preset, key, velocity, info))
            {
                std::cerr << "Key not found in the SoundFont file" << std::endl;
                return 1;
            }

            // Do something with 'info'

            return 0;
        }


LICENSE

    knm::soundfont is made available under the MIT License.

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

#include <filesystem>
#include <string>
#include <map>
#include <vector>


#ifdef KNM_SOUNDFONT_IMPLEMENTATION
    #include <fstream>
    #include <streambuf>
    #include <string>
    #include <cstring>
#endif


#define KNM_SOUNDFONT_VERSION_MAJOR 1
#define KNM_SOUNDFONT_VERSION_MINOR 0
#define KNM_SOUNDFONT_VERSION_PATCH 0


namespace knm {
namespace sf {

    //------------------------------------------------------------------------------------
    /// @brief  Contains all the non-audio information about a SoundFont file
    //------------------------------------------------------------------------------------
    struct information_t
    {
        uint16_t major_version = 0;     ///< Major part of the SoundFont specification
                                        ///  version level to which the file complies
        uint16_t minor_version = 0;     ///< Minor part of the SoundFont specification
                                        ///  version level to which the file complies
        std::string name;               ///< The name of the SoundFont compatible bank
        std::string target_engine;      ///< The wavetable sound engine for which the file
                                        ///  was optimized
        std::string rom_name;           ///< (optional) The wavetable sound data ROM to
                                        ///  which any ROM samples refer
        uint16_t rom_major_version = 0; ///< (optional) The major part of the version of
                                        ///  the wavetable sound data ROM
        uint16_t rom_minor_version = 0; ///< (optional) The minor part of the version of
                                        ///  the wavetable sound data ROM
        std::string creation_date;      ///< (optional) The creation date of the SoundFont
                                        ///  compatible bank
        std::string engineers;          ///< (optional) The names of any sound designers
                                        ///  or engineers responsible for the SoundFont
                                        ///  compatible bank
        std::string product;            ///< (optional) Any specific product for which
                                        ///  the SoundFont compatible bank is intended
        std::string copyright;          ///< (optional) Any copyright assertion string
                                        ///  associated with the SoundFont compatible bank
        std::string comments;           ///< (optional) Any comments associated with the
                                        ///  SoundFont compatible bank
        std::string creation_tool;      ///< (optional) The SoundFont compatible tools
                                        ///  used to create and most recently modify the
                                        ///  SoundFont compatible bank.
    };


    //------------------------------------------------------------------------------------
    /// @brief  The different types of generators
    //------------------------------------------------------------------------------------
    enum generator_type_t
    {
        GEN_TYPE_START_ADDRESS_OFFSET = 0,
        GEN_TYPE_END_ADDRESS_OFFSET = 1,
        GEN_TYPE_START_LOOP_ADDRESS_OFFSET = 2,
        GEN_TYPE_END_LOOP_ADDRESS_OFFSET = 3,
        GEN_TYPE_START_ADDRESS_COARSE_OFFSET = 4,
        GEN_TYPE_MODULATION_LFO_TO_PITCH = 5,
        GEN_TYPE_VIBRATO_LFO_TO_PITCH = 6,
        GEN_TYPE_MODULATION_ENVELOPE_TO_PITCH = 7,
        GEN_TYPE_INITIAL_FILTER_CUTOFF_FREQUENCY = 8,
        GEN_TYPE_INITIAL_FILTER_Q = 9,
        GEN_TYPE_MODULATION_LFO_TO_FILTER_CUTOFF_FREQUENCY = 10,
        GEN_TYPE_MODULATION_ENVELOPE_TO_FILTER_CUTOFF_FREQUENCY = 11,
        GEN_TYPE_END_ADDRESS_COARSE_OFFSET = 12,
        GEN_TYPE_MODULATION_LFO_TO_VOLUME = 13,
        GEN_TYPE_UNUSED_1 = 14,
        GEN_TYPE_CHORUS_EFFECTS_SEND = 15,
        GEN_TYPE_REVERB_EFFECTS_SEND = 16,
        GEN_TYPE_PAN = 17,
        GEN_TYPE_UNUSED_2 = 18,
        GEN_TYPE_UNUSED_3 = 19,
        GEN_TYPE_UNUSED_4 = 20,
        GEN_TYPE_DELAY_MODULATION_LFO = 21,
        GEN_TYPE_FREQUENCY_MODULATION_LFO = 22,
        GEN_TYPE_DELAY_VIBRATO_LFO = 23,
        GEN_TYPE_FREQUENCY_VIBRATO_LFO = 24,
        GEN_TYPE_DELAY_MODULATION_ENVELOPE = 25,
        GEN_TYPE_ATTACK_MODULATION_ENVELOPE = 26,
        GEN_TYPE_HOLD_MODULATION_ENVELOPE = 27,
        GEN_TYPE_DECAY_MODULATION_ENVELOPE = 28,
        GEN_TYPE_SUSTAIN_MODULATION_ENVELOPE = 29,
        GEN_TYPE_RELEASE_MODULATION_ENVELOPE = 30,
        GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_HOLD = 31,
        GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_DECAY = 32,
        GEN_TYPE_DELAY_VOLUME_ENVELOPE = 33,
        GEN_TYPE_ATTACK_VOLUME_ENVELOPE = 34,
        GEN_TYPE_HOLD_VOLUME_ENVELOPE = 35,
        GEN_TYPE_DECAY_VOLUME_ENVELOPE = 36,
        GEN_TYPE_SUSTAIN_VOLUME_ENVELOPE = 37,
        GEN_TYPE_RELEASE_VOLUME_ENVELOPE = 38,
        GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_HOLD = 39,
        GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_DECAY = 40,
        GEN_TYPE_INSTRUMENT = 41,
        GEN_TYPE_RESERVED_1 = 42,
        GEN_TYPE_KEY_RANGE = 43,
        GEN_TYPE_VELOCITY_RANGE = 44,
        GEN_TYPE_START_LOOP_ADDRESS_COARSE_OFFSET = 45,
        GEN_TYPE_KEY_NUMBER = 46,
        GEN_TYPE_VELOCITY = 47,
        GEN_TYPE_INITIAL_ATTENUATION = 48,
        GEN_TYPE_RESERVED_2 = 49,
        GEN_TYPE_END_LOOP_ADDRESS_COARSE_OFFSET = 50,
        GEN_TYPE_COARSE_TUNE = 51,
        GEN_TYPE_FINE_TUNE = 52,
        GEN_TYPE_SAMPLE_ID = 53,
        GEN_TYPE_SAMPLE_MODES = 54,
        GEN_TYPE_RESERVED_3 = 55,
        GEN_TYPE_SCALE_TUNING = 56,
        GEN_TYPE_EXCLUSIVE_CLASS = 57,
        GEN_TYPE_OVERRIDING_ROOT_KEY = 58,
        GEN_TYPE_UNUSED_5 = 59,
        GEN_TYPE_UNUSED_END = 60,
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a range of values
    //------------------------------------------------------------------------------------
    struct range_t
    {
        int8_t lo;  ///< The lower bound of the range
        int8_t hi;  ///< The upper bound of the range
    };


    //------------------------------------------------------------------------------------
    /// @brief  A union that represents the value or range associated with a generator
    //------------------------------------------------------------------------------------
    union generator_amount_t
    {
        range_t range;      ///< A range of values
        int16_t ivalue;     ///< A signed integer value
        uint16_t uvalue;    ///< An unsigned integer value
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a list of generators
    //------------------------------------------------------------------------------------
    typedef std::map<generator_type_t, generator_amount_t> generator_map_t;


    //------------------------------------------------------------------------------------
    /// @brief  The different modulator source types (for General Controller modulator
    ///         sources)
    //------------------------------------------------------------------------------------
    enum modulator_controller_source_t
    {   
        /// @brief No controller is to be used. The output of this controller module should
        /// be treated as if its value were set to ‘1’. It should not be a means to
        /// turn off a modulator.
        MOD_CTRL_SRC_NONE = 0,

        /// @brief The controller source to be used is the velocity value which is sent from
        /// the MIDI note-on command which generated the given sound.
        MOD_CTRL_SRC_NOTE_ON_VELOCITY = 2,

        /// @brief The controller source to be used is the key number value which was
        /// sent from the MIDI note-on command which generated the given sound.
        MOD_CTRL_SRC_NOTE_ON_KEY_NUMBER = 3,

        /// @brief The controller source to be used is the poly-pressure amount that is
        /// sent from the MIDI poly-pressure command.
        MOD_CTRL_SRC_POLY_PRESSURE = 10,

        /// @brief The controller source to be used is the channel pressure amount that is
        /// sent from the MIDI channel-pressure command.
        MOD_CTRL_SRC_CHANNEL_PRESSURE = 13,

        /// @brief The controller source to be used is the pitch wheel amount which is
        /// sent from the MIDI pitch wheel command.
        MOD_CTRL_SRC_PITCH_WHEEL = 14,

        /// @brief The controller source to be used is the pitch wheel sensitivity amount
        /// which is sent from the MIDI RPN 0 pitch wheel sensitivity command.
        MOD_CTRL_SRC_PITCH_WHEEL_SENSITIVITY = 16,
    };


    //------------------------------------------------------------------------------------
    /// @brief  The different modulator source directions
    //------------------------------------------------------------------------------------
    enum modulator_source_direction_t
    {
        /// @brief The direction of the controller should be from the minimum value to the
        /// maximum value.
        MOD_SRC_DIR_MIN_TO_MAX = 0,

        /// @brief The direction of the controller should be from the maximum value to the
        /// minimum value.
        MOD_SRC_DIR_MAX_TO_MIN = 1,
    };


    //------------------------------------------------------------------------------------
    /// @brief  The different modulator source polarities
    //------------------------------------------------------------------------------------
    enum modulator_source_polarity_t
    {
        /// @brief The controller should be mapped with a minimum value of 0 and a maximum
        /// value of 1.
        MOD_SRC_POL_UNIPOLAR = 0,

        /// @brief The controller should be mapped with a minimum value of -1 and a maximum
        /// value of 1.
        MOD_SRC_POL_BIPOLAR= 1,
    };


    //------------------------------------------------------------------------------------
    /// @brief  The different modulator source types
    //------------------------------------------------------------------------------------
    enum modulator_source_type_t
    {
        /// @brief The SoundFont modulator controller moves linearly from the minimum
        /// to the maximum value in the direction and with the polarity specified.
        MOD_SRC_TYPE_LINEAR = 0,

        /// @brief The SoundFont modulator controller moves in a concave fashion from
        /// the minimum to the maximum value in the direction and with the
        /// polarity specified. The negative unipolar concave characteristic follows
        /// variations of the mathematical equation:
        /// output = -20/96 * log((value^2)/(range^2))
        /// where value = input value - min value
        /// range = max value – min value
        MOD_SRC_TYPE_CONCAVE= 1,

        /// @brief The SoundFont modulator controller moves in a convex fashion from
        /// the minimum to the maximum value in the direction and with the
        /// polarity specified. The convex curve is the same curve as the concave
        /// curve, except the start and end points are reversed.
        MOD_SRC_TYPE_CONVEX = 2,

        /// @brief The SoundFont modulator controller output is at a minimum value
        /// while the controller input moves from the minimum to half of the
        /// maximum, after which the controller output is at a maximum. This
        /// occurs in the direction and with the polarity specified.
        MOD_SRC_TYPE_SWITCH = 3,
    };


    //------------------------------------------------------------------------------------
    /// @brief  The different types of modulator controller
    //------------------------------------------------------------------------------------
    enum modulator_controller_type_t
    {
        /// @brief The General Controller palette of controllers is selected.
        /// The 'source' field value corresponds to one of the controller sources.
        MOD_CTRL_TYPE_SRC = 0,

        /// @brief the MIDI Controller Palette is selected. The 'source' field
        /// value corresponds to one of the 128 MIDI Continuous Controller messages
        /// as defined in the MIDI specification.
        MOD_CTRL_TYPE_MIDI = 1,
    };

 
    //------------------------------------------------------------------------------------
    /// @brief  The different types of modulator transforms
    ///
    /// The SoundFont v2.1 specification only defines one transform.
    //------------------------------------------------------------------------------------
    enum modulator_transform_t
    {
        MOD_TRANSFORM_LINEAR = 0,
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a modulator source
    //------------------------------------------------------------------------------------
    struct modulator_source_t
    {
        modulator_source_type_t type;                   ///< Type of the source (linear,
                                                        ///  convex, ...)
        modulator_source_direction_t direction;         ///< Direction (min to max, max to
                                                        ///  min)
        modulator_source_polarity_t polarity;           ///< Polarity (unipolar, bipolar)
        modulator_controller_type_t controller_type;    ///< The controller type (general
                                                        ///  or MIDI)
        union
        {
            modulator_controller_source_t source;       ///< The source of the modulator
                                                        ///  (if general controller)
            uint8_t midi;                               ///< The MIDI controller number
                                                        ///  (if MIDI controller)
        };

        /// @brief Comparison operator for use in maps
        inline bool operator<(const modulator_source_t& other) const
        {
            return (type < other.type) ||
                   ((type == other.type) &&
                    ((direction < other.direction) ||
                     ((direction == other.direction) &&
                      ((polarity < other.polarity) ||
                       ((polarity == other.polarity) &&
                        ((controller_type < other.controller_type) ||
                         ((controller_type == other.controller_type) && (midi < other.midi))))))));
        }

        /// @brief Comparison operator for use in maps
        inline bool operator==(const modulator_source_t& other) const
        {
            return (type == other.type) && (direction == other.direction) &&
                   (polarity == other.polarity) && (controller_type == other.controller_type) &&
                   (midi == other.midi);
        }
    };


    //------------------------------------------------------------------------------------
    /// @brief  An unique identifier for a modulator
    ///
    /// In SoundFont, a modulator is uniquely identified by the combination of its source,
    /// its destination and its amount source.
    //------------------------------------------------------------------------------------
    struct modulator_id_t
    {
        modulator_source_t src;         ///< Source of the modulator
        generator_type_t dest;          ///< Destination generator
        modulator_source_t amount_src;  ///< The degree to which the source modulates the
                                        ///  destination is to be controlled by this
                                        ///  modulation source

        /// @brief Comparison operator for use in maps
        inline bool operator<(const modulator_id_t& other) const
        {
            return (src < other.src) ||
                   ((src == other.src) &&
                    ((dest < other.dest) ||
                     ((dest == other.dest) && (amount_src < other.amount_src))));
        }
    };


    //------------------------------------------------------------------------------------
    /// @brief  Data attached a modulator (minus its ID)
    //------------------------------------------------------------------------------------
    struct modulator_t
    {
        int16_t amount;                     ///< Indicates the degree to which the source
                                            ///  modulates the destination
        modulator_transform_t transform;    ///< A transform of the specified type will be
                                            ///  applied to the modulation source before
                                            ///  application to the modulator
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a list of modulators
    //------------------------------------------------------------------------------------
    typedef std::map<modulator_id_t, modulator_t> modulator_map_t;


    //------------------------------------------------------------------------------------
    /// @brief  Represents a zone of a preset
    ///
    /// Each zone contains a list of generators and modulators to use (additively to the
    /// ones in the instrument zone) when synthetising notes from that zone.
    ///
    /// The range of keys and velocities the zone is valid for is specified using
    /// attributes (the GEN_TYPE_KEY_RANGE and GEN_TYPE_VELOCITY_RANGE generators are
    /// not present in 'generators').
    ///
    /// Note: there is no 'global zone' in this API, the corresponding values have
    /// already been set in each local zone.
    //------------------------------------------------------------------------------------
    struct preset_zone_t
    {
        range_t keys_range;
        range_t velocities_range;
        generator_map_t generators;
        modulator_map_t modulators;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a zone of an instrument
    ///
    /// Each zone contains a list of generators and modulators to use when synthetising
    /// notes from that zone.
    ///
    /// The range of keys and velocities the zone is valid for is specified using the
    /// GEN_TYPE_KEY_RANGE and GEN_TYPE_VELOCITY_RANGE generators.
    ///
    /// Note: there is no 'global zone' in this API, the corresponding values have
    /// already been set in each local zone.
    //------------------------------------------------------------------------------------
    typedef preset_zone_t instrument_zone_t;


    //------------------------------------------------------------------------------------
    /// @brief  Identifier for a preset (bank:number)
    //------------------------------------------------------------------------------------
    struct preset_id_t
    {
        uint16_t bank;
        uint16_t number;

        /// @brief Comparison operator for use in maps
        inline bool operator<(const preset_id_t& other) const
        {
            return (bank < other.bank) || (number < other.number);
        }
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a preset
    //------------------------------------------------------------------------------------
    struct preset_t
    {
        std::string name;
        std::vector<preset_zone_t> zones;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a list of presets
    //------------------------------------------------------------------------------------
    typedef std::map<preset_id_t, preset_t> preset_map_t;


    //------------------------------------------------------------------------------------
    /// @brief  Represents an instrument
    //------------------------------------------------------------------------------------
    struct instrument_t
    {
        std::string name;
        std::vector<instrument_zone_t> zones;
    };


    //------------------------------------------------------------------------------------
    /// @brief  The different types of samples
    //------------------------------------------------------------------------------------
    enum sample_type_t
    {
        SAMPLE_TYPE_MONO = 0x0001,
        SAMPLE_TYPE_RIGHT = 0x0002,
        SAMPLE_TYPE_LEFT = 0x0004,
        SAMPLE_TYPE_LINKED = 0x0008,
        SAMPLE_TYPE_ROM_MONO = 0x8001,
        SAMPLE_TYPE_ROM_RIGHT = 0x8002,
        SAMPLE_TYPE_ROM_LEFT = 0x8004,
        SAMPLE_TYPE_ROM_LINKED = 0x8008,
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents an audio sample
    ///
    /// Contains the instructions needed to retrieve the actual audio data from the
    /// buffer (and loop over it).
    //------------------------------------------------------------------------------------
    struct sample_t
    {
        std::string name;           ///< Name of the sample
        uint32_t start;             ///< Start index of the sample in the buffer
        uint32_t end;               ///< End index of the sample in the buffer
        uint32_t loop_start;        ///< Starting point of the loop
        uint32_t loop_end;          ///< Ending point of the loop
        uint32_t sample_rate;       ///< Sample rate, in hertz, at which this sample was acquired
        uint8_t original_pitch;     ///< The MIDI key number of the recorded pitch of the sample
        int8_t pitch_correction;    ///< Pitch correction in cents that should be applied to the
                                    ///  sample on playback
        sample_type_t sample_type;  ///< Type of the sample
        uint16_t sample_link;       ///< Index of the other channel sample (if non-mono)
    };


    //------------------------------------------------------------------------------------
    /// @brief  Contains all the information about a sample to synthetise a key
    //------------------------------------------------------------------------------------
    struct sample_info_t
    {
        generator_map_t generators;         ///< The list of generators to use
        modulator_map_t modulators;         ///< The list of modulators to use
        const sample_t* sample = nullptr;   ///< The audio sample to use

        //--------------------------------------------------------------------------------
        /// @brief  Returns the value of a generator or a default value if not present
        ///
        /// @param type             The type of the generator
        /// @param default_value    The default value to use
        /// @return The value
        //--------------------------------------------------------------------------------
        generator_amount_t generator(generator_type_t type, generator_amount_t default_value) const
        {
            auto iter = generators.find(type);
            return (iter != generators.end() ? iter->second : default_value);
        }
    };


    //------------------------------------------------------------------------------------
    /// @brief  Contains all the information needed to synthetise a key
    ///
    /// Either the 'left' attribute is valid, or both the 'left' and 'right' ones,
    /// according to the type of those audio samples.
    //------------------------------------------------------------------------------------
    struct key_info_t
    {
        bool stereo = false;    ///< Indicates if there are two samples to use
        sample_info_t left;     ///< The LEFT or MONO audio sample to use
        sample_info_t right;    ///< The RIGHT audio sample to use
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a SoundFont file
    //------------------------------------------------------------------------------------
    class SoundFont
    {
    public:
    /// @name Construction / Destruction
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Constructor
        //--------------------------------------------------------------------------------
        SoundFont();

        //--------------------------------------------------------------------------------
        /// @brief  Destructor
        //--------------------------------------------------------------------------------
        ~SoundFont();
    /// @}

    /// @name SoundFont file loading methods
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Load a SoundFont file
        ///
        /// @param  path    Path to the SoundFont file
        /// @return True if the file was loaded successfully, false otherwise
        //--------------------------------------------------------------------------------
        bool load(const std::filesystem::path& path);

        //--------------------------------------------------------------------------------
        /// @brief  Load a SoundFont file from a buffer
        ///
        /// @param  buffer  The buffer
        /// @param  size    Size of the buffer
        /// @return True if the file was loaded successfully, false otherwise
        //--------------------------------------------------------------------------------
        bool load(const char* buffer, size_t size);
    /// @}

    /// @name Principal methods for synthesis
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Returns all the information needed to synthetise a key using a given
        ///         preset
        ///
        /// All the relevant generators and modulators from the preset and the instrument
        /// are combined according to the specification and returned along with the
        /// relevant sample.
        ///
        /// @param      bank        The preset bank
        /// @param      preset      The preset number
        /// @param      key         The MIDI key
        /// @param      velocity    The velocity
        /// @param[out] result      The information about the key
        /// @return True if the information was found
        //--------------------------------------------------------------------------------
        bool getKeyInfo(
            uint16_t bank, uint16_t preset, uint8_t key, uint8_t velocity,
            key_info_t&result
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the buffer of audio data, from which samples must be extracted
        ///
        /// @return The buffer of audio data
        //--------------------------------------------------------------------------------
        inline const float* getBuffer() const
        {
            return buffer;
        }
    /// @}

    /// @name SoundFont file content retrieval
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Returns the information about the SoundFont file
        ///
        /// @return The information
        //--------------------------------------------------------------------------------
        inline const information_t& getInformation() const
        {
            return information;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of presets
        ///
        /// @return The number of presets
        //--------------------------------------------------------------------------------
        inline size_t nbPresets() const
        {
            return presets.size();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the presets
        ///
        /// @return The list of presets
        //--------------------------------------------------------------------------------
        inline const preset_map_t& getPresets() const
        {
            return presets;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the specified preset, identified by a 'bank:number' pair
        ///
        /// @param  bank    The preset bank
        /// @param  number  The preset number
        /// @return The preset, nullptr if not found
        //--------------------------------------------------------------------------------
        const preset_t* getPreset(uint16_t bank, uint16_t number) const;

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of instruments
        ///
        /// @return The number of instruments
        //--------------------------------------------------------------------------------
        inline size_t nbInstruments() const
        {
            return instruments.size();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the instruments
        ///
        /// @return The list of instruments
        //--------------------------------------------------------------------------------
        inline const std::vector<instrument_t>& getInstruments() const
        {
            return instruments;
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the number of samples
        ///
        /// @return The number of samples
        //--------------------------------------------------------------------------------
        inline size_t nbSamples() const
        {
            return samples.size();
        }

        //--------------------------------------------------------------------------------
        /// @brief  Returns the samples
        ///
        /// @return The list of samples
        //--------------------------------------------------------------------------------
        inline const std::vector<sample_t>& getSamples() const
        {
            return samples;
        }
    /// @}

        friend std::ostream& operator<<(std::ostream& stream, const SoundFont& soundfont);


    private:
    /// @name Internal methods
    /// @{
        //--------------------------------------------------------------------------------
        /// @brief  Load a SoundFont file from a stream
        ///
        /// @param  stream  The stream to use
        /// @return True if the file was loaded successfully, false otherwise
        //--------------------------------------------------------------------------------
        bool load(std::istream& stream);

        //--------------------------------------------------------------------------------
        /// @brief  Release all the memory used by this object (to restart fresh)
        //--------------------------------------------------------------------------------
        void cleanup();

        //--------------------------------------------------------------------------------
        /// @brief  Load the information chunk into our internal representation
        //--------------------------------------------------------------------------------
        bool loadInformation(std::istream& stream, off_t end_of_chunk);

        //--------------------------------------------------------------------------------
        /// @brief  Load the Hydra chunk into our internal representation
        //--------------------------------------------------------------------------------
        bool loadHydra(std::istream& stream);

        //--------------------------------------------------------------------------------
        /// @brief  Return the preset zone corresponding to the key/velocity pair
        //--------------------------------------------------------------------------------
        const preset_zone_t* findPresetZone(
            const preset_t* preset, uint8_t key, uint8_t velocity
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Return the instrument zone corresponding to the key/velocity pair
        //--------------------------------------------------------------------------------
        const instrument_zone_t* findInstrumentZone(
            const instrument_t* instrument, uint8_t key, uint8_t velocity,
            int exclude_sample_id=-1
        ) const;

        //--------------------------------------------------------------------------------
        /// @brief  Fill a sample info structure with its generators and modulators 
        //--------------------------------------------------------------------------------
        void fillSampleInfo(
            const instrument_zone_t* instrument_zone, const preset_zone_t* preset_zone,
            sample_info_t* result
        ) const;
    /// @}

        //_____ Attributes __________
    protected:
        information_t information;
        float* buffer = nullptr;
        uint32_t buffer_size = 0;
        preset_map_t presets;
        std::vector<instrument_t> instruments;
        std::vector<sample_t> samples;
    };


#ifdef KNM_SOUNDFONT_IMPLEMENTATION

    /********************************* INTERNAL TYPES ***********************************/

    //------------------------------------------------------------------------------------
    /// @brief  Represents a chunk header in the SoundFont file
    //------------------------------------------------------------------------------------
    struct chunk_header_t
    {
        char type[4];   // FOURCC chunk type (ie. 'RIFF', 'LIST')
        uint32_t size;  // Size of the chunk
        char id[4];     // FOURCC id (ie. 'sfbk')
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a field header in the SoundFont file
    //------------------------------------------------------------------------------------
    struct field_info_t
    {
        char id[4];
        uint32_t size;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a preset in the SoundFont file
    //------------------------------------------------------------------------------------
    struct sf_preset_t
    {
        char name[20];
        uint16_t preset;
        uint16_t bank;
        uint16_t preset_bag_index;
        uint32_t library;
        uint32_t genre;
        uint32_t morphology;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a (preset or instrument) bag in the SoundFont file
    //------------------------------------------------------------------------------------
    struct sf_bag_t
    {
        uint16_t generators_index;
        uint16_t modulators_index;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a modulator in the SoundFont file
    //------------------------------------------------------------------------------------
    struct sf_modulator_t
    {
        uint16_t src_operation;
        uint16_t dest_operation;
        int16_t amount;
        uint16_t amount_src_operation;
        uint16_t transform_operation;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a generator in the SoundFont file
    //------------------------------------------------------------------------------------
    struct sf_generator_t
    {
        uint16_t type;
        generator_amount_t amount;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents an instrument in the SoundFont file
    //------------------------------------------------------------------------------------
    struct sf_instrument_t
    {
        char name[20];
        uint16_t bag_index;
    };


    //------------------------------------------------------------------------------------
    /// @brief  Represents a sample in the SoundFont file
    //------------------------------------------------------------------------------------
    struct sf_sample_t
    {
        char name[20];
        uint32_t start;
        uint32_t end;
        uint32_t loop_start;
        uint32_t loop_end;
        uint32_t sample_rate;
        uint8_t original_pitch;
        int8_t pitch_correction;
        uint16_t sample_link;
        uint16_t sample_type;
    };


    /************************************ CONSTANTS *************************************/

    //------------------------------------------------------------------------------------
    /// @brief  List of the default instrument generators always present in a SoundFont
    ///         file
    //------------------------------------------------------------------------------------
    static const generator_map_t DEFAULT_GENERATORS = {
        { GEN_TYPE_INITIAL_FILTER_CUTOFF_FREQUENCY, { .uvalue=13500} },
        { GEN_TYPE_DELAY_MODULATION_LFO, { .ivalue=-12000 } },
        { GEN_TYPE_DELAY_VIBRATO_LFO, { .ivalue= -12000 } },
        { GEN_TYPE_DELAY_MODULATION_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_ATTACK_MODULATION_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_HOLD_MODULATION_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_DECAY_MODULATION_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_RELEASE_MODULATION_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_DELAY_VOLUME_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_ATTACK_VOLUME_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_HOLD_VOLUME_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_DECAY_VOLUME_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_RELEASE_VOLUME_ENVELOPE, { .ivalue= -12000 } },
        { GEN_TYPE_KEY_RANGE, { { 0, 127 } } },
        { GEN_TYPE_VELOCITY_RANGE, { { 0, 127 } } },
        { GEN_TYPE_KEY_NUMBER, { .ivalue= -1}},
        { GEN_TYPE_VELOCITY, { .ivalue= -1}},
        { GEN_TYPE_SCALE_TUNING, { .uvalue=100}},
        { GEN_TYPE_OVERRIDING_ROOT_KEY, { .ivalue= -1}},
    };


    //------------------------------------------------------------------------------------
    /// @brief  List of the default instrument modulators always present in a SoundFont
    ///         file
    //------------------------------------------------------------------------------------
    static const modulator_map_t DEFAULT_MODULATORS = {
        // MIDI Note-On Velocity to Initial Attenuation
        {
            {
                { MOD_SRC_TYPE_CONCAVE, MOD_SRC_DIR_MAX_TO_MIN, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NOTE_ON_VELOCITY },
                GEN_TYPE_INITIAL_ATTENUATION,
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NONE },
            },
            {
                960, MOD_TRANSFORM_LINEAR
            }
        },

        // MIDI Note-On Velocity to Filter Cutoff
        {
            {
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MAX_TO_MIN, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NOTE_ON_VELOCITY },
                GEN_TYPE_INITIAL_FILTER_CUTOFF_FREQUENCY,
                { MOD_SRC_TYPE_SWITCH, MOD_SRC_DIR_MAX_TO_MIN, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NOTE_ON_VELOCITY },
            },
            {
                -2400, MOD_TRANSFORM_LINEAR
            }
        },

        // MIDI Channel Pressure to Vibrato LFO Pitch Depth
        {
            {
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_CHANNEL_PRESSURE },
                GEN_TYPE_VIBRATO_LFO_TO_PITCH,
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NONE },
            },
            {
                50, MOD_TRANSFORM_LINEAR
            }
        },

        // MIDI Continuous Controller 1 to Vibrato LFO Pitch Depth
        {
            {
                {
                    .type=MOD_SRC_TYPE_LINEAR, .direction=MOD_SRC_DIR_MIN_TO_MAX,
                    .polarity=MOD_SRC_POL_UNIPOLAR, .controller_type=MOD_CTRL_TYPE_MIDI, .midi=1
                },
                GEN_TYPE_VIBRATO_LFO_TO_PITCH,
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NONE },
            },
            {
                50, MOD_TRANSFORM_LINEAR
            }
        },

        // MIDI Continuous Controller 7 to Initial Attenuation
        {
            {
                {
                    .type=MOD_SRC_TYPE_CONCAVE, .direction=MOD_SRC_DIR_MAX_TO_MIN,
                    .polarity=MOD_SRC_POL_UNIPOLAR, .controller_type=MOD_CTRL_TYPE_MIDI, .midi=7
                },
                GEN_TYPE_INITIAL_ATTENUATION,
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NONE },
            },
            {
                960, MOD_TRANSFORM_LINEAR
            }
        },

        //  MIDI Continuous Controller 10 to Pan Position
        {
            {
                {
                    .type=MOD_SRC_TYPE_LINEAR, .direction=MOD_SRC_DIR_MIN_TO_MAX,
                    .polarity=MOD_SRC_POL_BIPOLAR, .controller_type=MOD_CTRL_TYPE_MIDI, .midi=10
                },
                GEN_TYPE_PAN,
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NONE },
            },
            {
                1000, MOD_TRANSFORM_LINEAR
            }
        },

        //  MIDI Continuous Controller 11 to Initial Attenuation
        {
            {
                {
                    .type=MOD_SRC_TYPE_CONCAVE, .direction=MOD_SRC_DIR_MAX_TO_MIN,
                    .polarity=MOD_SRC_POL_UNIPOLAR, .controller_type=MOD_CTRL_TYPE_MIDI, .midi=11
                },
                GEN_TYPE_INITIAL_ATTENUATION,
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NONE },
            },
            {
                960, MOD_TRANSFORM_LINEAR
            }
        },

        //  MIDI Continuous Controller 91 to Reverb Effects Send
        {
            {
                {
                    .type=MOD_SRC_TYPE_LINEAR, .direction=MOD_SRC_DIR_MIN_TO_MAX,
                    .polarity=MOD_SRC_POL_UNIPOLAR, .controller_type=MOD_CTRL_TYPE_MIDI, .midi=91
                },
                GEN_TYPE_REVERB_EFFECTS_SEND,
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NONE },
            },
            {
                200, MOD_TRANSFORM_LINEAR
            }
        },

        //  MIDI Continuous Controller 93 to Chorus Effects Send
        {
            {
                {
                    .type=MOD_SRC_TYPE_LINEAR, .direction=MOD_SRC_DIR_MIN_TO_MAX,
                    .polarity=MOD_SRC_POL_UNIPOLAR, .controller_type=MOD_CTRL_TYPE_MIDI, .midi=93
                },
                GEN_TYPE_CHORUS_EFFECTS_SEND,
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_NONE },
            },
            {
                200, MOD_TRANSFORM_LINEAR
            }
        },

        //  MIDI Pitch Wheel to Initial Pitch Controlled by MIDI Pitch Wheel Sensitivity
        {
            {
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_BIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_PITCH_WHEEL },
                GEN_TYPE_FINE_TUNE,
                { MOD_SRC_TYPE_LINEAR, MOD_SRC_DIR_MIN_TO_MAX, MOD_SRC_POL_UNIPOLAR, MOD_CTRL_TYPE_SRC, MOD_CTRL_SRC_PITCH_WHEEL_SENSITIVITY },
            },
            {
                12700, MOD_TRANSFORM_LINEAR
            }
        },
    };


    /******************************** HELPER FUNCTIONS **********************************/

    chunk_header_t readChunkHeader(std::istream& stream)
    {
        chunk_header_t header;
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        return header;
    }

    field_info_t readFieldInfo(std::istream& stream)
    {
        field_info_t info;
        stream.read(reinterpret_cast<char*>(&info), sizeof(info));
        return info;
    }

    void readVersion(std::istream& stream, uint16_t* major, uint16_t* minor)
    {
        stream.read(reinterpret_cast<char*>(major), sizeof(uint16_t));
        stream.read(reinterpret_cast<char*>(minor), sizeof(uint16_t));
    }

    void readString(std::istream& stream, uint32_t size, std::string& result)
    {
        if (size == 0)
            return;

        char* buffer = new char[size+1];
        buffer[size] = 0;

        stream.read(buffer, size);

        result = buffer;

        delete[] buffer;
    }

    std::string toString(modulator_controller_source_t src)
    {   
        switch (src)
        {
            case MOD_CTRL_SRC_NONE: return "None";
            case MOD_CTRL_SRC_NOTE_ON_VELOCITY: return "Note-on velocity";
            case MOD_CTRL_SRC_NOTE_ON_KEY_NUMBER: return "Note-on key number";
            case MOD_CTRL_SRC_POLY_PRESSURE: return "Poly pressure";
            case MOD_CTRL_SRC_CHANNEL_PRESSURE: return "Channel pressure";
            case MOD_CTRL_SRC_PITCH_WHEEL: return "Pitch wheel";
            case MOD_CTRL_SRC_PITCH_WHEEL_SENSITIVITY: return "Pitch wheel sensitivity";
        }

        return  "Unknown";
    };

    std::string toString(generator_type_t type)
    {   
        switch (type)
        {
            case GEN_TYPE_START_ADDRESS_OFFSET: return "Start address offset";
            case GEN_TYPE_END_ADDRESS_OFFSET: return "End address offset";
            case GEN_TYPE_START_LOOP_ADDRESS_OFFSET: return "Start loop address offset";
            case GEN_TYPE_END_LOOP_ADDRESS_OFFSET: return "End loop address offset";
            case GEN_TYPE_START_ADDRESS_COARSE_OFFSET: return "Start address coarse offset";
            case GEN_TYPE_MODULATION_LFO_TO_PITCH: return "Modulation LFO to pitch";
            case GEN_TYPE_VIBRATO_LFO_TO_PITCH: return "Vibrato LFO to pitch";
            case GEN_TYPE_MODULATION_ENVELOPE_TO_PITCH: return "Modulation envelope to pitch";
            case GEN_TYPE_INITIAL_FILTER_CUTOFF_FREQUENCY: return "Initial filter cutoff frequency";
            case GEN_TYPE_INITIAL_FILTER_Q: return "Initial filter Q";
            case GEN_TYPE_MODULATION_LFO_TO_FILTER_CUTOFF_FREQUENCY: return "Modulation LFO to filter cutoff frequency";
            case GEN_TYPE_MODULATION_ENVELOPE_TO_FILTER_CUTOFF_FREQUENCY: return "Modulation envelope to filter cutoff frequency";
            case GEN_TYPE_END_ADDRESS_COARSE_OFFSET: return "End address coarse offset";
            case GEN_TYPE_MODULATION_LFO_TO_VOLUME: return "Modulation LFO to volume";
            case GEN_TYPE_UNUSED_1: return "Unused 1";
            case GEN_TYPE_CHORUS_EFFECTS_SEND: return "Chorus effects send";
            case GEN_TYPE_REVERB_EFFECTS_SEND: return "Reverb effects send";
            case GEN_TYPE_PAN: return "Pan";
            case GEN_TYPE_UNUSED_2: return "Unused 2";
            case GEN_TYPE_UNUSED_3: return "Unused 3";
            case GEN_TYPE_UNUSED_4: return "Unused 4";
            case GEN_TYPE_DELAY_MODULATION_LFO: return "Delay modulation LFO";
            case GEN_TYPE_FREQUENCY_MODULATION_LFO: return "Frequency modulation LFO";
            case GEN_TYPE_DELAY_VIBRATO_LFO: return "Delay vibrato LFO";
            case GEN_TYPE_FREQUENCY_VIBRATO_LFO: return "Frequency vibrato LFO";
            case GEN_TYPE_DELAY_MODULATION_ENVELOPE: return "Delay modulation envelope";
            case GEN_TYPE_ATTACK_MODULATION_ENVELOPE: return "Attack modulation envelope";
            case GEN_TYPE_HOLD_MODULATION_ENVELOPE: return "Hold modulation envelope";
            case GEN_TYPE_DECAY_MODULATION_ENVELOPE: return "Decay modulation envelope";
            case GEN_TYPE_SUSTAIN_MODULATION_ENVELOPE: return "Sustain modulation envelope";
            case GEN_TYPE_RELEASE_MODULATION_ENVELOPE: return "Release modulation envelope";
            case GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_HOLD: return "Key number to modulation envelope hold";
            case GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_DECAY: return "Key number to modulation envelope decay";
            case GEN_TYPE_DELAY_VOLUME_ENVELOPE: return "Delay volume envelope";
            case GEN_TYPE_ATTACK_VOLUME_ENVELOPE: return "Attack volume envelope";
            case GEN_TYPE_HOLD_VOLUME_ENVELOPE: return "Hold volume envelope";
            case GEN_TYPE_DECAY_VOLUME_ENVELOPE: return "Decay volume envelope";
            case GEN_TYPE_SUSTAIN_VOLUME_ENVELOPE: return "Sustain volume envelope";
            case GEN_TYPE_RELEASE_VOLUME_ENVELOPE: return "Release volume envelope";
            case GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_HOLD: return "Key number to volume envelope hold";
            case GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_DECAY: return "Key number to volume envelope decay";
            case GEN_TYPE_INSTRUMENT: return "Instrument";
            case GEN_TYPE_RESERVED_1: return "Reserved 1";
            case GEN_TYPE_KEY_RANGE: return "Key range";
            case GEN_TYPE_VELOCITY_RANGE: return "Velocity range";
            case GEN_TYPE_START_LOOP_ADDRESS_COARSE_OFFSET: return "Start loop address coarse offset";
            case GEN_TYPE_KEY_NUMBER: return "Key number";
            case GEN_TYPE_VELOCITY: return "Velocity";
            case GEN_TYPE_INITIAL_ATTENUATION: return "Initial attenuation";
            case GEN_TYPE_RESERVED_2: return "Reserved 2";
            case GEN_TYPE_END_LOOP_ADDRESS_COARSE_OFFSET: return "End loop address coarse offset";
            case GEN_TYPE_COARSE_TUNE: return "Coarse tune";
            case GEN_TYPE_FINE_TUNE: return "Fine tune";
            case GEN_TYPE_SAMPLE_ID: return "Sample ID";
            case GEN_TYPE_SAMPLE_MODES: return "Sample modes";
            case GEN_TYPE_RESERVED_3: return "Reserved 3";
            case GEN_TYPE_SCALE_TUNING: return "Scale tuning";
            case GEN_TYPE_EXCLUSIVE_CLASS: return "Exclusive class";
            case GEN_TYPE_OVERRIDING_ROOT_KEY: return "Overriding root key";
            case GEN_TYPE_UNUSED_5: return "Unused 5";
            case GEN_TYPE_UNUSED_END: return "Unused end";
        }

        return  "Unknown";
    };


    /*************************** CONSTRUCTION / DESTRUCTION *****************************/

    SoundFont::SoundFont()
    {
    }

    SoundFont::~SoundFont()
    {
        delete[] buffer;
    }


    /********************************** LOAD METHODS ************************************/

    bool SoundFont::load(const std::filesystem::path& path)
    {
        // Cleanup (just in case)
        cleanup();

        // Check if the file exists
        if (!std::filesystem::exists(path))
            return false;

        // Open the file
        std::ifstream file(path.string(), std::ios::binary);
        if (!file.is_open())
            return false;
        
        // Parse the file
        bool result = load(file);
        
        file.close();

        return result;
    }

    //-----------------------------------------------------------------------

    struct membuf : std::streambuf
    {
        membuf(char* begin, size_t size)
        {
            this->setg(begin, begin, begin + size);
        }

        pos_type seekoff(
            off_type off, std::ios_base::seekdir dir,
            std::ios_base::openmode which = std::ios_base::in
        ) override
        {
            if (dir == std::ios_base::cur)
                gbump(off);
            else if (dir == std::ios_base::end)
                setg(eback(), egptr() + off, egptr());
            else if (dir == std::ios_base::beg)
                setg(eback(), eback() + off, egptr());
            return gptr() - eback();
        }

        pos_type seekpos(pos_type sp, std::ios_base::openmode which) override
        {
            return seekoff(sp - pos_type(off_type(0)), std::ios_base::beg, which);
        }
    };

    bool SoundFont::load(const char* buffer, size_t size)
    {
        // Cleanup (just in case)
        cleanup();

        // Create a memory buffer around the provided buffer
        membuf memory_buffer(const_cast<char*>(buffer), size);

        // Create an input stream from the string buffer
        std::istream stream(&memory_buffer);

        // Parse the soundfont
        return load(stream);
    }

    //-----------------------------------------------------------------------

    bool SoundFont::getKeyInfo(
        uint16_t bank, uint16_t preset_number, uint8_t key, uint8_t velocity,
        key_info_t&result
    ) const
    {
        auto preset = getPreset(bank, preset_number);
        if (!preset)
            return false;

        const preset_zone_t* preset_zone = findPresetZone(preset, key, velocity);

        auto& instrument = instruments[preset_zone->generators.at(GEN_TYPE_INSTRUMENT).ivalue];

        const instrument_zone_t* instrument_zone = findInstrumentZone(&instrument, key, velocity);

        int sample_id = instrument_zone->generators.at(GEN_TYPE_SAMPLE_ID).ivalue;
        auto& sample = samples[sample_id];

        if ((sample.sample_type == SAMPLE_TYPE_MONO) || (sample.sample_type == SAMPLE_TYPE_ROM_MONO))
        {
            result.stereo = false;
            result.left.sample = &sample;
            result.right.sample = nullptr;

            fillSampleInfo(instrument_zone, preset_zone, &result.left);
        }
        else
        {
            result.stereo = true;

            const instrument_zone_t* instrument_zone2 = findInstrumentZone(&instrument, key, velocity, sample_id);
            auto& sample2 = samples[instrument_zone2->generators.at(GEN_TYPE_SAMPLE_ID).ivalue];

            if ((sample.sample_type == SAMPLE_TYPE_LEFT) || (sample.sample_type == SAMPLE_TYPE_ROM_LEFT))
            {
                result.left.sample = &sample;
                result.right.sample = &sample2;

                fillSampleInfo(instrument_zone, preset_zone, &result.left);
                fillSampleInfo(instrument_zone2, preset_zone, &result.right);
            }
            else
            {
                result.right.sample = &sample;
                result.left.sample = &sample2;

                fillSampleInfo(instrument_zone, preset_zone, &result.right);
                fillSampleInfo(instrument_zone2, preset_zone, &result.left);
            }
        }

        return true;
    }

    //-----------------------------------------------------------------------

    const preset_t* SoundFont::getPreset(uint16_t bank, uint16_t number) const
    {
        auto iter = presets.find({ bank, number });
        return (iter != presets.end() ? &iter->second : nullptr);
    }

    //-----------------------------------------------------------------------

    std::ostream& operator<<(std::ostream& stream, const generator_map_t& generators)
    {
        for (const auto& entry : generators)
        {
            switch (entry.first)
            {
                case GEN_TYPE_START_ADDRESS_OFFSET:
                case GEN_TYPE_END_ADDRESS_OFFSET:
                case GEN_TYPE_START_LOOP_ADDRESS_OFFSET:
                case GEN_TYPE_END_LOOP_ADDRESS_OFFSET:
                case GEN_TYPE_START_ADDRESS_COARSE_OFFSET:
                case GEN_TYPE_INITIAL_FILTER_CUTOFF_FREQUENCY:
                case GEN_TYPE_INITIAL_FILTER_Q:
                case GEN_TYPE_CHORUS_EFFECTS_SEND:
                case GEN_TYPE_REVERB_EFFECTS_SEND:
                case GEN_TYPE_SUSTAIN_MODULATION_ENVELOPE:
                case GEN_TYPE_SUSTAIN_VOLUME_ENVELOPE:
                case GEN_TYPE_INITIAL_ATTENUATION:
                case GEN_TYPE_SAMPLE_MODES:
                case GEN_TYPE_SCALE_TUNING:
                case GEN_TYPE_EXCLUSIVE_CLASS:
                    stream << "            " << toString(entry.first) << ": " << entry.second.uvalue << std::endl;
                    break;

                case GEN_TYPE_MODULATION_LFO_TO_PITCH:
                case GEN_TYPE_VIBRATO_LFO_TO_PITCH:
                case GEN_TYPE_MODULATION_ENVELOPE_TO_PITCH:
                case GEN_TYPE_MODULATION_LFO_TO_FILTER_CUTOFF_FREQUENCY:
                case GEN_TYPE_MODULATION_ENVELOPE_TO_FILTER_CUTOFF_FREQUENCY:
                case GEN_TYPE_END_ADDRESS_COARSE_OFFSET:
                case GEN_TYPE_MODULATION_LFO_TO_VOLUME:
                case GEN_TYPE_PAN:
                case GEN_TYPE_DELAY_MODULATION_LFO:
                case GEN_TYPE_FREQUENCY_MODULATION_LFO:
                case GEN_TYPE_DELAY_VIBRATO_LFO:
                case GEN_TYPE_FREQUENCY_VIBRATO_LFO:
                case GEN_TYPE_DELAY_MODULATION_ENVELOPE:
                case GEN_TYPE_ATTACK_MODULATION_ENVELOPE:
                case GEN_TYPE_HOLD_MODULATION_ENVELOPE:
                case GEN_TYPE_DECAY_MODULATION_ENVELOPE:
                case GEN_TYPE_RELEASE_MODULATION_ENVELOPE:
                case GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_HOLD:
                case GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_DECAY:
                case GEN_TYPE_DELAY_VOLUME_ENVELOPE:
                case GEN_TYPE_ATTACK_VOLUME_ENVELOPE:
                case GEN_TYPE_HOLD_VOLUME_ENVELOPE:
                case GEN_TYPE_DECAY_VOLUME_ENVELOPE:
                case GEN_TYPE_RELEASE_VOLUME_ENVELOPE:
                case GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_HOLD:
                case GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_DECAY:
                case GEN_TYPE_START_LOOP_ADDRESS_COARSE_OFFSET:
                case GEN_TYPE_KEY_NUMBER:
                case GEN_TYPE_VELOCITY:
                case GEN_TYPE_END_LOOP_ADDRESS_COARSE_OFFSET:
                case GEN_TYPE_COARSE_TUNE:
                case GEN_TYPE_FINE_TUNE:
                case GEN_TYPE_OVERRIDING_ROOT_KEY:
                    stream << "            " << toString(entry.first) << ": " << entry.second.ivalue << std::endl;
                    break;

                default:
                    break;
            }
        }

        return stream;
    }

    //-----------------------------------------------------------------------

    std::ostream& operator<<(std::ostream& stream, const modulator_source_t& op)
    {
        if (op.controller_type == MOD_CTRL_TYPE_SRC)
        {
            stream << toString(op.source);

            if (op.source == MOD_CTRL_SRC_NONE)
                return stream;
        }
        else
        {
            stream << "MIDI CC " << uint16_t(op.midi);
        }

        stream << " (";
        
        switch (op.type)
        {
            case MOD_SRC_TYPE_LINEAR: stream << "linear"; break;
            case MOD_SRC_TYPE_CONCAVE: stream << "concave"; break;
            case MOD_SRC_TYPE_CONVEX: stream << "convex"; break;
            case MOD_SRC_TYPE_SWITCH: stream << "switch"; break;
        }

        stream << ", " << (op.direction == MOD_SRC_DIR_MIN_TO_MAX ? "min to max" : "max to min")
               << ", " << (op.polarity == MOD_SRC_POL_UNIPOLAR ? "unipolar" : "bipolar")
               << ")";

        return stream;
    }

    //-----------------------------------------------------------------------

    std::ostream& operator<<(std::ostream& stream, const modulator_map_t& modulators)
    {
        bool first = true;

        for (const auto& entry : modulators)
        {
            if (!first)
                stream << "            --------------------------------" << std::endl;
            else
                first = false;

            stream << "            Source: " << entry.first.src << std::endl;
            stream << "            Amount source: " << entry.first.amount_src << std::endl;
            stream << "            Amount: " << entry.second.amount << std::endl;
        }

        return stream;
    }

    //-----------------------------------------------------------------------

    std::ostream& operator<<(std::ostream& stream, const information_t& information)
    {
        stream << "Information" << std::endl;
        stream << "===========" << std::endl;

        if ((information.major_version != 0) || (information.minor_version != 0))
        {
            stream << "    Version:       " << information.major_version << "."
                   << std::setfill('0') << std::setw(2) << information.minor_version << std::endl;
        }
        else
        {
            stream << "    Version:       UNSPECIFIED" << std::endl;
        }

        stream << "    Name:          "
               << (!information.name.empty() ? information.name : "UNSPECIFIED")
               << std::endl;

        stream << "    Engine:        "
               << (!information.target_engine.empty() ? information.target_engine : "UNSPECIFIED")
               << std::endl;

        if (!information.rom_name.empty())
            stream << "    ROM:           " << information.rom_name << std::endl;

        if ((information.rom_major_version != 0) || (information.rom_minor_version != 0))
        {
            stream << "    ROM version:   " << information.rom_major_version << "."
                   << information.rom_minor_version << std::endl;
        }

        if (!information.creation_date.empty())
            stream << "    Creation date: " << information.creation_date << std::endl;

        if (!information.engineers.empty())
            stream << "    Engineers:     " << information.engineers << std::endl;

        if (!information.product.empty())
            stream << "    Product:       " << information.product << std::endl;

        if (!information.copyright.empty())
            stream << "    Copyright:     " << information.copyright << std::endl;

        if (!information.comments.empty())
            stream << "    Comments:      " << information.comments << std::endl;

        if (!information.creation_tool.empty())
            stream << "    Creation tool: " << information.creation_tool << std::endl;

        return stream;
    }
    
    //-----------------------------------------------------------------------

    std::ostream& operator<<(std::ostream& stream, const std::pair<preset_id_t, preset_t>& preset)
    {
        stream << "Preset " << preset.first.bank << ":" << preset.first.number << std::endl;
        stream << "===========" << std::endl;
        stream << "    Name: " << preset.second.name << std::endl;

        for (const auto& zone : preset.second.zones)
        {
            stream << std::endl;

            stream << "    Keys " << (uint16_t) zone.keys_range.lo << "-"
                   << (uint16_t) zone.keys_range.hi
                   << ", Velocities " << (uint16_t) zone.velocities_range.lo << "-"
                   << (uint16_t) zone.velocities_range.hi
                   << std::endl;

            stream << "        Generators (" << zone.generators.size() << ")"
                   << (!zone.generators.empty() ? ":" : "") << std::endl;

            stream << "            Instrument: "
                   << zone.generators.at(GEN_TYPE_INSTRUMENT).ivalue
                   << std::endl;
   
            stream << zone.generators;

            if (!zone.generators.empty())
                stream << std::endl;

            stream << "        Modulators (" << zone.modulators.size() << ")"
                   << (!zone.modulators.empty() ? ":" : "") << std::endl;

            stream << zone.modulators;
        }

        stream << std::endl;

        return stream;
    }

    //-----------------------------------------------------------------------

    std::ostream& operator<<(std::ostream& stream, const instrument_t& instrument)
    {
        stream << "    Name: " << instrument.name << std::endl;

        for (const auto& zone : instrument.zones)
        {
            stream << std::endl;

            stream << "    Keys " << (uint16_t) zone.keys_range.lo << "-"
                   << (uint16_t) zone.keys_range.hi
                   << ", Velocities " << (uint16_t) zone.velocities_range.lo << "-"
                   << (uint16_t) zone.velocities_range.hi
                   << std::endl;

            stream << "        Generators (" << zone.generators.size() << ")"
                   << (!zone.generators.empty() ? ":" : "") << std::endl;

            stream << "            Sample: "
                    << zone.generators.at(GEN_TYPE_SAMPLE_ID).ivalue
                    << std::endl;

            stream << zone.generators;

            if (!zone.generators.empty())
                stream << std::endl;

            stream << "        Modulators (" << zone.modulators.size() << ")"
                   << (!zone.modulators.empty() ? ":" : "") << std::endl;

            stream << zone.modulators;
        }

        return stream;
    }

    //-----------------------------------------------------------------------

    std::ostream& operator<<(std::ostream& stream, const sample_t& sample)
    {
        stream << "    Name:             " << sample.name << std::endl;
        stream << "    Start:            " << sample.start << std::endl;
        stream << "    End:              " << sample.end << std::endl;
        stream << "    Loop start:       " << sample.loop_start << std::endl;
        stream << "    Loop end:         " << sample.loop_end << std::endl;
        stream << "    Sample rate:      " << sample.sample_rate << std::endl;
        stream << "    Original pitch:   " << uint16_t(sample.original_pitch) << std::endl;
        stream << "    Pitch correction: " << uint16_t(sample.pitch_correction) << std::endl;

        stream << "    Sample type:      ";
        
        switch (sample.sample_type)
        {
            case SAMPLE_TYPE_MONO: stream << "mono"; break;
            case SAMPLE_TYPE_RIGHT: stream << "right"; break;
            case SAMPLE_TYPE_LEFT: stream << "left"; break;
            case SAMPLE_TYPE_LINKED: stream << "linked"; break;
            case SAMPLE_TYPE_ROM_MONO: stream << "ROM mono"; break;
            case SAMPLE_TYPE_ROM_RIGHT: stream << "ROM right"; break;
            case SAMPLE_TYPE_ROM_LEFT: stream << "ROM left"; break;
            case SAMPLE_TYPE_ROM_LINKED: stream << "ROM linked"; break;
        }
        
        stream << std::endl;

        stream << "    Sample link:      " << sample.sample_link << std::endl;

        return stream;
    }

    //-----------------------------------------------------------------------

    std::ostream& operator<<(std::ostream& stream, const SoundFont& soundfont)
    {
        auto information = soundfont.getInformation();
        stream << information;

        stream << std::endl;
        stream << "Buffer: " << soundfont.buffer_size << " samples" << std::endl;
        stream << std::endl;
    
        for (const auto& preset : soundfont.presets)
            stream << preset;

        for (int i = 0; i < soundfont.instruments.size(); ++i)
        {
            const auto& instrument = soundfont.instruments[i];

            stream << "Instrument " << i << std::endl;
            stream << "===========" << std::endl;
            stream << instrument;
            stream << std::endl;
        }

        for (int i = 0; i < soundfont.samples.size(); ++i)
        {
            const auto& sample = soundfont.samples[i];

            stream << "Sample " << i << std::endl;
            stream << "===========" << std::endl;
            stream << sample;
            stream << std::endl;
        }

        return stream;
    }


    /******************************** INTERNAL METHODS **********************************/

    bool SoundFont::load(std::istream& stream)
    {
        // Main chunk
        chunk_header_t sfbk_header = readChunkHeader(stream);
        if ((strncmp(sfbk_header.type, "RIFF", 4) != 0) ||
            (strncmp(sfbk_header.id, "sfbk", 4) != 0))
        {
            return false;
        }

        // Informations chunk
        chunk_header_t info_header = readChunkHeader(stream);
        if ((strncmp(info_header.type, "LIST", 4) != 0) ||
            (strncmp(info_header.id, "INFO", 4) != 0))
        {
            return false;
        }

        off_t end_of_chunk = off_t(stream.tellg()) + info_header.size - 4;

        if (!loadInformation(stream, end_of_chunk))
            return false;

        stream.seekg(end_of_chunk, std::ios::beg);

        // Sample data chunk
        chunk_header_t sdta_header = readChunkHeader(stream);
        if ((strncmp(sdta_header.type, "LIST", 4) != 0) ||
            (strncmp(sdta_header.id, "sdta", 4) != 0))
        {
            return false;
        }

        end_of_chunk = off_t(stream.tellg()) + sdta_header.size - 4;

        if (sdta_header.size != 4)
        {
            field_info_t smpl_field = readFieldInfo(stream);

            if (strncmp(smpl_field.id, "smpl", 4) != 0)
                return false;

            off_t smpl_data_start = stream.tellg();

            // Are the samples 24 bits?
            stream.seekg(smpl_field.size, std::ios_base::cur);
            field_info_t sm24_field = readFieldInfo(stream);

            uint8_t* lsb_buffer = nullptr;
            if (strncmp(sm24_field.id, "sm24", 4) == 0)
            {
                lsb_buffer = new uint8_t[sm24_field.size];
                stream.read(reinterpret_cast<char*>(lsb_buffer), sm24_field.size);
            }

            stream.seekg(smpl_data_start, std::ios::beg);

            buffer_size = smpl_field.size >> 1;
            buffer = new float[buffer_size];

            uint32_t remainder = buffer_size;

            const uint32_t ibuffer_size = 100000;

            int16_t* ibuffer = new int16_t[ibuffer_size];
            float* dest = buffer;
            uint8_t* lsb = lsb_buffer;

            while (remainder > 0)
            {
                uint32_t count = std::min(remainder, ibuffer_size);

                stream.read(reinterpret_cast<char*>(ibuffer), count << 1);

                if (lsb)
                {
                    for (int i = 0; i < count; ++i)
                    {
                        int32_t v = ibuffer[i] << 8 | lsb[i];
                        dest[i] = float(v) / 8388608.0f;
                    }
                }
                else
                {
                    for (int i = 0; i < count; ++i)
                        dest[i] = float(ibuffer[i]) / 32767.0f;
                }

                dest += count;
                remainder -= count;

                if (lsb)
                    lsb += count;
            }

            delete[] ibuffer;
            delete[] lsb_buffer;
        }

        stream.seekg(end_of_chunk, std::ios::beg);

        // Preset, Instrument, and Sample Header data chunk
        chunk_header_t ptda_header = readChunkHeader(stream);
        if ((strncmp(ptda_header.type, "LIST", 4) != 0) ||
            (strncmp(ptda_header.id, "pdta", 4) != 0))
        {
            return false;
        }
        
        if (!loadHydra(stream))
            return false;

        return true;
    }

    //-----------------------------------------------------------------------

    void SoundFont::cleanup()
    {
        information = information_t();

        delete[] buffer;
        buffer = nullptr;
        buffer_size = 0;

        presets.clear();
        instruments.clear();
    }

    //-----------------------------------------------------------------------

    bool SoundFont::loadInformation(std::istream& stream, off_t end_of_chunk)
    {
        while (stream.tellg() < end_of_chunk)
        {
            field_info_t info = readFieldInfo(stream);

            if (strncmp(info.id, "ifil", 4) == 0)
            {
                if (info.size != 4)
                    return false;

                readVersion(
                    stream, &information.major_version,
                    &information.minor_version
                );
            }
            else if (strncmp(info.id, "INAM", 4) == 0)
            {
                readString(stream, info.size, information.name);
            }
            else if (strncmp(info.id, "isng", 4) == 0)
            {
                readString(stream, info.size, information.target_engine);
            }
            else if (strncmp(info.id, "irom", 4) == 0)
            {
                readString(stream, info.size, information.rom_name);
            }
            else if (strncmp(info.id, "iver", 4) == 0)
            {
                if (info.size != 4)
                    return false;

                readVersion(
                    stream, &information.rom_major_version,
                    &information.rom_minor_version
                );
            }
            else if (strncmp(info.id, "ICRD", 4) == 0)
            {
                readString(stream, info.size, information.creation_date);
            }
            else if (strncmp(info.id, "IENG", 4) == 0)
            {
                readString(stream, info.size, information.engineers);
            }
            else if (strncmp(info.id, "IPRD", 4) == 0)
            {
                readString(stream, info.size, information.product);
            }
            else if (strncmp(info.id, "ICOP", 4) == 0)
            {
                readString(stream, info.size, information.copyright);
            }
            else if (strncmp(info.id, "ICMT", 4) == 0)
            {
                readString(stream, info.size, information.comments);
            }
            else if (strncmp(info.id, "ISFT", 4) == 0)
            {
                readString(stream, info.size, information.creation_tool);
            }
        }

        return true;
    }

    //-----------------------------------------------------------------------

    bool SoundFont::loadHydra(std::istream& stream)
    {
        // Presets
        field_info_t phdr_field = readFieldInfo(stream);

        if (strncmp(phdr_field.id, "phdr", 4) != 0)
            return false;

        uint32_t nb_presets = phdr_field.size / 38;

        std::vector<sf_preset_t> presets(nb_presets);

        for (int i = 0; i < nb_presets; ++i)
            stream.read(reinterpret_cast<char*>(&presets[i]), 38);

        // Preset bags
        field_info_t pbag_field = readFieldInfo(stream);

        if (strncmp(pbag_field.id, "pbag", 4) != 0)
            return false;

        uint32_t nb_preset_bags = pbag_field.size / 4;

        std::vector<sf_bag_t> preset_bags(nb_preset_bags);

        for (int i = 0; i < nb_preset_bags; ++i)
            stream.read(reinterpret_cast<char*>(&preset_bags[i]), 4);

        // Preset modulators
        field_info_t pmod_field = readFieldInfo(stream);

        if (strncmp(pmod_field.id, "pmod", 4) != 0)
            return false;

        uint32_t nb_preset_modulators = pmod_field.size / 10;

        std::vector<sf_modulator_t> preset_modulators(nb_preset_modulators);

        for (int i = 0; i < nb_preset_modulators; ++i)
            stream.read(reinterpret_cast<char*>(&preset_modulators[i]), 10);
            
        // Preset generators
        field_info_t pgen_field = readFieldInfo(stream);

        if (strncmp(pgen_field.id, "pgen", 4) != 0)
            return false;

        uint32_t nb_preset_generators = pgen_field.size / 4;

        std::vector<sf_generator_t> preset_generators(nb_preset_generators);

        for (int i = 0; i < nb_preset_generators; ++i)
            stream.read(reinterpret_cast<char*>(&preset_generators[i]), 4);
            
        // Instruments
        field_info_t inst_field = readFieldInfo(stream);

        if (strncmp(inst_field.id, "inst", 4) != 0)
            return false;

        uint32_t nb_instruments = inst_field.size / 22;

        std::vector<sf_instrument_t> instruments(nb_instruments);

        for (int i = 0; i < nb_instruments; ++i)
            stream.read(reinterpret_cast<char*>(&instruments[i]), 22);

        // Instrument bags
        field_info_t ibag_field = readFieldInfo(stream);

        if (strncmp(ibag_field.id, "ibag", 4) != 0)
            return false;

        uint32_t nb_instrument_bags = ibag_field.size / 4;

        std::vector<sf_bag_t> instrument_bags(nb_instrument_bags);

        for (int i = 0; i < nb_instrument_bags; ++i)
            stream.read(reinterpret_cast<char*>(&instrument_bags[i]), 4);

        // Instrument modulators
        field_info_t imod_field = readFieldInfo(stream);

        if (strncmp(imod_field.id, "imod", 4) != 0)
            return false;

        uint32_t nb_instrument_modulators = imod_field.size / 10;

        std::vector<sf_modulator_t> instrument_modulators(nb_instrument_modulators);

        for (int i = 0; i < nb_instrument_modulators; ++i)
            stream.read(reinterpret_cast<char*>(&instrument_modulators[i]), 10);
            
        // Instrument generators
        field_info_t igen_field = readFieldInfo(stream);

        if (strncmp(igen_field.id, "igen", 4) != 0)
            return false;

        uint32_t nb_instrument_generators = igen_field.size / 4;

        std::vector<sf_generator_t> instrument_generators(nb_instrument_generators);

        for (int i = 0; i < nb_instrument_generators; ++i)
            stream.read(reinterpret_cast<char*>(&instrument_generators[i]), 4);

        // Samples
        field_info_t shdr_field = readFieldInfo(stream);

        if (strncmp(shdr_field.id, "shdr", 4) != 0)
            return false;

        uint32_t nb_samples = shdr_field.size / 46;

        std::vector<sf_sample_t> samples(nb_samples);

        for (int i = 0; i < nb_samples; ++i)
            stream.read(reinterpret_cast<char*>(&samples[i]), 46);

        // Build the internal representation
        for (int i = 0; i < nb_presets - 1; ++i)
        {
            const auto& ref = presets[i];

            preset_id_t preset_id;
            preset_id.bank = ref.bank;
            preset_id.number = ref.preset;

            preset_t preset;
            preset.name = ref.name;

            preset_zone_t globals;
            bool has_globals = false;

            for (int j = ref.preset_bag_index; j < presets[i+1].preset_bag_index; ++j)
            {
                const auto& ref_bag = preset_bags[j];

                preset_zone_t zone;

                // Add the default ranges
                zone.generators[GEN_TYPE_KEY_RANGE] = { 0, 127 };
                zone.generators[GEN_TYPE_VELOCITY_RANGE] = { 0, 127 };

                // Add the global generators (if we have some)
                if (has_globals)
                {
                    for (const auto& entry : globals.generators)
                        zone.generators[entry.first] = entry.second;
                }
                
                // Add the generators of the zone
                for (int k = ref_bag.generators_index; k < preset_bags[j+1].generators_index; ++k)
                {
                    const auto& ref_generator = preset_generators[k];
                    zone.generators[static_cast<generator_type_t>(ref_generator.type)] = ref_generator.amount;
                }

                // Add the global modulators (if we have some)
                if (has_globals)
                {
                    for (const auto& entry : globals.modulators)
                        zone.modulators[entry.first] = entry.second;
                }
                
                // Add the modulators of the zone
                for (int k = ref_bag.modulators_index; k < preset_bags[j+1].modulators_index; ++k)
                {
                    const auto& ref_modulator = preset_modulators[k];

                    modulator_id_t modulator_id;

                    modulator_id.src.type = static_cast<modulator_source_type_t>((ref_modulator.src_operation & 0xFC00) >> 10);
                    modulator_id.src.polarity = (ref_modulator.src_operation & 0x0200) != 0 ? MOD_SRC_POL_BIPOLAR : MOD_SRC_POL_UNIPOLAR;
                    modulator_id.src.direction = (ref_modulator.src_operation & 0x0100) != 0 ? MOD_SRC_DIR_MAX_TO_MIN : MOD_SRC_DIR_MIN_TO_MAX;
                    modulator_id.src.controller_type = (ref_modulator.src_operation & 0x0080) != 0 ? MOD_CTRL_TYPE_MIDI : MOD_CTRL_TYPE_SRC;

                    if (modulator_id.src.controller_type == MOD_CTRL_TYPE_MIDI)
                        modulator_id.src.midi = ref_modulator.src_operation & 0x003F;
                    else
                        modulator_id.src.source = static_cast<modulator_controller_source_t>(ref_modulator.src_operation & 0x003F);

                    modulator_id.dest = static_cast<generator_type_t>(ref_modulator.dest_operation);

                    modulator_id.amount_src.type = static_cast<modulator_source_type_t>((ref_modulator.amount_src_operation & 0xFC00) >> 10);
                    modulator_id.amount_src.polarity = (ref_modulator.amount_src_operation & 0x0200) != 0 ? MOD_SRC_POL_BIPOLAR : MOD_SRC_POL_UNIPOLAR;
                    modulator_id.amount_src.direction = (ref_modulator.amount_src_operation & 0x0080) != 0 ? MOD_SRC_DIR_MAX_TO_MIN : MOD_SRC_DIR_MIN_TO_MAX;
                    modulator_id.amount_src.controller_type = (ref_modulator.amount_src_operation & 0x0080) != 0 ? MOD_CTRL_TYPE_MIDI : MOD_CTRL_TYPE_SRC;

                    if (modulator_id.amount_src.controller_type == MOD_CTRL_TYPE_MIDI)
                        modulator_id.amount_src.midi = ref_modulator.amount_src_operation & 0x003F;
                    else
                        modulator_id.amount_src.source = static_cast<modulator_controller_source_t>(ref_modulator.amount_src_operation & 0x003F);

                    modulator_t modulator;
                    modulator.amount = ref_modulator.amount;
                    modulator.transform = static_cast<modulator_transform_t>(ref_modulator.transform_operation);

                    zone.modulators[modulator_id] = modulator;
                }

                // Determine if this is a global zone or a local one
                if (zone.generators.empty() ||
                    (zone.generators.find(GEN_TYPE_INSTRUMENT) == zone.generators.end()))
                {
                    has_globals = true;
                    globals = zone;
                }
                else
                {
                    zone.keys_range = zone.generators[GEN_TYPE_KEY_RANGE].range;
                    zone.velocities_range = zone.generators[GEN_TYPE_VELOCITY_RANGE].range;

                    zone.generators.erase(GEN_TYPE_KEY_RANGE);
                    zone.generators.erase(GEN_TYPE_VELOCITY_RANGE);

                    preset.zones.push_back(zone);
                }
            }
    
            this->presets[preset_id] = preset;
        }

        for (int i = 0; i < nb_instruments - 1; ++i)
        {
            const auto& ref = instruments[i];

            instrument_t instrument;
            instrument.name = ref.name;

            instrument_zone_t globals;
            bool has_globals = false;

            for (int j = ref.bag_index; j < instruments[i+1].bag_index; ++j)
            {
                const auto& ref_bag = instrument_bags[j];

                instrument_zone_t zone;

                // Add either default generators of global ones (if we have some)
                if (has_globals)
                {
                    for (const auto& entry : globals.generators)
                        zone.generators[entry.first] = entry.second;
                }
                else
                {
                    for (const auto& entry : DEFAULT_GENERATORS)
                        zone.generators[entry.first] = entry.second;
                }

                // Add generators of the zone
                for (int k = ref_bag.generators_index; k < instrument_bags[j+1].generators_index; ++k)
                {
                    const auto& ref_generator = instrument_generators[k];
                    zone.generators[static_cast<generator_type_t>(ref_generator.type)] = ref_generator.amount;
                }

                // Add either default modulators of global ones (if we have some)
                if (has_globals)
                {
                    for (const auto& entry : globals.modulators)
                        zone.modulators[entry.first] = entry.second;
                }
                else
                {
                    for (const auto& entry : DEFAULT_MODULATORS)
                        zone.modulators[entry.first] = entry.second;
                }
                
                // Add the modulators of the zone
                for (int k = ref_bag.modulators_index; k < instrument_bags[j+1].modulators_index; ++k)
                {
                    const auto& ref_modulator = instrument_modulators[k];

                    modulator_id_t modulator_id;

                    modulator_id.src.type = static_cast<modulator_source_type_t>((ref_modulator.src_operation & 0xFC00) >> 10);
                    modulator_id.src.polarity = (ref_modulator.src_operation & 0x0200) != 0 ? MOD_SRC_POL_BIPOLAR : MOD_SRC_POL_UNIPOLAR;
                    modulator_id.src.direction = (ref_modulator.src_operation & 0x0080) != 0 ? MOD_SRC_DIR_MAX_TO_MIN : MOD_SRC_DIR_MIN_TO_MAX;
                    modulator_id.src.midi = ref_modulator.src_operation & 0x003F;

                    modulator_id.dest = static_cast<generator_type_t>(ref_modulator.dest_operation);

                    modulator_id.amount_src.type = static_cast<modulator_source_type_t>((ref_modulator.amount_src_operation & 0xFC00) >> 10);
                    modulator_id.amount_src.polarity = (ref_modulator.amount_src_operation & 0x0200) != 0 ? MOD_SRC_POL_BIPOLAR : MOD_SRC_POL_UNIPOLAR;
                    modulator_id.amount_src.direction = (ref_modulator.amount_src_operation & 0x0080) != 0 ? MOD_SRC_DIR_MAX_TO_MIN : MOD_SRC_DIR_MIN_TO_MAX;
                    modulator_id.amount_src.midi = ref_modulator.amount_src_operation & 0x003F;

                    modulator_t modulator;
                    modulator.amount = ref_modulator.amount;
                    modulator.transform = static_cast<modulator_transform_t>(ref_modulator.transform_operation);

                    zone.modulators[modulator_id] = modulator;
                }

                // Determine if this is a global zone or a local one
                if (zone.generators.empty() ||
                    (zone.generators.find(GEN_TYPE_SAMPLE_ID) == zone.generators.end()))
                {
                    has_globals = true;
                    globals = zone;
                }
                else
                {
                    zone.keys_range = zone.generators[GEN_TYPE_KEY_RANGE].range;
                    zone.velocities_range = zone.generators[GEN_TYPE_VELOCITY_RANGE].range;

                    zone.generators.erase(GEN_TYPE_KEY_RANGE);
                    zone.generators.erase(GEN_TYPE_VELOCITY_RANGE);

                    instrument.zones.push_back(zone);
                }
            }
    
            this->instruments.push_back(instrument);
        }

        for (int i = 0; i < nb_samples - 1; ++i)
        {
            const auto& ref = samples[i];

            sample_t sample;
            sample.name = ref.name;
            sample.start = ref.start;
            sample.end = ref.end;
            sample.loop_start = ref.loop_start;
            sample.loop_end = ref.loop_end;
            sample.original_pitch = ref.original_pitch;
            sample.pitch_correction = ref.pitch_correction;
            sample.sample_rate = ref.sample_rate;
            sample.sample_type = static_cast<sample_type_t>(ref.sample_type);
            sample.sample_link = ref.sample_link;

            this->samples.push_back(sample);
        }

        return true;
    }

    //-----------------------------------------------------------------------

    const preset_zone_t* SoundFont::findPresetZone(
        const preset_t* preset, uint8_t key, uint8_t velocity
    ) const
    {
        for (auto& preset_zone : preset->zones)
        {
            if ((key >= preset_zone.keys_range.lo) && (key <= preset_zone.keys_range.hi) &&
                (velocity >= preset_zone.velocities_range.lo) &&
                (velocity <= preset_zone.velocities_range.hi))
            {
                return &preset_zone;
            }
        }

        return nullptr;
    }

    //-----------------------------------------------------------------------

    const instrument_zone_t* SoundFont::findInstrumentZone(
        const instrument_t* instrument, uint8_t key, uint8_t velocity, int exclude_sample_id
    ) const
    {
        for (auto& instrument_zone : instrument->zones)
        {
            if ((key >= instrument_zone.keys_range.lo) && (key <= instrument_zone.keys_range.hi) &&
                (velocity >= instrument_zone.velocities_range.lo) &&
                (velocity <= instrument_zone.velocities_range.hi) &&
                (instrument_zone.generators.at(GEN_TYPE_SAMPLE_ID).ivalue != exclude_sample_id))
            {
                return &instrument_zone;
            }
        }

        return nullptr;
    }

    //-----------------------------------------------------------------------

    void SoundFont::fillSampleInfo(
        const instrument_zone_t* instrument_zone, const preset_zone_t* preset_zone,
        sample_info_t* result
    ) const
    {
        result->generators = instrument_zone->generators;

        for (auto& entry : preset_zone->generators)
        {
            switch (entry.first)
            {
                case GEN_TYPE_INITIAL_FILTER_CUTOFF_FREQUENCY:
                case GEN_TYPE_INITIAL_FILTER_Q:
                case GEN_TYPE_CHORUS_EFFECTS_SEND:
                case GEN_TYPE_REVERB_EFFECTS_SEND:
                case GEN_TYPE_SUSTAIN_MODULATION_ENVELOPE:
                case GEN_TYPE_SUSTAIN_VOLUME_ENVELOPE:
                case GEN_TYPE_INITIAL_ATTENUATION:
                case GEN_TYPE_SCALE_TUNING:
                {
                    auto iter = result->generators.find(entry.first);
                    if (iter != result->generators.end())
                        iter->second.uvalue += entry.second.uvalue;
                    else
                        result->generators[entry.first].uvalue = entry.second.uvalue;
                    break;
                }

                case GEN_TYPE_MODULATION_LFO_TO_PITCH:
                case GEN_TYPE_VIBRATO_LFO_TO_PITCH:
                case GEN_TYPE_MODULATION_ENVELOPE_TO_PITCH:
                case GEN_TYPE_MODULATION_LFO_TO_FILTER_CUTOFF_FREQUENCY:
                case GEN_TYPE_MODULATION_ENVELOPE_TO_FILTER_CUTOFF_FREQUENCY:
                case GEN_TYPE_MODULATION_LFO_TO_VOLUME:
                case GEN_TYPE_PAN:
                case GEN_TYPE_DELAY_MODULATION_LFO:
                case GEN_TYPE_FREQUENCY_MODULATION_LFO:
                case GEN_TYPE_DELAY_VIBRATO_LFO:
                case GEN_TYPE_FREQUENCY_VIBRATO_LFO:
                case GEN_TYPE_DELAY_MODULATION_ENVELOPE:
                case GEN_TYPE_ATTACK_MODULATION_ENVELOPE:
                case GEN_TYPE_HOLD_MODULATION_ENVELOPE:
                case GEN_TYPE_DECAY_MODULATION_ENVELOPE:
                case GEN_TYPE_RELEASE_MODULATION_ENVELOPE:
                case GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_HOLD:
                case GEN_TYPE_KEY_NUMBER_TO_MODULATION_ENVELOPE_DECAY:
                case GEN_TYPE_DELAY_VOLUME_ENVELOPE:
                case GEN_TYPE_ATTACK_VOLUME_ENVELOPE:
                case GEN_TYPE_HOLD_VOLUME_ENVELOPE:
                case GEN_TYPE_DECAY_VOLUME_ENVELOPE:
                case GEN_TYPE_RELEASE_VOLUME_ENVELOPE:
                case GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_HOLD:
                case GEN_TYPE_KEY_NUMBER_TO_VOLUME_ENVELOPE_DECAY:
                case GEN_TYPE_COARSE_TUNE:
                case GEN_TYPE_FINE_TUNE:
                {
                    auto iter = result->generators.find(entry.first);
                    if (iter != result->generators.end())
                        iter->second.ivalue += entry.second.ivalue;
                    else
                        result->generators[entry.first].ivalue = entry.second.ivalue;
                    break;
                }

                default:
                    break;
            }
        }

        result->modulators = instrument_zone->modulators;

        for (auto& entry : preset_zone->modulators)
        {
            auto iter = result->modulators.find(entry.first);
            if (iter != result->modulators.end())
                iter->second.amount += entry.second.amount;
            else
                result->modulators[entry.first] = entry.second;
        }
    }

#endif // KNM_SOUNDFONT_IMPLEMENTATION

}
}
