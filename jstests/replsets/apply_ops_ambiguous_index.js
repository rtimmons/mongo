(function() {
"use strict";

load("jstests/libs/fail_point_util.js");

// Start one-node repl-set.
var rst = new ReplSetTest({name: "initial_sync_move_forward", nodes: 1});
rst.startSet();
rst.initiate();


var masterColl = rst.getPrimary().getDB("test").coll;

for(let i=0; i< 10; ++i) {
    masterColl.insertOne({_id: i, a:i});
}

// Add a secondary.
var secondary = rst.add({setParameter: {
    "numInitialSyncAttempts": 1,
    'collectionClonerBatchSize': 1
}});

secondary.setSlaveOk();
var secondaryColl = secondary.getDB("test").coll;


var failPoint = configureFailPoint(secondary,
                                   "initialSyncHangCollectionClonerAfterHandlingBatchResponse",
                                   {nss: secondaryColl.getFullName(), numDocsToClone: 1});
rst.reInitiate();
failPoint.wait();


masterColl.dropIndex({"a.0": 1});
masterColl.insertOne({_id: 200, a:[{"0": 1}]});

// Resume initial sync.
failPoint.off();

// Wait for initial sync to finish.
rst.awaitSecondaryNodes();

// Check document count on secondary.
assert.eq(11, secondaryColl.find().itcount());

rst.stopSet();
})();
