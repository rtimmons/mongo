// /*
// works with
//
//
// in server_types.js
//
//     declare module assert {
//         function commandWorked();
//     }
// */
//
export declare module assert {
    function commandFailedWithCode(command: any, errCode:number);
    function commandWorked(command: any, message?: string);
    function writeOK(cmd: any);
    function neq(a: any, b: any, message?: string);
    function soon(f: () => {}, message, ts: number, n: number);
}
