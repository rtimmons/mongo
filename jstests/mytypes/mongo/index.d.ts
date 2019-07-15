declare namespace Mongo {
    export interface Client {
        adminCommand(command: string);
    }
    export interface Session {
        getClient(): Client;
    }
    export interface Database {
        getSession(): Session;
        adminCommand(command: string);
        serverStatus(): ServerStatus;
    }

    type StorageEngine = {
        supportsCommittedReads: boolean,
        name: string,
    };
    type ServerStatus = {
        storageEngine: StorageEngine,
    }
}

