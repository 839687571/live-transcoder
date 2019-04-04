import { Network } from "../utils";

class SystemObjectStatus {
    state:string;
}

export enum SystemObjectState {
    Pending,
    Ready,
    Working,
    Completed
}

export abstract class SystemObject {

    id:string;
    state:SystemObjectState;
    baseUrl:string;

    abstract setup():Promise<boolean>

    async isReady(): Promise<boolean> {
        let status=await this.getStatus();
        return Promise.resolve(status.state=="ready");
    }
    protected async getStatus(): Promise<SystemObjectStatus> {
        return await Network.get({ url: `${this.baseUrl}/status` }) as SystemObjectStatus;
    }
}
