type HasConnection = {
    [k: string]: Mongo.Collection
}
type ActualDb = Mongo.DatabaseImpl & HasConnection;
declare namespace Mongo {
    export type Document = {
        [k: string]: any,
    }
    export interface Client {
        adminCommand(command: string | object);
    }
    export interface Session {
        getClient(): Client;
    }
    export interface DatabaseImpl {
        getSession(): Session;
        adminCommand(command: string);
        runCommand(command: string | object);
        serverStatus(): ServerStatus;
        dropDatabase();
    }
    export type Database = ActualDb;

    interface Bulk {
        insert(doc: object);
        execute();
    }

    export interface Collection {
        getDB(): Database;
        ensureIndex(index: object, options?: object);
        getName(): string;
        insert(document: object);
        count(filter?:object): number;
        findOne(filter?:object): Document;
        initializeUnorderedBulkOp(): Bulk;
    }
    export interface Connection {
        adminCommand(command: string | object);
        getDB(name: string): Database;
    }

    type StorageEngine = {
        supportsCommittedReads: boolean,
        name: string,
    };
    type ServerStatus = {
        storageEngine: StorageEngine,
    }
}

