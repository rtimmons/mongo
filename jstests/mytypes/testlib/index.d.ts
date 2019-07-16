declare namespace testlib {
    interface Shard {
        shardName: string;
    }
    export interface ShardingTest {
        ensurePrimaryShard(db: string, shardName: string);
        shard0: Shard;
        // TODO
        shardColl(name: string, opts1: object, opts2: object, opts3: object, db: string, other: boolean);
        _rsObjects: ReplSetTest[];
        // TODO
        configRS: any;
    }
}
