// @file collection.js - DBCollection support in the mongo shell
// db.colName is a DBCollection object
// or db["colName"]

if ((typeof DBCollection) == "undefined") {
    DBCollection = function(mongo, db, shortName, fullName) {
        this._mongo = mongo;
        this._db = db;
        this._shortName = shortName;
        this._fullName = fullName;

        this.verify();
    };
}

DBCollection.prototype.verify = function() {
    assert(this._fullName, "no fullName");
    assert(this._shortName, "no shortName");
    assert(this._db, "no db");

    assert.eq(this._fullName, this._db._name + "." + this._shortName, "name mismatch");

    assert(this._mongo, "no mongo in DBCollection");
    assert(this.getMongo(), "no mongo from getMongo()");
};

DBCollection.prototype.getName = function() {
    return this._shortName;
};

DBCollection.prototype.help = function() {
    var shortName = this.getName();
    print("DBCollection help");
    print("\tdb." + shortName + ".find().help() - show DBCursor help");
    print(
        "\tdb." + shortName +
        ".bulkWrite( operations, <optional params> ) - bulk execute write operations, optional parameters are: w, wtimeout, j");
    print(
        "\tdb." + shortName +
        ".count( query = {}, <optional params> ) - count the number of documents that matches the query, optional parameters are: limit, skip, hint, maxTimeMS");
    print(
        "\tdb." + shortName +
        ".countDocuments( query = {}, <optional params> ) - count the number of documents that matches the query, optional parameters are: limit, skip, hint, maxTimeMS");
    print(
        "\tdb." + shortName +
        ".estimatedDocumentCount( <optional params> ) - estimate the document count using collection metadata, optional parameters are: maxTimeMS");
    print("\tdb." + shortName + ".convertToCapped(maxBytes) - calls {convertToCapped:'" +
          shortName + "', size:maxBytes}} command");
    print("\tdb." + shortName + ".createIndex(keypattern[,options])");
    print("\tdb." + shortName + ".createIndexes([keypatterns], <options>)");
    print("\tdb." + shortName + ".dataSize()");
    print(
        "\tdb." + shortName +
        ".deleteOne( filter, <optional params> ) - delete first matching document, optional parameters are: w, wtimeout, j");
    print(
        "\tdb." + shortName +
        ".deleteMany( filter, <optional params> ) - delete all matching documents, optional parameters are: w, wtimeout, j");
    print("\tdb." + shortName + ".distinct( key, query, <optional params> ) - e.g. db." +
          shortName + ".distinct( 'x' ), optional parameters are: maxTimeMS");
    print("\tdb." + shortName + ".drop() drop the collection");
    print("\tdb." + shortName + ".dropIndex(index) - e.g. db." + shortName +
          ".dropIndex( \"indexName\" ) or db." + shortName + ".dropIndex( { \"indexKey\" : 1 } )");
    print("\tdb." + shortName + ".hideIndex(index) - e.g. db." + shortName +
          ".hideIndex( \"indexName\" ) or db." + shortName + ".hideIndex( { \"indexKey\" : 1 } )");
    print("\tdb." + shortName + ".unhideIndex(index) - e.g. db." + shortName +
          ".unhideIndex( \"indexName\" ) or db." + shortName +
          ".unhideIndex( { \"indexKey\" : 1 } )");
    print("\tdb." + shortName + ".dropIndexes()");
    print("\tdb." + shortName +
          ".ensureIndex(keypattern[,options]) - DEPRECATED, use createIndex() instead");
    print("\tdb." + shortName + ".explain().help() - show explain help");
    print("\tdb." + shortName + ".reIndex()");
    print(
        "\tdb." + shortName +
        ".find([query],[fields]) - query is an optional query filter. fields is optional set of fields to return.");
    print("\t                                              e.g. db." + shortName +
          ".find( {x:77} , {name:1, x:1} )");
    print("\tdb." + shortName + ".find(...).count()");
    print("\tdb." + shortName + ".find(...).limit(n)");
    print("\tdb." + shortName + ".find(...).skip(n)");
    print("\tdb." + shortName + ".find(...).sort(...)");
    print("\tdb." + shortName + ".findOne([query], [fields], [options], [readConcern])");
    print(
        "\tdb." + shortName +
        ".findOneAndDelete( filter, <optional params> ) - delete first matching document, optional parameters are: projection, sort, maxTimeMS");
    print(
        "\tdb." + shortName +
        ".findOneAndReplace( filter, replacement, <optional params> ) - replace first matching document, optional parameters are: projection, sort, maxTimeMS, upsert, returnNewDocument");
    print(
        "\tdb." + shortName +
        ".findOneAndUpdate( filter, <update object or pipeline>, <optional params> ) - update first matching document, optional parameters are: projection, sort, maxTimeMS, upsert, returnNewDocument");
    print("\tdb." + shortName + ".getDB() get DB object associated with collection");
    print("\tdb." + shortName + ".getPlanCache() get query plan cache associated with collection");
    print("\tdb." + shortName + ".getIndexes()");
    print("\tdb." + shortName + ".insert(obj)");
    print(
        "\tdb." + shortName +
        ".insertOne( obj, <optional params> ) - insert a document, optional parameters are: w, wtimeout, j");
    print(
        "\tdb." + shortName +
        ".insertMany( [objects], <optional params> ) - insert multiple documents, optional parameters are: w, wtimeout, j");
    print("\tdb." + shortName + ".mapReduce( mapFunction , reduceFunction , <optional params> )");
    print(
        "\tdb." + shortName +
        ".aggregate( [pipeline], <optional params> ) - performs an aggregation on a collection; returns a cursor");
    print("\tdb." + shortName + ".remove(query)");
    print(
        "\tdb." + shortName +
        ".replaceOne( filter, replacement, <optional params> ) - replace the first matching document, optional parameters are: upsert, w, wtimeout, j");
    print("\tdb." + shortName +
          ".renameCollection( newName , <dropTarget> ) renames the collection.");
    print(
        "\tdb." + shortName +
        ".runCommand( name , <options> ) runs a db command with the given name where the first param is the collection name");
    print("\tdb." + shortName + ".save(obj)");
    print("\tdb." + shortName + ".stats({scale: N, indexDetails: true/false, " +
          "indexDetailsKey: <index key>, indexDetailsName: <index name>})");
    // print("\tdb." + shortName + ".diskStorageStats({[extent: <num>,] [granularity: <bytes>,]
    // ...}) - analyze record layout on disk");
    // print("\tdb." + shortName + ".pagesInRAM({[extent: <num>,] [granularity: <bytes>,] ...}) -
    // analyze resident memory pages");
    print("\tdb." + shortName +
          ".storageSize() - includes free space allocated to this collection");
    print("\tdb." + shortName + ".totalIndexSize() - size in bytes of all the indexes");
    print("\tdb." + shortName + ".totalSize() - storage allocated for all data and indexes");
    print(
        "\tdb." + shortName +
        ".update( query, <update object or pipeline>[, upsert_bool, multi_bool] ) - instead of two flags, you can pass an object with fields: upsert, multi, hint");
    print(
        "\tdb." + shortName +
        ".updateOne( filter, <update object or pipeline>, <optional params> ) - update the first matching document, optional parameters are: upsert, w, wtimeout, j, hint");
    print(
        "\tdb." + shortName +
        ".updateMany( filter, <update object or pipeline>, <optional params> ) - update all matching documents, optional parameters are: upsert, w, wtimeout, j, hint");
    print("\tdb." + shortName + ".validate( <full> ) - SLOW");
    print("\tdb." + shortName + ".getShardVersion() - only for use with sharding");
    print("\tdb." + shortName +
          ".getShardDistribution() - prints statistics about data distribution in the cluster");
    print(
        "\tdb." + shortName +
        ".getSplitKeysForChunks( <maxChunkSize> ) - calculates split points over all chunks and returns splitter function");
    print(
        "\tdb." + shortName +
        ".getWriteConcern() - returns the write concern used for any operations on this collection, inherited from server/db if set");
    print(
        "\tdb." + shortName +
        ".setWriteConcern( <write concern doc> ) - sets the write concern for writes to the collection");
    print(
        "\tdb." + shortName +
        ".unsetWriteConcern( <write concern doc> ) - unsets the write concern for writes to the collection");
    print("\tdb." + shortName +
          ".latencyStats() - display operation latency histograms for this collection");
    return __magicNoPrint;
};

