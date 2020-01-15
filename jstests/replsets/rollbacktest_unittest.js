
/**
 * Test of the RollbackTest helper lib.
 */

load("jstests/replsets/rslib.js");
load("jstests/replsets/libs/rollback_test.js");

(function(){
"use strict";

    // callback is type f: RollbackTest -> void
    const testCase = function(msg, requiredCallCount, callback) {
        let rollbackTest = new RollbackTest("rollbacktest_unittest");
        let callCount = 0;
        rollbackTest.checkDataConsistency = function() {
            ++callCount;
        };
        try {
            callback(rollbackTest);
        } finally {
            assert.eq(callCount, requiredCallCount, msg);
        }
    };

    testCase("single stop call only calls once", 1, rbt => {
        rbt.stop();
    });

    testCase("single stop call only calls once", 1, rbt => {

    });

})();
