import { IRegistry } from "../../interfaces/interfaces";
import { Packager } from "../../SystemObjects/Packager";
import { Transcoder } from "../../SystemObjects/Transcoder";
import { Source } from "../../SystemObjects/Source";
import { Logger } from "../../logger";
import { Kubernetes } from './Kubernetes'
import {TrackType} from "../../interfaces/channelOutput";
import {LocalCache} from "../../LocalCache";


export class DistributedRegistry implements IRegistry
{
    private readonly logger: Logger =new Logger("DistributedRegistry");
    private readonly kubernetes: Kubernetes;
    private readonly state:Map<string,any>;
    private readonly localCache:LocalCache = new LocalCache();

    constructor() {
        this.kubernetes=new Kubernetes();
        this.state=new Map<string,string>();
    }

    async initialize(): Promise<boolean> {
        await this.kubernetes.initialize();
        let packagers=await this.getPackagers();
        for (let packager of packagers) {
            this.logger.info("Retrieving sets served by packager ",packager.id);
            let setsIds = await packager.getChannels();
            this.logger.info("Got",setsIds);
            for (let setId of setsIds) {
                this.state.set(setId,packager.id);
            }
        }
        return true;
    }

    getPackagers(): Promise<Array<Packager>> {
        return this.kubernetes.getPackagers();
    }
    getSources(): Promise<Array<Source>> {
        return Promise.resolve( [] );
    }
    getPackagerById(packagerId: string): Promise<Packager> {
        return Promise.resolve(null);
    }

    getTranscoders(): Promise<Array<Transcoder>> {
        return this.kubernetes.getTranscoders();
    }

    runTranscoder(setId: string, inputId:string,trackType:TrackType, args:string[]): Promise<Transcoder> {
        args=["-c", "while true; do echo hello; sleep 10;done"];
        return this.kubernetes.runTranscoder(setId,inputId,trackType,args);
    }


    associateChannelIdAndPackager(setId:string, packagerId:string): Promise<boolean> {
        this.state.set("SETID_AND_PACKAGER_"+setId,packagerId)
        return Promise.resolve(true);
    }

    getPackagerByChannelId(setId:string): Promise<Packager> {
        let packagerId=this.state.get("SETID_AND_PACKAGER_"+setId);
        if (packagerId) {
            return this.getPackagerById(packagerId);
        }
        return Promise.resolve(null);
    }
}



//todo cron job to update CPU usage and all nodes available
//todo cron job that updates all transcoders