DBCollection.prototype.getFullName = function() {
    return this._fullName;
};
DBCollection.prototype.getMongo = function() {
    return this._db.getMongo();
};
DBCollection.prototype.getDB = function() {
    return this._db;
};

DBCollection.prototype._makeCommand = function(cmd, params) {
    var c = {};
    c[cmd] = this.getName();
    if (params)
        Object.extend(c, params);
    return c;
};

DBCollection.prototype._dbCommand = function(cmd, params) {
    if (typeof (cmd) === "object")
        return this._db._dbCommand(cmd, {}, this.getQueryOptions());

    return this._db._dbCommand(this._makeCommand(cmd, params), {}, this.getQueryOptions());
};

// Like _dbCommand, but applies $readPreference
DBCollection.prototype._dbReadCommand = function(cmd, params) {
    if (typeof (cmd) === "object")
        return this._db._dbReadCommand(cmd, {}, this.getQueryOptions());

    return this._db._dbReadCommand(this._makeCommand(cmd, params), {}, this.getQueryOptions());
};

DBCollection.prototype.runCommand = DBCollection.prototype._dbCommand;

DBCollection.prototype.runReadCommand = DBCollection.prototype._dbReadCommand;

DBCollection.prototype._massageObject = function(q) {
    if (!q)
        return {};

    var type = typeof q;

    if (type == "function")
        return {$where: q};

    if (q.isObjectId)
        return {_id: q};

    if (type == "object")
        return q;

    if (type == "string") {
        // If the string is 24 hex characters, it is most likely an ObjectId.
        if (/^[0-9a-fA-F]{24}$/.test(q)) {
            return {_id: ObjectId(q)};
        }

        return {$where: q};
    }

    throw Error("don't know how to massage : " + type);
};

DBCollection.prototype.find = function(query, fields, limit, skip, batchSize, options) {
    var cursor = new DBQuery(this._mongo,
                             this._db,
                             this,
                             this._fullName,
                             this._massageObject(query),
                             fields,
                             limit,
                             skip,
                             batchSize,
                             options || this.getQueryOptions());

    {
        const session = this.getDB().getSession();

        const readPreference = session._getSessionAwareClient().getReadPreference(session);
        if (readPreference !== null) {
            cursor.readPref(readPreference.mode, readPreference.tags);
        }

        const readConcern = session._getSessionAwareClient().getReadConcern(session);
        if (readConcern !== null) {
            cursor.readConcern(readConcern.level);
        }
    }

    return cursor;
};

DBCollection.prototype.findOne = function(query, fields, options, readConcern, collation) {
    var cursor = this.find(query, fields, -1 /* limit */, 0 /* skip*/, 0 /* batchSize */, options);

    if (readConcern) {
        cursor = cursor.readConcern(readConcern);
    }

    if (collation) {
        cursor = cursor.collation(collation);
    }

    if (!cursor.hasNext())
        return null;
    var ret = cursor.next();
    if (cursor.hasNext())
        throw Error("findOne has more than 1 result!");
    if (ret.$err)
        throw _getErrorWithCode(ret, "error " + tojson(ret));
    return ret;
};

// Returns a WriteResult for a single insert or a BulkWriteResult for a multi-insert if write
// command succeeded, but may contain write errors.
// Returns a WriteCommandError if the write command responded with ok:0.
DBCollection.prototype.insert = function(obj, options) {
    if (!obj)
        throw Error("no object passed to insert!");

    var flags = 0;

    var wc = undefined;
    var allowDottedFields = false;
    if (options === undefined) {
        // do nothing
    } else if (typeof (options) == 'object') {
        if (options.ordered === undefined) {
            // do nothing, like above
        } else {
            flags = options.ordered ? 0 : 1;
        }

        if (options.writeConcern)
            wc = options.writeConcern;
        if (options.allowdotted)
            allowDottedFields = true;
    } else {
        flags = options;
    }

    // 1 = continueOnError, which is synonymous with unordered in the write commands/bulk-api
    var ordered = ((flags & 1) == 0);

    // wc = undefined
    if (!wc)
        wc = this._createWriteConcern(options);

    var result = undefined;
    var startTime =
        (typeof (_verboseShell) === 'undefined' || !_verboseShell) ? 0 : new Date().getTime();

    if (this.getMongo().writeMode() != "legacy") {
        // Bit 1 of option flag is continueOnError. Bit 0 (stop on error) is the default.
        var bulk = ordered ? this.initializeOrderedBulkOp() : this.initializeUnorderedBulkOp();
        var isMultiInsert = Array.isArray(obj);

        // print("!= legacy with " + JSON.stringify({
        //     options: options,
        //     obj: obj,
        //     flags: flags,
        //     fullname: this._fullName,
        //     bulk: bulk,
        // }));

        if (isMultiInsert) { // false
            obj.forEach(function(doc) {
                bulk.insert(doc);
            });
        } else {
            // print("Doing bulk.insert(obj) = " + JSON.stringify(obj));
            bulk.insert(obj);
        }

        try {
            // print("wc = " + JSON.stringify(wc));
            result = bulk.execute(wc);
            if (!isMultiInsert)
                result = result.toSingleResult();
        } catch (ex) {
            // print("Caught exception " + ex);
            if (ex instanceof BulkWriteError) {
                result = isMultiInsert ? ex.toResult() : ex.toSingleResult();
            } else {
                // Other exceptions rethrown as-is.
                throw ex;
            }
        }
    } else {
        if (typeof (obj._id) == "undefined" && !Array.isArray(obj)) {
            var tmp = obj;  // don't want to modify input
            obj = {_id: new ObjectId()};
            for (var key in tmp) {
                obj[key] = tmp[key];
            }
        }

        this.getMongo().insert(this._fullName, obj, flags);

        // enforce write concern, if required
        if (wc)
            result = this.runCommand("getLastError", wc instanceof WriteConcern ? wc.toJSON() : wc);
    }

    this._lastID = obj._id;
    this._printExtraInfo("Inserted", startTime);
    return result;
};

