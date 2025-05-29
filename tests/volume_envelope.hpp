/*
 * SPDX-FileCopyrightText: 2025 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: MIT
*/

TEST_CASE("VolumeEnvelope")
{
    float ref[] = {
        0.0000f, 0.0000f, 0.0000f, 0.0805f, 0.2256f, 0.3707f, 0.5159f, 0.6610f, 0.8061f, 0.9512f,
        1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 0.9357f, 0.8185f, 0.7159f, 0.6262f, 0.5477f,
        0.5000f, 0.5000f, 0.5000f, 0.5000f, 0.5000f, 0.5000f, 0.5000f, 0.5000f, 0.5000f, 0.5000f,
        0.3825f, 0.2927f, 0.2239f, 0.1713f, 0.1311f, 0.1003f, 0.0767f, 0.0587f, 0.0449f, 0.0344f,
        0.0263f, 0.0201f, 0.0154f, 0.0118f, 0.0090f, 0.0069f, 0.0053f, 0.0040f, 0.0031f, 0.0024f,
        0.0018f, 0.0014f, 0.0011f,
    };

    float priorities[] = {
        3, 3, 3, 2.9195, 2.77438, 2.62925, 2.48413, 2.339, 2.19388, 2.04875, 2, 2, 2, 2, 2, 1.93573,
        1.81847, 1.71591, 1.62619, 1.54772, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5,
        0.3825f, 0.2927f, 0.2239f, 0.1713f, 0.1311f, 0.1003f, 0.0767f, 0.0587f, 0.0449f, 0.0344f,
        0.0263f, 0.0201f, 0.0154f, 0.0118f, 0.0090f, 0.0069f, 0.0053f, 0.0040f, 0.0031f, 0.0024f,
        0.0018f, 0.0014f, 0.0011f,
    };

    VolumeEnvelope envelope(22050);

    envelope.start(0.01f, 0.02f, 0.015f, 0.2f, 0.5f, 0.1f);

    int i;
    for (i = 0; i < 30; ++i)
    {
        envelope.process(64);
        REQUIRE(envelope.getValue() == Approx(ref[i]).margin(0.0001f));
        REQUIRE(envelope.getPriority() == Approx(priorities[i]).margin(0.0001f));
    }

    envelope.release();

    while (envelope.process(64))
    {
        REQUIRE(envelope.getValue() == Approx(ref[i]).margin(0.0001f));
        REQUIRE(envelope.getPriority() == Approx(priorities[i]).margin(0.0001f));
        ++i;
    }
}
