const nock = require('nock')



export function loadKalturaDALMock() {

    nock('https://www.kaltura.com:443', {"encodedQueryParams":true})
        .post('/api_v3/index.php', {"format":1,"ks":/.*/,"service":"liveStream","action":"get","entryId":"1_rdwhia6e"})
        .reply(200, {"bitrates":[{"bitrate":900,"width":640,"height":480,"tags":"source,ingest,web,mobile,ipad,ipadnew","objectType":"KalturaLiveStreamBitrate"}],"primaryBroadcastingUrl":"rtmp://1_rdwhia6e.p.kpublish.kaltura.com:1935/kLive/?p=1802381&e=1_rdwhia6e&i=0&t=af109cce","secondaryBroadcastingUrl":"rtmp://1_rdwhia6e.b.kpublish.kaltura.com:1935/kLive/?p=1802381&e=1_rdwhia6e&i=1&t=af109cce","primaryRtspBroadcastingUrl":"rtsp://1_rdwhia6e.p.s.kpublish.kaltura.com:554/kLive/1_rdwhia6e_%i/?p=1802381&e=1_rdwhia6e&i=0&t=af109cce","secondaryRtspBroadcastingUrl":"rtsp://1_rdwhia6e.b.s.kpublish.kaltura.com:554/kLive/1_rdwhia6e_%i/?p=1802381&e=1_rdwhia6e&i=1&t=af109cce","streamName":"1_rdwhia6e_%i","streamPassword":"af109cce","primaryServerNodeId":14001,"recordStatus":1,"dvrStatus":0,"dvrWindow":1440,"lastElapsedRecordingTime":0,"liveStreamConfigurations":[{"protocol":"hds","url":"http://cdnapi.kaltura.com/p/1802381/sp/180238100/playManifest/entryId/1_rdwhia6e/protocol/http/format/hds/a.f4m","objectType":"KalturaLiveStreamConfiguration"},{"protocol":"hls","url":"http://cdnapi.kaltura.com/p/1802381/sp/180238100/playManifest/entryId/1_rdwhia6e/protocol/http/format/applehttp/a.m3u8","objectType":"KalturaLiveStreamConfiguration"},{"protocol":"applehttp","url":"http://cdnapi.kaltura.com/p/1802381/sp/180238100/playManifest/entryId/1_rdwhia6e/protocol/http/format/applehttp/a.m3u8","objectType":"KalturaLiveStreamConfiguration"},{"protocol":"applehttp_to_mc","url":"http://cdnapi.kaltura.com/p/1802381/sp/180238100/playManifest/entryId/1_rdwhia6e/protocol/http/format/applehttp_to_mc/a.m3u8","objectType":"KalturaLiveStreamConfiguration"},{"protocol":"sl","url":"http://cdnapi.kaltura.com/p/1802381/sp/180238100/playManifest/entryId/1_rdwhia6e/protocol/http/format/sl/Manifest","objectType":"KalturaLiveStreamConfiguration"},{"protocol":"mpegdash","url":"http://cdnapi.kaltura.com/p/1802381/sp/180238100/playManifest/entryId/1_rdwhia6e/protocol/http/format/mpegdash/manifest.mpd","objectType":"KalturaLiveStreamConfiguration"}],"recordedEntryId":"","publishConfigurations":[],"firstBroadcast":1551864991,"lastBroadcast":1553764172,"currentBroadcastStartTime":0,"recordingOptions":{"shouldCopyEntitlement":true,"shouldMakeHidden":true,"objectType":"KalturaLiveEntryRecordingOptions"},"liveStatus":0,"segmentDuration":6000,"explicitLive":false,"viewMode":1,"recordingStatus":2,"lastBroadcastEndTime":1553770348,"mediaType":201,"conversionQuality":9851841,"sourceType":"32","flavorParamsIds":"32,33,34,35,42,2156182,2156192","plays":4,"views":25,"lastPlayedAt":1551877200,"duration":13085,"msDuration":13085487,"id":"1_rdwhia6e","name":"test for adi","partnerId":1802381,"userId":"webcastdemo@mailinator.com","creatorId":"webcastdemo@mailinator.com","tags":"","adminTags":"kms-webcast-event","status":2,"moderationStatus":6,"moderationCount":0,"type":7,"createdAt":1550663808,"updatedAt":1554383129,"rank":0,"totalRank":0,"votes":0,"downloadUrl":"https://cdnsecakmi.kaltura.com/p/1802381/sp/180238100/raw/entry_id/1_rdwhia6e/version/0","searchText":"_PAR_ONLY_ _1802381_ _MEDIA_TYPE_201|  test for adi kms-webcast-event","licenseType":-1,"version":0,"thumbnailUrl":"https://cfvod.kaltura.com/p/1802381/sp/180238100/thumbnail/entry_id/1_rdwhia6e/version/100001","accessControlId":1742421,"replacementStatus":0,"partnerSortValue":0,"conversionProfileId":9851841,"redirectEntryId":"","rootEntryId":"1_rdwhia6e","operationAttributes":[],"entitledUsersEdit":"","entitledUsersPublish":"","entitledUsersView":"noam.arad@kaltura.com,webcastdemo@mailinator.com","capabilities":"","displayInSearch":1,"objectType":"KalturaLiveStreamEntry"}, [ 'Date',
            'Thu, 04 Apr 2019 17:40:15 GMT',
            'Server',
            'Apache',
            'X-Me',
            'pa-front-api2',
            'Access-Control-Expose-Headers',
            'Server, Content-Length, Content-Range, Date, X-Kaltura, X-Kaltura-Session, X-Me',
            'X-Kaltura',
            'cached-dispatcher,cache_v3-24d7d86995d573edde26fd57e1f805b5,0.00065302848815918',
            'Access-Control-Allow-Origin',
            '*',
            'Expires',
            'Sun, 19 Nov 2000 08:52:00 GMT',
            'Cache-Control',
            'no-store, no-cache, must-revalidate, post-check=0, pre-check=0',
            'Pragma',
            'no-cache',
            'Vary',
            'Accept-Encoding',
            'Content-Length',
            '3742',
            'Keep-Alive',
            'timeout=5, max=100',
            'Connection',
            'Keep-Alive',
            'Content-Type',
            'application/json' ]);





    nock('https://www.kaltura.com:443', {"encodedQueryParams":true})
        .post('/api_v3/index.php', {"format":1,"ks":/.*/,"service":"conversionProfile","action":"get","id":9851841})
        .reply(200, {"id":9851841,"partnerId":1802381,"status":2,"type":2,"name":"Cloud Transcode 4k","systemName":"","tags":"","description":"","createdAt":1535956454,"flavorParamsIds":"32,33,34,35,42,2156182,2156192","isDefault":false,"isPartnerDefault":false,"cropDimensions":{"left":-1,"top":-1,"width":-1,"height":-1,"objectType":"KalturaCropDimensions"},"clipStart":-1,"clipDuration":-1,"mediaParserType":0,"calculateComplexity":true,"collectionTags":"mbr,ism","detectGOP":0,"objectType":"KalturaConversionProfile"}, [ 'Date',
            'Thu, 04 Apr 2019 17:40:15 GMT',
            'Server',
            'Apache',
            'X-Me',
            'pa-front-api2',
            'Access-Control-Expose-Headers',
            'Server, Content-Length, Content-Range, Date, X-Kaltura, X-Kaltura-Session, X-Me',
            'X-Kaltura-Session',
            '540408006',
            'Access-Control-Allow-Origin',
            '*',
            'Expires',
            'Sun, 19 Nov 2000 08:52:00 GMT',
            'Cache-Control',
            'no-store, no-cache, must-revalidate, post-check=0, pre-check=0',
            'Pragma',
            'no-cache',
            'Vary',
            'Accept-Encoding',
            'Content-Length',
            '501',
            'Keep-Alive',
            'timeout=5, max=99',
            'Connection',
            'Keep-Alive',
            'Content-Type',
            'application/json' ]);



    nock('https://www.kaltura.com:443', {"encodedQueryParams":true})
        .post('/api_v3/index.php', {"format":1,"ks":/.*/,"service":"multirequest","0:service":"flavorParams","0:action":"get","0:id":"32","1:service":"flavorParams","1:action":"get","1:id":"33","2:service":"flavorParams","2:action":"get","2:id":"34","3:service":"flavorParams","3:action":"get","3:id":"35","4:service":"flavorParams","4:action":"get","4:id":"42","5:service":"flavorParams","5:action":"get","5:id":"2156182","6:service":"flavorParams","6:action":"get","6:id":"2156192"})
        .reply(200, [{"streamSuffix":"1","videoCodec":"","videoBitrate":900,"audioCodec":"","audioBitrate":0,"audioChannels":0,"audioSampleRate":0,"width":640,"height":480,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"flv","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":"0","videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":32,"partnerId":0,"name":"Source","systemName":"Source","description":"Maintains the original format and settings of the input stream","createdAt":1388310390,"isSystemDefault":1,"tags":"source,ingest,web,mobile,ipad,ipadnew","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264b","videoBitrate":400,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":480,"height":0,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"mp4","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":33,"partnerId":0,"name":"Basic/Small - WEB/MBL (H264/400)","systemName":"Basic_Small_400","description":"Basic/Small - WEB/MBL (H264/400)","createdAt":1388310390,"isSystemDefault":1,"tags":"mbr,ipadnew,iphonenew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264b","videoBitrate":600,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":640,"height":0,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"mp4","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":34,"partnerId":0,"name":"Basic/Small - WEB/MBL (H264/600)","systemName":"Basic_Small_600","description":"Basic/Small - WEB/MBL (H264/600)","createdAt":1388310390,"isSystemDefault":1,"tags":"mbr,ipadnew,iphonenew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264m","videoBitrate":900,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":640,"height":0,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"mp4","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":35,"partnerId":0,"name":"SD/Small - WEB/MBL (H264/900)","systemName":"SD_Small_900","description":"SD/Small - WEB/MBL (H264)","createdAt":1388310390,"isSystemDefault":1,"tags":"mbr,ipadnew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264h","videoBitrate":1700,"audioCodec":"aac","audioBitrate":128,"audioChannels":2,"audioSampleRate":44100,"width":0,"height":720,"frameRate":0,"gopSize":0,"conversionEngines":"","conversionEnginesExtraParams":"{ \"constantBitrate\" : true }","twoPass":false,"deinterlice":0,"rotate":0,"operators":"","engineVersion":0,"format":"mp4","aspectRatioProcessingMode":"0","forceFrameToMultiplication16":"1","isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"multiStream":"","anamorphicPixels":"0","isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"watermarkData":"","subtitlesData":"","isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":42,"partnerId":0,"name":"HD/720 - WEB/MBL (H264/1700)","systemName":"HD/720 - WEB/MBL (H264/1700)","description":"HD/720 - WEB/MBL (H264/1700)","createdAt":1479642661,"isSystemDefault":1,"tags":"mbr,ipadnew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_HD_FLAVORS","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"sourceAssetParamsIds":"","objectType":"KalturaLiveParams"},{"videoCodec":"h264h","videoBitrate":3500,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":0,"height":1080,"frameRate":0,"gopSize":0,"conversionEnginesExtraParams":"{ \"constantBitrate\" : true }","twoPass":false,"deinterlice":0,"rotate":0,"format":"mp4","aspectRatioProcessingMode":0,"forceFrameToMultiplication16":1,"isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"anamorphicPixels":0,"isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":2156182,"partnerId":1802381,"name":"UHD/1080","systemName":"UHD/1080","description":"","createdAt":1535955455,"isSystemDefault":0,"tags":"mbr,ipadnew,iphonenew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"objectType":"KalturaLiveParams"},{"videoCodec":"h264h","videoBitrate":6500,"audioCodec":"aac","audioBitrate":64,"audioChannels":2,"audioSampleRate":44100,"width":0,"height":1440,"frameRate":0,"gopSize":0,"conversionEnginesExtraParams":"{ \"constantBitrate\" : true }","twoPass":false,"deinterlice":0,"rotate":0,"format":"mp4","aspectRatioProcessingMode":0,"forceFrameToMultiplication16":1,"isGopInSec":0,"isAvoidVideoShrinkFramesizeToSource":0,"isAvoidVideoShrinkBitrateToSource":0,"isVideoFrameRateForLowBrAppleHls":0,"anamorphicPixels":0,"isAvoidForcedKeyFrames":0,"forcedKeyFramesMode":1,"isCropIMX":1,"optimizationPolicy":1,"maxFrameRate":30,"videoConstantBitrate":0,"videoBitrateTolerance":0,"isEncrypted":0,"contentAwareness":0.5,"chunkedEncodeMode":0,"id":2156192,"partnerId":1802381,"name":"UHD/1440","systemName":"UHD/1440","description":"","createdAt":1535955467,"isSystemDefault":0,"tags":"mbr,ipadnew,iphonenew,web,mobile,optional_flavor","requiredPermissions":[{"value":"FEATURE_LIVE_STREAM","objectType":"KalturaString"},{"value":"FEATURE_KALTURA_LIVE_STREAM_TRANSCODE","objectType":"KalturaString"}],"sourceRemoteStorageProfileId":0,"mediaParserType":0,"objectType":"KalturaLiveParams"}], [ 'Date',
            'Thu, 04 Apr 2019 17:40:16 GMT',
            'Server',
            'Apache',
            'X-Me',
            'pa-front-api2',
            'Access-Control-Expose-Headers',
            'Server, Content-Length, Content-Range, Date, X-Kaltura, X-Kaltura-Session, X-Me',
            'X-Kaltura',
            'cached-dispatcher,cache_v3-229eb5f1302c2f71942584f72ff6a967,0.00054502487182617',
            'Access-Control-Allow-Origin',
            '*',
            'Expires',
            'Sun, 19 Nov 2000 08:52:00 GMT',
            'Cache-Control',
            'no-store, no-cache, must-revalidate, post-check=0, pre-check=0',
            'Pragma',
            'no-cache',
            'Vary',
            'Accept-Encoding',
            'Keep-Alive',
            'timeout=5, max=98',
            'Connection',
            'Keep-Alive',
            'Transfer-Encoding',
            'chunked',
            'Content-Type',
            'application/json' ]);
}