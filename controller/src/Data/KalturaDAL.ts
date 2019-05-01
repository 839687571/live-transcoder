import {
    IDAL,
    ChannelInfo,
    ChannelSessionType,
    RecordStatus,
    DvrStatus, IChannel,
} from '../interfaces/interfaces';
import { KalturaAPI, KalturaAPIConfig} from './KalturaAPI';
import * as config from "config";

import {
    ChannelAudioTrack,
    ChannelOutput, ChannelVariant, ChannelVideoTrack
} from "../interfaces/channelOutput";
import {Logger} from "../logger";
import {KalturaChannel} from "./KalturaChannel";

import { execSync } from 'child_process';
import {Packager} from "../SystemObjects/Packager";

export enum ChannelState {
    Stopped=0,
    Authenticated=3,
    Broadcasting=2,
    Playable=1,
    Suspended=100
}


export class KalturaDAL implements  IDAL
{
    static api: KalturaAPI;
    private serverNodeHostName: string = "";

    private serverNode: any = {};
    private readonly logger: Logger;
    private readonly channels:Map<string,KalturaChannel>=new Map<string,KalturaChannel>();

    constructor(logger:Logger) {
        this.logger=logger;
    }

    private get serverNodeId():string {
        return this.serverNode.id;
    }

    getChannel(channelId:string):IChannel {
        let entry=this.channels.get(channelId);


        if (!entry) {
            entry=new KalturaChannel(this,channelId);
            this.channels.set(channelId,entry);
        }
        entry.type=ChannelSessionType.Primary;
        return entry;
    }

    private static  convertToEntryInfo(obj: any) : ChannelInfo {
        let e:ChannelInfo = new ChannelInfo();

        e.conversionProfileId=obj.conversionProfileId;
        e.recordStatus=obj.recordStatus as RecordStatus;
        e.dvrStatus=obj.dvrStatus as DvrStatus;
        e.segmentDuration=obj.segmentDuration;
        e.recordedEntryId=obj.recordedEntryId;
        e.explicitLive=obj.explicitLive;
        e.viewMode=obj.viewMode;
        e.recordingStatus=obj.recordingStatus;
        e.partnerId=obj.partnerId;
        e.flavorParamsIds=obj.flavorParamsIds.split(",").map(parseFloat);
        e.dvrWindow=obj.dvrWindow;
        e.name=obj.name;
        return e;

    }

    private static  convertToRenditionParams(obj: any) : ChannelVariant {
        let fp:ChannelVariant = new ChannelVariant();

        fp.name=obj.name;
        fp.id=obj.id;
        fp.description=obj.description;
        fp.inputId=obj.streamSuffix;
        fp.isSource=obj.tags.indexOf("source")>-1;
        fp.audioTrack=new ChannelAudioTrack();
        fp.audioTrack.id="a"+obj.id;
        fp.audioTrack.media_type="audio";
        fp.audioTrack.bitrate=obj.audioBitrate;
        fp.audioTrack.codec=obj.audioCodec;
        fp.audioTrack.SamplingRate=obj.audioSampleRate;
        fp.audioTrack.channels=obj.channels;

        fp.videoTrack=new ChannelVideoTrack();
        fp.videoTrack.id="a"+obj.id;
        fp.videoTrack.media_type="video";
        fp.videoTrack.bitrate=obj.videoBitrate;
        if (obj.videoCodec) {
            switch (obj.videoCodec.toLowerCase()) {
                case "h264h":
                    fp.videoTrack.codec = "h264";
                    fp.videoTrack.profile = "high";
                    break;
                case "h264m":
                    fp.videoTrack.codec = "h264";
                    fp.videoTrack.profile = "main";
                    break;
                case "h264b":
                    fp.videoTrack.codec = "h264";
                    fp.videoTrack.profile = "baseline";
                    break;
            }
            fp.videoTrack.height=obj.height;
            fp.videoTrack.width=obj.width;
            fp.videoTrack.maxFrameRate=obj.maxFrameRate;
        }

        return fp;

    }

