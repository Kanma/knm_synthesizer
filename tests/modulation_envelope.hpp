/*
 * SPDX-FileCopyrightText: 2025 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: MIT
*/

TEST_CASE("ModulationEnvelope")
{
    float ref[] = {
        0.0000f, 0.0000f, 0.0000f, 0.0805f, 0.2256f, 0.3707f, 0.5159f, 0.6610f, 0.8061f, 0.9512f,
        1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 0.9928f, 0.9783f, 0.9638f, 0.9493f, 0.9348f,
        0.9202f, 0.9057f, 0.8912f, 0.8767f, 0.8622f, 0.8477f, 0.8332f, 0.8187f, 0.8041f, 0.7896f,
        0.7667f, 0.7438f, 0.7209f, 0.6980f, 0.6750f, 0.6521f, 0.6292f, 0.6063f, 0.5834f, 0.5604f,
        0.5375f, 0.5146f, 0.4917f, 0.4688f, 0.4458f, 0.4229f, 0.4000f, 0.3771f, 0.3542f, 0.3312f,
        0.3083f, 0.2854f, 0.2625f, 0.2396f, 0.2167f, 0.1937f, 0.1708f, 0.1479f, 0.1250f, 0.1021f,
        0.0791f, 0.0562f, 0.0333f, 0.0104f,
    };

    ModulationEnvelope envelope(22050);

    envelope.start(0.01f, 0.02f, 0.015f, 0.2f, 0.5f, 0.1f);

    int i;
    for (i = 0; i < 30; ++i)
    {
        envelope.process(64);
        REQUIRE(envelope.getValue() == Approx(ref[i]).margin(0.0001f));
    }

    envelope.release();

    while (envelope.process(64))
    {
        REQUIRE(envelope.getValue() == Approx(ref[i]).margin(0.0001f));
        ++i;
    }
}
