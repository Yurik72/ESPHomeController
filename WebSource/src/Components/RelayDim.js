import React from "react";
import Checkbox from "./Checkbox";
import { getBaseuri, doFetch } from "./utils"
import RangeCtl from "./RangeCtl";
import { Card, Row, Col } from "./Card"

class RelayDim extends React.Component {
    constructor(props) {
        super(props);
        console.log(props);
        const { compprops } = props;
        this.state = { isOn: false,isLdr:false, brightness:0 };
        this.toggleCheckbox = st => {
            console.log("relaydim");
            console.log(st);
           // let newstate = { ...this.state, ...st };
            this.setState(st,()=> this.SendStateUpdate());

        };
        this.API = getBaseuri() + "/" + compprops.name;
        this.onBrigthnessChanged = this.onBrigthnessChanged.bind(this);
        this.internal_setState = this.internal_setState.bind(this);

    }
    componentDidMount() {

        doFetch(this.API + "/get_state", (data) => { this.setState(data) });

    }
    internal_setState(newstate, delay) {


        console.log(newstate);
        this.setState(newstate, () => { 
                if (delay) {
                    clearTimeout(this.timer_id);
                    var self = this;
                    this.timer_id = setTimeout(() => { self.SendStateUpdate() }, delay);
                }
                else {
                    this.SendStateUpdate();
                }
        });
    }
    onBrigthnessChanged(ctl) {

        this.internal_setState({ brightness: ctl.state.rangevalue }, 300)

    }
    SendStateUpdate() {

        return fetch(this.API + "/set_state", {
            method: 'post',
            mode: 'no-cors',
            body: JSON.stringify(this.state),
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
            <Card title={() => { return (<h3>{compprops.name} </h3>); }}>
                <Row>
                    <Col num={6}>
                        <Checkbox
                            isChecked={this.state.isOn}
                            label={compprops.name}
                            handleCheckboxChange={(ch) => this.toggleCheckbox({ isOn: ch.state.isChecked })}
                            key={compprops.name}
                        />
                    </Col>
                    <Col num={6}>
                        <Checkbox
                            isChecked={this.state.isLdr}
                            label={"Is LDR"}
                            handleCheckboxChange={(ch) => this.toggleCheckbox({ isLdr: ch.state.isChecked })}
                            key="ldr"
                        />
                    </Col>
                </Row>
                <Row>
                    <Col num={12}>
                        <RangeCtl
                            ref={el => this.brigctl = el}
                            label="Brigthness"

                            rangevalue={this.state.brightness}
                            handleRangeChange={this.onBrigthnessChanged}
                        />
                    </Col>
                </Row>
            </Card>


        );
    }
}

export default RelayDim;