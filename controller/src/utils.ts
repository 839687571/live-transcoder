
import { getLogger } from 'log4js';
const logger = getLogger("utils");

import * as request from "request-promise-native";

import  { CacheManager } from './cache'

export  class Locker
{
    static async lock(name:string): Promise<boolean> {
        logger.debug("locking ",name);
        return CacheManager.instance.add(name+"__LOCKER__","1",60)
    }
    static async unlock(name:string): Promise<boolean> {
        logger.debug("unlock ",name);
        return CacheManager.instance.del(name+"__LOCKER__")
    }
}



export function sleep(timeout) {
    return new Promise ( (s,r)=> {
        setTimeout(s,timeout)
    })
}

export async function retry(fn ,retries:number,interval:number=0): Promise<any> {
    for (let i=1; ;i++) {
        let res:any
        try {
            res = await fn()
            if (res) {
                return res
            }
        }
        catch(err) {
            logger.warn(err)
        }
        if (i==retries) {
            throw new Error("timeout");
        }
        if (interval!=0) {
            await sleep(interval)
        }
    }
}


export class Network
{
    static mockedAnswers:Map<string,any>;

    static addMock(url:string,value:any) {
        if (!Network.mockedAnswers) {
            Network.mockedAnswers = new Map<string,any>();
        }
        this.mockedAnswers.set(url,value);
    }

    static get(obj): Promise<any> {
        if (Network.mockedAnswers) {
            let response=Network.mockedAnswers.get(obj.url);
            if (response) {
                return Promise.resolve(response);
            }
        }
        return request.get(obj);
    }
    static post(obj): Promise<any>  {
        if (Network.mockedAnswers) {
            let response=Network.mockedAnswers.get(obj.url);
            if (response) {
                return Promise.resolve(response);
            }
        }
        return request.post(obj);
    }
}