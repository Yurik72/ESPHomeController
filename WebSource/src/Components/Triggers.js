import React from "react";
import RGBTimeRecord from "./RGBTimeRecord"
import Button from "./Button"
import Popup from "reactjs-popup";
import { getBaseuri } from "./utils"
import ColorStatus from "./ColorStatus"
import ItemSelector from "./ItemSelector"
import Arrow from "./Arrow"

import { Card, Row, Col } from "./Card"
const InpText = (props) => {

   
    const { item, name, onchange, idx } = props;
    //console.debug(props);
    return (

        <>

            <label htmlFor="1" className="input-label">{name}</label>
            <input key={name + idx} type="text" value={item[name]} name={name}
                onChange={onchange} className="input-input"
                />
            
        </>
    );
}
class Triggers extends React.Component {
    constructor(props) {
        super(props);

        //const { triggedata, servicedata } = props;
        //console.log("Triggers constructor");
        ///console.log(props);
        //this.triggers = triggedata;
        this.state = {};
        const { triggedata, servicedata } = props;
        ///only test purpose !
        this.setState({ triggers: triggedata, services: servicedata });

        this.handleChange = (idx) => event => {
            console.debug("handleChange triggers");
            this.assigntotrigger(idx, function (it) { it[event.target.name] = event.target.value; });

        }
        this.doSave = this.doSave.bind(this);
        this.state.triggers = [];
        this.state.services = [];
        this.state.expstate = {};

       
       
       
    };
    componentDidMount() {
        const { servicedata, triggedata } = this.props;
        this.setState({ triggers: triggedata, services: servicedata });
    }
    componentWillReceiveProps(nextProps) {
        console.log("Triggers componentWillReceiveProps");
        console.log(nextProps);
        if (nextProps.triggedata !== this.props.triggedata) {
            const { triggedata, servicedata } = nextProps;

            this.setState({ triggers: triggedata, services: servicedata});
        }
    }
    setTriggerprop(idx,name,val) {
        this.assigntotrigger(idx, function (it) { it[name] = val; });
    }
    clonetriggers() {
        return [...this.state.triggers];
    }
    clonetrigger(idx) {
        return { ...this.clonetriggers()[idx] };
    }
    assigntotrigger(idx, callback) {
       // console.debug("assigntotrigger");
        let triggers = this.clonetriggers(idx) //all new copy
        let item = { ...triggers[idx] }; //edited item
        //item = { ...item, ...obj };
        callback(item);
        triggers[idx] = item;
        console.debug(triggers);
        this.setState({ triggers });
        return triggers;
    }
    assigntotriggervalue(val, tidx, idx) {
        //console.debug("assigntotriggervalue");
        return this.assigntotrigger(tidx, function (item) {
            let values = [...item.value];//copy of values
            let itemvalue = { ...values[idx] }; //edited value of item
            itemvalue = { ...itemvalue, ...val }; ///merge within new fields
            values[idx] = itemvalue; //back edited value
            item.value = values; //back values 
        });
    }
    handlecomponentindexedchange(val, tidx, idx) {
        //console.debug("handlecomponentchange triggers");
        this.assigntotriggervalue(val, tidx, idx);

    }

