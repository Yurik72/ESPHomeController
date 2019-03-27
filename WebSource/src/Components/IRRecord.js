import React from "react";
import Checkbox from "./Checkbox"

import { Card, Row, Col } from "./Card"

import Button from "./Button"
import Popup from "reactjs-popup";
import { getBaseuri, doFetch } from "./utils"

class IRRecord extends React.Component {
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
    activateAssignIRToken() {
        const number = 5;
        this.setState({ selector: [] });
        
        const cb = ()=> {
            if (this.state.selector.length < number) {
                this.dofetchirtoken(cb);
                console.log("loop" + this.state.selector.length );
            }
            else {
                console.log("done");
                const irdata = this.state.selector.shift();
                this.onChangeVal({
                    token: irdata.irtoken
                
                })
              
            }
        };
        this.dofetchirtoken(cb);
    }
    dofetchirtoken(cb) {
        doFetch(this.API + "/get_state", (data) => {
            let sel = [...this.state.selector];
            if (sel.length > 0) {  // to be sure that new is arrived
                let last = sel[sel.length - 1];
                if (data.timetick !== last.timetick && data.irtoken !== last.irtoken) {
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
                        <label htmlFor="irkey" className="input-label" >IR KEY</label>
                        <input name="irkey" type="text" value={item.token} 
                            onChange={ev => (this.onChangeVal({ token: ev.target.value }))}
                        />
                    </Col>

                </Row>
                <Row>
                    <Col num={4}>
                        <Popup trigger={<Button className="left btn-small" label="Assign key" />}
                            onOpen={() => { this.activateAssignIRToken() }}
                            position="right center">
                            {close => (
                                <div>
                                    <Row>
                                        Send IR signal a few times..
                                        <a className="close" onClick={close}>

                                        </a>
                                    </Row>
                                    <Row>
                                        <ul>
                                            {
                                                this.state.selector.map((it, idx) => {
                                                    return(
                                                    <li>
                                                        {it.irtoken}
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

export default IRRecord;