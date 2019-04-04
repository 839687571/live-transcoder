const Memcached = require('memcached');


export class CacheManager {

    memcacheClient: any
    static PREFIX: string = "XXX"
    static instance:CacheManager = new CacheManager()

    constructor() {
        this.memcacheClient = new Memcached("localhost:11211");
    }

    private wrap(funcName:string,...args): Promise<any> {

        return new Promise((resolve, reject)=>{

            args.push((err, val)=>{
                if (err) {
                    reject(err);
                    return err;
                }
                resolve(val);
            })
            this.memcacheClient[funcName].apply(this.memcacheClient,args);
        });

    }

    get(key:string): Promise<any> {
        if (!key)
            return Promise.reject('no key given');

        return this.wrap("get",CacheManager.PREFIX + key)
    }
    set(key:string,value:any,ttl:number): Promise<any> {
        return this.wrap("set",CacheManager.PREFIX + key,value, ttl)

    }
    add(key:string,value:any,ttl:number): Promise<any> {
        return this.wrap("add",CacheManager.PREFIX + key,value, ttl)

    }
    del(key:string): Promise<any> {
        return this.wrap("del",CacheManager.PREFIX + key)

    }

}