//@ run("integer-range-optimization-constant-representation", *NO_CJIT_OPTIONS, "--useConcurrentJIT=false")
//@ run("integer-range-optimization-constant-representation", *FTL_OPTIONS, "--useConcurrentJIT=false")

function opaque()
{
    // This exists to hide side effects to the optimizer.
}
noInline(opaque);

function test(i, opaqueCondition) {
    do {
        if (opaqueCondition == 1) {
            if (i < 42) {
                opaque(i);
                if (i != 41) {
                    break;
                }
            }
        } else if (opaqueCondition == 2) {
            if (i < 42) {
                opaque(i);
                if (i < 41) {
                    opaque(i);
                    if (i == 0) {
                        break;
                    }
                }
            }
        }
    } while (true);

    opaque(i);
    opaque(42);
    opaque(41);
    return i;
}
noInline(test);

function loop() {
    for (let i = 0; i < testLoopCount; ++i)
        test(1, 1);
}
noInline(loop);
noDFG(loop);

loop();