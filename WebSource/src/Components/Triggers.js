import React from "react";
import RGBTimeRecord from "./RGBTimeRecord"
import Button from "./Button"
import Popup from "reactjs-popup";
import { getBaseuri } from "./utils"
import ColorStatus from "./ColorStatus"
import ItemSelector from "./ItemSelector"
const InpText = (props) => {

   
    const { item, name, onchange, idx } = props;
    //console.debug(props);
    return (

        <div className="col s12 m6">
            <div className="left "><br />
                <label htmlFor="1">{name}</label>
                <input key={name+idx} type="text" value={item[name]} name={name}
                    onChange={onchange}
                />
            </div>
        </div>
    );
}
class Triggers extends React.Component {
    constructor(props) {
        super(props);

        const { triggedata, servicedata} = props;
        this.triggers = triggedata;
        this.state = {};
       
        this.handleChange = (idx) => event => {
            console.debug("handleChange triggers");
            this.assigntotrigger(idx, function (it) { it[event.target.name] = event.target.value; });
            /*
            let triggers = this.clonetriggers(); //all new copy

            let item = { ...triggers[idx] }; //edited item

            item[event.target.name] = event.target.value;
            
            triggers[idx] = item; 
            console.debug("handleChange triggers");
            console.debug(triggers);
            this.setState({ triggers });
            */
        }
        this.doSave = this.doSave.bind(this);
        this.state.triggers = this.triggers;
        this.state.services = servicedata;
        this.state.expstate = {};
        console.log(this.state.services)

        console.log(this.state.servicenames )
    };
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
        //console.log(val);
        /*
        let triggers = [...this.state.triggers]; //all new copy
        let item = { ...triggers[tidx] }; //edited item
        let values = [...item.value];//copy of values
        let itemvalue = { ...values[idx] }; //edited value of item
        itemvalue = { ...itemvalue,...val }; ///merge within new fields
        values[idx] = itemvalue; //back edited value
        item.value = values; //back values
        //let value = { ...values[idx]}
        triggers[tidx] = item;   //back item  
        //console.debug(triggers);
        this.setState({ triggers });
        */
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
        if (item.type === 'TimeToRGBStrip')
            return this.renderTimeRgb(item.value, idx);
    }
    renderTimeRgb(times, tidx) {
        if (!times || !Array.isArray(times))
            times = [];
        console.log(this.state.expstate);
        return (
            <>
                <div className="row" >
                    <div className="col s2">
                        Timing rules 
                    </div>
                    <div className="col s2">
                        <Button label="+" classNames="green" onClick={() => tidx > this.addValueRecord(tidx, "time")} />
                    </div>
                </div>
                {
                   
                    times.map((item, idx) => {
                        console.debug("render trigger");
                        console.debug(item);
                        var trkey = "tri" + tidx + idx;
                        var expstate = this.state.expstate;
                        var isshow = expstate[trkey];
                        return (
                            < div key={trkey}>
                                <div className="row blue" >

                                    <div className="col s2">
                                        <Button label="X" classNames="red" onClick={() => (tidx, idx) > this.removeValueRecord(tidx, idx)} />
                                    </div>
                                    <div className="col s4">
                                        <h6>{"Time record #:" + idx} {" Trigger time:" + item.time}</h6>
                                        <div className={item.isOn ? "green" : "red"}>{item.isOn?"ON":"OFF"}</div>
                                    </div>
                                    <div className="col s4">
                                        <ColorStatus  color={item.color} />
                                     </div>
                                    <div className="col s2">
                                        <Button classNames="yellow" onClick={() => { expstate[trkey] = !isshow; this.setState({ expstate }) }}
                                            label={isshow ? "Hide" : "Show"}/>
                                        
                                    </div>
                                </div>

                                {isshow &&
                                    <div className="row" >
                                    <RGBTimeRecord item={item} idx={idx} handlechange={val => this.handlecomponentindexedchange(val, tidx, idx)} />
                                    </div>
                                }
                            </div>
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
        if (rtype === "time") {
            values.push({ "isOn": true, "isLdr": true, "time": 0,  "bg": 1, "wxmode": -1 });
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
        this.setState({ triggers });
    }
    addtrigger(triggertype) {
        let triggers = this.clonetriggers();
        triggers.push({ type: triggertype });
        console.log(triggers);
        this.setState({ triggers });
    }
    handleselectchange(ev,close) {
        console.log("handle select");
       
        this.addtrigger(ev.target.value);
        close();
    }
    render() {

        return (

            <div>
 
                <div className="row">
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
                                            
                                        </select>
                                    </div>

                                </div>
                            )}
                        </Popup>
                     
                    </div>
                    <div className="col s2">
                        <Button label="Save" classNames="blue"
                            onClick={this.doSave}
                        />
                    </div>
                </div>
                {

                    this.state.triggers.map((item, idx) => {

                        const tkey = "tr" + idx;
                        var expstate = this.state.expstate;
                        var isshow = expstate[tkey];
                        return (


                            <div key={tkey}>
                            <div className="row" >
                                <div className="col s2">
                                    <Button label="X" classNames="red"
                                        onClick={() => (idx) > this.removeTrigger(idx)}
                                    />
                                </div>
                                <div className="col s2">
                                    <p> {item.type}</p>
                                </div>
                                <div className="col s2">
                                        <Button classNames="yellow" onClick={() => { expstate[tkey] = !isshow; this.setState({ expstate }) }}
                                        label={isshow ? "Hide" : "Show"} />

                                </div>
                            </div>
                                {isshow &&
                                    <div className="row" >
                                        <div className=" col s1" >
                                            <ItemSelector label="..." message="select service"
                                                items={this.state.services.reduce((acc, item) => { if (item.service === "TimeController") acc.push(item.name); return acc; }, [])}
                                                onSelect={(v) => { this.setTriggerprop(idx, "source", v) }}
                                            />
                                        </div>
                                        <div className="col s4">
                                            <InpText item={item} idx={idx} onchange={this.handleChange(idx)} name={"source"} />

                                        </div>
                                        <div className=" col s1" >
                                            <ItemSelector label="..." message="select service"
                                                items={this.state.services.reduce((acc, item) => { if (item.service === "RGBStripController") acc.push(item.name); return acc; }, [])}
                                                onSelect={(v) => { this.setTriggerprop(idx, "destination", v) }}
                                            />
                                        </div>
                                        <div className="col s4">
                                            <InpText item={item} idx={idx} onchange={this.handleChange(idx)} name={"destination"} />
                                        </div>
                                    </div>
                                }
                                {isshow &&
                                    <div className="row" >

                                        {this.renderSwitchType(item, idx)}
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
