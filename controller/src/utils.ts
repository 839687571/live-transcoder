
import { getLogger } from 'log4js';
const modLogger = getLogger("utils");
import { randomBytes } from "crypto";
import * as request from "request-promise-native";

import {Logger} from "./logger";
import {TrackType} from "./interfaces/channelOutput";

export class CallContext {
    logger: Logger;
    readonly  id:string;
    readonly name:string;

    constructor(name) {

        this.name=name;
        this.id = randomBytes(6).toString("hex");
        this.logger=new Logger("",`[${this.id}]${name}`);
    }
}

export function getTrackName(inputId:string,type:TrackType):string {
    return (type==TrackType.Video ? "v" : "a") + inputId;
}

export  function  timePassed(from:Date,to:Date=null):number {
    if (!to) {
        to=new Date();
    }
    return to.getTime() - from.getTime();
}
export  function  hasExpired(from:Date,timeout:number=0,expiry:Date=null):boolean {
    return timePassed(from,expiry)>timeout;
}


export function addSeconds(date:Date,seconds:number):Date {
    return new Date(date.getTime() + seconds*1000);

}

export  class Timer {
    readonly  fn:Function;
    private timeout:any;
    private stopped:boolean=false;

    constructor(fn:Function) {
        this.fn=fn;
    }

    async start(immediate:boolean, interval:number) {
        let t0:Date=new Date();

        if (immediate) {
            try {
                await this.fn();
            }catch (e) {
                console.warn(e);
            }
        }
        if (!this.stopped) {
            this.timeout=setTimeout(() => {
                this.start(true, interval);
            },Math.max(0,interval-timePassed(t0)));
        }
    }

    stop() {
        this.stopped=true;
        if (this.timeout) {
            clearTimeout(this.timeout);
            this.timeout=null;
        }
    }
}


export function sleep(timeout:number) {
    return new Promise ( (s,r)=> {
        setTimeout(s,timeout)
    })
}


export async function waitFor(interval:number,tries:number,condition:Function):Promise<boolean> {
    while(tries>=0) {
        tries--;
        if (await condition()) {
            return true;
        }
        await sleep(interval);
    }
}

export async function retry(fn:Function,retries:number,interval:number=0,wakeTime:number=0,logger:any=null): Promise<any> {
    let lastError=null;
    if (logger==null)
        logger=modLogger;

    if (wakeTime>0) {
        await sleep(wakeTime);
    }

    for (let i=1; ;i++) {
        let res:any;
        try {
            res = await fn();
            if (res) {
                return res
            }
        }
        catch(err) {
            logger.warn("Exception in retry #",i,"out of",retries,":", err);
            lastError=err;
        }
        if (i==retries) {
            throw lastError;
        }
        if (interval!=0) {
            await sleep(interval);
        }
    }
}


export class Network
{
    static mockedAnswers:Map<RegExp,any>;

    static addMock(url:string,value:any) {
        if (!Network.mockedAnswers) {
            Network.mockedAnswers = new Map<RegExp,any>();
        }
        this.mockedAnswers.set(new RegExp(url+"$"),value);
    }

    static findMock(method:string,url:string,body:any=null): any {
        for (let [pattern,answer] of Network.mockedAnswers.entries()) {
            let m=pattern.exec(url);
            if (m) {
                if (answer instanceof Function) {
                    return answer(method,m,body);
                }
                return answer;
            }
        }
        return null;
    }

    static get(obj): Promise<any> {
        modLogger.debug("curl -i  %s",obj.url);
        if (Network.mockedAnswers) {
            let response=this.findMock("GET",obj.url);
            if (response) {
                return Promise.resolve(response);
            }
        }
        return request.get(obj);
    }
    static post(obj): Promise<any>  {
        modLogger.debug("curl -i -d '%j' -H \"Content-Type: application/json\" -X POST  %s",obj.json,obj.url);
        if (Network.mockedAnswers) {
            let response=this.findMock("POST",obj.url,obj.json);
            if (response) {
                return Promise.resolve(response);
            }
        }
        return request.post(obj);
    }
}



export class SynchronizedPromises {
    private readonly  logger: Logger;

    private readonly defaultTimeOut:number = 20*1000;//~ 20 seconds

    private lastCallback:Promise<any> = Promise.resolve();

    constructor(logger:Logger) {
        this.logger=logger;
    }

    exec(fn:Function):Promise<any> {
        this.lastCallback = this.lastCallback.finally(()=> {
            return fn();
        });
        return  this.lastCallback;
    }
}
