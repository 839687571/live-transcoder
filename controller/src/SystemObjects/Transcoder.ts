import {SystemObject} from "./SystemObject";


export class Transcoder extends  SystemObject{


    getKMPEndpoint():string{
        return `kmp://${this.baseUrl}:10000`;
    }
}