    private static ConversionProfileToTranscodingProfile(cp:any,flavorsParams:Array<any>): ChannelOutput {
        let res: ChannelOutput = new ChannelOutput();
        res.id=cp.id;
        res.description=cp.description;
        res.name=cp.name;
        res.variants=flavorsParams.map(KalturaDAL.convertToRenditionParams) as  ChannelVariant[];
        return res;
    }

    //API requests
    async getEntryInfo(id: string): Promise<ChannelInfo> {

        let entry=await KalturaDAL.api.call({
            service: "liveStream",
            action: "get",
            entryId: id
        },this.logger);
        return KalturaDAL.convertToEntryInfo(entry);
    }

    async getChannelInfoAndTranscodingProfile(id:string) : Promise<[ChannelInfo,ChannelOutput]> {
        let requests=[{
            service: "liveStream",
            action: "get",
            entryId: id
        },{
            service: "conversionProfile",
            action: "get",
            id: "{1:result:conversionProfileId}"
        }];

        let res:any=await KalturaDAL.api.call(requests,this.logger);

        requests= res[1].flavorParamsIds.split(",").map( (paramId:string)=> { return {
            service: "flavorParams",
            action: "get",
            id: paramId
        }});

        let flavorParams=await KalturaDAL.api.call(requests,this.logger) as Array<any>;

        return [KalturaDAL.convertToEntryInfo(res[0]),KalturaDAL.ConversionProfileToTranscodingProfile(res[1],flavorParams)];
    }
    async getTranscodingProfile(channelInfo:ChannelInfo) : Promise<ChannelOutput> {

        let res:any=await KalturaDAL.api.call({
            service: "conversionProfile",
            action: "get",
            id: channelInfo.conversionProfileId
        },this.logger);

        let requests= res.flavorParamsIds.split(",").map( (paramId:string)=> { return {
            service: "flavorParams",
            action: "get",
            id: paramId
        }});

        let flavorParams=await KalturaDAL.api.call(requests,this.logger) as Array<any>;

        return KalturaDAL.ConversionProfileToTranscodingProfile(res,flavorParams);
    }

    async authenticate(id:string,token:string,mediaServerIndex:ChannelSessionType) : Promise<ChannelInfo> {
        let entry:any=await KalturaDAL.api.call({
            service: "liveStream",
            action: "authenticate",
            entryId: id,
            token: token,
            hostname: this.serverNodeHostName,
            applicationName: "kLive",
            mediaServerIndex: mediaServerIndex.valueOf()
        },this.logger);

        return KalturaDAL.convertToEntryInfo(entry)
    }

    async registerMediaServer(id:string, status:ChannelState, mediaServerIndex:ChannelSessionType): Promise<ChannelInfo> {

        let entry:any=await KalturaDAL.api.call({
            service: "liveStream",
            action: status===ChannelState.Stopped ? "unregisterMediaServer" : "registerMediaServer",
            entryId: id,
            liveEntryStatus: status.valueOf(),
            hostname: this.serverNodeHostName,
            applicationName: "kLive",
            mediaServerIndex: mediaServerIndex.valueOf(),
            shouldCreateRecordedEntry: false
        },this.logger);

        return KalturaDAL.convertToEntryInfo(entry)
    }

