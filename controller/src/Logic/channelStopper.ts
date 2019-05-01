import {Logger} from "../logger";
import {IChannel, IDAL} from "../interfaces/interfaces";
import * as config from 'config';

export class ChannelStopper {

    private channelsPendingStop:Map<string,any>= new Map<string,any>();
    private logger:Logger = new Logger("ChannelStopper");
    private data:IDAL = null;

    constructor(data:IDAL) {
        this.data=data;
    }

    stop(channel:IChannel) {
        if (this.channelsPendingStop.has(channel.id)) {
            return;
        }
        this.logger.info("[%s] Adding stop request for channel", channel.id);

        this.channelsPendingStop.set(channel.id,setTimeout( ()=>{
            this.logger.info("[%s] Removing channel", channel.id);
            this.data.removeChannel(channel);
        },config.get("channelStopper.stopGracePeriod")));
    }

    cancel(channel:IChannel) {
        if (this.channelsPendingStop.has(channel.id)) {
            this.logger.info("[%s] Cancel stop request for channel", channel.id);
            clearTimeout(this.channelsPendingStop.get(channel.id));
            this.channelsPendingStop.delete(channel.id);
        }
    }

}