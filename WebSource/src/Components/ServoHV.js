import React from "react";
import Checkbox from "./Checkbox";
import { getBaseuri, doFetch } from "./utils"
import RangeCtl from "./RangeCtl";
import { Card, Row, Col } from "./Card"

class ServoHV extends React.Component {
    constructor(props) {
        super(props);
        console.log(props);
        const { compprops } = props;
        this.state = { isOn: false, posH:0,posV:0 };
        this.toggleCheckbox = st => {

            this.setState(st, () => this.SendStateUpdate());

        };
        this.API = getBaseuri() + "/" + compprops.name;
        this.onPosChanged = this.onPosChanged.bind(this);
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
    onPosChanged(ctl) {
    
        if (ctl.props.name ==="posH")
            this.internal_setState({ posH: ctl.state.rangevalue }, 300)
        else
            this.internal_setState({ posV: ctl.state.rangevalue }, 300)

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
                    <Col num={12}>
                        <Checkbox
                            isChecked={this.state.isOn}
                            label={compprops.name}
                            
                            handleCheckboxChange={(ch) => this.toggleCheckbox({ isOn: ch.state.isChecked })}
                            key={compprops.name}
                        />
                    </Col>

                </Row>
                <Row>
                    <Col num={12}>
                        <RangeCtl
                           
                            label="Position H"
                            name="posH"
                            maxval={180}
                            rangevalue={this.state.posH}
                            handleRangeChange={this.onPosChanged}
                        />
                    </Col>
                </Row>
                <Row>
                    <Col num={12}>
                        <RangeCtl
                           
                            label="Position V"
                            name="posV"
                            maxval={180}
                            rangevalue={this.state.posV}
                            handleRangeChange={this.onPosChanged}
                        />
                    </Col>
                </Row>
            </Card>


        );
    }
}

export default ServoHV;