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
    function commandWorked(command: any, message?: string);
    function writeOK(cmd: any);
    function neq(a: any, b: any, message?: string);
}

