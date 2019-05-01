import {SetupRequest} from "../Logic/BusinessLogic";
import {
    TrackType
} from "./channelOutput";

import {ChannelAudioTrack, ChannelOutput, ChannelTrack, ChannelVariant, ChannelVideoTrack} from "./channelOutput";



export class InputConfiguration  {
    channelId:string;
    inputId:string;
    trackType: TrackType;
    private transcodingInstructions:any= { outputTracks: []};
    private output:ChannelOutput=new ChannelOutput();
    isPassthrough:boolean=true;
    private passthroughTrackId:string;

    constructor() {
    }

    setup(profile:ChannelOutput, setupRequest:SetupRequest,variantsWhiteList:Array<string>=[]) {

        this.inputId=setupRequest.inputId;
        this.trackType=setupRequest.trackType;
        this.channelId=setupRequest.channelId;

        let participatingVariants=profile.variants.filter( (rp:ChannelVariant)=> {

            if (variantsWhiteList.length>0 && variantsWhiteList.indexOf(rp.id)==-1) {
                return false;
            }

            if (rp.inputId && rp.inputId != setupRequest.inputId)
                return false;

            if (rp.isSource) {
                return true;
            }

            if (setupRequest.trackType == TrackType.Video) {
                if (setupRequest.codec == rp.videoTrack.codec) {

                    if (setupRequest.bitrate < rp.videoTrack.bitrate)
                        return false;

                    if (setupRequest.videoHeight < rp.videoTrack.height)
                        return false;

                    if (setupRequest.videoWidth < rp.videoTrack.width)
                        return false;

                }
            }
            if (setupRequest.trackType == TrackType.Audio) {

                if (setupRequest.codec == rp.audioTrack.codec) {

                    if (setupRequest.bitrate < rp.audioTrack.bitrate)
                        return false;

                    if (setupRequest.audioSamplingRate === rp.audioTrack.SamplingRate)
                        return false;
                }
            }
            return true;
        });

        this.output.variants=[];
        this.transcodingInstructions.outputTracks=[];

        let uniqueConversions:Map<string,ChannelTrack>=new Map<string,ChannelTrack>();
        participatingVariants.forEach( (rp:ChannelVariant)=>{
            let track = InputConfiguration.RenditionParamToOutputTrack(setupRequest, rp);

            let uniqueId=JSON.stringify(track);

            let variant= this.output.getVariant(rp.id);
            if (!variant) {
                variant = new ChannelVariant();
                variant.id=rp.id;
                variant.inputId=this.inputId;
                this.output.variants.push(variant);
            }

            if (uniqueConversions.has(uniqueId)) {
                variant.addTrack(uniqueConversions.get(uniqueId));
                return;
            }
            variant.addTrack(track);
            this.isPassthrough=this.isPassthrough && rp.isPassthrough(setupRequest.trackType);
            this.transcodingInstructions.outputTracks.push(rp.getTranscoderInstructions(setupRequest.trackType));
            this.passthroughTrackId=track["id"];

            uniqueConversions.set(uniqueId,track);
        });
    }

    private static RenditionParamToOutputTrack(setupRequest: SetupRequest, rp: ChannelVariant) {
        if (setupRequest.trackType == TrackType.Video) {
            let track = new ChannelVideoTrack();
            if (rp.videoTrack.isPassthrough()) {

                if (setupRequest.videoWidth > 0)
                    track.width = setupRequest.videoWidth;
                if (setupRequest.videoHeight > 0)
                    track.height = setupRequest.videoHeight;
                if (setupRequest.bitrate > 0)
                    track.bitrate = setupRequest.bitrate;
            }
            track.media_type = "video";
            track.id = "v" + rp.id;
            track.inputId = setupRequest.inputId;
            return track;

        }
        if (setupRequest.trackType == TrackType.Audio) {
            let track = new ChannelAudioTrack();
            track.media_type = "audio";
            track.id = "a" + rp.id;
            track.inputId = setupRequest.inputId;
            return track;
        }
        return null;
    }

    getTranscodingInstructions():any {
        return this.transcodingInstructions;
    }
    getChannelOutput():ChannelOutput {
        return this.output;
    }
    getPassthroughTrackId():string {
        if (this.isPassthrough)
            return this.passthroughTrackId;

        return "";
    }
}