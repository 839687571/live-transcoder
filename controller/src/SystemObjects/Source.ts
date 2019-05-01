import * as request from "request-promise-native";
import {SystemObject} from "./SystemObject";

export class Source extends SystemObject{

    getChannels():Promise<any> {
        return Promise.resolve({})
    }
}
