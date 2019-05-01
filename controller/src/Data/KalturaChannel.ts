import {addSeconds, hasExpired, SynchronizedPromises} from "../utils";
import {KalturaDAL, ChannelState} from "./KalturaDAL";
import {ChannelInfo, IChannel, ChannelSessionType} from "../interfaces/interfaces";
import {Logger} from "../logger";
import {ChannelOutput, TrackType} from "../interfaces/channelOutput";
import * as config from "config";
import {EventEmitter} from "events";

const lastAuthenticationTimeout:number=config.get("channel.authenticationTimeout");

export class KalturaChannel extends EventEmitter implements IChannel  {

    sync:SynchronizedPromises;
    id:string;
    token:string;
    authenticationExpiry:Date;

    logger:Logger=null;
    state:ChannelState = ChannelState.Stopped;
    dal:KalturaDAL;

    _channelInfo:ChannelInfo;
    _entryServerNode:any;
    transcodingProfile:ChannelOutput;
    type:ChannelSessionType;
    lastModified:Date;
    actualOutput:ChannelOutput=new ChannelOutput();


    constructor(dal:KalturaDAL,id:string) {
        super();
        this.id=id;
        this.dal=dal;
        this.sync=new SynchronizedPromises(null);
        this.logger=new Logger("KalturaChannel",`[${id}]`);
    }

    public addOutputs(output:ChannelOutput) {
        this.lastModified=new Date();
        //first time
        if (this.actualOutput.variants.length==0) {
            this.markBroadcasting();
        }
        this.actualOutput.merge(output);
    }
    public removeInput(inputId:string, type:TrackType) {
        this.actualOutput.removeInputTracks(inputId,type);

        if (this.actualOutput.variants.length==0) {
            this.markSuspended();
        }
    }

    public async retrieveChannelMetadata():Promise<boolean> {

        if (this.channelInfo) {
            this.transcodingProfile = await this.dal.getTranscodingProfile(this.channelInfo);
        } else {
            [this.channelInfo, this.transcodingProfile] = await this.dal.getChannelInfoAndTranscodingProfile(this.id);
        }
        return true;
    }

    public authenticate(token:string):Promise<boolean>{
        if (this.state==ChannelState.Stopped) {
            this.state=ChannelState.Authenticated;
        }
        if (this.token==token && hasExpired(this.authenticationExpiry)) {
            return Promise.resolve(true);
        }
        return this.sync.exec(async()=> {
            try {
                if (this.token==token && hasExpired(this.authenticationExpiry)) {
                    return Promise.resolve(true);
                }
                this.channelInfo=await this.dal.authenticate(this.id, token, this.type);
                this.authenticationExpiry=addSeconds(new Date(),lastAuthenticationTimeout);
                return true;
            } catch (err) {
                this.token=null;
                this.authenticationExpiry=null;
                this.logger.error("Error while calling authenticate for entry ", err);
                return false;
            }
        });
    }

    public markPlayable(packagerChannelData:any) {
        this.state=ChannelState.Playable;
        this.sync.exec(async()=> {
            try {
                let res=await this.dal.registerMediaServerAndUpdateEntryServerNode( this.id, ChannelState.Playable,this.type,this.entryServerNode,packagerChannelData);
                this.channelInfo=res[0];
                this.entryServerNode=res[1];
                return true;
            } catch (err) {
                this.logger.error("Error while calling Playable for entry ", err);
                return false;
            }
        });
    }
    public get channelInfo() {
        return this._channelInfo;
    }
    public set channelInfo(newChannelInfo:ChannelInfo) {
        let oldValue=this._channelInfo;
        this._channelInfo=newChannelInfo;
        if (!oldValue || oldValue.recordedEntryId!=newChannelInfo.recordedEntryId) {
            super.emit('recordedEntryIdChanged',newChannelInfo.recordedEntryId,oldValue==null);
        }
    }
    public get entryServerNode() {
        return this._entryServerNode;
    }
    public set entryServerNode(entryServerNode:any) {
        this._entryServerNode=entryServerNode;
    }

    private markBroadcasting() {
        if (this.state==ChannelState.Playable) {
            return;
        }
        this.sync.exec(async()=> {
            try {
                this.channelInfo=await this.dal.registerMediaServer(this.id, ChannelState.Broadcasting,this.type);
            } catch (err) {
                this.logger.error("Error while calling Broadcasting for entry. ", err);
            }
        });
        this.state=ChannelState.Broadcasting;
    }
    private markSuspended() {
        this.state=ChannelState.Suspended;
    }

    public stop() {
        this.sync.exec(async()=> {
            try {
                this.channelInfo=await this.dal.registerMediaServer(this.id, ChannelState.Stopped,this.type);
            } catch (err) {
                this.logger.error("Error while calling Stopped for entry. ", err);
            }
        });
        this.state=ChannelState.Stopped;

    }

}