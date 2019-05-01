import {getLogger} from 'log4js';
import {Application} from "../src/app"
import {Network, sleep} from '../src/utils'
import {loadKalturaDALMock} from "./kalturaDALMock";

const logger = getLogger("test");


Network.addMock("http://localhost:9001/status", { "state": "ready"})
Network.addMock("http://localhost:9002/status", { "state": "ready"})
Network.addMock("http://localhost:19001/status", { "state": "ready"})
Network.addMock("http://localhost/status", { "state": "ready"})

let dummy = {

};


function getVariant(variants:any,id:string) {
    return variants.find( (variant:any)=> variant.id==id);
}


//loadKalturaDALMock();

Network.addMock("http://localhost:9001/API/sets(/([^/]+))?",(method,r,body)=> {
    if (body) {
        if (!dummy[body.id]) {
            dummy[body.id] = {
                id: body.id,
                variants: []
            }
        }
        return dummy[body.id];
    }

    if (r[2]) {

        return dummy[r[2]];
    }

    return dummy;
})

Network.addMock("http://localhost:9001/API/control/sets/(.*)/variants", (method,r,body)=> {
    let channelId = r[1];
    let channel:any = dummy[channelId];
    if (!channel) {
        return null;
    }
    if (method == "GET") {
        return channel.variants;
    }
    if (method == "POST") {

        let variant:any=getVariant(channel.variants,body.id);
        if (!variant) {
            variant={ id: body.id, tracks: []};
            channel.variants.push(variant);
        }
        return variant;
    }
});

Network.addMock("http://localhost:9001/API/control/sets/(.*)/variants/(.*)/tracks", (method,r,body)=> {

    let channelId=r[1];
    let variantId=r[2];
    let channel=dummy[channelId];
    if (!channel) {
        return null;
    }
    let variant=getVariant(channel.variants,variantId);
    if (!variant) {
        return null;
    }
    variant.tracks.push(body);
    return variant.tracks;
});



async function test() {
    //let k:IRegistry=new DistributedRegistry();

    //let x= await k.getPackagers();
   // let x=await  k.runTranscoder("1",null)
    try {
       // let x=await request.post({ url: 'http://localhost:8084/authenticate', json: { channelId: "1_6m5ls6eh", "token": "9f3f44cd", "type": 1} })

        //let req=new SetupRequest()Sending request to kaltura server:
        //req.channelId="1_rdwhia6e";
        //req.trackType=TrackType.Video;
        //req.inputId="1";


        let entries=["1_rdwhia6e"]


        let sync:boolean=false;

        entries=["0_gfh4ovbm","1_a5me48cv","0_r16j4clv","0_sko1s6pn","1_489ujaey","1_qtl1ai8f","0_q5rtxmrf","0_w3oqh7oh","1_osx6xiyo","0_tqf481q7","1_4pspwwwd","1_s64agfrm","1_do4yf1fn","1_x0uykxtk","0_qjuayau0","1_7fxyick4","1_ehak6050","1_g1llv1pj","1_ugz4mnx3","1_vtm7qm4r","0_6xf2ekk3","1_jxbdeae2","0_xrkf5jgp","0_vit49e0a","1_j9cbbpz8","0_c5x52cb6","1_p2ehvhjr","0_bpkec72l","0_147dj7zj","1_upxl3jw1","1_x0v2hubo","0_m8aips09","0_1w52xhah","0_go9mwxno","1_dc5tjyo8","1_agwp79hb","1_dr43f9vb","0_q8afixsm","1_tf9omyqj","0_s6lsaje9","0_ytk8rhtc","0_3k8ro28c","1_n12f94mc","1_q4xxlvrg","1_d1cziizq","0_88phs8hc","0_gub19d5k"];
        for (let entry of entries) {
            let req0={"channelId": entry, "token":"af109cce"};
            let p0= Network.post({ url: 'http://localhost:8084/authenticate', json: req0, resolveWithFullResponse:true }).then ( async(res)=>{

                let req1 = {
                    "name": entry + "_1",
                    "type": "live",
                    "args": "videoKeyframeFrequency=5&totalDatarate=248",
                    "media_type": "video",
                    "width": 1920,
                    "height": 1080,
                    "duration": 0,
                    "frame_rate": 0.00,
                    "video_data_rate": 12000,
                    "video_codec_id": 7,
                    "avc_profile": 66,
                    "avc_compat": 0,
                    "avc_level": 31,
                    "avc_nal_bytes": 4,
                    "avc_ref_frames": 3
                }

                let p1 =  Network.post({
                    url: 'http://localhost:8084/setupRTMP',
                    json: req1,
                    resolveWithFullResponse: true
                });
                if (sync) {
                    let res=await p1;
                    logger.warn("response: %j %j", res.headers["x-live-controller-session-id"], res.body);
                    logger.warn(JSON.stringify(dummy, null, "\t"));
                }


                let req2 = {
                    "name": entry + "_1",
                    "type": "live",
                    "media_type": "audio",
                    "audio_codec_id": 1
                }
                let p2 =  Network.post({
                    url: 'http://localhost:8084/setupRTMP',
                    json: req2,
                    resolveWithFullResponse: true
                });

                if (sync) {
                    let res=await p2;
                    logger.warn("response: %j %j", res.headers["x-live-controller-session-id"], res.body);
                    logger.warn(JSON.stringify(dummy, null, "\t"));
                }
            })
            if (sync) {
                await p0;
            }


        }



        await sleep(10000);

        let req3={"channelId": "1_rdwhia6e", "inputId":"1", "trackType": 0};
        await Network.post({ url: 'http://localhost:8084/teardown', json: req3, resolveWithFullResponse:true });
        let req4={"channelId": "1_rdwhia6e", "inputId":"1", "trackType": 1};
        await Network.post({ url: 'http://localhost:8084/teardown', json: req4, resolveWithFullResponse:true });

        return;
        //let x = await dal.getEntryInfo("1_6m5ls6eh")
        //let x= await dal. getTranscodingProfile("6028001")

        await sleep(10000);

    }catch(e) {
        console.warn(e);
        logger.warn("Error: ",e);
    }

}

Application.instance.ready.then ( async ()=> {
    test();
});

