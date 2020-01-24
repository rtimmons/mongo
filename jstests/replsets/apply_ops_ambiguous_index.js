(function() {
"use strict";

load("jstests/libs/fail_point_util.js");

// Start one-node repl-set.
var rst = new ReplSetTest({name: "initial_sync_move_forward", nodes: 1});
rst.startSet();
rst.initiate();

var masterColl = rst.getPrimary().getDB("test").coll;

// Insert _id={0,1}
masterColl.insertOne({_id: 0, a:0});
masterColl.insertOne({_id: 1, a:1});

// Add a secondary.
var secondary = rst.add({setParameter: "numInitialSyncAttempts=1"});
secondary.setSlaveOk();
var secondaryColl = secondary.getDB("test").coll;


var failPoint = configureFailPoint(secondary,
                                   "initialSyncHangDuringCollectionClone",
                                   {namespace: secondaryColl.getFullName(), numDocsToClone: 2});
rst.reInitiate();
failPoint.wait();

masterColl.insertOne({_id: 2, a:[{"0": 1}]});
masterColl.deleteOne({_id: 2});
masterColl.ensureIndex({"a.0": 1});

// Resume initial sync.
failPoint.off();

// Wait for initial sync to finish.
rst.awaitSecondaryNodes();

// Check document count on secondary.
assert.eq(2, secondaryColl.find().itcount());

rst.stopSet();
})();
