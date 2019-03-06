import React from "react";
import Button from "./Button"
import Popup from "reactjs-popup";
import { getHomeUrl, getConfigData, getBaseuri, doFetch } from "./utils"
import { Card, Row, Col } from "./Card"
import Arrow from "./Arrow"

const RunState = (props) => {

    const { enabled } = props;
    const style = { fill: (enabled ? "green" : "red") };
    return (
       
        <svg viewBox="0 0 10 10" width="10px" height="10px">
            <circle cx="5" cy="5" r="5" style={style} />
        </svg>
    );
}

class Services extends React.Component {
    constructor(props) {
        super(props);
        //to do further ask esp...
        this.servicelist = "RelayController,TimeController,RGBStripController,LDRController,BME280Controller".split(",");
        //const { compprops } = props;
        this.state = { services: [], servicelist: this.servicelist};

        this.handleChange = (idx) => event => {
            //alert(idx);
            let services = [...this.state.services]; //all new copy
            
            let item = { ...services[idx] }; //edited item

            item[event.target.name] = event.target.value;
            alert(JSON.stringify(item));
            services[idx] = item;

            this.setState({ services });
            
        }
        this.doSave = this.doSave.bind(this);
        this.handleselectchange = this.handleselectchange.bind(this);
        this.addservice = this.addservice.bind(this);
        this.cloneservices = this.cloneservices.bind(this);
        this.sendSave = this.sendSave.bind(this);

        
        this.state.expstate = {};
    }
    componentDidMount() {
        const { servicedata } = this.props;
        this.setState({ services: servicedata });
        doFetch(getBaseuri() + "/get_availablecontrollers", (data) => {
            var lst = data.reduce((acc, item) => { acc.push(item.name); return acc; }, []);
            this.setState({ servicelist: lst });
        });
    }
    doSave(e) {
        e.preventDefault();
        console.log("do save");
        console.log(this.state.services);
        if (window && window.confirm("Please confirm to save services ..."))
            this.sendSave();
    }
    componentWillReceiveProps(nextProps) {
        console.log("Services componentWillReceiveProps");
        console.log(nextProps);
        if (nextProps.servicedata !== this.props.servicedata) {
            const {  servicedata } = nextProps;

            this.setState({ services: servicedata });
        }
    }
    sendSave() {
        var url = getBaseuri() + "/jsonsave?";
        url += "file=services.json";
        url += "&";
        url += "data=" + JSON.stringify(this.state.services);
        console.log(url);
        return fetch(url, {
            method: 'GET',
            mode: 'no-cors',

            headers: {
                'Content-Type': 'application/json'
            }
        }).then(res => {

            return res;
        }).catch(err => err);

    }
    cloneservices() {
        return [...this.state.services];
    }
    addservice(srvdata) {
        console.log("addtrigger");
        let services = this.cloneservices();
        services.push(srvdata);
        console.log(services);
        this.setState({services: services});
    }
    handleselectchange(ev, close) {
        console.log("handle select");
        console.log(ev.target.value);
        doFetch(getBaseuri() + "/get_defaultconfig?name=" + ev.target.value, (data) => {
            console.log(data);
            this.addservice(data);
            // this.cstate.setBkColor("#" + this.intToHex(this.RGBState.color));
        });
        close();
    }
    renderAllProps(item, sidx) {
        //console.debug(getBaseuri());
        return (
            <>

            
               
                {
                   
                    Object.keys(item).map((key, idx) => {
                       
                        return (
                            <Row className="valign-wrapper" style={{marginBottom:"0px"}}>
                                <Col num={2} className="left">
                                    <label for={sidx + key} className="input-label">{key}</label>
                                </Col>
                                <Col num={4} className="left">
                                    <input type="text" id={sidx + key} name={sidx +key} value={item[key]}
                                        onChange={this.handleChange(sidx)}
                                        
                                    />
                                </Col>

                            </Row>


                            )
                    })
                }
               
            </>   
             );
            
           };
          
    render() {

        return ( 

            <div>
                <h3 >Services</h3>

                <Row>
                    <Col num={2}>
                        <Button label="Save" className="blue"
                            onClick={this.doSave}
                        />
                    </Col>
                    <Col num={2}>
                        <Popup trigger={<Button label="Add" />} position="right center" contentStyle={{width:"300px"}}>
                            {close => (
                                <div >
                                    <Row className="valign-wrapper popup-header">
                                        <Col num={10} className="left"> 
                                            <p>Select service</p>
                                        </Col>
                                        <Col num={1} className="right">
                                            <a className="close" onClick={close}> X </a>
                                        </Col>
                                    </Row>
                                    <Row  className="valign-wrapper">

                                        <select
                                            onChange={(ev) => this.handleselectchange(ev, close)}
                                            defaultValue="" required style={{ display: 'block' }}>
                                            <option key={"t"} value="" disabled>Select below</option>
                                            {
                                                this.state.servicelist.map((it, idx) => {
                                                    return (
                                                        <option key={"t"+idx}  value={it} >{it}</option>
                                                        )
                                                })
                                            }


                                        </select>
                                    </Row>

                                </div>
                            )}
                        </Popup>
                    </Col>
                </Row>
                {
                    this.state.services.map((item, sidx) => {
                        var skey = "s" + sidx;
                        var expstate = this.state.expstate;
                        var isshow = expstate[skey];
                        return(
                            <>
                                <Card  hidecontent={!isshow} title={() => {
                                    return (

                                        <>
                                            <Col num={1} className="left">
                                                <RunState enabled={item.enabled} />
                                            </Col>
                                            <Col num={4}>
                                                <h6>{ item.service }</h6>
                                                
                                            </Col>

                                            <Col num={1} className="btn-collapse">
                                                <Button className="btn-collapse" nostyle={true} onClick={() => { expstate[skey] = !isshow; this.setState({ expstate }) }}
                                                    label={isshow ? "Hide" : "Show"}>
                                                    <Arrow dir={isshow ? "up" : "down"} />
                                                </Button>

                                            </Col>
                                        </>

                                    );
                                }}>
                                    {this.renderAllProps(item, sidx)}
                                </Card>
  
                        </>
                    )}
                    )
                }
            </div>
        );
    }
}
export default Services;