//
// Asserts that inserting a document like `{a:[{"0":1}]}` on the
// primary doesn't cause initial-sync to fail if there was index
// on `a.0` at the beginning of initial-sync but the `dropIndex`
// hasn't yet been applied on the secondary prior to applying the
// insert oplog entry.
//

(function() {
"use strict";

load("jstests/libs/fail_point_util.js");

// Start one-node repl-set.
const rst = new ReplSetTest({name: "apply_ops_ambiguous_index", nodes: 1});
rst.startSet();
rst.initiate();
const masterColl = rst.getPrimary().getDB("test").coll;

// Insert 10 docs that need to be applied on the secondary.
for (let i = 0; i < 10; ++i) {
    masterColl.insertOne({_id: i, a: i});
}

// Add a secondary.
const secondary =
    rst.add({setParameter: {"numInitialSyncAttempts": 1, 'collectionClonerBatchSize': 1}});
secondary.setSlaveOk();
const secondaryColl = secondary.getDB("test").coll;

// We set the collectionClonerBatchSize low above, so we will definitely hit
// this fail-point and hang after the first batch is applied. While the
// secondary is hung we apply the problematic document.
const failPoint = configureFailPoint(secondary,
                                     "initialSyncHangCollectionClonerAfterHandlingBatchResponse",
                                     {nss: secondaryColl.getFullName()});
rst.reInitiate();
failPoint.wait();

// Master no longer has the problematic index so the insert succeeds on master.
// The collection-cloner will resume when the failpoint is turned off.
masterColl.dropIndex({"a.0": 1});
masterColl.insertOne({_id: 200, a: [{"0": 1}]});

// Resume initial sync. The "bad" document will be applied; the dropIndex will
// be applied later.
failPoint.off();

// Wait for initial sync to finish.
rst.awaitSecondaryNodes();

// Check document count on secondary.
// 10 from the initial `for` loop and the 1 "bad" document.
assert.eq(11, secondaryColl.find().itcount());

rst.stopSet();
})();
