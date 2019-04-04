import { EntryInfo, IDAL, IRegistry, SessionType } from './interfaces/interfaces'
import { KalturaDAL } from "./KalturaDAL";
import { getRegistry } from "./Registries/RegistryFactory";
import { Packager } from "./SystemObjects/Packager";
import {Transcoder} from "./SystemObjects/Transcoder";
import {Locker, retry} from "./utils";
import { getLogger } from 'log4js';
import {RenditionTrackType, TranscodingProfile} from "./interfaces/transcodingProfile";
const logger = getLogger("BusinessLogic");

export class BaseRequest
{
    setId: string;
    variantId: string;
    trackType:RenditionTrackType;
}

export class AuthenticateRequest
{
    setId: string;
    token: string;
    type:SessionType;
}
export class SetupRequest extends  BaseRequest
{
    bitrate:number;
    codec:string;
    videoWidth:number;
    videoHeight:number;
    audioSamplingRate:number;
}

export class SetupResponse
{
    setupRequest: SetupRequest;
    entryInfo: EntryInfo;
    transcodingProfile: TranscodingProfile;
    packager: Packager;
    transcoder: Transcoder;
}


export class TearDownRequest extends  BaseRequest
{

}

export class TearDownResponse
{
}

export class BusinessLogic {

    dal : IDAL;
    registry : IRegistry;
    setIds: Map<string,Packager> = new Map<string,Packager>();

    constructor() {
        this.dal=new KalturaDAL();
        this.registry=getRegistry();
    }

    async authenticate(req:AuthenticateRequest): Promise<any> {
        logger.warn("authenticate %j",req);
        return this.dal.authenticate(req.setId,req.token,req.type);
    }

    async setup( req:SetupRequest): Promise<SetupResponse> {
        try {
            await Locker.lock(req.setId);

           // await this.dal.register(setId)

            let res:SetupResponse = new SetupResponse();
            res.setupRequest=req;
            res.entryInfo = await this.dal.getEntryInfo(req.setId);
            res.transcodingProfile = await this.dal.getTranscodingProfile(res.entryInfo)

            //get transcoding profile

            res.packager=await this.assignPackager(req);


            if (!res.transcodingProfile.isPassthrough(req.trackType)) {
                res.transcoder = await this.runTranscoder(req,res.transcodingProfile);
            }
            return res
        }
        catch(e) {
            logger.warn(e);
        }
        finally
        {
            await Locker.unlock(req.setId);
        }
    }

    private buildTranscodingInstructions(req: SetupRequest): any {

    }

    async teardown( req:TearDownRequest) {
        try {
            await Locker.lock(req.setId);

            // await this.dal.register(setId)

            let res:TearDownResponse = new TearDownResponse();
            return res
        }
        finally
        {
            await Locker.unlock(req.setId);
        }
        return true
    }

    private async assignPackager(req:SetupRequest): Promise<Packager>
    {
        return await retry( async()=> {
            let packager:Packager  = await this.setIds.get(req.setId);
            if (!packager) {

                let packagers:Array<Packager> = await this.registry.getPackagers();
                //TODO : find best packager
                packager = packagers[0];
                let result:any = await packager.setup();

                this.setIds.set(req.setId,packager);
            }

            if (await packager.isReady()) {
                return packager;
            }
            return null;
        },5,1000); //wait 1 second

    }


    private async runTranscoder(req:SetupRequest,profile:TranscodingProfile): Promise<Transcoder> {

        let transcoder:Transcoder = await this.registry.runTranscoder(req.setId,profile);

        return await retry( async()=> {
            if (await transcoder.isReady()) {
                return transcoder;
            }
            return null;
        },10,100)
    }


}

