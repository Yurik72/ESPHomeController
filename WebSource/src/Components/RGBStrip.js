import React from "react";
import Checkbox from "./Checkbox";
import ColorStatus from "./ColorStatus";
import RangeCtl from "./RangeCtl";
import { getBaseuri, doFetch } from "./utils"
import ColorWeel from "./ColorWeel"
import ItemSelector from "./ItemSelector"
class RGBStrip extends React.Component {
    constructor(props) {
        super(props);

        const { compprops } = props;
        
        //this.state = { isOn: false };
        this.toggleCheckbox = ch => {
            let newstate={ isOn: ch.state.isChecked }
            this.RGBState = Object.assign(this.RGBState, newstate);
            this.setState(newstate);
            this.SendStateUpdate();

        };
        this.API = getBaseuri() + "/" + compprops.name;
        //this.getMousePos = this.getMousePos.bind(this);
       // this.setupColorWill = this.setupColorWill.bind(this);
        //this.doClick = this.doClick.bind(this);
       // this.updateStatus = this.updateStatus.bind(this);
        this.onBrigthnessChanged = this.onBrigthnessChanged.bind(this);
       // this.doTouch = this.doTouch.bind(this);
       // this.canvas = {};
       // this.context = {};
        this.cstate = {};
        this.brigctl = {};
        this.modeselector = {};
        this.RGBState = { isOn: false ,brightness:150,color:280};
        this.state = this.RGBState;
    }
    componentWillMount() {
        //this.canvas = document.getElementById("myCanvas");
        //this.context=this.canvas.getContext('2d');
        
    }
    componentDidMount() {
       
   
        doFetch(this.API + "/get_state", (data) => {
            this.setState(data);
            this.RGBState = Object.assign(this.RGBState, data);
           // this.cstate.setBkColor("#" + this.intToHex(this.RGBState.color));
        });

        doFetch(this.API + "/get_modes", (data) => {
           
            this.modeselector.setState({ items:data });
        });
    }
    onBrigthnessChanged(ctl) {
        //this.setState({ brigthness: ctl.state.rangevalue });
        let newstate = { brightness: ctl.state.rangevalue }
        this.RGBState = { ...this.RGBState, ...newstate };

        this.setState(newstate);
        this.SendStateUpdate();
    }
    SendStateUpdate() {

        return fetch(this.API + "/set_state", {
            method: 'post',
            mode: 'no-cors',
            body: JSON.stringify(this.RGBState),
            headers: {
                'Content-Type': 'application/json'
            }
        }).then(res => {

            return res;
        }).catch(err => err);
    }
    render() {

        const { compprops } = this.props;

        return (

            <div>

                <h2>{compprops.name} </h2>
                <div className="section">
                    <div className="row">
                        <div className="col s12">
                            <Checkbox
                                isChecked={this.state.isOn}
                                label={compprops.name}
                                handleCheckboxChange={this.toggleCheckbox}
                                key={compprops.name}
                            />
                        </div>
                    </div>
                    <div className="row">
                        <div className="col s12">
                            <RangeCtl
                                ref={el => this.brigctl = el}
                                label="Brigthness"
                                rangevalue={this.state.brightness}
                                handleRangeChange={this.onBrigthnessChanged}
                                />
                        </div>
                    </div>
                    <div className="row">
                        <div className="col s12 m6">
                            <ColorWeel
                                width={330}
                                height={330}
                                onSelect={(intcolor, hexcolor) => {
                                    
                                    let newstate = { color: intcolor }
                                    //console.log(newstate);
                                    this.RGBState = { ...this.RGBState, ...newstate };
                                    this.setState(newstate);
                                    this.SendStateUpdate();
                                }}
                            />
                        </div>
                        <div className="col s12 m6">
                            <ColorStatus ref={el => this.cstate = el} color={this.state.color}>

                            </ColorStatus>
                        </div>
                    </div>
                    <div className="row">
                    <div className="col s4">
                            <label htmlFor="mode">Mode</label>
                            <input type="text" value={this.state.wxmode} name="mode"
                            onChange={ev => {
                                var val = parseInt(ev.target.value) || 0;

                                let newstate = { wxmode: val }
                                //console.log(newstate);
                                this.RGBState = { ...this.RGBState, ...newstate };
                                this.setState(newstate);
                                this.SendStateUpdate();
                            }}
                            />

                        </div>
                        <div className=" col s1" >
                            <ItemSelector ref={el => this.modeselector = el} label="..." message={"select mode"}
                                valuekey={"mode"}
                                textkey={"name"}
                                onSelect={(selval) => {
                                    console.log("mode changed");
                                    var val = parseInt(selval) || 0;
                                    
                                    let newstate = { wxmode: val }
                                    //console.log(newstate);
                                    this.RGBState = { ...this.RGBState, ...newstate };
                                    this.setState(newstate);
                                    this.SendStateUpdate();
                                }}
                            />
                        </div>
                    </div>
                </div>
            </div>
        );
    }

    
}
export default RGBStrip;



