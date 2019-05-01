import { Network } from "../utils";
import {Logger} from "../logger";

class SystemObjectStatus {
    state:string;
}

export enum SystemObjectState {
    Pending,
    Ready,
    Working,
    Completed
}

export class SystemObject {

    id:string;
    state:SystemObjectState;
    baseUrl:string;
    logger: Logger =new Logger("Packager");

    async isReady(): Promise<boolean> {
        let status=await this.getStatus();
        return Promise.resolve(status.state=="ready");
    }
    protected async getStatus(): Promise<SystemObjectStatus> {
        return await Network.get({ url: `${this.baseUrl}/status` }) as SystemObjectStatus;
    }
}
