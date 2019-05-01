import {Logger} from "../logger";
import {IChannel, IDAL} from "../interfaces/interfaces";
import {Packager} from "../SystemObjects/Packager";
import {retry} from "../utils";
import * as config from 'config';


export class ChannelStarter {
    private channelsPendingStart:Set<string>= new Set<string>();
    private logger:Logger = new Logger("ChannelStarter");
    private data:IDAL = null;

    constructor(data:IDAL) {
        this.data=data;
    }

    start(channel:IChannel,packager:Packager) {

        if (!this.channelsPendingStart.has(channel.id)) {
            this.logger.info("[%s] Adding start request for channel", channel.id);

            this.channelsPendingStart.add(channel.id);
            retry(async () => {
                    if (!this.channelsPendingStart.has(channel.id)) {
                        this.logger.info("[%s] Canceled start request for channel", channel.id);
                        return true;
                    }

                    this.logger.info("[%s] Check if stream is playable", channel.id);
                    let packagerChannelData: any = await packager.getChannel(channel.id);

                    if (!this.channelsPendingStart.has(channel.id)) {
                        this.logger.info("[%s] Canceled start request for channel", channel.id);
                        return true;
                    }

                    if (packagerChannelData) {
                        this.logger.info("[%s] Channel is playable updating backend %j", channel.id, packagerChannelData);
                        channel.markPlayable(packagerChannelData);
                        this.channelsPendingStart.delete(channel.id);
                        return true;
                    }
                    return false;

                },
                config.get("channelStarter.playableTestWaitTime"),
                config.get("channelStarter.playableTestInterval"),
                config.get("channelStarter.playableTestWaitTime"),
                this.logger);
        }
    }

    cancel(channel:IChannel) {

        if (this.channelsPendingStart.has(channel.id)) {
            this.logger.info("[%s] Canceling start request for channel", channel.id);
            this.channelsPendingStart.delete(channel.id);
        }

    }
}