/**
 * Does validation of the remove args. Throws if the parse is not successful, otherwise
 * returns a document {query: <query>, justOne: <limit>, wc: <writeConcern>}.
 */
DBCollection.prototype._parseRemove = function(t, justOne) {
    if (undefined === t)
        throw Error("remove needs a query");

    var query = this._massageObject(t);

    var wc = undefined;
    var collation = undefined;
    if (typeof (justOne) === "object") {
        var opts = justOne;
        wc = opts.writeConcern;
        justOne = opts.justOne;
        collation = opts.collation;
    }

    // Normalize "justOne" to a bool.
    justOne = justOne ? true : false;

    // Handle write concern.
    if (!wc) {
        wc = this.getWriteConcern();
    }

    return {"query": query, "justOne": justOne, "wc": wc, "collation": collation};
};

// Returns a WriteResult if write command succeeded, but may contain write errors.
// Returns a WriteCommandError if the write command responded with ok:0.
DBCollection.prototype.remove = function(t, justOne) {
    var parsed = this._parseRemove(t, justOne);
    var query = parsed.query;
    var justOne = parsed.justOne;
    var wc = parsed.wc;
    var collation = parsed.collation;

    var result = undefined;
    var startTime =
        (typeof (_verboseShell) === 'undefined' || !_verboseShell) ? 0 : new Date().getTime();

    if (this.getMongo().writeMode() != "legacy") {
        var bulk = this.initializeOrderedBulkOp();
        var removeOp = bulk.find(query);

        if (collation) {
            removeOp.collation(collation);
        }

        if (justOne) {
            removeOp.removeOne();
        } else {
            removeOp.remove();
        }

        try {
            result = bulk.execute(wc).toSingleResult();
        } catch (ex) {
            if (ex instanceof BulkWriteError) {
                result = ex.toSingleResult();
            } else if (ex instanceof WriteCommandError) {
                result = ex;
            } else {
                // Other exceptions thrown
                throw ex;
            }
        }
    } else {
        if (collation) {
            throw new Error("collation requires use of write commands");
        }

        this.getMongo().remove(this._fullName, query, justOne);

        // enforce write concern, if required
        if (wc)
            result = this.runCommand("getLastError", wc instanceof WriteConcern ? wc.toJSON() : wc);
    }

    this._printExtraInfo("Removed", startTime);
    return result;
};

/**
 * Does validation of the update args. Throws if the parse is not successful, otherwise returns a
 * document containing fields for query, updateSpec, upsert, multi, wc, collation, and arrayFilters.
 *
 * Throws if the arguments are invalid.
 */
DBCollection.prototype._parseUpdate = function(query, updateSpec, upsert, multi) {
    if (!query)
        throw Error("need a query");
    if (!updateSpec)
        throw Error("need an update object or pipeline");

    var wc = undefined;
    var collation = undefined;
    var arrayFilters = undefined;
    let hint = undefined;

    // can pass options via object for improved readability
    if (typeof (upsert) === "object") {
        if (multi) {
            throw Error("Fourth argument must be empty when specifying " +
                        "upsert and multi with an object.");
        }

        var opts = upsert;
        multi = opts.multi;
        wc = opts.writeConcern;
        upsert = opts.upsert;
        collation = opts.collation;
        arrayFilters = opts.arrayFilters;
        hint = opts.hint;
    }

    // Normalize 'upsert' and 'multi' to booleans.
    upsert = upsert ? true : false;
    multi = multi ? true : false;

    if (!wc) {
        wc = this.getWriteConcern();
    }

    return {
        "query": query,
        "updateSpec": updateSpec,
        "hint": hint,
        "upsert": upsert,
        "multi": multi,
        "wc": wc,
        "collation": collation,
        "arrayFilters": arrayFilters
    };
};

// Returns a WriteResult if write command succeeded, but may contain write errors.
// Returns a WriteCommandError if the write command responded with ok:0.
DBCollection.prototype.update = function(query, updateSpec, upsert, multi) {
    var parsed = this._parseUpdate(query, updateSpec, upsert, multi);
    var query = parsed.query;
    var updateSpec = parsed.updateSpec;
    const hint = parsed.hint;
    var upsert = parsed.upsert;
    var multi = parsed.multi;
    var wc = parsed.wc;
    var collation = parsed.collation;
    var arrayFilters = parsed.arrayFilters;

    var result = undefined;
    var startTime =
        (typeof (_verboseShell) === 'undefined' || !_verboseShell) ? 0 : new Date().getTime();

    if (this.getMongo().writeMode() != "legacy") {
        var bulk = this.initializeOrderedBulkOp();
        var updateOp = bulk.find(query);

        if (hint) {
            updateOp.hint(hint);
        }

        if (upsert) {
            updateOp = updateOp.upsert();
        }

        if (collation) {
            updateOp.collation(collation);
        }

        if (arrayFilters) {
            updateOp.arrayFilters(arrayFilters);
        }

        if (multi) {
            updateOp.update(updateSpec);
        } else {
            updateOp.updateOne(updateSpec);
        }

        try {
            result = bulk.execute(wc).toSingleResult();
        } catch (ex) {
            if (ex instanceof BulkWriteError) {
                result = ex.toSingleResult();
            } else if (ex instanceof WriteCommandError) {
                result = ex;
            } else {
                // Other exceptions thrown
                throw ex;
            }
        }
    } else {
        if (collation) {
            throw new Error("collation requires use of write commands");
        }

        if (arrayFilters) {
            throw new Error("arrayFilters requires use of write commands");
        }

        this.getMongo().update(this._fullName, query, updateSpec, upsert, multi);

        // Enforce write concern, if required
        if (wc) {
            result = this.runCommand("getLastError", wc instanceof WriteConcern ? wc.toJSON() : wc);
        }
    }

    this._printExtraInfo("Updated", startTime);
    return result;
};

