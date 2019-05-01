import {IRegistry} from '../../interfaces/interfaces'
import {Packager} from "../../SystemObjects/Packager";
import {Transcoder} from "../../SystemObjects/Transcoder";
import {SingleServerJob} from './SingleServerJob'
import {Logger} from "../../logger";
import {retry} from "../../utils";
import * as config from 'config';
import {TrackType} from "../../interfaces/channelOutput";
import {Source} from "../../SystemObjects/Source";

export class SingleServerRegistry  implements IRegistry {

    private packagers: Array<Packager> = [];
    private logger: Logger =new Logger("SingleServerRegistry");
    private localJobs: Map<string,SingleServerJob> = new Map<string,SingleServerJob>();

    constructor() {
    }

    initialize(): Promise<boolean> {
        this.discoverSystemObjects();
        return Promise.resolve(true);
    }

    discoverSystemObjects() {
        let packager1: Packager = new Packager();
        packager1.id="==1==";
        packager1.baseUrl="http://localhost:9001";
        this.packagers.push(packager1);
    }
    async getPackagerById(packagerId:string): Promise<Packager> {
        return Promise.resolve(this.packagers.find( (packager)=>{
            return packager.id==packagerId;
        }));
    }

    async getPackagers(): Promise<Array<Packager>> {
        return Promise.resolve( this.packagers );
    }

    async getTranscoders(): Promise<Array<Transcoder>> {
        return Promise.resolve( [] );
    }
    getSources(): Promise<Array<Source>> {
        return Promise.resolve( [] );
    }

    removeJob(job) {
        this.localJobs.delete(job.channelId)
    }

    async runTranscoder(setId: string, inputId:string,trackType:TrackType, args:string[]): Promise<Transcoder> {
        let transcoder: Transcoder = new Transcoder();
        transcoder.id=`${setId}:${inputId}:${trackType==TrackType.Video ? "v" : "a"}`;
        transcoder.baseUrl="http://localhost";

        this.logger.info("Starting transcoder %j",args)
        let job:SingleServerJob = new SingleServerJob(transcoder);
        job.on("finished", this.removeJob.bind(this));
        job.start("ls",[]);
        this.localJobs.set(setId,job);

        return await retry( async()=> {
            this.logger.info("testing if transcoder %s has started",transcoder.id);
            if (await transcoder.isReady()) {
                //await this.registry.setTranscoderId(req.channelId,"",transcoder.id);
                return transcoder;
            }
            return null;
        },config.get("transcoderSpawn.retries"),config.get("transcoderSpawn.interval"),0,this.logger);
    }



    associateChannelIdAndPackager(setId:string, packagerId:string): Promise<boolean> {
        return Promise.resolve(true);
    }
    getPackagerByChannelId(setId:string): Promise<Packager> {
        return Promise.resolve(this.packagers[0]);
    }
}