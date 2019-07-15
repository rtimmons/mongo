'use strict';

/// <reference path="../../types/assert">
var assert;
/// <reference path="../../types/mongo">
var Mongo;

/**
 * Returns true if the process is a mongos, and false otherwise.
 * @param {Mongo.Database} db
 * @return boolean
 */
function isMongos(db) {
    // Run isMaster directly on the database's session's client to bypass any session machinery.
    const res = assert.commandWorked(db.getSession().getClient().adminCommand('ismaster'));
    return 'isdbgrid' === res.msg;
}

/**
 * Returns true if the process is a mongod, and false otherwise.
 * @param {Mongo.Database} db
 */
function isMongod(db) {
    return !isMongos(db);
}

/**
 * Returns true if the process is a mongod configsvr, and false otherwise.
 * @param {Mongo.Database} db
 * @return boolean
 */
function isMongodConfigsvr(db) {
    if (!isMongod(db)) {
        return false;
    }
    var res = db.adminCommand('getCmdLineOpts');
    assert.commandWorked(res);

    return res.parsed && res.parsed.sharding && res.parsed.sharding.clusterRole === 'configsvr';
}

/**
 * Returns the name of the current storage engine.
 *
 * Throws an error if db is connected to a mongos, or if there is no reported storage engine.
 *
 * @param {Mongo.Database} db
 * @return boolean
 */
function getStorageEngineName(db) {
    var status = db.serverStatus();
    assert.commandWorked(status);

    assert(isMongod(db), 'no storage engine is reported when connected to mongos');
    assert.neq(
        'undefined', typeof status.storageEngine, 'missing storage engine info in server status');

    return status.storageEngine.name;
}

/**
 * Returns true if the current storage engine is wiredTiger, and false otherwise.
 * @param {Mongo.Database} db
 * @return boolean
 */
function isWiredTiger(db) {
    return getStorageEngineName(db) === 'wiredTiger';
}

/**
 * Returns true if the current storage engine is ephemeralForTest, and false otherwise.
 * @param {Mongo.Database} db
 * @return boolean
 */
function isEphemeralForTest(db) {
    return getStorageEngineName(db) === 'ephemeralForTest';
}

/**
 * Returns true if the current storage engine is ephemeral, and false otherwise.
 * @param {Mongo.Database} db
 * @return boolean
 */
function isEphemeral(db) {
    var engine = getStorageEngineName(db);
    return (engine === 'inMemory') || (engine === 'ephemeralForTest');
}

/**
 * Returns true if the current storage engine supports document-level concurrency, and false
 * otherwise.
 * @param {Mongo.Database} db
 * @return boolean
 */
function supportsDocumentLevelConcurrency(db) {
    var engine = getStorageEngineName(db);
    return ['wiredTiger', 'rocksdb', 'inMemory'].indexOf(engine) !== -1;
}

/**
 * Returns true if the current storage engine supports committed reads.
 *
 * Throws an error if db is connected to a mongos, or if there is no reported storage engine.
 *
 * @param {Mongo.Database} db
 * @return boolean
 */
function supportsCommittedReads(db) {
    var status = db.serverStatus();
    assert.commandWorked(status);

    assert(isMongod(db), 'no storage engine is reported when connected to mongos');
    assert.neq(
        'undefined', typeof status.storageEngine, 'missing storage engine info in server status');

    return status.storageEngine.supportsCommittedReads;
}
