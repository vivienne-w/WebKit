function shouldBe(actual, expected) {
    if (actual !== expected)
        throw new Error('bad value: ' + actual);
}

import("./resources/module-namespace-access-transitive-exports.js").then((ns) => {
    for (var i = 0; i < testLoopCount; ++i) {
        shouldBe(ns.test, 42);
        shouldBe(ns.cocoa(), 42);
    }
});
drainMicrotasks();
