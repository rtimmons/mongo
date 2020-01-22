/**
 * Initial sync runs in several phases - the first 3 are as follows:
 * 1) fetches the last oplog entry (op_start1) on the source;
 * 2) copies all non-local databases from the source and fetches operations from sync source; and
 * 3) applies operations from the source after op_start1.
 *
 * This test renames a collection on the source between phases 1 and 2, but renameCollection is not
 * supported in initial sync. The secondary will initially fail to apply the command in phase 3
 * and subsequently have to retry the initial sync.
 */

(function() {
"use strict";

const name = 'initial_sync_applier_error';
const replSet = new ReplSetTest({
    name: name,
    nodes: [{}, {}],
});

replSet.startSet();
replSet.initiate();
const primary = replSet.getPrimary();

const primaryColl = primary.getDB('test').getCollection(name);
assert.writeOK(primaryColl.insert({_id: 0, a: 0}));

// Restart the secondary as a standalone node.
replSet.stop(1);

// restart secondary without replset config
const options = replSet.getSecondary().savedOptions;
options.noCleanData = true;
const originalOptions = Object.merge({}, options);
delete options.replSet;
let secondary = MongoRunner.runMongod(options);
assert.neq(null, secondary, "secondary failed to start");

// Add index on secondary and restart
assert.commandWorked(secondary.getDB("test").getCollection(name).createIndex({"a.0": 1}));
print("==================================== Created Index");
MongoRunner.stopMongod(secondary);

assert.writeOK(primaryColl.insert({_id: 1, a: [{0:1}]}));

// Add secondary back (now with replset config)
// It will now have to catch up to the primary.
// It should ignore index violations.
secondary = MongoRunner.runMongod(originalOptions);


replSet.awaitReplication();

replSet.stopSet();

})();
