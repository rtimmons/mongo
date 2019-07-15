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
        serverStatus(): ServerStatus;
    }

    export interface Connection {
        adminCommand(command: string | object);
    }

    type StorageEngine = {
        supportsCommittedReads: boolean,
        name: string,
    };
    type ServerStatus = {
        storageEngine: StorageEngine,
    }
}

