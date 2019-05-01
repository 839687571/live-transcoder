import {Logger} from "../logger";

const moduleLogger = require("log4js").getLogger("KalturaAPI");
import * as http from 'http'
import * as https from 'https'
import {createCipheriv, createHash, randomBytes} from "crypto";
import {stringify} from "querystring";
import {Network, timePassed} from "../utils";


http.globalAgent.maxSockets = 10000;

export class KalturaAPIConfig {
    url: string;
    secret: string;
    partnerId: number;
    ksDuration: number = 86000;
    ksType: number;
    userId: string;
    privileges: string = "";
    timeout: number = 20000;
}

function hash(buf) {
    let sha1 = createHash('sha1');
    sha1.update(buf);
    return sha1.digest();
}

export class KalturaAPIException
{
    code:string;
    message:string;
    args:any;
}

export class KalturaAPI {
    private _ks_expiry: Date;
    private _ks: string;
    private readonly _config: KalturaAPIConfig;
    private readonly _httpAgent: any;

    constructor(config:KalturaAPIConfig) {
        this._ks_expiry = new Date(0);
        this._ks=null;
        this._config = config;
        let lib=this._config.url.startsWith("https:") ?  https : http;
        this._httpAgent = new lib.Agent({ keepAlive: true});

    }

    private _generateKS() {
        let expiry = null;


        // build fields array
        let fields:any = {};
        fields._e = expiry ? expiry : Math.round(new Date().getTime()/1000) +  this._config.ksDuration;
        fields._t = this._config.ksType;
        fields._u = this._config.userId;
        this._config.privileges.split(',').forEach( privilege => {
            privilege = privilege.trim();
            if (privilege.length === 0) {
                return;
            }
            if (privilege === '*') {
                privilege = 'all:*';
            }
            let splittedPrivilege = privilege.split(':');
            if (splittedPrivilege.length > 1) {
                fields[splittedPrivilege[0]] = splittedPrivilege[1];
            }
            else {
                fields[splittedPrivilege[0]] = '';
            }
        });


        let fieldsStr = stringify(fields);

        let fieldsBuf = Buffer.from(fieldsStr);

        let rnd = Buffer.from(randomBytes(16));

        fieldsBuf = Buffer.concat([rnd,fieldsBuf]);

        let sha1Buf = hash (fieldsBuf);

        let message = Buffer.concat([sha1Buf, fieldsBuf]);

        if(message.length % 16) {
            let padding =  Buffer.alloc(16 - message.length % 16,0,'binary');
            message = Buffer.concat( [message, padding]);
        }

        let iv =  Buffer.alloc(16,0,'binary');
        let key = hash(this._config.secret).slice(0, 16);
        let cipher = createCipheriv("aes-128-cbc", key, iv);

        cipher.setAutoPadding(false);

        let ciphertext = cipher.update(message);

        let header = 'v2|'+ this._config.partnerId +'|';
        let $decodedKs = Buffer.concat([Buffer.from(header), Buffer.from(ciphertext)]).toString('base64');

        this._ks = $decodedKs.split('+').join('-').split('/').join('_');

        //console.warn("Generated KS ",this._ks)
        this._ks_expiry =   new Date(1000*(fields._e - 2*60*60));// new KS every 2 hour...
    }

    async call(requests,logger=null) {
        let now = new Date();
        if (now > this._ks_expiry) {
            this._generateKS()
        }
        return this.internalAPICall(requests,logger);
    }

    private static convertException(res) {

        let e=new KalturaAPIException();
        e.code=res.code;
        e.message=res.message;
        e.args=res.args;
        return e;
    }

    private async internalAPICall(requests:any,logger:Logger):Promise<any> {
        //we chain the requests in a multi-request calls
        let startTime: Date = new Date();
        let body: any = {
            format: 1
        };

        if (this._ks)
            body.ks = this._ks;

        if (!logger) {
            logger = moduleLogger
        }

        let  id = randomBytes(8).toString("hex");

        let isMultirequest:boolean = Array.isArray(requests);

        if (isMultirequest) {
            body['service'] = "multirequest";
            requests.forEach( (item,index) => {
                Object.entries(item).forEach( ([key,value])=> {
                    body[index + ":" + key] = value;
                });
            });
        } else {
            body=Object.assign(body,requests);
        }
        logger.debug(`[${id}], Sending request to kaltura server:  curl -i -d '${JSON.stringify(body)}' -H "Content-Type: application/json" -X POST ${this._config.url}`);


        let response=null;
        try {
            response = await Network.post({
                url: this._config.url,
                json: body,
                resolveWithFullResponse: true,
                timeout: this._config.timeout
                //agent: this._httpAgent,
            });
        }
        catch(e) {
            logger.warn(`[${id}] Exception doing API call: ${this._config.url} %j  with error %j time it took ${timePassed(startTime)}  ms`,body,e);
            throw e;
        }
        let result = response.body;

        let headersString = "";
        if (response && response.headers) {
            headersString = `Connection: ${response.headers["connection"]}; X-Me: ${response.headers['x-me']}; X-Kaltura-Session: ${response.headers['x-kaltura-session']}; x-kaltura: ${response.headers["x-kaltura"]}`;
        }
        let totalTime:number = timePassed(startTime);
        delete body.ks;

        if (result && result.objectType === "KalturaAPIException") {
            logger.warn(`[${id}] Exception doing API call: ${this._config.url} %j ${headersString} %j  time it took ${totalTime}  ms`,body,result);
            throw KalturaAPI.convertException(result);
        }

        if (isMultirequest) {
            for (let index=0; index<result.length;index++) {
                let res=result[index];
                if (res && res.objectType === "KalturaAPIException") {
                    console.warn(`${id} Exception doing API call: ${this._config.url} %j ${headersString} %j time it took ${totalTime}  ms`, requests[index],res);
                    throw KalturaAPI.convertException(res);
                }
            }
        }

        logger.info(`[${id}] API call was successful: ${this._config.url} %j ${headersString}  time it took ${totalTime}  ms`,body);

        return result;
    }
}
