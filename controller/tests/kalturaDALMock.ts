import {Network} from "../src/utils";


let flavorParams=[{"streamSuffix":"1","videoCodec":"","videoBitrate":900,"audioCodec":"","audioBitrate":0,"audioChannels":0,"audioSampleRate":0,"width":640,"height":480,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"flv","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":"0","videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":32,"partnerId":0,"name":"Source","systemName":"Source","description":"Maintains the original format and settings of the input stream","createdAt":1388310390,"isSystemDefault":1,"tags":"source,ingest,web,mobile,ipad,ipadnew","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264b","videoBitrate":400,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":480,"height":0,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"mp4","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":33,"partnerId":0,"name":"Basic/Small - WEB/MBL (H264/400)","systemName":"Basic_Small_400","description":"Basic/Small - WEB/MBL (H264/400)","createdAt":1388310390,"isSystemDefault":1,"tags":"mbr,ipadnew,iphonenew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264b","videoBitrate":600,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":640,"height":0,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"mp4","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":34,"partnerId":0,"name":"Basic/Small - WEB/MBL (H264/600)","systemName":"Basic_Small_600","description":"Basic/Small - WEB/MBL (H264/600)","createdAt":1388310390,"isSystemDefault":1,"tags":"mbr,ipadnew,iphonenew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264m","videoBitrate":900,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":640,"height":0,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"mp4","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":35,"partnerId":0,"name":"SD/Small - WEB/MBL (H264/900)","systemName":"SD_Small_900","description":"SD/Small - WEB/MBL (H264)","createdAt":1388310390,"isSystemDefault":1,"tags":"mbr,ipadnew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264h","videoBitrate":1700,"audioCodec":"aac","audioBitrate":128,"audioChannels":2,"audioSampleRate":44100,"width":0,"height":720,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"{ \"constantBitrate\" : true }","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"mp4","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":42,"partnerId":0,"name":"HD/720 - WEB/MBL (H264/1700)","systemName":"HD/720 - WEB/MBL (H264/1700)","description":"HD/720 - WEB/MBL (H264/1700)","createdAt":1479642661,"isSystemDefault":1,"tags":"mbr,ipadnew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_HD_FLAVORS","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264h","videoBitrate":3500,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":0,"height":1080,"frameRate":0,"gopSize":0,"conversionEnginesExtraParams":"{ \"constantBitrate\" : true }","twoPass":false,"deinterlice":0,"rotate":0,"format":"mp4","aspectRatioProcessingMode":0,"forceFrameToMultiplication16":1,"isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"anamorphicPixels":0,"isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":2156182,"partnerId":1802381,"name":"UHD/1080","systemName":"UHD/1080","description":"","createdAt":1535955455,"isSystemDefault":0,"tags":"mbr,ipadnew,iphonenew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"objectType":"KalturaLiveParams"},{"videoCodec":"h264h","videoBitrate":6500,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":0,"height":1440,"frameRate":0,"gopSize":0,"conversionEnginesExtraParams":"{ \"constantBitrate\" : true }","twoPass":false,"deinterlice":0,"rotate":0,"format":"mp4","aspectRatioProcessingMode":0,"forceFrameToMultiplication16":1,"isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"anamorphicPixels":0,"isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":2156192,"partnerId":1802381,"name":"UHD/1440","systemName":"UHD/1440","description":"","createdAt":1535955467,"isSystemDefault":0,"tags":"mbr,ipadnew,iphonenew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"objectType":"KalturaLiveParams"}];


let conversionProfileTemplate= {"id":9851841,"partnerId":1802381,"status":2,"type":2,"name":"Cloud Transcode 4k","systemName":"","tags":"","description":"","createdAt":1535956454,"flavorParamsIds":"32,33,34,35,42,2156182,2156192","isDefault":false,"isPartnerDefault":false,"objectType":"KalturaConversionProfile"};


let entryDataTemplate = {"streamName":"1_rdwhia6e_%i","streamPassword":"af109cce","recordStatus":1,"dvrStatus":0,"dvrWindow":1440,"recordedEntryId":"","publishConfigurations":[],"segmentDuration":6000,"explicitLive":false,"viewMode":1,"recordingStatus":2,"sourceType":"32","flavorParamsIds":conversionProfileTemplate.flavorParamsIds,"id":"1_rdwhia6e","name":"test for adi","partnerId":1802381,"userId":"webcastdemo@mailinator.com","creatorId":"webcastdemo@mailinator.com","tags":"","adminTags":"kms-webcast-event","status":2,"type":7,"createdAt":1550663808,"updatedAt":1554383129,"rank":0,"totalRank":0,"votes":0,"conversionProfileId":conversionProfileTemplate.id,"rootEntryId":"1_rdwhia6e","objectType":"KalturaLiveStreamEntry"}

let entryServerNodeTemplate= {

}

function getEntryData(req) {
    let res=Object.assign({},entryDataTemplate);
    res.id=req.entryId;
    return res;
}

function getEntryServerNode(req) {
    let res:any=Object.assign({},entryServerNodeTemplate);
    res.entryId=req.entryId;
    return res;
}
let responses= [
    { req: { "service": "serverNode" }, res:{ "totalCount":1, objects: [ { "id":1234}]} },
    { req: {"service":"liveStream","action":"get"}, res:getEntryData},
    { req: {"service":"liveStream","action":"authenticate"}, res:getEntryData},
    { req: {"service":"conversionProfile","action":"get"}, res:conversionProfileTemplate},
    {
        req: {
            "service": "multirequest",
            "0:service": "flavorParams",
            "0:action": "get"
        },
        res: flavorParams
    },
    {
        req: {"service":"multirequest","0:service":"liveStream","0:action":"get","1:service":"conversionProfile"},
        res:(req)=> {
            return [getEntryData(req),conversionProfileTemplate];
        }
    },
    {
        req: {"service":"multirequest","0:service":"liveStream","0:action":"registerMediaServer","1:action":"list","2:action":"update"},
        res:(req)=> {
            return [getEntryData(req),getEntryServerNode(req),{}]
        }
    },
    { req: {"service":"liveStream","action":"registerMediaServer"}, res:getEntryData},
    { req: {"service":"liveStream","action":"unregisterMediaServer"}, res:getEntryData},


];


function match(body:any,req:any) {
        for ( let [key,value] of Object.entries(req)) {
                if (body[key]!=value) {
                        return null;
                }
        }
        return body;
}


export function loadKalturaDALMock() {


    Network.addMock("http://www.kaltura.com/.*",(method,r,body)=> {

        for (let response of responses) {
            delete body.ks;
            let m=match(body,response.req);
            if (m) {
                return {
                        "headers": {
                            "x-me": "pa-front-api2",
                            "x-kaltura-session": "540408006"
                        },
                        "body": response.res instanceof Function  ? response.res(body) : response.res
                }
            }
        }
        console.warn("coudln't match API request %j",body)
        return {};
    })




}