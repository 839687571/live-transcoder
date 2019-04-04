import {Packager } from "../SystemObjects/Packager";
import {Transcoder} from "../SystemObjects/Transcoder";
import {TranscodingProfile} from "./transcodingProfile";

export enum DvrStatus {
    Enabled=1,
    Disabled=2
}
export enum SessionType {
    Primary=1,
    Backup=2
}
export enum RecordStatus {
    Disabled=0,
    Session=1,
    Append=2
}

export class EntryInfo
{
    id: string;
    name:string;
    recordStatus: RecordStatus;
    dvrStatus:DvrStatus;
    dvrWindow:number;
    segmentDuration:number;

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
    getEntryInfo(id:string) : Promise<EntryInfo>
    getTranscodingProfile(entryInfo:EntryInfo) : Promise<TranscodingProfile>
    authenticate(id:string,token:string,type:SessionType) : Promise<EntryInfo>
}




export interface IRegistry {

    getPackagers(): Promise<Array<Packager>>
    runTranscoder(setId:string,profile:TranscodingProfile): Promise<Transcoder>
    getTranscoders(): Promise<Array<Transcoder>>
}

