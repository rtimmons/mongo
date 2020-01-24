(function() {
"use strict";

load("jstests/libs/fail_point_util.js");

// Start one-node repl-set.
var rst = new ReplSetTest({name: "initial_sync_move_forward", nodes: 1});
rst.startSet();
rst.initiate();


var masterColl = rst.getPrimary().getDB("test").coll;

// Insert _id={0,1,2}
masterColl.ensureIndex({"a.0": 1}); // (1)
masterColl.insertOne({_id: 0, a:0});
masterColl.insertOne({_id: 1, a:1});
masterColl.insertOne({_id: 2, a:2});

// Add a secondary.
var secondary = rst.add({setParameter: "numInitialSyncAttempts=1"});
secondary.setSlaveOk();
var secondaryColl = secondary.getDB("test").coll;


var failPoint = configureFailPoint(secondary,
                                   "initialSyncHangDuringCollectionClone",
                                   {namespace: secondaryColl.getFullName(), numDocsToClone: 4});
rst.reInitiate();
failPoint.wait(10000);


// masterColl.insertOne({_id: 2, a:[{"0": 1}]}); (2)
// masterColl.deleteOne({_id: 2});
// masterColl.ensureIndex({"a.0": 1});

masterColl.dropIndex({"a.0": 1}); // (1)
masterColl.insertOne({_id: 200, a:[{"0": 1}]});

// Resume initial sync.
failPoint.off();

// Wait for initial sync to finish.
rst.awaitSecondaryNodes();

// Check document count on secondary.
assert.eq(4, secondaryColl.find().itcount());

rst.stopSet();
})();
