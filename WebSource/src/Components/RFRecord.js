import React from "react";
import Checkbox from "./Checkbox"

import { Card, Row, Col } from "./Card"

import Button from "./Button"
import Popup from "reactjs-popup";
import { getBaseuri, doFetch } from "./utils"

class RFRecord extends React.Component {
    constructor(props) {
        super(props);

        const { handlechange } = props;

        this.handlechange = handlechange;
        this.state = {
            selector: []
        };
        const { triggeritem } = props;
        this.API = getBaseuri() + "/" + triggeritem.source;
    };
    onChangeVal(newVal) {

        this.handlechange(newVal);
    }
    activateAssignRFToken() {
        const number = 5;
        this.setState({ selector: [] });
        
        const cb = ()=> {
            if (this.state.selector.length < number) {
                this.dofetchrftoken(cb);
                console.log("loop" + this.state.selector.length );
            }
            else {
                console.log("done");
                const rfdata = this.state.selector.shift();
                this.onChangeVal({
                    token: rfdata.rftoken,
                    len: rfdata.rfdatalen,
                    pulse: rfdata.rfdelay,
                    protocol: rfdata.rfprotocol
                })
              
            }
        };
        this.dofetchrftoken(cb);
    }
    dofetchrftoken(cb) {
        doFetch(this.API + "/get_state", (data) => {
            let sel = [...this.state.selector];
            if (sel.length > 0) {  // to be sure that new is arrived
                let last = sel[sel.length - 1];
                if (data.timetick !== last.timetick && data.rftoken !== last.rftoken) {
                    sel.push(data);
                }
            } else {
                sel.push(data);
            }
            this.setState({ selector: sel},cb);
        });
    }
    render() {



        const { item} = this.props ;

        return (
            <>
                <Row>
                    <Col num={4}>
                        <Checkbox
                            isChecked={item.isSwitch}
                            label="Is Switch"
                            handleCheckboxChange={ch => (this.onChangeVal({ isSwitch: ch.state.isChecked }))}
                            key="isOn"
                        />
                    </Col>
                    {!item.isSwitch &&
                        <Col num={4}>
                            <Checkbox
                                isChecked={item.isOn}
                                label="Is On"
                                handleCheckboxChange={ch => (this.onChangeVal({ isOn: ch.state.isChecked }))}
                                key="isOn"
                            />
                        </Col>
                    }
                </Row>
                <Row>
                    <Col num={2}>
                        <label htmlFor="rfkey" className="input-label" >RF KEY</label>
                        <input name="rfkey" type="text" value={item.token} 
                            onChange={ev => (this.onChangeVal({ token: ev.target.value }))}
                        />
                    </Col>
                    <Col num={2}>
                        <label htmlFor="rfprotocol" className="input-label" >Protocol</label>
                        <input name="rfprotocol" type="text" value={item.protocol} 
                            onChange={ev => (this.onChangeVal({ protocol: ev.target.value }))}
                        />
                    </Col>
                    <Col num={2}>
                        <label htmlFor="rflen" className="input-label" >Length</label>
                        <input name="rflen" type="text" value={item.len} 
                            onChange={ev => (this.onChangeVal({ len: ev.target.value }))}
                        />
                    </Col>
                    <Col num={2}>
                        <label htmlFor="rfpulse" className="input-label" >Pulse</label>
                        <input name="rfpulse" type="text" value={item.pulse}
                            onChange={ev => (this.onChangeVal({ pulse: ev.target.value }))}
                        />
                    </Col>
                </Row>
                <Row>
                    <Col num={4}>
                        <Popup trigger={<Button className="left btn-small" label="Assign key" />}
                            onOpen={() => { this.activateAssignRFToken() }}
                            position="right center">
                            {close => (
                                <div>
                                    <Row>
                                        Send RF signal a few times..
                                        <a className="close" onClick={close}>

                                        </a>
                                    </Row>
                                    <Row>
                                        <ul>
                                            {
                                                this.state.selector.map((it, idx) => {
                                                    return(
                                                    <li>
                                                        {it.rftoken}
                                                        </li>
                                                        )
                                                })
                                            }
                                        </ul>
                                    </Row>
                                </div>
                            )}
                         </Popup>
                    </Col>
                </Row>
            </>
        )
    }
}

export default RFRecord;