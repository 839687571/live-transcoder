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

    translate(res:any): any {

        res.bitrate=this.bitrate;
        res.codec=this.codec;
        res.passthrough=this.isPassthrough();
    }
}

export  class RenditionAudioTrackParams extends  RenditionTrackParams
{
    channels: number;
    sampleRate: number;

    translate(res:any): any {
        super.translate(res);
        if (!this.isPassthrough()) {
            res.audioParams = {
                samplingRate: this.sampleRate,
                channels: 2
            }
        }
    }
}
export  class RenditionVideoTrackParams extends  RenditionTrackParams
{
    width: number;
    height: number;
    maxFrameRate: number;
    profile:string;
    preset:string="veryfast";

    translate(res:any): any {
        super.translate(res);
        if (!this.isPassthrough()) {
            res.videoParams = {
                profile: this.profile,
                preset: this.preset,
                height: this.height,
                skipFrame: 1
            }
        }
    }
}


export  class RenditionParams
{
    id: string;
    name: string;
    description: string;
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

    translate(track:RenditionTrackType): any {

        let res:any={
            id: this.id,
            description: this.description
        };


        if (track===RenditionTrackType.Audio) {
            this.audioTrack.translate(res);
        }

        if (track===RenditionTrackType.Video) {
            this.videoTrack.translate(res);
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

    translate(trackType:RenditionTrackType): any {

        let res:any={};
        res.outputs=this.renditions.map( (fp:RenditionParams)=> fp.translate(trackType) );
        return res;
    }

}
