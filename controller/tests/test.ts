import {getLogger} from 'log4js';
import {Application} from "../src/app"
import {Network} from '../src/utils'
import {RenditionTrackType, TranscodingProfile} from "../src/interfaces/transcodingProfile";
import {SetupRequest} from "../src/BusinessLogic";
import {KalturaDAL} from "../src/KalturaDAL";
import {EntryInfo, IDAL} from "../src/interfaces/interfaces";

const logger = getLogger("test");

async function test() {



    let dal:IDAL=new KalturaDAL();
    let entry:EntryInfo=await dal.getEntryInfo("1_rdwhia6e");
    let tp:TranscodingProfile=await dal.getTranscodingProfile(entry);

    let s1:SetupRequest= new SetupRequest();
    s1.setId="1_rdwhia6e"
    s1.variantId="1";
    s1.trackType=RenditionTrackType.Video;
    s1.videoHeight=720;
    s1.bitrate=3000;
    s1.codec="h264";

    let z1=tp.getTranscoderInstructions(s1);

    let s2:SetupRequest= new SetupRequest();
    s2.setId="1_rdwhia6e"
    s2.variantId="1";
    s2.bitrate=128;
    s2.trackType=RenditionTrackType.Audio;
    s2.codec="aac";

    let z2=tp.getTranscoderInstructions(s2);
    console.warn("%j %j",z1,z2);

    //let k:IRegistry=new KubernetesRegistry();

    //let x= await k.getPackagers();
   // let x=await  k.runTranscoder("1",null)

    return;
    Network.addMock("http://localhost:9001/status", { "state": "ready"})
    Network.addMock("http://localhost:9002/status", { "state": "ready"})
    Network.addMock("http://localhost:19001/status", { "state": "ready"})
    Network.addMock("http://localhost:9001/set",{ "state": "ready"})
    Network.addMock("http://10.1.0.20/status",{ "state": "ready"})
    Network.addMock("http://10.1.0.20/set",{ "state": "ready"})

    try {
       // let x=await request.post({ url: 'http://localhost:8084/authenticate', json: { setId: "1_6m5ls6eh", "token": "9f3f44cd", "type": 1} })

        let req=new SetupRequest()
        req.setId="1_rdwhia6e";
        req.trackType=RenditionTrackType.Video;
        req.variantId="1";
        let x=await Network.post({ url: 'http://localhost:8084/setup', json: req })
        logger.warn(x);
        return
        //let x = await dal.getEntryInfo("1_6m5ls6eh")
        //let x= await dal. getTranscodingProfile("6028001")


    }catch(e) {
        logger.warn(e);
    }

}

Application.instance.ready.then ( async ()=> {
    test();
});

