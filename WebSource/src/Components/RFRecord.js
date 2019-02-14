import React from "react";
import Checkbox from "./Checkbox"
import RangeCtl from "./RangeCtl"

import ColorStatus from "./ColorStatus"
import ColorWeel from "./ColorWeel"
import { Card, Row, Col } from "./Card"
class RFRecord extends React.Component {
    constructor(props) {
        super(props);

        const { handlechange } = props;

        this.handlechange = handlechange;
        this.cs = {};
    };
    onChangeVal(newVal) {

        this.handlechange(newVal);
    }
    render() {



        const { item} = this.props ;

        return (
            <>
                <Row>
                    <Col num={4}>
                        <Checkbox
                            isChecked={item.isOn}
                            label="IsOn"
                            handleCheckboxChange={ch => (this.onChangeVal({ isOn: ch.state.isChecked }))}
                            key="isOn"
                        />
                        <Checkbox
                            isChecked={item.isSwitch}
                            label="IsSwitch"
                            handleCheckboxChange={ch => (this.onChangeVal({ isSwitch: ch.state.isChecked }))}
                            key="isOn"
                        />
                    </Col>
                </Row>
            </>
        )
    }
}

export default RFRecord;