import {
    TrackType,
    ChannelOutput,
} from "../src/interfaces/channelOutput";

import {SetupRequest} from "../src/Logic/BusinessLogic";
import { IChannel, IDAL} from "../src/interfaces/interfaces";
import {KalturaDAL} from "../src/Data/KalturaDAL";
import {loadKalturaDALMock} from './kalturaDALMock';

import {Logger} from "../src/logger";
import {InputConfiguration} from "../src/interfaces/InputConfiguration";


function checkFlavors(obj:any,flavors:Array<string>) {
    let ids=obj.outputTracks.map( (output:any)=> {
        return output.trackId+"";
    })
    ids.sort()
    flavors.sort();
    expect(ids).toEqual(flavors);

}

function runFlavorTest(tp:ChannelOutput, s:SetupRequest, expected:Array<string>) {

    let z:InputConfiguration=new InputConfiguration();
    z.setup(tp,s);
    let r1=z.getTranscodingInstructions();
    let r2=z.getChannelOutput();

    //console.warn(JSON.stringify(r1,null,"\t"),JSON.stringify(r2,null,"\t"));

    checkFlavors(r1,expected);
}

let dal: IDAL=null;
let channel: IChannel = null;

describe('test transcoding profile', () => {

    beforeAll(async(done) => {

        loadKalturaDALMock();

        dal = new KalturaDAL(new Logger("test"));
        channel = await dal.getChannel("1_rdwhia6e");
        await channel.retrieveChannelMetadata();

        done();
    });

    test('transcoding profile - video track', () => {
        let s1: SetupRequest = new SetupRequest();
        s1.channelId = "1_rdwhia6e"
        s1.inputId = "1";
        s1.trackType = TrackType.Video;
        s1.videoHeight = 1080;
        s1.bitrate = 12000;
        s1.codec = "h264";

        runFlavorTest(channel.transcodingProfile,s1,["v2156182","v32","v33","v34","v35","v42"]);
    });

    test('transcoding profile - check limit flavor because of low bitrate', () => {
        let s1 = new SetupRequest();
        s1.channelId = "1_rdwhia6e"
        s1.inputId = "1";
        s1.trackType = TrackType.Video;
        s1.videoHeight = 640;
        s1.bitrate = 200;
        s1.codec = "h264";
        runFlavorTest(channel.transcodingProfile,s1,["v32"]);
    });

    test('transcoding profile - check limit flavor because of low resolution', () => {
        let s1: SetupRequest = new SetupRequest();
        s1.channelId = "1_rdwhia6e"
        s1.inputId = "1";
        s1.trackType = TrackType.Video;
        s1.videoHeight = 640;
        s1.bitrate = 12000;
        s1.codec = "h264";

        runFlavorTest(channel.transcodingProfile,s1,["v32","v33","v34","v35"]);
    });

    test('transcoding profile - audio track', () => {
        let s1= new SetupRequest();
        s1.channelId = "1_rdwhia6e"
        s1.inputId = "1";
        s1.trackType = TrackType.Audio;
        s1.bitrate = 128;
        s1.codec = "aac";
        runFlavorTest(channel.transcodingProfile,s1,["a2156182","a2156192","a32","a33","a34","a35","a42"]);
    });


}

