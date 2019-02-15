import React from "react";
import Checkbox from "./Checkbox"
import RangeCtl from "./RangeCtl"
import TimePickCtl from "./TimePickCtl"
import ColorStatus from "./ColorStatus"
import ColorWeel from "./ColorWeel"
import { Card, Row, Col } from "./Card"
class RGBTimeRecord extends React.Component {
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

        //console.debug("render rgbtimerecord");
        const dfprops = { showcolor: true, showldr: true, showbrightness:true,showmode:true}
        const { item, idx, showcolor, showldr, showbrightness } = { ...dfprops, ...this.props };
        
        return (
            <>
                <Row>
                <Col num={4}>
                <TimePickCtl
                    label="Time"
                    timevalue={item.time}
                    handleTimeChange={t => (this.onChangeVal({ time: t.getIntTime() /*t.state.timevalue*/ }))}
                    />
                </Col>
                <Col num={2}>
                <Checkbox
                    isChecked={item.isOn}
                    label="IsOn"
                    handleCheckboxChange={ch => (this.onChangeVal({ isOn: ch.state.isChecked }))}
                    key="isOn"
                    />
                </Col>
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
                            <input name="color" type="text" value={item.color} name="color"
                                onChange={ev => {
                                    var val = parseInt(ev.target.value);
                                    this.onChangeVal({ color: val });
                                    //this.cs.setBkColor(col);
                                }}
                            />
                        </Col>
                        <Col num={4}>
                            <ColorStatus ref={el => this.cs = el} color={item.color} />
                        </Col>
                    </Row>
                }
            </>
     );
    }
}
export default RGBTimeRecord;