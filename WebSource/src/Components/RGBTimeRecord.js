import React from "react";
import Checkbox from "./Checkbox"
import RangeCtl from "./RangeCtl"
import TimePickCtl from "./TimePickCtl"
import ColorStatus from "./ColorStatus"
import ColorWeel from "./ColorWeel"
import { Card, Row, Col } from "./Card"
import ItemSelector from "./ItemSelector"
import { getBaseuri, doFetch } from "./utils"

import CronEdit  from './Cronedit';

class RGBTimeRecord extends React.Component {
    constructor(props) {
        super(props);

        const { handlechange } = props;

        
        this.handlechange = handlechange;
        this.modeselector = {};
        this.cs = {};
        
       
    };
    componentDidMount() {
        //console.log("componentDidMount RGBTimeRecord");
        if (!this.props.trigger)
            return;
        //console.log("select modes");
        const { destination } = this.props.trigger;
        let API = getBaseuri() + "/" + destination;
        doFetch(API + "/get_modes", (data) => {

            this.modeselector.setState({ items: data });
        });
    }
    onChangeVal(newVal) {

        this.handlechange(newVal);
    }
    render() {

        //console.debug("render rgbtimerecord");
        const dfprops = { showcolor: true, showldr: true, showbrightness:true,showmode:true,showfade:true}
        const { item, idx, showcolor, showldr, showbrightness, showmode, showfade} = { ...dfprops, ...this.props };
       
       
        return (
            <>
                <Row>
                    <Col num={12}>
                        <CronEdit
                            cronvalue={item.time}
                            onCronChange={t => (this.onChangeVal({ time: t}))}
                        />
                    </Col>
               </Row>
               <Row>

                <Col num={4}>
                <TimePickCtl
                    label="Time"
                    timevalue={item.time}
                    handleTimeChange={t => (this.onChangeVal({ time: t.getIntTime() /*t.state.timevalue*/ }))}
                    />
                    </Col>
                </Row>
                {showfade &&
                    <Row>
                        <Col num={4}>
                            <label htmlFor="fadetm" className="input-label">Fade time sec</label>
                            <input name="fadetm" type="text" value={item.fadetm} name="fadetm"
                                onChange={ev => {
                                    var val = parseInt(ev.target.value);
                                    this.onChangeVal({ fadetm: val });

                                }}
                            />
                        </Col>
                    </Row>
                }
                <Row>
                <Col num={2}>
                <Checkbox
                    isChecked={item.isOn}
                    label="IsOn"
                    handleCheckboxChange={ch => (this.onChangeVal({ isOn: ch.state.isChecked }))}
                    key="isOn"
                        />

                    </Col>
                    <Col num={2}></Col>
                    {showldr &&
                        <Col num={2}>
                            <Checkbox
                                isChecked={item.isLdr}
                                label="IsLdr"
                                handleCheckboxChange={ch => (this.onChangeVal({ isLdr: ch.state.isChecked }))}
                                key="isLdr"
                            />
                        </Col>
                    }
                </Row>

                {showbrightness &&
                    <Row>
                        <RangeCtl

                            label="Brigthness"
                            rangevalue={item.bg}
                            handleRangeChange={br => this.onChangeVal({ bg: br.state.rangevalue })}
                        />
                    </Row>
                }
                {showcolor &&
                    <Row>
                        <Col num={4}>
                            <ColorWeel
                                width={200}
                                height={200}
                                onSelect={(intcolor, hexcolor) => { this.onChangeVal({ color: intcolor }) }}
                            />
                        </Col>
                        <Col num={4}>
                        <label htmlFor="color" className="input-label" >Color</label>
                           cc
                        </Col>
                        <Col num={4}>
                            <ColorStatus ref={el => this.cs = el} color={item.color} />
                        </Col>
                    </Row>
                }
                {showmode &&
                    <Row>
                    <Col num={4} className="left valign-wrapper">
                        <label htmlFor="mode" className="input-label">Mode</label>
                        <input type="text" className="input-input" value={item.wxmode} name="mode"
                            onChange={ev => {
                                var val = parseInt(ev.target.value) || 0;
                                this.onChangeVal({ wxmode: val });
                                

                            }}
                        />
                    </Col>
                    <Col num={4}>
                        <ItemSelector ref={el => this.modeselector = el} label="..." message={"select mode"}
                            valuekey={"mode"}
                            textkey={"name"}
                            showcurrent={true}
                            currentval={item.wxmode}
                            onSelect={(selval) => {

                                var val = parseInt(selval) || 0;
                                this.onChangeVal({ wxmode: val });
                               // this.internal_setState({ wxmode: val });

                            }}
                        />
                        </Col>
                    </Row>
                }
            </>
     );
    }
}
export default RGBTimeRecord;