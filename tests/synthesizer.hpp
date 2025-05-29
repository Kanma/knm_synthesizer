/*
 * SPDX-FileCopyrightText: 2025 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: MIT
*/

TEST_CASE("Synthesizer")
{
    SynthesizerSettings settings(22050);
    Synthesizer synthesizer(settings);

    REQUIRE(synthesizer.loadSoundFont(DATA_DIR "440_16bits.sf2"));

    SECTION("Mono, from mono bank, A4")
    {
        // Note: the sample in the file is an A4, so no pitch tuning is performed
        synthesizer.configureChannel(0, 0, 1);
        synthesizer.noteOn(0, 69, 100);

        float buffer[640];
        synthesizer.render(buffer, 640);

        for (int i = 0; i < 640; ++i)
            REQUIRE(buffer[i] == Approx(0.33726f * ref_A4[i]).margin(0.0001f));
    }

    SECTION("Mono, from mono bank, C4")
    {
        // Note: the sample in the file is an A4, so some pitch tuning is performed
        synthesizer.configureChannel(0, 0, 1);
        synthesizer.noteOn(0, 60, 100);

        float buffer[640];
        synthesizer.render(buffer, 640);

        for (int i = 0; i < 640; ++i)
            REQUIRE(buffer[i] == Approx(0.33726f * ref_C4[i]).margin(0.0001f));
    }

    SECTION("Mono, from stereo bank, A4")
    {
        // Note: the sample in the file is an A4, so no pitch tuning is performed
        synthesizer.configureChannel(0, 0, 0);
        synthesizer.noteOn(0, 69, 100);

        float buffer[640];
        synthesizer.render(buffer, 640);

        for (int i = 0; i < 640; ++i)
            REQUIRE(buffer[i] == Approx(0.953853279f * ref_A4[i]).margin(0.0001f));
    }

    SECTION("Mono, from stereo bank, A4")
    {
        // Note: the sample in the file is an A4, so some pitch tuning is performed
        synthesizer.configureChannel(0, 0, 0);
        synthesizer.noteOn(0, 60, 100);

        float buffer[640];
        synthesizer.render(buffer, 640);

        for (int i = 0; i < 640; ++i)
            REQUIRE(buffer[i] == Approx(0.953853279f * ref_C4[i]).margin(0.0001f));
    }

    SECTION("Stereo, from stereo bank, A4")
    {
        // Note: the sample in the file is an A4, so no pitch tuning is performed
        synthesizer.configureChannel(0, 0, 0);
        synthesizer.noteOn(0, 69, 100);

        float left[640];
        float right[640];
        synthesizer.render(left, right, 640);

        for (int i = 0; i < 640; ++i)
        {
            REQUIRE(left[i] == Approx(0.47693f * ref_A4[i]).margin(0.0001f));
            REQUIRE(right[i] == Approx(0.47692f * ref_A4[i]).margin(0.0001f));
        }
    }

    SECTION("Stereo, from stereo bank, C4")
    {
        // Note: the sample in the file is an A4, so some pitch tuning is performed
        synthesizer.configureChannel(0, 0, 0);
        synthesizer.noteOn(0, 60, 100);

        float left[640];
        float right[640];
        synthesizer.render(left, right, 640);

        for (int i = 0; i < 640; ++i)
        {
            REQUIRE(left[i] == Approx(0.47693f * ref_C4[i]).margin(0.0001f));
            REQUIRE(right[i] == Approx(0.47692f * ref_C4[i]).margin(0.0001f));
        }
    }

    SECTION("Stereo, from mono bank, A4")
    {
        // Note: the sample in the file is an A4, so no pitch tuning is performed
        synthesizer.configureChannel(0, 0, 1);
        synthesizer.noteOn(0, 69, 100);

        float left[640];
        float right[640];
        synthesizer.render(left, right, 640);

        for (int i = 0; i < 640; ++i)
        {
            REQUIRE(left[i] == Approx(0.33726f * ref_A4[i]).margin(0.0001f));
            REQUIRE(right[i] == Approx(0.33722f * ref_A4[i]).margin(0.0001f));
        }
    }

    SECTION("Stereo, from mono bank, C4")
    {
        // Note: the sample in the file is an A4, so some pitch tuning is performed
        synthesizer.configureChannel(0, 0, 1);
        synthesizer.noteOn(0, 60, 100);

        float left[640];
        float right[640];
        synthesizer.render(left, right, 640);

        for (int i = 0; i < 640; ++i)
        {
            REQUIRE(left[i] == Approx(0.33726f * ref_C4[i]).margin(0.0001f));
            REQUIRE(right[i] == Approx(0.33722f * ref_C4[i]).margin(0.0001f));
        }
    }
}