DBCollection.prototype.save = function(obj, opts) {
    if (obj == null)
        throw Error("can't save a null");

    if (typeof (obj) == "number" || typeof (obj) == "string")
        throw Error("can't save a number or string");

    if (typeof (obj._id) == "undefined") {
        obj._id = new ObjectId();
        return this.insert(obj, opts);
    } else {
        return this.update({_id: obj._id}, obj, Object.merge({upsert: true}, opts));
    }
};

DBCollection.prototype._genIndexName = function(keys) {
    var name = "";
    for (var k in keys) {
        var v = keys[k];
        if (typeof v == "function")
            continue;

        if (name.length > 0)
            name += "_";
        name += k + "_";

        name += v;
    }
    return name;
};

DBCollection.prototype._indexSpec = function(keys, options) {
    var ret = {ns: this._fullName, key: keys, name: this._genIndexName(keys)};

    if (!options) {
    } else if (typeof (options) == "string")
        ret.name = options;
    else if (typeof (options) == "boolean")
        ret.unique = true;
    else if (typeof (options) == "object") {
        if (Array.isArray(options)) {
            if (options.length > 3) {
                throw new Error("Index options that are supplied in array form may only specify" +
                                " three values: name, unique, dropDups");
            }
            var nb = 0;
            for (var i = 0; i < options.length; i++) {
                if (typeof (options[i]) == "string")
                    ret.name = options[i];
                else if (typeof (options[i]) == "boolean") {
                    if (options[i]) {
                        if (nb == 0)
                            ret.unique = true;
                        if (nb == 1)
                            ret.dropDups = true;
                    }
                    nb++;
                }
            }
        } else {
            Object.extend(ret, options);
        }
    } else {
        throw Error("can't handle: " + typeof (options));
    }

    return ret;
};

DBCollection.prototype.createIndex = function(keys, options, commitQuorum) {
    return this.createIndexes([keys], options, commitQuorum);
};

DBCollection.prototype.createIndexes = function(keys, options, commitQuorum) {
    if (!Array.isArray(keys)) {
        throw new Error("createIndexes first argument should be an array");
    }

    var indexSpecs = Array(keys.length);
    for (var i = 0; i < indexSpecs.length; i++) {
        indexSpecs[i] = this._indexSpec(keys[i], options);
    }

    for (var i = 0; i < indexSpecs.length; i++) {
        delete (indexSpecs[i].ns);  // ns is passed to the first element in the command.
    }

    if (commitQuorum === undefined) {
        return this._db.runCommand({createIndexes: this.getName(), indexes: indexSpecs});
    }
    return this._db.runCommand(
        {createIndexes: this.getName(), indexes: indexSpecs, commitQuorum: commitQuorum});
};

DBCollection.prototype.ensureIndex = function(keys, options) {
    var result = this.createIndex(keys, options);

    if (this.getMongo().writeMode() != "legacy") {
        return result;
    }

    err = this.getDB().getLastErrorObj();
    if (err.err) {
        return err;
    }
    // nothing returned on success
};

DBCollection.prototype.reIndex = function() {
    return this._db.runCommand({reIndex: this.getName()});
};

DBCollection.prototype.dropIndexes = function(indexNames) {
    indexNames = indexNames || '*';
    var res = this._db.runCommand({dropIndexes: this.getName(), index: indexNames});
    assert(res, "no result from dropIndex result");
    if (res.ok)
        return res;

    if (res.errmsg.match(/not found/))
        return res;

    throw _getErrorWithCode(res, "error dropping indexes : " + tojson(res));
};

DBCollection.prototype.drop = function(options = {}) {
    const cmdObj = Object.assign({drop: this.getName()}, options);
    ret = this._db.runCommand(cmdObj);
    if (!ret.ok) {
        if (ret.errmsg == "ns not found")
            return false;
        throw _getErrorWithCode(ret, "drop failed: " + tojson(ret));
    }
    return true;
};

DBCollection.prototype.findAndModify = function(args) {
    var cmd = {findandmodify: this.getName()};
    for (var key in args) {
        cmd[key] = args[key];
    }

    {
        const kWireVersionSupportingRetryableWrites = 6;
        const serverSupportsRetryableWrites =
            this.getMongo().getMinWireVersion() <= kWireVersionSupportingRetryableWrites &&
            kWireVersionSupportingRetryableWrites <= this.getMongo().getMaxWireVersion();

        const session = this.getDB().getSession();
        if (serverSupportsRetryableWrites && session.getOptions().shouldRetryWrites() &&
            _ServerSession.canRetryWrites(cmd)) {
            cmd = session._serverSession.assignTransactionNumber(cmd);
        }
    }

    var ret = this._db.runCommand(cmd);
    if (!ret.ok) {
        if (ret.errmsg == "No matching object found") {
            return null;
        }
        throw _getErrorWithCode(ret, "findAndModifyFailed failed: " + tojson(ret));
    }
    return ret.value;
};

DBCollection.prototype.renameCollection = function(newName, dropTarget) {
    if (arguments.length === 1 && typeof newName === 'object') {
        if (newName.hasOwnProperty('dropTarget')) {
            dropTarget = newName['dropTarget'];
        }
        newName = newName['to'];
    }
    if (typeof dropTarget === 'undefined') {
        dropTarget = false;
    }
    if (typeof newName !== 'string' || typeof dropTarget !== 'boolean') {
        throw Error(
            'renameCollection must either take a string and an optional boolean or an object.');
    }
    return this._db._adminCommand({
        renameCollection: this._fullName,
        to: this._db._name + "." + newName,
        dropTarget: dropTarget
    });
};

// Display verbose information about the operation
DBCollection.prototype._printExtraInfo = function(action, startTime) {
    if (typeof _verboseShell === 'undefined' || !_verboseShell) {
        __callLastError = true;
        return;
    }

    // explicit w:1 so that replset getLastErrorDefaults aren't used here which would be bad.
    var res = this._db.getLastErrorCmd(1);
    if (res) {
        if (res.err != undefined && res.err != null) {
            // error occurred, display it
            print(res.err);
            return;
        }

        var info = action + " ";
        // hack for inserted because res.n is 0
        info += action != "Inserted" ? res.n : 1;
        if (res.n > 0 && res.updatedExisting != undefined)
            info += " " + (res.updatedExisting ? "existing" : "new");
        info += " record(s)";
        var time = new Date().getTime() - startTime;
        info += " in " + time + "ms";
        print(info);
    }
};

