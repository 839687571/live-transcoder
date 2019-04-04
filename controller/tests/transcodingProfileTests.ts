import {RenditionTrackType, TranscodingProfile} from "../src/interfaces/transcodingProfile";
import {SetupRequest} from "../src/BusinessLogic";
import {EntryInfo, IDAL} from "../src/interfaces/interfaces";
import {KalturaDAL} from "../src/KalturaDAL";
import {Network} from "../src/utils";
import {loadKalturaDALMock} from './kalturaDALMock';

import { configure, getLogger } from 'log4js';
const logger = getLogger();

configure({
    appenders: { 'out': { type: 'stdout' } },
    categories: { default: { appenders: ['out'], level: 'debug' } }
});


async function test() {


    loadKalturaDALMock();

    function checkFlavors(obj:any,flavors:Array<string>) {
        let ids=obj.outputs.map( (output:any)=> {
            return output.id+"";
        })
        ids.sort()
        flavors.sort();
        if (JSON.stringify(ids)==JSON.stringify(flavors)) {
            console.warn("YES");
        }

    }



    try {
        let dal: IDAL = new KalturaDAL();
        let entry: EntryInfo = await dal.getEntryInfo("1_rdwhia6e");
        let tp: TranscodingProfile = await dal.getTranscodingProfile(entry);

        let s1: SetupRequest = new SetupRequest();
        s1.setId = "1_rdwhia6e"
        s1.variantId = "1";
        s1.trackType = RenditionTrackType.Video;
        s1.videoHeight = 640;
        s1.bitrate = 12000;
        s1.codec = "h264";

        let z1 = tp.getTranscoderInstructions(s1);
        checkFlavors(z1,["32","33","34","35"]);

        s1 = new SetupRequest();
        s1.setId = "1_rdwhia6e"
        s1.variantId = "1";
        s1.trackType = RenditionTrackType.Video;
        s1.videoHeight = 640;
        s1.bitrate = 200;
        s1.codec = "h264";
        let z2 = tp.getTranscoderInstructions(s1);

        checkFlavors(z2,["32"]);


        s1= new SetupRequest();
        s1.setId = "1_rdwhia6e"
        s1.variantId = "1";
        s1.trackType = RenditionTrackType.Audio;
        s1.bitrate = 128;
        s1.codec = "aac";

        let z3= tp.getTranscoderInstructions(s1);
        checkFlavors(z1,["32","33","34","35"]);



        console.warn(JSON.stringify(z3,null,"\t"));

    }catch(e){
        console.warn(e);
    }

}


test();