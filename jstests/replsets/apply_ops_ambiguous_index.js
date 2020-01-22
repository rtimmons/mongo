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
MongoRunner.stopMongod(secondary);

assert.writeOK(primaryColl.insert({_id: 1, a: [{0:1}]}));

// Add secondary back (now with replset config)
// It will now have to catch up to the primary.
// It should ignore index violations.
secondary = MongoRunner.runMongod(originalOptions);


replSet.awaitReplication();

// There are different indexes on secondary and primary.
replSet.stopSet(undefined, undefined, {skipCheckDBHashes: true, skipValidation: true});

})();
