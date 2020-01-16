
/**
 * Test of the RollbackTest helper library.
 */

load("jstests/replsets/rslib.js");
load("jstests/replsets/libs/rollback_test.js");

(function() {
"use strict";

// callback is type f: RollbackTest -> void
const testCase = function(callback) {
    const context = {
        checkDataConsistencyCallCount: 0,
        stopSetCallCount: 0,
    };

    const rollbackTest = new RollbackTest("rollbacktest_unittest");
    rollbackTest._checkDataConsistencyImpl = function() {
        ++context.checkDataConsistencyCallCount;
    };

    const rst = rollbackTest.getTestFixture();
    rst.stopSet = function(signal, forRestart, opts) {
        // Unoconditionally skip in rst.stopSet because rbt.stop does its own validation.
        assert.eq(opts, {"skipCheckDBHashes": true, "skipValidation": true});
        ++context.stopSetCallCount;

        // We don't care about doing the regular stopSet actions.
        for (let i = rst.nodeList().length - 1; i >= 0; --i) {
            rst.stop(i);
        }
    };

    callback(rollbackTest, context);
};

// Each call to testCase takes about 10 seconds, so be judicious about
// adding too many of them.

testCase((rbt, ctx) => {
    assert.eq(ctx.checkDataConsistencyCallCount, 0);
    assert.eq(ctx.stopSetCallCount, 0);
    rbt.stop();
    assert.eq(ctx.checkDataConsistencyCallCount, 1);
    assert.eq(ctx.stopSetCallCount, 1);
});

testCase((rbt, ctx) => {
    // Force it.
    rbt.checkDataConsistency();
    // Above test asserts that the call counts are 1 now.

    // We haven't done anything that should require more changes.
    rbt.stop();
    assert.eq(ctx.checkDataConsistencyCallCount, 1);
    assert.eq(ctx.stopSetCallCount, 1);
});

testCase((rbt, ctx) => {
    // Force it.
    rbt.checkDataConsistency();

    // State-transitions forces an additional consistency-check.
    rbt.transitionToRollbackOperations();
    rbt.transitionToSyncSourceOperationsBeforeRollback();
    rbt.transitionToSyncSourceOperationsDuringRollback();
    rbt.transitionToSteadyStateOperations();

    // This will do another call.
    rbt.stop();
    assert.eq(ctx.checkDataConsistencyCallCount, 2);
    assert.eq(ctx.stopSetCallCount, 1);
});

testCase((rbt, ctx) => {
    // Force it.
    rbt.checkDataConsistency();

    // Add a node. It may not be consistent so we'll need to run checks at time of stop().
    rbt.add({
        config: {
            rsConfig: {priority: 0, votes: 0},
        },
    });

    rbt.stop();

    assert.eq(ctx.checkDataConsistencyCallCount, 2);
    assert.eq(ctx.stopSetCallCount, 1);
});
})();
