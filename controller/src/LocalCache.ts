import * as LRU from "lru-cache";
import * as config from 'config';
import {retry, timePassed} from "./utils";
import {ICache} from "./interfaces/interfaces";
import {Logger} from "./logger";


export class LocalCache implements ICache {

    private readonly cache:LRU =  new LRU(500);
    private readonly locker:Map<string,Date>=new Map<string,Date>();
    private readonly logger:Logger=new Logger("LocalCache");

    private static lockTimeout=config.get("cache.lockTimeout")
    private static lockInterval=config.get("cache.lockInterval")

    get(key:string):Promise<any> {
        return Promise.resolve(this.cache.get(key));
    }
    set(key:string,value:any,timeout:Number):Promise<boolean>
    {
        return Promise.resolve(this.cache.set(key,value,timeout));
    }

    async lock(name:string,logger:Logger=null): Promise<boolean> {
        try {
            return await retry(async () => {
                let now: Date = new Date();
                let t = this.locker.get(name);
                if (t == null || timePassed(t,now) > LocalCache.lockTimeout) {
                    if (t != null) { //timeout
                        this.logger.warn("timeout when trying to lock %s %s+%d > %s", name, t,  LocalCache.lockTimeout,now);
                    }
                    this.locker.set(name, now);
                    return Promise.resolve(true);
                }
                return Promise.resolve(false);
            },  10000, LocalCache.lockInterval,0,logger);
        } catch(e) {
            //override
            return true;
        }
    }

    async unlock(name:string,logger:Logger=null): Promise<boolean> {
        this.locker.delete(name);
        return Promise.resolve(true);
    }
}