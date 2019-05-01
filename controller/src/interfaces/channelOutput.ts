
export enum TrackType {
    Video=0,
    Audio
}


export  class ChannelTrack
{
    id: string;
    bitrate: number=0;
    codec: string;
    inputId:string;


    isPassthrough():boolean {
        return !this.codec;
    }

    getTranscoderInstructions(res:any): any {

        res.bitrate=this.bitrate;
        res.codec=this.codec;
        res.passthrough=this.isPassthrough();
    }

}


export  class ChannelAudioTrack extends  ChannelTrack
{
    channels: number;
    SamplingRate: number;
    media_type:string = "audio";

    getTranscoderInstructions(res:any): any {
        super.getTranscoderInstructions(res);
        if (!this.isPassthrough()) {
            res.audioParams = {
                samplingRate: this.SamplingRate,
                channels: 2
            }
        }
    }

}
export  class ChannelVideoTrack extends  ChannelTrack
{
    width: number;
    height: number;
    maxFrameRate: number;
    profile:string;
    preset:string="veryfast";
    media_type:string = "video";

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
}


export  class ChannelVariant
{
    id: string;
    name: string;
    description: string;
    inputId:string;
    isSource:boolean;
    audioTrack: ChannelAudioTrack;
    videoTrack: ChannelVideoTrack;

    isPassthrough(track:TrackType) : boolean {
        if (track===TrackType.Audio) {
            return this.audioTrack.isPassthrough();
        }
        if (track===TrackType.Video) {
            return this.videoTrack.isPassthrough();
        }
        throw new Error("invalid track")
    }

    getTranscoderInstructions(trackType:TrackType): any {

        let trackName=(trackType==TrackType.Video ? "v" : "a")+this.id;
        let res:any={
            trackId: trackName,
            type: trackType,
            enabled: true,
            description: this.description
        };


        if (trackType===TrackType.Audio) {
            this.audioTrack.getTranscoderInstructions(res);
        }

        if (trackType===TrackType.Video) {
            this.videoTrack.getTranscoderInstructions(res);
        }

        return res;
    }

    getTrack(id:string):ChannelTrack {
        if (this.videoTrack && this.videoTrack.id==id) {
            return this.videoTrack;
        }
        if (this.audioTrack && this.audioTrack.id==id) {
            return this.audioTrack;
        }
        return null;
    }

    get tracks() {
        let ret=[];
        if (this.videoTrack ) {
            ret.push(this.videoTrack);
        }
        if (this.audioTrack) {
            ret.push(this.audioTrack);
        }
        return ret;
    }
    addTrack(track:ChannelTrack) {
        if (track instanceof ChannelVideoTrack)
            this.videoTrack=track;
        if (track instanceof ChannelAudioTrack)
            this.audioTrack=track;
    }
}



export  class ChannelOutput
{
    id: string;
    name: string;
    description: string;
    variants: Array<ChannelVariant> = [];

    getVariant(id:string) {
        return this.variants.find( variant=>variant.id==id);
    }
    removeInputTracks(inputId:string,type:TrackType){

        for (let variant of this.variants) {
            if (variant.inputId!=inputId) {
                continue;
            }
            for (let track of variant.tracks) {
                if (track instanceof ChannelVideoTrack && type==TrackType.Video)
                    variant.videoTrack=null;
                if (track instanceof ChannelAudioTrack && type==TrackType.Audio)
                    variant.audioTrack=null;
            }
        }
        //remove 'empty' variants
        this.variants=this.variants.filter(  variant=>  variant.videoTrack!=null || variant.audioTrack!=null);
    }

    merge(channelOutput:ChannelOutput) {
        for (let newVariant of channelOutput.variants) {
            let variant = this.getVariant(newVariant.id);
            if (!variant) {
                variant = newVariant;
                this.variants.push(variant);
            }
            for (let newTrack of newVariant.tracks) {
                variant.addTrack(newTrack);
            }
        }
    }

    static fromJSON(json):ChannelOutput {
        let output=new ChannelOutput();
        output.variants=json.variants.map( variant=>{
            let ret=new ChannelVariant();
            ret.id=variant.id;
            ret.inputId=variant.inputId;
            variant.tracks.forEach( track=> {
                let ret:ChannelTrack;
                if (track.media_type=="video") {
                    variant.videoTrack= ret = new ChannelVideoTrack();
                } else {
                    variant.audioTrack= ret = new ChannelAudioTrack();
                }
                ret.id=track.id;
                ret.inputId=track;

            });
            return ret;
        });
        return output;
    }
}