    async registerMediaServerAndUpdateEntryServerNode(id:string, status:ChannelState, mediaServerIndex:ChannelSessionType, serverNode:any, packagerChannelInfo:any): Promise<[ChannelInfo,any]> {

        let requests:Array<any>=[{
            service: "liveStream",
            action: "registerMediaServer",
            entryId: id,
            liveEntryStatus: status.valueOf(),
            hostname: this.serverNodeHostName,
            applicationName: "kLive",
            mediaServerIndex: mediaServerIndex.valueOf(),
            shouldCreateRecordedEntry: false
        }];

        if (!serverNode) {
            requests.push({
                "service": "entryServerNode",
                "action": "list",
                "filter:objectType": "KalturaEntryServerNodeFilter",
                "filter:statusIn": "1, 2, 3",
                "filter:serverNodeIdEqual": this.serverNodeId,
                "filter:entryIdEqual": id
            })
        }

        if (packagerChannelInfo) {

            let updateReq: any = {
                "service": "entryServerNode",
                "action": "update",
                "id": serverNode ? serverNode.id : "{2:result:objects:0:id}",
                "entryServerNode": {
                    "objectType": "KalturaLiveEntryServerNode",
                    "streams": packagerChannelInfo.variants.map((variant, i) => {
                        let summary: any = Packager.getVariantsSummary(variant);
                        return summary;
                    })
                }
            };
            requests.push(updateReq);
        }


        let res:any=await KalturaDAL.api.call(requests,this.logger);

        return [ KalturaDAL.convertToEntryInfo(res[0]), packagerChannelInfo ? res[2] : (serverNode ? serverNode : res[1].objects[0]) ]
    }

    async getEntryServerNodes():Promise<Array<any>> {
        let entryServerNodes:Array<any> = [];
        for (let pageIndex=0;pageIndex<5;pageIndex++) {
            let res:any=await KalturaDAL.api.call({
                "service": "entryServerNode",
                "action": "list",
                "filter:objectType": "KalturaEntryServerNodeFilter",
                "filter:statusIn": "1, 2, 3",
                "filter:serverNodeIdEqual": this.serverNodeId,
                "pager:pageIndex": pageIndex,
                "pager:pageSize": 300
            });
            entryServerNodes=entryServerNodes.concat(res.objects);
            if (entryServerNodes.length<=res.totalCount) {

                break;
            }
        }
        return entryServerNodes;
    }

    async getServerNode():Promise<any> {
        let res:any =await KalturaDAL.api.call({
            service: "serverNode",
            action: "list",
            "filter:objectType": "KalturaWowzaMediaServerNodeFilter",
            "filter:nameLike": this.serverNodeHostName
        },this.logger);

        if (res.totalCount!==1) {
            this.logger.error("Cannot find serverNode %s ",this.serverNodeHostName);
            throw new Error("Cannot find serverNode "+this.serverNodeHostName);
        }

        return res.objects[0];
    }


    public async initialize():Promise<boolean> {

        this.serverNodeHostName = config.get("backend.serverNodeHostName");
        if (!this.serverNodeHostName  || this.serverNodeHostName == "@HOSTNAME@") {
            let cmd = "hostname -f";
            this.serverNodeHostName = execSync(cmd).toString().trim();
        }

        this.serverNode=await this.getServerNode();

        this.logger.info("initialize serverNodeHostName=%s serverNode=%j ",this.serverNodeHostName, this.serverNode);
        return true;
    }


    removeChannel(channel:IChannel) {
        (channel as KalturaChannel).stop();
        this.channels.delete(channel.id);

    }


    async alignState(packagerChannelInfos:any) {
        return;
        this.logger.info("[alignState] Retrieve all entryServerNodes");
        let entryServerNodes = await this.getEntryServerNodes();
        for (let entryServerNode of  entryServerNodes) {
            let channel:KalturaChannel = this.getChannel(entryServerNode.entryId) as KalturaChannel;
            channel.entryServerNode=entryServerNode;
        }

        let channelsToDelete:Array<KalturaChannel>=[];
        for (let channel of  this.channels.values()) {
            let packagerChannelInfo=packagerChannelInfos[channel.id];
            if (packagerChannelInfo) {
                channel.markPlayable(packagerChannelInfo);
            } else {
                channelsToDelete.push(channel);
            }
        }
        for ( let channel of  channelsToDelete) {
            this.logger.info("[alignState] stopping channel %s",channel.id);
            channel.stop();
            this.channels.delete(channel.id);
        }
    }
}



let kconfig: KalturaAPIConfig = new KalturaAPIConfig();
kconfig.url = config.get("backend.url");
kconfig.secret = config.get("backend.secret");
kconfig.userId = config.get("backend.userId");
kconfig.partnerId = config.get("backend.partnerId");
kconfig.ksType = 0;
KalturaDAL.api = new KalturaAPI(kconfig);