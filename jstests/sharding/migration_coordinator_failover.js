/**
 * Tests that a donor resumes coordinating a migration if it fails over after creating the
 * migration coordinator document but before deleting it.
 *
 * @tags: [requires_fcv_44]
 */

// This test induces failovers on shards.
TestData.skipCheckingUUIDsConsistentAcrossCluster = true;

(function() {
'use strict';

load("jstests/libs/fail_point_util.js");
load('jstests/libs/parallel_shell_helpers.js');

function getNewNs(dbName) {
    if (typeof getNewNs.counter == 'undefined') {
        getNewNs.counter = 0;
    }
    getNewNs.counter++;
    const collName = "ns" + getNewNs.counter;
    return [collName, dbName + "." + collName];
}

const dbName = "test";

var st = new ShardingTest({shards: 2, rs: {nodes: 2}});

assert.commandWorked(st.s.adminCommand({enableSharding: dbName}));
assert.commandWorked(st.s.adminCommand({movePrimary: dbName, to: st.shard0.shardName}));

function runMoveChunkMakeDonorStepDownAfterFailpoint(
    failpointName, shouldMakeMigrationFailToCommitOnConfig, expectAbortDecisionWithCode) {
    const [collName, ns] = getNewNs(dbName);
    jsTest.log("Running migration, making donor step down after failpoint " + failpointName +
               "; shouldMakeMigrationFailToCommitOnConfig is " +
               shouldMakeMigrationFailToCommitOnConfig + "; expectAbortDecisionWithCode is " +
               expectAbortDecisionWithCode + "; ns is " + ns);

    // Wait for mongos to see a primary node on the primary shard, because mongos does not retry
    // writes on NotMaster errors, and we are about to insert docs through mongos.
    awaitRSClientHosts(st.s, st.rs0.getPrimary(), {ok: true, ismaster: true});

    // Insert some docs into the collection so that the migration leaves orphans on either the
    // donor or recipient, depending on the decision.
    const numDocs = 1000;
    var bulk = st.s.getDB(dbName).getCollection(collName).initializeUnorderedBulkOp();
    for (var i = 0; i < numDocs; i++) {
        bulk.insert({_id: i});
    }
    assert.commandWorked(bulk.execute());

    // Shard the collection.
    assert.commandWorked(st.s.adminCommand({shardCollection: ns, key: {_id: 1}}));

    if (shouldMakeMigrationFailToCommitOnConfig) {
        // Turn on a failpoint to make the migration commit fail on the config server.
        assert.commandWorked(st.configRS.getPrimary().adminCommand(
            {configureFailPoint: "migrationCommitVersionError", mode: "alwaysOn"}));
    }

    jsTest.log("Run the moveChunk asynchronously and wait for " + failpointName + " to be hit.");
    let failpointHandle = configureFailPoint(st.rs0.getPrimary(), failpointName);
    const awaitResult = startParallelShell(
        funWithArgs(function(ns, toShardName, expectAbortDecisionWithCode) {
            if (expectAbortDecisionWithCode) {
                assert.commandFailedWithCode(
                    db.adminCommand({moveChunk: ns, find: {_id: 0}, to: toShardName}),
                    expectAbortDecisionWithCode);
            } else {
                assert.commandWorked(
                    db.adminCommand({moveChunk: ns, find: {_id: 0}, to: toShardName}));
            }
        }, ns, st.shard1.shardName, expectAbortDecisionWithCode), st.s.port);
    failpointHandle.wait();

    jsTest.log("Make the donor primary step down.");
    assert.commandWorked(
        st.rs0.getPrimary().adminCommand({replSetStepDown: 10 /* stepDownSecs */, force: true}));
    failpointHandle.off();

    jsTest.log("Allow the moveChunk to finish.");
    awaitResult();

    if (expectAbortDecisionWithCode) {
        jsTest.log("Expect abort decision, so wait for recipient to clean up the orphans.");
        assert.soon(() => {
            return 0 === st.rs1.getPrimary().getDB(dbName).getCollection(collName).count();
        });

    } else {
        jsTest.log("Expect commit decision, so wait for donor to clean up the orphans.");
        assert.soon(() => {
            return 0 === st.rs0.getPrimary().getDB(dbName).getCollection(collName).count();
        });
    }

    // The data should still be present on the shard that owns the chunk.
    assert.eq(numDocs, st.s.getDB(dbName).getCollection(collName).count());

    jsTest.log("Wait for the donor to delete the migration coordinator doc");
    assert.soon(() => {
        return 0 ===
            st.rs0.getPrimary().getDB("config").getCollection("migrationCoordinators").count();
    });

    if (shouldMakeMigrationFailToCommitOnConfig) {
        // Turn off the failpoint on the config server before returning.
        assert.commandWorked(st.configRS.getPrimary().adminCommand(
            {configureFailPoint: "migrationCommitVersionError", mode: "off"}));
    }
}

//
// Decision is commit
//

runMoveChunkMakeDonorStepDownAfterFailpoint("hangBeforeMakingCommitDecisionDurable",
                                            false /* shouldMakeMigrationFailToCommitOnConfig */);
runMoveChunkMakeDonorStepDownAfterFailpoint("hangBeforeSendingCommitDecision",
                                            false /* shouldMakeMigrationFailToCommitOnConfig */);
runMoveChunkMakeDonorStepDownAfterFailpoint("hangBeforeForgettingMigrationAfterCommitDecision",
                                            false /* shouldMakeMigrationFailToCommitOnConfig */);

//
// Decision is abort
//

runMoveChunkMakeDonorStepDownAfterFailpoint("moveChunkHangAtStep3",
                                            false /* shouldMakeMigrationFailToCommitOnConfig */,
                                            ErrorCodes.OperationFailed);

runMoveChunkMakeDonorStepDownAfterFailpoint("moveChunkHangAtStep4",
                                            false /* shouldMakeMigrationFailToCommitOnConfig */,
                                            ErrorCodes.OperationFailed);

runMoveChunkMakeDonorStepDownAfterFailpoint("moveChunkHangAtStep5",
                                            false /* shouldMakeMigrationFailToCommitOnConfig */,
                                            ErrorCodes.OperationFailed);

runMoveChunkMakeDonorStepDownAfterFailpoint("hangBeforeMakingAbortDecisionDurable",
                                            true /* shouldMakeMigrationFailToCommitOnConfig */,
                                            ErrorCodes.StaleEpoch);

runMoveChunkMakeDonorStepDownAfterFailpoint("hangBeforeSendingAbortDecision",
                                            true /* shouldMakeMigrationFailToCommitOnConfig */,
                                            ErrorCodes.StaleEpoch);

runMoveChunkMakeDonorStepDownAfterFailpoint("hangBeforeForgettingMigrationAfterAbortDecision",
                                            true /* shouldMakeMigrationFailToCommitOnConfig */,
                                            ErrorCodes.StaleEpoch);

st.stop();
})();
