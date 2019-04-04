import * as request from "request-promise-native";
import {SystemObject} from "./SystemObject";

export class Source extends SystemObject{

    setup():Promise<boolean> {
        return Promise.resolve(true);
    }
}