DBCollection.prototype.validate = function(options) {
    if (typeof (options) != 'object' && typeof (options) != 'undefined') {
        return "expected optional options to be of the following format: {full: <bool>, background: <bool>}.";
    }

    var cmd = {validate: this.getName()};
    Object.assign(cmd, options || {});

    var res = this._db.runCommand(cmd);

    if (typeof (res.valid) == 'undefined') {
        // old-style format just put everything in a string. Now using proper fields

        res.valid = false;

        var raw = res.result || res.raw;

        if (raw) {
            var str = "-" + tojson(raw);
            res.valid = !(str.match(/exception/) || str.match(/corrupt/));

            var p = /lastExtentSize:(\d+)/;
            var r = p.exec(str);
            if (r) {
                res.lastExtentSize = Number(r[1]);
            }
        }
    }

    return res;
};

DBCollection.prototype.getShardVersion = function() {
    return this._db._adminCommand({getShardVersion: this._fullName});
};

DBCollection.prototype.getIndexes = function(filter) {
    var res = this.runCommand("listIndexes", filter);

    if (!res.ok) {
        if (res.code == ErrorCodes.NamespaceNotFound) {
            // For compatibility, return []
            return [];
        }

        throw _getErrorWithCode(res, "listIndexes failed: " + tojson(res));
    }

    return new DBCommandCursor(this._db, res).toArray();
};

DBCollection.prototype.getIndices = DBCollection.prototype.getIndexes;
DBCollection.prototype.getIndexSpecs = DBCollection.prototype.getIndexes;

DBCollection.prototype.getIndexKeys = function() {
    return this.getIndexes().map(function(i) {
        return i.key;
    });
};

DBCollection.prototype.hashAllDocs = function() {
    var cmd = {dbhash: 1, collections: [this._shortName]};
    var res = this._dbCommand(cmd);
    var hash = res.collections[this._shortName];
    assert(hash);
    assert(typeof (hash) == "string");
    return hash;
};

/**
 * Drop a specified index.
 *
 * "index" is the name or key pattern of the index. For example:
 *    db.collectionName.dropIndex( "myIndexName" );
 *    db.collectionName.dropIndex( { "indexKey" : 1 } );
 *
 * @param {String} name or key object of index to delete.
 * @return A result object.  result.ok will be true if successful.
 */
DBCollection.prototype.dropIndex = function(index) {
    assert(index, "need to specify index to dropIndex");

    // Need an extra check for array because 'Array' is an 'object', but not every 'object' is an
    // 'Array'.
    if (typeof index != "string" && typeof index != "object" || index instanceof Array) {
        throw new Error(
            "The index to drop must be either the index name or the index specification document");
    }

    if (typeof index == "string" && index === "*") {
        throw new Error(
            "To drop indexes in the collection using '*', use db.collection.dropIndexes()");
    }

    var res = this._dbCommand("dropIndexes", {index: index});
    return res;
};

/**
 * Hide an index from the query planner.
 */
DBCollection.prototype._hiddenIndex = function(index, hidden) {
    assert(index, "please specify index to hide");

    // Need an extra check for array because 'Array' is an 'object', but not every 'object' is an
    // 'Array'.
    var indexField = {};
    if (typeof index == "string") {
        indexField = {name: index, hidden: hidden};
    } else if (typeof index == "object") {
        indexField = {keyPattern: index, hidden: hidden};
    } else {
        throw new Error("Index must be either the index name or the index specification document");
    }
    var cmd = {"collMod": this._shortName, index: indexField};
    var res = this._db.runCommand(cmd);
    return res;
};

DBCollection.prototype.hideIndex = function(index) {
    return this._hiddenIndex(index, true);
};

DBCollection.prototype.unhideIndex = function(index) {
    return this._hiddenIndex(index, false);
};

DBCollection.prototype.getCollection = function(subName) {
    return this._db.getCollection(this._shortName + "." + subName);
};

/**
 * scale: The scale at which to deliver results. Unless specified, this command returns all data
 *        in bytes.
 * indexDetails: Includes indexDetails field in results. Default: false.
 * indexDetailsKey: If indexDetails is true, filter contents in indexDetails by this index key.
 * indexDetailsname: If indexDetails is true, filter contents in indexDetails by this index name.
 *
 * It is an error to provide both indexDetailsKey and indexDetailsName.
 */
DBCollection.prototype.stats = function(args) {
    'use strict';

    // For backwards compatibility with db.collection.stats(scale).
    var scale = isObject(args) ? args.scale : args;

    var options = isObject(args) ? args : {};
    if (options.indexDetailsKey && options.indexDetailsName) {
        throw new Error('Cannot filter indexDetails on both indexDetailsKey and ' +
                        'indexDetailsName');
    }
    // collStats can run on a secondary, so we need to apply readPreference
    var res = this._db.runReadCommand({collStats: this._shortName, scale: scale});
    if (!res.ok) {
        return res;
    }

    var getIndexName = function(collection, indexKey) {
        if (!isObject(indexKey))
            return undefined;
        var indexName;
        collection.getIndexes().forEach(function(spec) {
            if (friendlyEqual(spec.key, options.indexDetailsKey)) {
                indexName = spec.name;
            }
        });
        return indexName;
    };

    var filterIndexName = options.indexDetailsName || getIndexName(this, options.indexDetailsKey);

    var updateStats = function(stats, keepIndexDetails, indexName) {
        if (!stats.indexDetails)
            return;
        if (!keepIndexDetails) {
            delete stats.indexDetails;
            return;
        }
        if (!indexName)
            return;
        for (var key in stats.indexDetails) {
            if (key == indexName)
                continue;
            delete stats.indexDetails[key];
        }
    };

    updateStats(res, options.indexDetails, filterIndexName);

    if (res.sharded) {
        for (var shardName in res.shards) {
            updateStats(res.shards[shardName], options.indexDetails, filterIndexName);
        }
    }

    return res;
};

DBCollection.prototype.dataSize = function() {
    return this.stats().size;
};

DBCollection.prototype.storageSize = function() {
    return this.stats().storageSize;
};

DBCollection.prototype.totalIndexSize = function(verbose) {
    var stats = this.stats();
    if (verbose) {
        for (var ns in stats.indexSizes) {
            print(ns + "\t" + stats.indexSizes[ns]);
        }
    }
    return stats.totalIndexSize;
};

DBCollection.prototype.totalSize = function() {
    var total = this.storageSize();
    var totalIndexSize = this.totalIndexSize();
    if (totalIndexSize) {
        total += totalIndexSize;
    }
    return total;
};

DBCollection.prototype.convertToCapped = function(bytes) {
    if (!bytes)
        throw Error("have to specify # of bytes");
    return this._dbCommand({convertToCapped: this._shortName, size: bytes});
};

DBCollection.prototype.exists = function() {
    var res = this._db.runCommand("listCollections", {filter: {name: this._shortName}});
    if (res.ok) {
        const cursor = new DBCommandCursor(this._db, res);
        if (!cursor.hasNext())
            return null;
        return cursor.next();
    }

    throw _getErrorWithCode(res, "listCollections failed: " + tojson(res));
};

