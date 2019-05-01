import {Packager } from "../SystemObjects/Packager";
import {Transcoder} from "../SystemObjects/Transcoder";
import {TrackType, ChannelOutput} from "./channelOutput";
import {InputConfiguration} from "./InputConfiguration";
import {Source} from "../SystemObjects/Source";
import {Logger} from "../logger";

export enum DvrStatus {
    Enabled=1,
    Disabled=2
}
export enum ChannelSessionType {
    Primary=1,
    Backup=2
}
export enum RecordStatus {
    Disabled=0,
    Session=1,
    Append=2
}

export class ChannelInfo
{
    id: string;
    name:string;
    recordStatus: RecordStatus;
    dvrStatus:DvrStatus;
    dvrWindow:number;
    segmentDuration:number;
    recordedEntryId:string;

//kaltura specific
    explicitLive:boolean;
    viewMode:number;
    recordingStatus:number;
    partnerId:number;
    conversionProfileId: string;
    flavorParamsIds:Array<number>;
}


export  interface IDAL
{

    getChannel(channelId:string):IChannel;
    initialize():Promise<boolean>;
    alignState(packagerChannelInfos:any):Promise<void>;
    removeChannel(channel:IChannel);

   // getEntryInfo(id:string) : Promise<ChannelInfo>
   // getTranscodingProfile(channelInfo:ChannelInfo) : Promise<ChannelOutput>
   // authenticate(id:string,token:string,type:ChannelSessionType) : Promise<ChannelInfo>
}


export  interface IChannel
{
    channelInfo:ChannelInfo;
    transcodingProfile:ChannelOutput;
    id:string;
    actualOutput:ChannelOutput;

    addOutputs(output:ChannelOutput);
    removeInput(inputId:string, type:TrackType);
    retrieveChannelMetadata():Promise<boolean>;
    authenticate(token:string):Promise<boolean>;
    markPlayable(packagerChannelData:any);
}


export  interface ICache {
    get(key:string): Promise<any>;
    set(key:string,value:any,timeout:Number): Promise<any>;
    lock(name:string,logger:Logger): Promise<boolean>;
    unlock(name:string,logger:Logger): Promise<boolean>;
}


export interface IRegistry {

    initialize(): Promise<boolean>;
    getPackagers(): Promise<Array<Packager>>;
    getSources(): Promise<Array<Source>>;
    getPackagerById(packagerId:string): Promise<Packager>;
    runTranscoder(setId: string, inputId:string,trackType:TrackType, args:string[]): Promise<Transcoder>;
    getTranscoders(): Promise<Array<Transcoder>>;
    associateChannelIdAndPackager(channelId:string, packagerId:string): Promise<any>;
    getPackagerByChannelId(channelId:string): Promise<Packager>;
}


