declare function load(path: string);
declare function jsTestLog(msg: string);
declare function tojson(x: any, indent?:number, nolint?:boolean, depth?:number);
declare function print(...args:any[]);

// TODO: do we need more than just this file?
// comes from src/shell/replsettest.js
declare interface ReplSetTest {
    getSecondaries(): Mongo.Connection[];
    nodes: Mongo.Connection[];
}

// src/shell/servers.js
declare class MongoRunner {
    static runMongod(opts: object);
    static stopMongod(conn: Mongo.Connection);
}

// src/shell/startParallelShell
declare function startParallelShell(jsCode: string, port?:number, noConnect?:boolean);