DBCollection.prototype.isCapped = function() {
    var e = this.exists();
    return (e && e.options && e.options.capped) ? true : false;
};

//
// CRUD specification aggregation cursor extension
//
DBCollection.prototype.aggregate = function(pipeline, aggregateOptions) {
    if (!(pipeline instanceof Array)) {
        // Support legacy varargs form. Also handles db.foo.aggregate().
        pipeline = Array.from(arguments);
        aggregateOptions = {};
    } else if (aggregateOptions === undefined) {
        aggregateOptions = {};
    }

    const cmdObj = this._makeCommand("aggregate", {pipeline: pipeline});

    return this._db._runAggregate(cmdObj, aggregateOptions);
};

DBCollection.prototype.convertToSingleObject = function(valueField) {
    var z = {};
    this.find().forEach(function(a) {
        z[a._id] = a[valueField];
    });
    return z;
};

/**
 * @param optional object of optional fields;
 */
DBCollection.prototype.mapReduce = function(map, reduce, optionsOrOutString) {
    var c = {mapreduce: this._shortName, map: map, reduce: reduce};
    assert(optionsOrOutString, "need to supply an optionsOrOutString");

    if (typeof (optionsOrOutString) == "string")
        c["out"] = optionsOrOutString;
    else
        Object.extend(c, optionsOrOutString);

    var output;

    if (c["out"].hasOwnProperty("inline") && c["out"]["inline"] === 1) {
        // if inline output is specified, we need to apply readPreference on the command
        // as it could be run on a secondary
        output = this._db.runReadCommand(c);
    } else {
        output = this._db.runCommand(c);
    }

    if (!output.ok) {
        __mrerror__ = output;
        throw _getErrorWithCode(output, "map reduce failed:" + tojson(output));
    }
    return output;
};

DBCollection.prototype.toString = function() {
    return this.getFullName();
};

DBCollection.prototype.tojson = DBCollection.prototype.toString;

DBCollection.prototype.shellPrint = DBCollection.prototype.toString;

DBCollection.autocomplete = function(obj) {
    var colls = DB.autocomplete(obj.getDB());
    var ret = [];
    for (var i = 0; i < colls.length; i++) {
        var c = colls[i];
        if (c.length <= obj.getName().length)
            continue;
        if (c.slice(0, obj.getName().length + 1) != obj.getName() + '.')
            continue;

        ret.push(c.slice(obj.getName().length + 1));
    }
    return ret;
};

/**
 * Return true if the collection has been sharded.
 *
 * @method
 * @return {boolean}
 */
DBCollection.prototype._isSharded = function() {
    return !!this._db.getSiblingDB("config").collections.countDocuments(
        {_id: this._fullName, dropped: false});
};

/**
 * Prints statistics related to the collection's data distribution
 */
DBCollection.prototype.getShardDistribution = function() {
    var config = this.getDB().getSiblingDB("config");

    if (!this._isSharded()) {
        print("Collection " + this + " is not sharded.");
        return;
    }

    var collStats = this.aggregate({"$collStats": {storageStats: {}}});

    var totals = {numChunks: 0, size: 0, count: 0};
    var conciseShardsStats = [];

    collStats.forEach(function(extShardStats) {
        // Extract and store only the relevant subset of the stats for this shard
        const shardStats = {
            shardId: extShardStats.shard,
            host: config.shards.findOne({_id: extShardStats.shard}).host,
            size: extShardStats.storageStats.size,
            count: extShardStats.storageStats.count,
            numChunks:
                config.chunks.countDocuments({ns: extShardStats.ns, shard: extShardStats.shard}),
            avgObjSize: extShardStats.storageStats.avgObjSize
        };

        print("\nShard " + shardStats.shardId + " at " + shardStats.host);

        var estChunkData =
            (shardStats.numChunks == 0) ? 0 : (shardStats.size / shardStats.numChunks);
        var estChunkCount =
            (shardStats.numChunks == 0) ? 0 : Math.floor(shardStats.count / shardStats.numChunks);
        print(" data : " + sh._dataFormat(shardStats.size) + " docs : " + shardStats.count +
              " chunks : " + shardStats.numChunks);
        print(" estimated data per chunk : " + sh._dataFormat(estChunkData));
        print(" estimated docs per chunk : " + estChunkCount);

        totals.size += shardStats.size;
        totals.count += shardStats.count;
        totals.numChunks += shardStats.numChunks;

        conciseShardsStats.push(shardStats);
    });

    print("\nTotals");
    print(" data : " + sh._dataFormat(totals.size) + " docs : " + totals.count +
          " chunks : " + totals.numChunks);
    for (const shardStats of conciseShardsStats) {
        var estDataPercent =
            (totals.size == 0) ? 0 : (Math.floor(shardStats.size / totals.size * 10000) / 100);
        var estDocPercent =
            (totals.count == 0) ? 0 : (Math.floor(shardStats.count / totals.count * 10000) / 100);

        print(" Shard " + shardStats.shardId + " contains " + estDataPercent + "% data, " +
              estDocPercent + "% docs in cluster, " +
              "avg obj size on shard : " + sh._dataFormat(shardStats.avgObjSize));
    }

    print("\n");
};

/*

Generates split points for all chunks in the collection, based on the default maxChunkSize
> collection.getSplitKeysForChunks()

or alternately a specified chunk size in Mb.
> collection.getSplitKeysForChunks( 10 )

By default, the chunks are not split, the keys are just found. A splitter function is returned which
will actually do the splits.
> var splitter = collection.getSplitKeysForChunks()
> splitter()

*/

