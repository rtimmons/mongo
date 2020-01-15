
/**
 * Test of the RollbackTest helper lib.
 */

load("jstests/replsets/rslib.js");
load("jstests/replsets/libs/rollback_test.js");

(function(){
"use strict";

    // callback is type f: RollbackTest -> void
    const testCase = function(callback) {
        const context = {
            checkDataConsistencyCallCount: 0,
            stopSetCalls: [],
        };

        const rollbackTest = new RollbackTest("rollbacktest_unittest");
        const rst = rollbackTest.getTestFixture();
        rst.stopSet = function(signal, forRestart, opts) {
            context.stopSetCalls.push(opts);

            // We don't care about doing the regular stopSet actions.
            for(let i=rst.nodeList().length-1; i >= 0; --i) {
                rst.stop(i);
            }
        };

        rollbackTest.checkDataConsistency = function() {
            ++context.checkDataConsistencyCallCount;
        };
        callback(rollbackTest, context);
    };

    testCase((rbt, ctx) => {
        assert.eq(ctx.checkDataConsistencyCallCount, 0);
        assert.eq(ctx.stopSetCalls, []);
        rbt.stop();
        assert.eq(ctx.checkDataConsistencyCallCount, 1);
        assert.eq(ctx.stopSetCalls, [{"skipCheckDBHashes" : true, "skipValidation" : true}]);
    });

})();
