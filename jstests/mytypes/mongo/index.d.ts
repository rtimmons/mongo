declare namespace Mongo {
    export interface Client {
        adminCommand(command: string | object);
    }
    export interface Session {
        getClient(): Client;
    }
    export interface Database {
        getSession(): Session;
        adminCommand(command: string);
        runCommand(command: string | object);
        serverStatus(): ServerStatus;
    }

    export interface Collection {
        getDB(): Database;
        ensureIndex(index: object, options?: object);
        getName(): string;
        insert(document: object);
        count(): number;
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

