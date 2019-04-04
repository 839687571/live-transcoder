import {SetupRequest} from "../BusinessLogic";

export enum RenditionTrackType {
    Video,
    Audio
}

export  class RenditionTrackParams
{
    bitrate: number;
    codec: string;

    isPassthrough():boolean {
        return !this.codec;
    }

    getTranscoderInstructions(res:any): any {

        res.bitrate=this.bitrate;
        res.codec=this.codec;
        res.passthrough=this.isPassthrough();
    }

    getDuplicationUniqueId():string {
        return this.bitrate+this.codec;
    }
}

export  class RenditionAudioTrackParams extends  RenditionTrackParams
{
    channels: number;
    SamplingRate: number;

    getTranscoderInstructions(res:any): any {
        super.getTranscoderInstructions(res);
        if (!this.isPassthrough()) {
            res.audioParams = {
                samplingRate: this.SamplingRate,
                channels: 2
            }
        }
    }
    getDuplicationUniqueId():string {
        return super.getDuplicationUniqueId()+this.SamplingRate;
    }

}
export  class RenditionVideoTrackParams extends  RenditionTrackParams
{
    width: number;
    height: number;
    maxFrameRate: number;
    profile:string;
    preset:string="veryfast";

    getTranscoderInstructions(res:any): any {
        super.getTranscoderInstructions(res);
        if (!this.isPassthrough()) {
            res.videoParams = {
                profile: this.profile,
                preset: this.preset,
                height: this.height ? this.height : -2,
                width: this.width ? this.width : -2,
                skipFrame: 1
            }
        }
    }
    getDuplicationUniqueId():string {
        return super.getDuplicationUniqueId()+this.profile+this.preset+this.width+this.height;
    }
}


export  class RenditionParams
{
    id: string;
    name: string;
    description: string;
    inputId:string;
    isSource:boolean;
    audioTrack: RenditionAudioTrackParams;
    videoTrack: RenditionVideoTrackParams;

    isPassthrough(track:RenditionTrackType) : boolean {
        if (track===RenditionTrackType.Audio) {
            return this.audioTrack.isPassthrough();
        }
        if (track===RenditionTrackType.Video) {
            return this.videoTrack.isPassthrough();
        }
        throw new Error("invalid track")
    }

    getTranscoderInstructions(track:RenditionTrackType): any {

        let res:any={
            id: this.id,
            description: this.description
        };


        if (track===RenditionTrackType.Audio) {
            this.audioTrack.getTranscoderInstructions(res);
        }

        if (track===RenditionTrackType.Video) {
            this.videoTrack.getTranscoderInstructions(res);
        }

        return res;
    }
}



export  class TranscodingProfile
{
    id: string;
    name: string;
    description: string;
    renditions: Array<RenditionParams> = [];

    isPassthrough(trackType:RenditionTrackType) : boolean {
        return null===this.renditions.find( (rendition:RenditionParams)=> {
            return !rendition.isPassthrough(trackType);
        });
    }

    getTranscoderInstructions(setupRequest:SetupRequest): any {
        let res:any={};

        res.outputs=[];
        let s:Set<string>=new Set<string>();
        let participatingRenditions=this.renditions.filter( (rp:RenditionParams)=> {

            if (rp.inputId && rp.inputId != setupRequest.variantId)
                return false;

            if (rp.isSource) {
                return true;
            }

            if (setupRequest.trackType == RenditionTrackType.Video) {
                if (setupRequest.codec == rp.videoTrack.codec) {

                    if (setupRequest.bitrate < rp.videoTrack.bitrate)
                        return false;

                    if (setupRequest.videoHeight < rp.videoTrack.height)
                        return false;

                    if (setupRequest.videoWidth < rp.videoTrack.width)
                        return false;

                }
            }
            if (setupRequest.trackType == RenditionTrackType.Audio) {

                if (setupRequest.codec == rp.audioTrack.codec) {

                    if (setupRequest.bitrate < rp.audioTrack.bitrate)
                        return false;

                    if (setupRequest.audioSamplingRate === rp.audioTrack.SamplingRate)
                        return false;
                }
            }
            return true;
        });



        res.outputs=[];

        participatingRenditions.forEach( (rp:RenditionParams)=>{
                let uniqueId:string="";
                if (setupRequest.trackType == RenditionTrackType.Video) {
                    uniqueId=rp.videoTrack.getDuplicationUniqueId();
                }
                if (setupRequest.trackType == RenditionTrackType.Audio) {
                    uniqueId=rp.audioTrack.getDuplicationUniqueId();
                }
                if (s.has(uniqueId)) {
                    return;
                }
                s.add(uniqueId);
                res.outputs.push(rp.getTranscoderInstructions(setupRequest.trackType));
        });
        return res;
    }

}
