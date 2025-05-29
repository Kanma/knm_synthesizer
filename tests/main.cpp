/*
 * SPDX-FileCopyrightText: 2025 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: MIT
*/

#define KNM_SYNTHESIZER_IMPLEMENTATION
#include <knm_synthesizer.hpp>

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

using namespace knm::synth;


#ifndef DATA_DIR
    #define DATA_DIR ""
#endif


#include "filter.hpp"
#include "lfo.hpp"
#include "modulation_envelope.hpp"
#include "sampler.hpp"
#include "voice.hpp"
#include "synthesizer.hpp"
#include "volume_envelope.hpp"


int main(int argc, char** argv)
{
    // Executes the tests
    int result = Catch::Session().run(argc, argv);

    return result;
}