    componentWillMount() {

    }
    doSave(e) {
        e.preventDefault();
        console.log("do save");
        console.log(this.state.triggers);
        this.sendSave();
    }
    sendSave() {
        var url = getBaseuri() + "/jsonsave?";
        url += "file=triggers.json";
        url += "&";
        url += "data=" + JSON.stringify(this.state.triggers);
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
    renderSwitchType(item, idx) {
        //console.log("renderSwitchType");
        if (item.type === 'TimeToRGBStrip')
            return this.renderTimeRgb(item.value, idx, "timergb");
        if (item.type === 'TimeToRelay')
            return this.renderTimeRgb(item.value, idx, "timerelay");
    }
    renderTimeRgb(times, tidx,timetype) {
       // console.log("renderTimeRgb");
        if (!timetype)
            timetype="timergb"
        if (!times || !Array.isArray(times))
            times = [];
        const showcolor = timetype  === "timergb";
        const showbrightness = timetype === "timergb";
        const showldr = timetype === "timergb";

        return (
            <>
                <div className="row blue-grey lighten-5 valign-wrapper" >
                    <div className="col s2">
                        Timing rules 
                    </div>
                    <div className="col s2">
                        <Button label="+" className="green" onClick={() => tidx > this.addValueRecord(tidx, timetype)} />
                    </div>
                </div>
                {
                   
                    times.map((item, idx) => {
                      //  console.debug("render trigger");
                       // console.debug(item);
                        var trkey = "tri" + tidx + idx;
                        var expstate = this.state.expstate;
                        var isshow = expstate[trkey];
                        return (
                            <Card key={trkey} hidecontent={!isshow} title={() => { return (
                               
                                <>
                                    <Col num={2} className="left leftcolholder">
                                        <Button label="X" className="red left btn-small" onClick={() => (tidx, idx) > this.removeValueRecord(tidx, idx)} />
                                    </Col>
                                    <Col num={4}>
                                        <h6>{"Time record #:" + idx} {" Trigger time:" + item.time}</h6>
                                        <div className={item.isOn ? "green" : "red"}>{item.isOn?"ON":"OFF"}</div>
                                    </Col>
                                    {showcolor &&
                                        <Col num={4}>
                                            <ColorStatus color={item.color} />
                                        </Col>
                                    }
                                    <Col num={2}>
                                        <Button className="btn-collapse" nostyle={true} onClick={() => { expstate[trkey] = !isshow; this.setState({ expstate }) }}
                                            label={isshow ? "Hide" : "Show"}>
                                            <Arrow dir={isshow ? "up" : "down" }/>
                                          </Button>
                                        
                                    </Col>
                                </>
                                
                            );
                            }}>
                               
                                   
                                <RGBTimeRecord item={item} idx={idx}
                                    showcolor={showcolor}
                                    showldr={showldr}
                                    showbrightness={showbrightness}
                                    handlechange={val => this.handlecomponentindexedchange(val, tidx, idx)} />
                                  
                                
                            </Card>
                        )

                    })
                }
            </>
        );
    }
    removeTrigger(idx) {
        console.debug("removeTrigger" + idx);
        let triggers = this.clonetriggers(); //all new copy
        triggers.splice(idx, 1);
        this.setState({ triggers });
    }
    addValueRecord(trindex, rtype) {
        console.debug("addTimeRecord" + trindex);
        let triggers = [...this.state.triggers]; //all new copy
        let item = { ...triggers[trindex] }; //edited item
        if (!item.value || !Array.isArray(item.value))
            item.value = [];

        let values = [...item.value];//copy of values
        if (rtype === "timergb") {
            values.push({ "isOn": true, "isLdr": true, "time": 0,  "bg": 1, "wxmode": -1 });
        }
        if (rtype === "timerelay") {
            values.push({ "isOn": true, "time": 0 });
        }
        item.value = values; //back values
        triggers[trindex] = item;   //back item  
        
        this.setState({ triggers });
    }

    removeValueRecord(trindex,itemidx) {
        console.log("removeValueRecord" + trindex + ":" + itemidx);
        let triggers = [...this.state.triggers]; //all new copy
        let item = { ...triggers[trindex] }; //edited item
        let values = [...item.value];//copy of values
        //console.debug(values);
        values.splice(itemidx, 1);
        //console.debug(values);
        item.value = values; //back values
        console.debug(item.value);
        triggers[trindex] = item;   //back item  
        console.log(triggers);
      //  this.setState({ triggers });
    }
    addtrigger(triggertype) {
        console.log("addtrigger");
        let triggers = this.clonetriggers();
        triggers.push({ type: triggertype, source: this.getsourceservices(triggertype).shift(), destination: this.getdestinationservices(triggertype).shift() });
        console.log(triggers);
        this.setState({ triggers });
    }
    handleselectchange(ev,close) {
        console.log("handle select");
       
        this.addtrigger(ev.target.value);
        close();
    }
    getsourceservices(triggertype) {
        if (triggertype === "TimeToRGBStrip" || triggertype === "TimeToRelay")
            return this.state.services.reduce((acc, item) => { if (item.service === "TimeController") acc.push(item.name); return acc; }, []);
        return [];
    }

    getdestinationservices(triggertype) {
        return this.state.services.reduce((acc, sitem) => {
            if ((sitem.service === "RGBStripController" && triggertype === "TimeToRGBStrip")
                ||
                (sitem.service === "RelayController" && triggertype === "TimeToRelay")
            )
                acc.push(sitem.name); return acc;
        }, [])
    }
    render() {
        //console.log("triggers render");
        //console.log(this.state);
        console.log(this.state.triggers);
        return (

            <div>
 
                <div className="row  blue-grey lighten-1 valign-wrapper">
                    <div className="col s2">
                        <h5>Triggers</h5>
                    </div>
                    <div className="col s2">
                        <Popup trigger={<Button label="Add"/> } position="right center">
                            {close => (
                                <div>
                                    <div className="row">
                                        Select a type of trigger
                             <a className="close" onClick={close}>
                                            &times;
                            </a>
                                    </div>
                                    <div className="row">

                                        <select
                                            onChange={(ev) => this.handleselectchange(ev, close)}
                                            defaultValue="" required style={{ display: 'block' }}>
                                            <option value="" disabled>select type service below</option>
                                            <option value="TimeToRGBStrip" >TimeToRGBStrip</option>
                                            <option value="LDRToRGBStrip" >LDRToRGBStrip</option>
                                            <option value="TimeToRelay" >TimeToRelay</option>
                                            
                                        </select>
                                    </div>

                                </div>
                            )}
                        </Popup>
                    
                    </div>
                    <div className="col s2">
                        <Button label="Save" className="blue"
                            onClick={this.doSave}
                        />
                    </div>
                </div>
                {
                   
                    this.state.triggers.map((item, idx) => {

                        const tkey = "tr" + idx;
                        var expstate = this.state.expstate;
                        var isshow = expstate[tkey];
                        console.log("triggers item render");
                        return (


                            <div key={tkey} className="card" >
                                
                                <div className="row blue-grey lighten-3 valign-wrapper cardtitle" >
                                    <div className="col s1 left leftcolholder">
                                        <Button label="X" className="red left btn-small"
                                        onClick={() => (idx) > this.removeTrigger(idx)}
                                    />
                                    </div>
                                    <Col num={4}>
                                        <p> {item.type}</p>
                                    </Col>
                                    <div className="col s1  btn-collapse">
                                        <Button className="btn-collapse" nostyle={true} onClick={() => { expstate[tkey] = !isshow; this.setState({ expstate }) }}
                                                label={isshow ? "Hide" : "Show"} >
                                                <Arrow dir={isshow ? "up" : "down" }/>
                                            </Button>

                                    </div>

                                </div>
                               
                                {isshow &&
                                    <div className="card-content">
                                        <Row className="valign-wrapper" >

                                            <div className="col s4 left valign-wrapper">
                                                <InpText item={item} idx={idx} onchange={this.handleChange(idx)} name={"source"} />
                                                <ItemSelector label="..." message="select service"
                                                items={this.getsourceservices(item.type)}
                                                onSelect={(v) => { this.setTriggerprop(idx, "source", v) }}
                                                />
                                            </div>

                                        <Col num={4} className="left valign-wrapper">
                                                <InpText item={item} idx={idx} onchange={this.handleChange(idx)} name={"destination"} />
                                                <ItemSelector label="..." message="select service"
                                                items={this.getdestinationservices(item.type)}
                                                    onSelect={(v) => { this.setTriggerprop(idx, "destination", v) }}
                                                />
                                            </Col>
                                        </Row>

                                        <Row>

                                            {this.renderSwitchType(item, idx)}
                                        </Row>
                                    </div>      
                                }
                             
                        </div>
                    )
                        }
                    )
                }
            </div>
        );
    }
}
export default Triggers;
