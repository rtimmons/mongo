declare function load(path: string);
declare function jsTestLog(msg: string);
declare function tojson(x: any, indent?:number, nolint?:boolean, depth?:number);

// TODO: do we need more than just this file?
// comes from src/shell/replsettest.js
declare interface ReplSetTest {
    getSecondaries(): Mongo.Connection[];
    nodes: Mongo.Connection[];
}