DBCollection.prototype.getSplitKeysForChunks = function(chunkSize) {
    var stats = this.stats();

    if (!stats.sharded) {
        print("Collection " + this + " is not sharded.");
        return;
    }

    var config = this.getDB().getSiblingDB("config");

    if (!chunkSize) {
        chunkSize = config.settings.findOne({_id: "chunksize"}).value;
        print("Chunk size not set, using default of " + chunkSize + "MB");
    } else {
        print("Using chunk size of " + chunkSize + "MB");
    }

    var shardDocs = config.shards.find().toArray();

    var allSplitPoints = {};
    var numSplits = 0;

    for (var i = 0; i < shardDocs.length; i++) {
        var shardDoc = shardDocs[i];
        var shard = shardDoc._id;
        var host = shardDoc.host;
        var sconn = new Mongo(host);

        var chunks = config.chunks.find({_id: sh._collRE(this), shard: shard}).toArray();

        print("\nGetting split points for chunks on shard " + shard + " at " + host);

        var splitPoints = [];

        for (var j = 0; j < chunks.length; j++) {
            var chunk = chunks[j];
            var result = sconn.getDB("admin").runCommand(
                {splitVector: this + "", min: chunk.min, max: chunk.max, maxChunkSize: chunkSize});
            if (!result.ok) {
                print(" Had trouble getting split keys for chunk " + sh._pchunk(chunk) + " :\n");
                printjson(result);
            } else {
                splitPoints = splitPoints.concat(result.splitKeys);

                if (result.splitKeys.length > 0)
                    print(" Added " + result.splitKeys.length + " split points for chunk " +
                          sh._pchunk(chunk));
            }
        }

        print("Total splits for shard " + shard + " : " + splitPoints.length);

        numSplits += splitPoints.length;
        allSplitPoints[shard] = splitPoints;
    }

    // Get most recent migration
    var migration = config.changelog.find({what: /^move.*/}).sort({time: -1}).limit(1).toArray();
    if (migration.length == 0)
        print("\nNo migrations found in changelog.");
    else {
        migration = migration[0];
        print("\nMost recent migration activity was on " + migration.ns + " at " + migration.time);
    }

    var admin = this.getDB().getSiblingDB("admin");
    var coll = this;
    var splitFunction = function() {
        // Turn off the balancer, just to be safe
        print("Turning off balancer...");
        config.settings.update({_id: "balancer"}, {$set: {stopped: true}}, true);
        print(
            "Sleeping for 30s to allow balancers to detect change.  To be extra safe, check config.changelog" +
            " for recent migrations.");
        sleep(30000);

        for (var shard in allSplitPoints) {
            for (var i = 0; i < allSplitPoints[shard].length; i++) {
                var splitKey = allSplitPoints[shard][i];
                print("Splitting at " + tojson(splitKey));
                printjson(admin.runCommand({split: coll + "", middle: splitKey}));
            }
        }

        print("Turning the balancer back on.");
        config.settings.update({_id: "balancer"}, {$set: {stopped: false}});
        sleep(1);
    };

    splitFunction.getSplitPoints = function() {
        return allSplitPoints;
    };

    print("\nGenerated " + numSplits + " split keys, run output function to perform splits.\n" +
          " ex : \n" +
          "  > var splitter = <collection>.getSplitKeysForChunks()\n" +
          "  > splitter() // Execute splits on cluster !\n");

    return splitFunction;
};

DBCollection.prototype.setSlaveOk = function(value) {
    if (value == undefined)
        value = true;
    this._slaveOk = value;
};

DBCollection.prototype.getSlaveOk = function() {
    if (this._slaveOk != undefined)
        return this._slaveOk;
    return this._db.getSlaveOk();
};

DBCollection.prototype.getQueryOptions = function() {
    // inherit this method from DB but use apply so
    // that slaveOk will be set if is overridden on this DBCollection
    return this._db.getQueryOptions.apply(this, arguments);
};

/**
 * Returns a PlanCache for the collection.
 */
DBCollection.prototype.getPlanCache = function() {
    return new PlanCache(this);
};

// Overrides connection-level settings.
//

DBCollection.prototype.setWriteConcern = function(wc) {
    if (wc instanceof WriteConcern) {
        this._writeConcern = wc;
    } else {
        this._writeConcern = new WriteConcern(wc);
    }
};

DBCollection.prototype.getWriteConcern = function() {

    if (this._writeConcern)
        return this._writeConcern;

    if (this._db.getWriteConcern())
        return this._db.getWriteConcern();

    return null;
};

DBCollection.prototype.unsetWriteConcern = function() {
    delete this._writeConcern;
};

//
// CRUD specification read methods
//

/**
 * Count number of matching documents in the db to a query.
 *
 * @method
 * @param {object} query The query for the count.
 * @param {object} [options=null] Optional settings.
 * @param {number} [options.limit=null] The limit of documents to count.
 * @param {number} [options.skip=null] The number of documents to skip for the count.
 * @param {string|object} [options.hint=null] An index name hint or specification for the query.
 * @param {number} [options.maxTimeMS=null] The maximum amount of time to allow the query to run.
 * @param {string} [options.readConcern=null] The level of readConcern passed to the count command
 * @param {object} [options.collation=null] The collation that should be used for string comparisons
 * for this count op.
 * @return {number}
 *
 */
DBCollection.prototype.count = function(query, options) {
    const cmd =
        Object.assign({count: this.getName(), query: this._massageObject(query || {})}, options);
    if (cmd.readConcern) {
        cmd.readConcern = {level: cmd.readConcern};
    }
    const res = this._db.runReadCommand(cmd);
    if (!res.ok) {
        throw _getErrorWithCode(res, "count failed: " + tojson(res));
    }
    return res.n;
};

/**
 * Count number of matching documents in the db to a query using aggregation.
 *
 * @method
 * @param {object} query The query for the count.
 * @param {object} [options=null] Optional settings.
 * @param {number} [options.limit=null] The limit of documents to count.
 * @param {number} [options.skip=null] The number of documents to skip for the count.
 * @param {string|object} [options.hint=null] An index name hint or specification for the query.
 * @param {number} [options.maxTimeMS=null] The maximum amount of time to allow the query to run.
 * @param {object} [options.collation=null] The collation that should be used for string comparisons
 * for this count op.
 * @return {number}
 */
DBCollection.prototype.countDocuments = function(query, options) {
    "use strict";
    let pipeline = [{"$match": query}];
    options = options || {};
    assert.eq(typeof options, "object", "'options' argument must be an object");

    if (options.skip) {
        pipeline.push({"$skip": options.skip});
    }
    if (options.limit) {
        pipeline.push({"$limit": options.limit});
    }

    // Construct an aggregation pipeline stage with sum to calculate the number of all documents.
    pipeline.push({"$group": {"_id": null, "n": {"$sum": 1}}});

    // countDocument options other than filter, skip, and limit, are added to the aggregate command.
    let aggregateOptions = {};

    if (options.hint) {
        aggregateOptions.hint = options.hint;
    }
    if (options.maxTimeMS) {
        aggregateOptions.maxTimeMS = options.maxTimeMS;
    }
    if (options.collation) {
        aggregateOptions.collation = options.collation;
    }

    // Format cursor into an array.
    const res = this.aggregate(pipeline, aggregateOptions).toArray();
    if (res.length) {
        return res[0].n;
    }

    return 0;
};

/**
 * Estimates the count of documents in a collection using collection metadata.
 *
 * @method
 * @param {object} [options=null] Optional settings.
 * @param {number} [options.maxTimeMS=null] The maximum amount of time to allow the query to run.
 * @return {number}
 */
