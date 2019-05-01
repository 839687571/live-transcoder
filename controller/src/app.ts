import {
    AuthenticateRequest, AuthenticateResponse,
    BusinessLogic,
    SetupRequest,
    SetupResponse,
    TearDownRequest,
    TearDownResponse
} from './Logic/BusinessLogic'

import * as config from 'config';

import {Logger} from './logger';

import * as express from 'express'
import {CallContext, timePassed} from "./utils";
import {rtmpRequstBodyToSetupRequest, rtmpUrlToAuthenticateRequest} from "./rtmpUtils";
import {TrackType} from "./interfaces/channelOutput";
import {initializeRegistry} from "./Registries/RegistryFactory";

const  bodyParser = require('body-parser')

let logger:Logger = new Logger("")


process.on('uncaughtException', function (err: Error) {
    try {
        logger.error("uncaughtException",err);
    } catch (err) {

    }
});
process.on('unhandledRejection', function (err: Error) {
    try {
        logger.error("unhandledRejection",err);
    } catch (err) {

    }
});

const asyncMiddleware = fn =>
    async (req, res, next) => {
        let t0:Date = new Date();
        let callContext = new CallContext(`[${req.originalUrl.substr(1)}]`);
        res.header('X-Live-Controller-Session-Id' , callContext.id);

        try {
            await fn(callContext, req, res, next);
        }catch(err) {
            res.status(200);
            res.send(err);
            logger.error("Exception while processing ",err);

        }
        callContext.logger.info("Completed in %d ms",timePassed(t0))
    }



export class Application {

    app: any;
    server:any;
    bl: BusinessLogic;
    ready: Promise<any>;

    static instance:Application=new Application();

    constructor() {
        this.app = express();
        this.app.use(bodyParser.json());
        this.app.use(bodyParser.urlencoded({extended: false}));
        this.register();
        this.app.use((err, req, res, next)=> {
            if (err!=null) {
                res.status(200);
                res.send(err);
                logger.error(err);
                return
            }
            next();

        });
    }


    private register() {

        this.app.post('/authenticateRTMP', asyncMiddleware(async (callContext, req, res) => {
            let authRequest=rtmpUrlToAuthenticateRequest(req.body);
            callContext.logger.prefix+=`[${authRequest.channelId}]`;
            let result: AuthenticateResponse = await this.bl.authenticate(callContext,authRequest);
            res.send(result);
        }));

        this.app.post('/authenticate', asyncMiddleware(async (callContext, req, res) => {
            let authRequest = req.body as AuthenticateRequest;
            callContext.logger.prefix+=`[${authRequest.channelId}]`;
            let result: AuthenticateResponse = await this.bl.authenticate(callContext,authRequest);
            res.send(result);
        }));


        this.app.post('/setupRTMP', asyncMiddleware(async (callContext, req, res) => {

            //let args:string=req.body.args;
            let setupRequest:SetupRequest=rtmpRequstBodyToSetupRequest(req.body);
            callContext.logger.prefix+=`[${setupRequest.channelId}][${setupRequest.inputId}][${setupRequest.trackType==TrackType.Video
             ? "V":"A"}]`;

            let result: SetupResponse = await this.bl.setup(callContext,setupRequest);

            res.send(result);
        }));


        this.app.post('/setup', asyncMiddleware(async (callContext, req, res) => {
            let setupRequest = req.body as SetupRequest;
            callContext.logger.prefix+=`[${setupRequest.channelId}][${setupRequest.inputId}][${setupRequest.trackType==TrackType.Video
                ? "V":"A"}]`;
            let result: SetupResponse = await this.bl.setup(callContext,setupRequest);

            res.send(result);
        }));

        this.app.post('/teardown', asyncMiddleware(async (callContext, req, res) => {
            let teardownRequest = req.body as TearDownRequest;
            callContext.logger.prefix+=`[${teardownRequest.channelId}][${teardownRequest.inputId}][${teardownRequest.trackType==TrackType.Video
                ? "V":"A"}]`;
            let result: TearDownResponse = await this.bl.teardown(callContext,teardownRequest);
            res.send(result);
        }));

        this.app.post('/reportAlert', asyncMiddleware(async (req, res) => {
            res.send("{}");
        }));
    }

    private listen() {
        return new Promise( (resolve,reject)=> {
            this.server = this.app.listen(config.get('port'),  ()=> {
                let host = this.server.address().address;
                let port = this.server.address().port;

                logger.info("Example app listening at http://"+host+":"+port);

                resolve();
            })
        })
    }

    start() {
        this.ready=new Promise( async (resolve,reject)=> {
            try {
                await initializeRegistry();
                this.bl = new BusinessLogic();
                await this.bl.initialize();
                await this.listen();
                resolve();
            }catch(err){
                logger.error("Cannot start instance ",err)
            }
        });
    }
}

Application.instance.start();
