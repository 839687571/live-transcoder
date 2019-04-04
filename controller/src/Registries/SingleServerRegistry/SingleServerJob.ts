import {EventEmitter} from "events";
import {randomBytes} from "crypto";
import {spawn} from 'child_process'

import {getLogger} from 'log4js';
import {SystemObject, SystemObjectState} from "../../SystemObjects/SystemObject";

const logger = getLogger("SingleServerJob");

export class SingleServerJob extends  EventEmitter
{
    private process: any;
    private readonly id: string;
    private readonly setId: string;
    private readonly sysObject: SystemObject;

    constructor(setId:string,sysObject:SystemObject) {
        super();
        this.setId=setId;
        this.sysObject=sysObject;
        this.sysObject.state=SystemObjectState.Pending;
        this.id=randomBytes(20).toString('hex');
    }

    private processError (err) {
        logger.warn("[Job] exited with error: ",err);
        this.process=null;
        this.sysObject.state=SystemObjectState.Completed;
        this.emit("finished",this);
    }

    start(command:string,args:Array<string>) {

        this.process=spawn(command,args);

        this.process.on('exit', this.processError.bind(this));
        this.process.on('error', this.processError.bind(this));

        this.sysObject.state=SystemObjectState.Ready;
    }

}