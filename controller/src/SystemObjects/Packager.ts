import { Network } from "../utils";
import {SystemObject} from "./SystemObject";
import { InputConfiguration} from "../interfaces/InputConfiguration";
import {Logger} from "../logger";
import {ChannelOutput, ChannelTrack, ChannelVariant} from "../interfaces/channelOutput";



export class Packager extends  SystemObject{

    getKMPEndpoint():string{
        return `kmp://${this.baseUrl}:10000`;
    }

    get APIUrl():string {
        return this.baseUrl+"/API";
    }

    async setup(inputConfig:InputConfiguration):Promise<boolean> {

        this.logger.info("add channelId: %s",inputConfig.channelId);
        await this.addChannel(inputConfig.channelId);
        let channelOutput:ChannelOutput=inputConfig.getChannelOutput();
        for (let variant of channelOutput.variants) {
            this.logger.info("add variant channelId:%s variantId: %s",inputConfig.channelId, variant.id);
            await this.addVariant(inputConfig.channelId, variant);
            for (let track of variant.tracks) {
                this.logger.info("add track to channelId: %s, variantId: %s track: %j",inputConfig.channelId, variant.id, track);
                await this.addTrack(inputConfig.channelId, variant, track);
            }
        }

        return true;
    }


    static getVariantsSummary(obj:any):any {
        let ret:any= {
            flavorId:obj.id,
            bitrate:0,
        };
        obj.tracks.forEach( (track)=>{
            ret.bitrate+=track.bitrate;
            if (track.media_type=="video") {
                ret.width=track.width;
                ret.height=track.height;
                ret.bitrate+=track.bitrate;
                ret.keyFrameInterval=track.keyFrameInterval;
                ret.frameRate=track.frameRate;
            }
            if (track.media_type=="audio") {
                ret.language=track.language;
            }
        });
        return ret;
    }


    async getChannel(channelId:string): Promise<any> {

        try {
            let json:any=await Network.get({ url: `${this.APIUrl}/sets/${channelId}`});
            return  json;
        }catch(e) {
            return null;
        }
    }

    async getChannels(): Promise<Array<string>> {

        try {
            let x=await Network.get({ url: `${this.APIUrl}/sets`});
            return x;
        }catch(e) {
            return [];
        }
    }

    async addChannel(channelId:string):Promise<boolean> {
        let body:any = {
            "id": channelId
        };
        try {
            let x=await Network.post({ url: `${this.APIUrl}/sets` , json: body});
            return true;
        }catch(e) {
            return false;
        }

    }

    async addVariant(setId:string,variant:ChannelVariant):Promise<boolean> {
        let body:any = {
            "id":variant.id
        };
        try {
            let x = await Network.post({url: `${this.APIUrl}/control/sets/${setId}/variants`, json: body});
            return true;
        }catch(e) {
            return false;
        }

    }
    async addTrack(setId:string,variant:ChannelVariant,track:ChannelTrack):Promise<boolean> {
        try {
            let x = await Network.post({url: `${this.APIUrl}/control/sets/${setId}/variants/${variant.id}/tracks`, json: track});
            return true;
        }catch(e) {
            return false;
        }

    }

}