DBCollection.prototype.estimatedDocumentCount = function(options) {
    "use strict";
    let cmd = {count: this.getName()};
    options = options || {};
    assert.eq(typeof options, "object", "'options' argument must be an object");

    if (options.maxTimeMS) {
        cmd.maxTimeMS = options.maxTimeMS;
    }

    const res = this.runCommand(cmd);

    if (!res.ok) {
        throw _getErrorWithCode(res, "Error estimating document count: " + tojson(res));
    }

    // Return the 'n' field, which should be the count of documents.
    return res.n;
};

/**
 * The distinct command returns returns a list of distinct values for the given key across a
 *collection.
 *
 * @method
 * @param {string} key Field of the document to find distinct values for.
 * @param {object} query The query for filtering the set of documents to which we apply the distinct
 *filter.
 * @param {object} [options=null] Optional settings.
 * @param {number} [options.maxTimeMS=null] The maximum amount of time to allow the query to run.
 * @return {object}
 */
DBCollection.prototype.distinct = function(keyString, query, options) {
    var opts = Object.extend({}, options || {});
    var keyStringType = typeof keyString;
    var queryType = typeof query;

    if (keyStringType != "string") {
        throw new Error("The first argument to the distinct command must be a string but was a " +
                        keyStringType);
    }

    if (query != null && queryType != "object") {
        throw new Error("The query argument to the distinct command must be a document but was a " +
                        queryType);
    }

    // Distinct command
    var cmd = {distinct: this.getName(), key: keyString, query: query || {}};

    // Set maxTimeMS if provided
    if (opts.maxTimeMS) {
        cmd.maxTimeMS = opts.maxTimeMS;
    }

    if (opts.collation) {
        cmd.collation = opts.collation;
    }

    // Execute distinct command
    var res = this.runReadCommand(cmd);
    if (!res.ok) {
        throw _getErrorWithCode(res, "distinct failed: " + tojson(res));
    }

    return res.values;
};

DBCollection.prototype._distinct = function(keyString, query) {
    return this._dbReadCommand({distinct: this._shortName, key: keyString, query: query || {}});
};

DBCollection.prototype.latencyStats = function(options) {
    options = options || {};
    return this.aggregate([{$collStats: {latencyStats: options}}]);
};

DBCollection.prototype.watch = function(pipeline, options) {
    pipeline = pipeline || [];
    assert(pipeline instanceof Array, "'pipeline' argument must be an array");
    let changeStreamStage;
    [changeStreamStage, aggOptions] = this.getMongo()._extractChangeStreamOptions(options);
    pipeline.unshift(changeStreamStage);
    return this.aggregate(pipeline, aggOptions);
};

/**
 * PlanCache
 * Holds a reference to the collection.
 * Proxy for planCache* commands.
 */
if ((typeof PlanCache) == "undefined") {
    PlanCache = function(collection) {
        this._collection = collection;
    };
}

/**
 * Name of PlanCache.
 * Same as collection.
 */
PlanCache.prototype.getName = function() {
    return this._collection.getName();
};

/**
 * toString prints the name of the collection
 */
PlanCache.prototype.toString = function() {
    return "PlanCache for collection " + this.getName() + '. Type help() for more info.';
};

PlanCache.prototype.shellPrint = PlanCache.prototype.toString;

/**
 * Displays help for a PlanCache object.
 */
PlanCache.prototype.help = function() {
    var shortName = this.getName();
    print("PlanCache help");
    print("\tdb." + shortName + ".getPlanCache().help() - show PlanCache help");
    print("\tdb." + shortName + ".getPlanCache().clear() - " +
          "drops all cached queries in a collection");
    print("\tdb." + shortName +
          ".getPlanCache().clearPlansByQuery(query[, projection, sort, collation]) - " +
          "drops query shape from plan cache");
    print("\tdb." + shortName + ".getPlanCache().list([pipeline]) - " +
          "displays a serialization of the plan cache for this collection, " +
          "after applying an optional aggregation pipeline");
    return __magicNoPrint;
};

/**
 * Internal function to parse query shape.
 */
PlanCache.prototype._parseQueryShape = function(query, projection, sort, collation) {
    if (query == undefined) {
        throw new Error("required parameter query missing");
    }

    // Accept query shape object as only argument.
    // Query shape must contain 'query', 'projection', and 'sort', and may optionally contain
    // 'collation'. 'collation' must be non-empty if present.
    if (typeof (query) == 'object' && projection == undefined && sort == undefined &&
        collation == undefined) {
        var keysSorted = Object.keys(query).sort();
        // Expected keys must be sorted for the comparison to work.
        if (bsonWoCompare(keysSorted, ['projection', 'query', 'sort']) == 0) {
            return query;
        }
        if (bsonWoCompare(keysSorted, ['collation', 'projection', 'query', 'sort']) == 0) {
            if (Object.keys(query.collation).length === 0) {
                throw new Error("collation object must not be empty");
            }
            return query;
        }
    }

    // Extract query shape, projection, sort and collation from DBQuery if it is the first
    // argument. If a sort or projection is provided in addition to DBQuery, do not
    // overwrite with the DBQuery value.
    if (query instanceof DBQuery) {
        if (projection != undefined) {
            throw new Error("cannot pass DBQuery with projection");
        }
        if (sort != undefined) {
            throw new Error("cannot pass DBQuery with sort");
        }
        if (collation != undefined) {
            throw new Error("cannot pass DBQuery with collation");
        }

        var queryObj = query._query["query"] || {};
        projection = query._fields || {};
        sort = query._query["orderby"] || {};
        collation = query._query["collation"] || undefined;
        // Overwrite DBQuery with the BSON query.
        query = queryObj;
    }

    var shape = {
        query: query,
        projection: projection == undefined ? {} : projection,
        sort: sort == undefined ? {} : sort,
    };

    if (collation !== undefined) {
        shape.collation = collation;
    }

    return shape;
};

/**
 * Internal function to run command.
 */
PlanCache.prototype._runCommandThrowOnError = function(cmd, params) {
    var res = this._collection.runCommand(cmd, params);
    if (!res.ok) {
        throw new Error(res.errmsg);
    }
    return res;
};

/**
 * Clears plan cache in a collection.
 */
PlanCache.prototype.clear = function() {
    this._runCommandThrowOnError("planCacheClear", {});
    return;
};

/**
 * Drop query shape from the plan cache.
 */
PlanCache.prototype.clearPlansByQuery = function(query, projection, sort, collation) {
    this._runCommandThrowOnError("planCacheClear",
                                 this._parseQueryShape(query, projection, sort, collation));
    return;
};

/**
 * Returns an array of plan cache data for the collection, after applying the given optional
 * aggregation pipeline.
 */
PlanCache.prototype.list = function(pipeline) {
    const additionalPipeline = pipeline || [];
    const completePipeline = [{$planCacheStats: {}}].concat(additionalPipeline);
    return this._collection.aggregate(completePipeline).toArray();
};
