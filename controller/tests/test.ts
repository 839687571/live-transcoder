import {getLogger} from 'log4js';
import {Application} from "../src/app"
import {Network} from '../src/utils'
import {RenditionTrackType} from "../src/interfaces/transcodingProfile";
import {SetupRequest} from "../src/BusinessLogic";

const logger = getLogger("test");

async function test() {

 /*
    let dal:IDAL=new KalturaDAL();
    let entry:EntryInfo=await dal.getEntryInfo("1_rdwhia6e");
    let tp:TranscodingProfile=await dal.getTranscodingProfile(entry);

    let z1=tp.translate("video")
    let z2=tp.translate("audio")
    console.warn("%j %j",z1,z2);

    //let k:IRegistry=new KubernetesRegistry();

    //let x= await k.getPackagers();
   // let x=await  k.runTranscoder("1",null)

    return;*/
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

