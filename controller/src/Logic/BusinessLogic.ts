import {ChannelSessionType, ICache, IChannel, IDAL, IRegistry} from '../interfaces/interfaces'
import {KalturaDAL} from "../Data/KalturaDAL";
import {getCache, getRegistry} from "../Registries/RegistryFactory";
import {Packager} from "../SystemObjects/Packager";
import {Transcoder} from "../SystemObjects/Transcoder";
import {CallContext, retry, timePassed, Timer} from "../utils";
import {TrackType} from "../interfaces/channelOutput";
import {Logger} from "../logger";
import {InputConfiguration} from "../interfaces/InputConfiguration";
import * as config from "config";
import {ChannelStarter} from "./channelStarter";
import {ChannelStopper} from "./channelStopper";

export class BaseRequest
{
    channelId: string;
    inputId: string;
    trackType:TrackType;
}

export class AuthenticateRequest
{
    channelId: string;
    token: string;
    type:ChannelSessionType;
}


export class AuthenticateResponse
{
    reason: string;
}

export class SetupRequest extends  BaseRequest
{
    bitrate:number;
    codec:string;
    videoWidth:number;
    videoHeight:number;
    frameRate:number;
    audioSamplingRate:number;
}

export class SetupResponse
{
    url:string;
    trackId:string;
}


export class TearDownRequest extends  BaseRequest
{

}

export class TearDownResponse
{
}

export class BusinessLogic {

    private registry: IRegistry;
    private cache: ICache;
    private logger: Logger = new Logger("BusinessLogic");

    private channelStarter:ChannelStarter;
    private channelStopper:ChannelStopper;
    private data: IDAL = null;

    private timer: Timer;


    constructor() {
        this.registry = getRegistry();
        this.cache = getCache();
    }

    async authenticate(context: CallContext, req: AuthenticateRequest): Promise<AuthenticateResponse> {
        context.logger.info("[%s] authenticate %j", context.id, req);
        let res: AuthenticateResponse = new AuthenticateResponse();
        let channel: IChannel = this.data.getChannel(req.channelId);
        //channel.type = req.type;
        await channel.authenticate(req.token);
        return res;
    }

    async setup(context: CallContext, req: SetupRequest): Promise<SetupResponse> {
        let t0=new Date();
        context.logger.info("Receive setup request %j", req);
        let packager: Packager = null;
        let transcoder: Transcoder = null;
        let inputConfig: InputConfiguration = new InputConfiguration();
        let res: SetupResponse = new SetupResponse();
        try {

            context.logger.info("locking channel");

            await this.cache.lock(req.channelId,context.logger);
            context.logger.info("Get channel");
            let channel: IChannel = this.data.getChannel(req.channelId);
            await channel.retrieveChannelMetadata();
            let variantsWhiteList=req.trackType==TrackType.Audio ?
                channel.actualOutput.variants.filter( variant=>variant.inputId==req.inputId).map( variant=>variant.id)
                : [];
            inputConfig.setup(channel.transcodingProfile, req,variantsWhiteList);
            channel.addOutputs(inputConfig.getChannelOutput());

            context.logger.info("got channelInfo %j and transcoding profile %j ", channel.channelInfo, channel.transcodingProfile);
            //get transcoding profile

            packager = await this.assignPackager(context, req, inputConfig);
            this.channelStarter.start(channel,packager);
            this.channelStopper.cancel(channel);
        }
        catch (e) {
            context.logger.warn("exception in setup: ", e);
        }
        finally {
            context.logger.info("assigning packager completed, unlocking channel");
            await this.cache.unlock(req.channelId,context.logger);
        }

        if (!inputConfig.isPassthrough) {
            //in case of cloud transcode, we need to run transcoder
            transcoder = await this.runTranscoder(context, req, inputConfig, packager);
            res.trackId=`${(req.trackType==TrackType.Video) ? "v":"a"}${req.inputId}`;
        } else {
            //in case of cloud passthrough, just return the packager url
            res.url = packager.getKMPEndpoint();
            res.trackId = inputConfig.getPassthroughTrackId();
        }
        return res
    }

