import {
    IDAL,
    EntryInfo,
    SessionType,
    RecordStatus,
    DvrStatus,
} from './interfaces/interfaces';
import { KalturaAPI, KalturaAPIConfig} from './KalturaAPI';

import {
    RenditionAudioTrackParams,
    RenditionParams,
    RenditionVideoTrackParams,
    TranscodingProfile
} from "./interfaces/transcodingProfile";

export class KalturaDAL implements  IDAL
{
    api: KalturaAPI;
    hostname: string = "";

    constructor() {

        let config: KalturaAPIConfig = new KalturaAPIConfig();
        config.url = "https://www.kaltura.com/api_v3/index.php";
        config.secret = "cd957546b1f1528bdc1288527556ab69";
        config.userId = "webcastdemo@mailinator.com";
        config.partnerId = 1802381;
        config.ksType = 0;
        this.api = new KalturaAPI(config);
    }

    private static  convertToEntryInfo(obj: any) : EntryInfo
    {
        let e:EntryInfo = new EntryInfo();

        e.conversionProfileId=obj.conversionProfileId;
        e.recordStatus=obj.recordStatus as RecordStatus;
        e.dvrStatus=obj.dvrStatus as DvrStatus;
        e.segmentDuration=obj.segmentDuration;
        e.explicitLive=obj.explicitLive;
        e.viewMode=obj.viewMode;
        e.recordingStatus=obj.recordingStatus;
        e.partnerId=obj.partnerId;
        e.flavorParamsIds=obj.flavorParamsIds.split(",").map(parseFloat);
        e.dvrWindow=obj.dvrWindow;
        e.name=obj.name;
        return e;

    }
    private static  convertToRenditionParams(obj: any) : RenditionParams
    {
        let fp:RenditionParams = new RenditionParams();

        fp.name=obj.name;
        fp.id=obj.id;
        fp.description=obj.description;
        fp.inputId=obj.streamSuffix;
        fp.isSource=obj.tags.indexOf("source")>-1;
        fp.audioTrack=new RenditionAudioTrackParams();
        fp.audioTrack.bitrate=obj.audioBitrate;
        fp.audioTrack.codec=obj.audioCodec;
        fp.audioTrack.SamplingRate=obj.audioSampleRate;
        fp.audioTrack.channels=obj.channels;

        fp.videoTrack=new RenditionVideoTrackParams();
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

    async getEntryInfo(id: string): Promise<EntryInfo> {

        let entry=await this.api.call({
            service: "liveStream",
            action: "get",
            entryId: id
        });
        return KalturaDAL.convertToEntryInfo(entry);
    }

    static ConversionProfileToTranscodingProfile(cp:any,flavorsParams:Array<any>): TranscodingProfile {


        let res: TranscodingProfile = new TranscodingProfile();
        res.id=cp.id;
        res.description=cp.description;
        res.name=cp.name;
        res.renditions=flavorsParams.map(KalturaDAL.convertToRenditionParams) as  RenditionParams[];

        return res;
    }

    async getTranscodingProfile(entryInfo:EntryInfo) : Promise<TranscodingProfile>
    {
        let cp:any=await this.api.call({
            service: "conversionProfile",
            action: "get",
            id: entryInfo.conversionProfileId
        });

        let requests= cp.flavorParamsIds.split(",").map( (paramId:string)=> { return {
            service: "flavorParams",
            action: "get",
            id: paramId
        }});

        let flavorParams=await this.api.call(requests) as Array<any>;

        return KalturaDAL.ConversionProfileToTranscodingProfile(cp,flavorParams);
    }

    async authenticate(id:string,token:string,mediaServerIndex:SessionType) : Promise<EntryInfo>
    {
        let entry:any=await this.api.call({
            service: "liveStream",
            action: "authenticate",
            entryId: id,
            token: token,
            hostname: this.hostname,
            applicationName: "kLive",
            mediaServerIndex: mediaServerIndex.valueOf()
        });

        return KalturaDAL.convertToEntryInfo(entry)
    }
}