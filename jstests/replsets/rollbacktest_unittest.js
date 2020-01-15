
/**
 * Test of the RollbackTest helper lib.
 */

load("jstests/replsets/rslib.js");
load("jstests/replsets/libs/rollback_test.js");

(function(){
"use strict";

    // callback is type f: RollbackTest -> void
    const testCase = function(callback) {
        const context = {checkDataConsistencyCalls: 0};

        const rollbackTest = new RollbackTest("rollbacktest_unittest");

        rollbackTest.checkDataConsistency = function() {
            ++context.checkDataConsistencyCalls;
        };
        callback(rollbackTest, context);
    };

    testCase((rbt, ctx) => {
        assert.eq(ctx.checkDataConsistencyCalls, 0);
        rbt.stop();
        assert.eq(ctx.checkDataConsistencyCalls, 1);
    });

})();
