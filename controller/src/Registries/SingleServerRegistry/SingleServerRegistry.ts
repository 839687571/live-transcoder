import {IRegistry} from '../../interfaces/interfaces'

import { Packager} from "../../SystemObjects/Packager";
import { Transcoder } from "../../SystemObjects/Transcoder";

import { getLogger } from 'log4js';
const logger = getLogger("SingleServerRegistry");

import { SingleServerJob } from './SingleServerJob'
import {SystemObjectState} from "../../SystemObjects/SystemObject";
import {TranscodingProfile} from "../../interfaces/transcodingProfile";

export class SingleServerRegistry  implements IRegistry{

    packagers: Array<Packager> = [];

    localJobs: Map<string,SingleServerJob> = new Map<string,SingleServerJob>();

    constructor() {
        this.discoverSystemObjects();
    }


    discoverSystemObjects() {
        let packager1: Packager = new Packager();
        packager1.id="1228321832131";
        packager1.baseUrl="http://localhost:9001";
        let packager2: Packager = new Packager();
        packager2.id="1228321832131";
        packager2.baseUrl="http://localhost:9002";

        this.packagers.push(packager1);
        this.packagers.push(packager2);
    }

    async getPackagers(): Promise<Array<Packager>> {
        return Promise.resolve( this.packagers );
    }

    async getTranscoders(): Promise<Array<Transcoder>> {
        return Promise.resolve( [] );
    }

    removeJob(job) {
        this.localJobs.delete(job.setId)
    }

    async runTranscoder(setId:string,profile:TranscodingProfile): Promise<Transcoder> {
        let transcoder: Transcoder = new Transcoder();
        transcoder.baseUrl="http://localhost:19001";
        let job:SingleServerJob = new SingleServerJob(setId,transcoder);
        job.on("finished", this.removeJob.bind(this));
        job.start("ls",[]);
        this.localJobs.set(setId,job);

        return transcoder;
    }


}