    async teardown(context: CallContext, req: TearDownRequest) {
        try {
            context.logger.info("Receive teardown request %j", req);
            await this.cache.lock(req.channelId,context.logger);
            let channel: IChannel = this.data.getChannel(req.channelId);
            channel.removeInput(req.inputId, req.trackType);
            if (channel.actualOutput.variants.length==0) {
                this.channelStarter.cancel(channel);
                this.channelStopper.stop(channel);
            }
            let res: TearDownResponse = new TearDownResponse();
            return res
        }
        finally {
            context.logger.info("Unlocking ");
            await this.cache.unlock(req.channelId,context.logger);
        }
        return true
    }

    private async assignPackager(context: CallContext, req: SetupRequest, inputConfig: InputConfiguration): Promise<Packager> {
        context.logger.info("finding packager");
        return await retry(async () => {

            let packager: Packager = await this.registry.getPackagerByChannelId(req.channelId);
            if (!packager) {

                context.logger.info("no packager is assigned: ");
                let packagers: Array<Packager> = await this.registry.getPackagers();

                if (packagers.length == 0) {
                    throw new Error("No packager available!")
                }
                packager=this.findAvailablePackager(packagers);

                context.logger.info("selected packager id=", packager.id);

            } else {
                context.logger.info("already found packager for channelId: ", req.channelId);
            }

            if (!await packager.isReady()) {
                context.logger.warn(`Packager id = ${packager.id} not ready`);
                return null;
            }

            let t1:Date=new Date();
            context.logger.info("setup packager");
            await packager.setup(inputConfig);
            context.logger.info("packager has being initialized successfully is %d ms",timePassed(t1));

            await this.registry.associateChannelIdAndPackager(req.channelId, packager.id);

            context.logger.info("packager assignment completed");
            return packager;

        }, config.get("packagerAssignment.retries"), config.get("packagerAssignment.interval"), 0,context.logger); //wait 1 second

    }

    private findAvailablePackager(packagers:Array<Packager>):Packager {

        //TODO : find best packager
        let index = Math.round(Math.random() * (packagers.length - 1));
        return packagers[index];
    }


    private async runTranscoder(context: CallContext, req: SetupRequest, inputConfig: InputConfiguration, packager: Packager): Promise<Transcoder> {
        context.logger.info("spawning a transcoder for %j", req);


        let transcodingInstructions: any = inputConfig.getTranscodingInstructions();
        transcodingInstructions.actualOutput = {
            "streamingUrl": packager.getKMPEndpoint()
        };

        let args: string[] = ['-f', JSON.stringify(transcodingInstructions)];
        let transcoder: Transcoder = await this.registry.runTranscoder(req.channelId, req.inputId, req.trackType, args);

        return transcoder;
    }


    async initialize():Promise<boolean> {
        this.logger.info("initialization started");

        this.data = new KalturaDAL(new Logger("DAL"));
        await this.data.initialize();



        this.channelStarter=new ChannelStarter(this.data);
        this.channelStopper=new ChannelStopper(this.data);

        this.timer = new Timer(this.alignState.bind(this));
        await this.alignState();
        this.timer.start(false,60000); 1

        this.logger.info("initialization completed");

        return true;
    }

    async alignState() {

        this.logger.info("checkState started");
/*
        let sources=await this.registry.getSources();
        let sourceState=await Promise.all(sources.map( source=>source.getChannels().catch( (err)=> {
            return Promise.resolve({})
        }) ));*/
        let packagers=await this.registry.getPackagers();
        let packagerState=await Promise.all(packagers.map( packager=>packager.getChannels().catch( (err)=> {
            return Promise.resolve({})
        }) ));


        this.data.alignState(packagerState);


        this.logger.info("checkState completed");
    }


}

