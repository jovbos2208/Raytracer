#include <gtest/gtest.h>
#include "DragForceCalculator.h"
#include "Ray.h"
#include "Vector3.h"
#include <cmath>

TEST(DragForceCalculatorTest, AccumulatesForceCorrectly) {
    DragForceCalculator calc;

    Ray in, out;
    in.momentum = {1.0, 0.0, 0.0};     // früher: velocity
    out.momentum = {0.0, 0.0, 0.0};
    in.weight = 2.0;
    in.speciesMass = 1.0;
    in.panelId = 3;

    double dummyArea = 0.0;  // wird nicht mehr verwendet in accumulateForce
    calc.accumulateForce(in, out, dummyArea);

    auto total = calc.getTotalDragForce();
    EXPECT_DOUBLE_EQ(total.x, -2.0); // (0 - 1) * 2.0
    EXPECT_DOUBLE_EQ(total.y, 0.0);
    EXPECT_DOUBLE_EQ(total.z, 0.0);

    auto perPanel = calc.getPanelForces();
    ASSERT_TRUE(perPanel.contains(3));

    EXPECT_DOUBLE_EQ(perPanel[3].force.x, -2.0);
    // Flächenwerte wurden zuvor über setMesh() berechnet – wir geben hier keinen neuen Wert vor
}

TEST(DragForceCalculatorTest, MergeCombinesForces) {
    DragForceCalculator a, b;

    Ray in1, out1;
    in1.momentum = {1.0, 0.0, 0.0};
    out1.momentum = {0.0, 0.0, 0.0};
    in1.weight = 1.0;
    in1.speciesMass = 1.0;
    in1.panelId = 2;

    Ray in2 = in1;
    Ray out2 = out1;
    in2.panelId = 2;

    a.accumulateForce(in1, out1, 0.0);  // area wird ignoriert
    b.accumulateForce(in2, out2, 0.0);

    a.merge(b);

    auto total = a.getTotalDragForce();
    EXPECT_DOUBLE_EQ(total.x, -2.0); // 2x -1.0 impulse

    auto perPanel = a.getPanelForces();
    ASSERT_TRUE(perPanel.contains(2));
    EXPECT_DOUBLE_EQ(perPanel[2].force.x, -2.0);
    // Die Fläche bleibt 0, da keine setMesh()-Initialisierung stattfand
}

