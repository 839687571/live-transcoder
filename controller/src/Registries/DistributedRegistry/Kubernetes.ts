import {
    Batch_v1Api,
    Core_v1Api,
    KubeConfig,
    V1Container,
    V1Node,
    V1NodeList,
    V1ObjectMeta,
    V1Pod,
    V1PodList,
    V1PodSpec,
    V1Status
} from "@kubernetes/client-node";
import {Logger} from "../../logger";
import {retry} from "../../utils";
import {Transcoder} from "../../SystemObjects/Transcoder";
import {Packager} from "../../SystemObjects/Packager";
import {SystemObjectState} from "../../SystemObjects/SystemObject";
import {Source} from "../../SystemObjects/Source";
import {randomBytes} from 'crypto';
import {TrackType} from "../../interfaces/channelOutput";

type StringsKeyValue ={[key: string]: string };

export class Kubernetes {

    private k8sApi : any;
    private batchApi: any;
    readonly namespace: string;
    private readonly logger: Logger =new Logger("Kubernetes");

    constructor() {

        const kc = new KubeConfig();
        kc.loadFromDefault();
        this.k8sApi = kc.makeApiClient(Core_v1Api);
        this.batchApi= kc.makeApiClient(Batch_v1Api);
        this.namespace="default";//"kube-system"
    }

    async initialize(): Promise<boolean> {
        await this.cleaner();
        return true;
    }

    private static podToPackager(pod:V1Pod): Packager {
        if ( pod.status.phase!="Running" ) {
            return null;
        }
        let p =new Packager();
        p.baseUrl=`http://${pod.status.podIP}`;
        p.id=pod.metadata.name;

        p.baseUrl=`http://localhost`
        return p
    }

    private static podToTranscoder(pod:V1Pod): Transcoder {
        let p =new Transcoder();
        p.baseUrl=`http://${pod.status.podIP}`;
        p.id=pod.metadata.name;
        p.state= Kubernetes.PodPhaseToSystemObjectState(pod.status.phase);
        return p;
    }

    private static PodPhaseToSystemObjectState(phase:string) : SystemObjectState {
        if (phase==="Pending")
            return SystemObjectState.Pending;
        if (phase==="Running")
            return SystemObjectState.Working;
        if (phase==="Succeeded" || phase==="Failed" || phase==="Completed")
            return SystemObjectState.Completed;

        return SystemObjectState.Completed;
    }

    private static podToSource(pod:V1Pod): Source {
        let p =new Source();
        return p;
    }

    async getPackagers(): Promise<Array<Packager>> {
        let list:V1PodList = await this.listPods("component=packager"); //component=etcd
        return list.items.map(Kubernetes.podToPackager).filter(Boolean);
    }

    async getTranscoders(): Promise<Array<Transcoder>> {
        let list:V1PodList = await this.listPods("component=transcoder");
        return list.items.map(Kubernetes.podToTranscoder).filter(Boolean);;
    }
    async getPackagerById(id:string): Promise<Packager> {
        let pod:V1Pod = await this.getPod(id);
        return Kubernetes.podToPackager(pod);
    }

    async getSources(): Promise<Array<Source>> {
        let list:V1PodList = await this.listPods("");
        return list.items.map(Kubernetes.podToSource).filter(Boolean);
    }

    async selectBestNode(): Promise<string> {
        let nodes:V1NodeList = await this.listNodes();
        //todo: select node
        let node:V1Node = nodes.items[0];
        return node.metadata.name;
    }

    private async listPods(labelSelector?: string): Promise<V1PodList> {
        let res=await this.k8sApi.listNamespacedPod(this.namespace,null,null,null,null,labelSelector );
        return res.body;
    }

    private async listNodes(labelSelector?: string): Promise<V1NodeList> {
        let res=await this.k8sApi.listNode(false,null,null,null,null,labelSelector );
        return res.body;
    }

    private async waitForPodToBeReady(name:string): Promise<V1Pod> {
        return retry( async ()=> {
            this.logger.debug("Waiting for pod ",name," to be ready")
            let pod:V1Pod = await this.getPod(name);
            if (pod.status.phase=="Running") {
                this.logger.debug("Pod ",name," is ready")
                return pod;
            }
            this.logger.debug("Pod %s isn't ready phase %s: %j",name,pod.status.phase, pod);
            return null;
        },10,1000,0,this.logger);
    }

    private async runPod(name:string,nodeName:string,image:string,labels?:StringsKeyValue,command?:Array<string>,args?:Array<string>): Promise<V1Pod> {

        let container= new V1Container();
        container.name=name;
        container.image=image;
        container.imagePullPolicy="Never";
        container.command = command;
        container.args=args;

        let pod= new V1Pod();
        pod.metadata=new V1ObjectMeta();
        pod.metadata.name=name;
        pod.metadata.labels=labels;
        pod.spec = new V1PodSpec();
        pod.spec.restartPolicy="Never";
        pod.spec.containers=[];
        pod.spec.containers.push(container);
        pod.spec.nodeName=nodeName;
        let xx = await this.k8sApi.createNamespacedPod(this.namespace, pod, null, null, null);
        return xx.body;
    }

    private async getPod(podName:string): Promise<V1Pod> {
        let res=await this.k8sApi.readNamespacedPod( podName,this.namespace);
        let pod:V1Pod =res.body as V1Pod;
        return pod
    }

    private async deletePod(podName:string): Promise<V1Status> {
        let res=await this.k8sApi.deleteNamespacedPod( podName,this.namespace);
        let status:V1Status =res.body as V1Status;
        return status;
    }
    async runTranscoder(setId:string,inputId:string,trackType:TrackType,args:string[] ): Promise<Transcoder> {

        let podName:string=`transcoder-${setId.replace("_","-")}-${inputId}-${trackType==TrackType.Video ? "v" :"a"}-${randomBytes(4).toString("hex")}`;
        let commands:string[]=["/bin/sh"];

        let image:string = "kaltura/transcoder-dev";
        let labels:StringsKeyValue={};
        labels.setId=setId;
        labels.inputId=inputId;
        labels.trackType=trackType==TrackType.Video ? "Video" : "Audio";
        labels.component="transcoder";

        let nodeName=await this.selectBestNode();
        let pod:V1Pod= await this.runPod(podName,nodeName,image,labels,commands,args);

        pod=await this.waitForPodToBeReady(pod.metadata.name);
        return Kubernetes.podToTranscoder(pod);
    }

    /*
        private async runJob(config) {

            let job=new V1Job();
            let container= new V1Container();
            container.name="job12354"
            container.image="kaltura/transcode;r-dev";
            job.metadata=new V1ObjectMeta();
            job.metadata.name="job12354";
            job.spec = new V1JobSpec();
            job.spec.template= new V1PodTemplateSpec();
            job.spec.template.spec= new V1PodSpec();
            job.spec.template.spec.restartPolicy="Never";
            job.spec.template.spec.containers=[];
            job.spec.template.spec.containers.push(container);
            let xx = await this.batchApi.createNamespacedJob("default", job);
            return xx.body;

        }
        private async getJobStatus(id) {
            try {
                let res = await this.batchApi.readNamespacedJob(id,"default");
                return res.body;
            }catch(err) {
                console.warn(err);
            }
        }
    */
    private async cleaner() {
        let transcoders:Array<Transcoder>=await this.getTranscoders();

        transcoders.forEach( (transcoder:Transcoder)=> {
            if (true || transcoder.state==SystemObjectState.Completed) {
                this.deletePod(transcoder.id);
            }
        })
    }
}
