declare function load(path: string);
declare function jsTestLog(msg: string);
declare function tojson(x: any, indent?:number, nolint?:boolean, depth?:number);
declare function print(...args:any[]);
declare function arrayEq(a:any[], b:any[]): boolean;
declare function doassert(op: any, msg?:string);

declare var db: Mongo.Database;

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

declare class ObjectIdImpl {
}
declare function ObjectId(): ObjectIdImpl;

// src/shell/utils.js
declare var jsTest: {
    name(): string;
};
declare function jsTestName(): string;

interface Object {
    merge(a,b): Object;
}
