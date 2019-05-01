import {AuthenticateRequest, SetupRequest} from "./Logic/BusinessLogic";
import {TrackType} from "./interfaces/channelOutput";


enum FLV_AUDIO_CODEC {
    FLV_CODECID_PCM                  = 0,
    FLV_CODECID_ADPCM                = 1,
    FLV_CODECID_MP3                  = 2,
    FLV_CODECID_PCM_LE               = 3,
    FLV_CODECID_NELLYMOSER_16KHZ_MONO = 4,
    FLV_CODECID_NELLYMOSER_8KHZ_MONO = 5,
    FLV_CODECID_NELLYMOSER           = 6,
    FLV_CODECID_PCM_ALAW             = 7,
    FLV_CODECID_PCM_MULAW            = 8,
    FLV_CODECID_AAC                  = 10,
    FLV_CODECID_SPEEX                = 11
}


enum FLV_VIDEO_CODEC {
    FLV_CODECID_H263    = 2,
    FLV_CODECID_SCREEN  = 3,
    FLV_CODECID_VP6     = 4,
    FLV_CODECID_VP6A    = 5,
    FLV_CODECID_SCREEN2 = 6,
    FLV_CODECID_H264    = 7,
    FLV_CODECID_REALH263= 8,
    FLV_CODECID_MPEG4   = 9
}


function fillAudioCodec(setupRequest:SetupRequest,req:any) {

    switch(req.audio_codec_id){
        case FLV_AUDIO_CODEC.FLV_CODECID_PCM:
                if (req.bits_per_coded_sample==8) {
                    setupRequest.codec = "AV_CODEC_ID_PCM_U8";
                    return;
                }
                setupRequest.codec="AV_CODEC_ID_PCM_S16LE";//AV_CODEC_ID_PCM_S16BE?
                return;
        case FLV_AUDIO_CODEC.FLV_CODECID_PCM_LE:
            if (req.bits_per_coded_sample==8) {
                setupRequest.codec = "AV_CODEC_ID_PCM_U8";
                return;
            }
            setupRequest.codec="AV_CODEC_ID_PCM_S16LE";//AV_CODEC_ID_PCM_S16BE?
            return;
        case FLV_AUDIO_CODEC.FLV_CODECID_AAC:  return setupRequest.codec ="AAC";
        case FLV_AUDIO_CODEC.FLV_CODECID_ADPCM:  return  setupRequest.codec ="AV_CODEC_ID_ADPCM_SWF";
        case FLV_AUDIO_CODEC.FLV_CODECID_SPEEX:
            setupRequest.audioSamplingRate=16000;
            return  setupRequest.codec ="AV_CODEC_ID_SPEEX";
        case FLV_AUDIO_CODEC.FLV_CODECID_MP3:  return  setupRequest.codec ="AV_CODEC_ID_MP3";
        case FLV_AUDIO_CODEC.FLV_CODECID_NELLYMOSER_8KHZ_MONO:
            setupRequest.audioSamplingRate=8000;
            return  setupRequest.codec ="AV_CODEC_ID_NELLYMOSER";
        case FLV_AUDIO_CODEC.FLV_CODECID_NELLYMOSER_16KHZ_MONO:
            setupRequest.audioSamplingRate=16000;
            return  setupRequest.codec ="AV_CODEC_ID_NELLYMOSER";
        case FLV_AUDIO_CODEC.FLV_CODECID_NELLYMOSER:
            return  setupRequest.codec ="FLV_CODECID_NELLYMOSER";
        case FLV_AUDIO_CODEC.FLV_CODECID_PCM_MULAW:
            setupRequest.audioSamplingRate=8000;
            return  setupRequest.codec ="FLV_CODECID_NELLYMOSER";
        case FLV_AUDIO_CODEC.FLV_CODECID_PCM_ALAW:
            setupRequest.audioSamplingRate=8000;
            return  setupRequest.codec ="FLV_CODECID_NELLYMOSER";

        default:throw new Error("Invalid codec")
    }
}



function fillVideoCodec(setupRequest:SetupRequest,req:any) {

    switch(req.video_codec_id){
        case FLV_VIDEO_CODEC.FLV_CODECID_H263:
            setupRequest.codec="FLV1";
            return;
        case FLV_VIDEO_CODEC.FLV_CODECID_REALH263:
            setupRequest.codec="h263";
            return;
        case FLV_VIDEO_CODEC.FLV_CODECID_VP6:
            setupRequest.codec="VP6";
            return;
        case FLV_VIDEO_CODEC.FLV_CODECID_VP6A:
            setupRequest.codec="VP6A";
            return;
        case FLV_VIDEO_CODEC.FLV_CODECID_H264:
            setupRequest.codec="h264";
            return;
        case FLV_VIDEO_CODEC.FLV_CODECID_MPEG4:
            setupRequest.codec="MPEG4";
            return;

        default:throw new Error("Invalid codec")
    }
}

export function rtmpRequstBodyToSetupRequest(req:any): SetupRequest
{
    let z=/(?<channelId>.*)_(?<inputId>.*)/.exec(req.name);
    if (!z || z.length!=3) {
        throw new Error("");
    }
    let setupRequest:SetupRequest=new SetupRequest();
    setupRequest.channelId=z.groups["channelId"];
    setupRequest.inputId=z.groups["inputId"];
    switch (req.media_type) {
        case "video":
            setupRequest.trackType=TrackType.Video;
            fillVideoCodec(setupRequest,req);
            setupRequest.bitrate=req.video_data_rate;
            setupRequest.videoWidth=req.width;
            setupRequest.videoHeight=req.height;
            setupRequest.frameRate=req.frame_rate;
            break;
        case "audio":
            setupRequest.trackType=TrackType.Audio;
            setupRequest.bitrate=req.audio_data_rate;
            setupRequest.audioSamplingRate=req.sample_rate;
            fillAudioCodec(setupRequest,req);
            //setupRequest.audioChannels=req.body.audio_channels;
            break;
        default:
            throw new Error("AAA");
    }

    return setupRequest;
}




export function rtmpUrlToAuthenticateRequest(req:any): AuthenticateRequest
{
    let authenticateRequest:AuthenticateRequest=new AuthenticateRequest();

    return authenticateRequest;
}