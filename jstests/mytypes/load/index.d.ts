declare function load(path: string);
declare function jsTestLog(msg: string);
declare function tojson(x: any, indent?:number, nolint?:boolean, depth?:number);
declare function print(...args:any[]);
declare function arrayEq(a:any[], b:any[]): boolean;
declare function doassert(op: any, msg?:string);

declare var db: Mongo.Database;
declare var TestData: {
    storageEngine: string,
    skipGossipingClusterTime: boolean;
};

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
declare function ObjectId(val?:string): ObjectIdImpl;

// src/shell/utils.js
declare var jsTest: {
    name(): string;
    log(msg: string);
};
declare function jsTestName(): string;

interface Object {
    merge(a,b, other?:boolean): Object;
    extend(...args:any[]): Object;
}

// TODO: this doesn't seem possible
// declare interface Array<T> {
//     sum(...args:any[]): Number;
// }

interface UUIDImpl {}
declare function UUID(value:string): UUIDImpl;

declare class BinData {
    constructor(val: number, name: string);
}

declare class TimestampImpl {}
declare function Timestamp(val?:number): TimestampImpl;
