import React from "react";
import Checkbox from "./Checkbox";
import { getBaseuri, doFetch, map, constrain } from "./utils"
import RangeCtl from "./RangeCtl";
import { Card, Row, Col } from "./Card"



class JoyStickPoint extends React.Component {
    constructor(props) {
        super(props);
        this.coords = { x: 0, y: 0 };
       
    }
    handleMouseDown = (e) => {
        this.coords = {
            x: e.pageX,
            y: e.pageY
        }
        document.addEventListener('mousemove', this.handleMouseMove);
    };

    handleMouseUp = () => {
        document.removeEventListener('mousemove', this.handleMouseMove);
        this.coords = {};
    };

    handleMouseMove = (e) => {
        const { xpos, ypos, onPositionChange, minx, maxx, miny, maxy} = this.getDftProps();
        const xDiff = this.coords.x - e.pageX;
        const yDiff = this.coords.y - e.pageY;

        this.coords.x = e.pageX;
        this.coords.y = e.pageY;

        onPositionChange({
            x: constrain(xpos - xDiff,minx,maxx),
            y: constrain(ypos - yDiff, miny, maxy)
        });
    };
    getDftProps() {
        const dftlprops = {
            xpos: 0, ypos: 0, onPositionChange: (pos) => { }, minx:0, maxx:300, miny:0, maxy:300};
        return { ...dftlprops, ...this.props };
    }
    render() {
       
        const { xpos, ypos } = this.getDftProps();
        return (
            <svg height="100" width="100" style={{ position: 'absolute', left: xpos,top:ypos}}>
                <circle cx="50" cy="50" r="40" stroke="black" stroke-width="3" fill="red"
                    onMouseDown={this.handleMouseDown}
                    onMouseUp={this.handleMouseUp}
                />
            </svg>
            );
    }
}
function JoyStickBox({
    x = 0, y = 0,
    width, height,
    className,
    children,
    style,
}) {
    const baseStyles = {
        width,
        height,
        position:'relative'
  //      transformOrigin: '50% 50%',
        
    };

    return (
        <div className={className} style={{ ...baseStyles, ...style }}>
            {children}
        </div>
    );
}

class ServoHV extends React.Component {
    constructor(props) {
        super(props);
        this.jswidth = 300;
        this.jsheight = 300;
        console.log(props);
        const { compprops } = props;
        this.state = { isOn: false, posH:0,posV:0 };
        this.toggleCheckbox = st => {

            this.setState(st, () => this.SendStateUpdate());

        };
        this.API = getBaseuri() + "/" + compprops.name;
        this.onPosChanged = this.onPosChanged.bind(this);
        this.internal_setState = this.internal_setState.bind(this);
        this.axis = {};
    }
    componentDidMount() {

        doFetch(this.API + "/get_state", (data) => { this.setState(data) });
        this.setupJoystick();
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
    setupJoystick() {
        var canvas = this.axis;
        var context = canvas.getContext('2d');
        console.log(canvas.height / 2);
        console.log(context);
        context.beginPath();
        context.rect(0, canvas.height / 2, canvas.width, 1);
        context.fillStyle = "blue";
        context.fill();
        context.beginPath();
        context.rect(canvas.width / 2, 0, 1, canvas.height);
        context.fillStyle = "blue";
        context.fill();
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
                <Row>
                    <Col num={12}>
                        <JoyStickBox style={{ height: this.jsheight, width: this.jswidth}}>
                            <canvas id="axis" ref={el => this.axis = el} height={this.jsheight} width={this.jswidth}>
                            </canvas>
                            <JoyStickPoint
                                xpos={this.state.posH}
                                ypos={this.state.posV}
                                onPositionChange={(pos) => { this.internal_setState({ posH:pos.x ,posV:pos.y},10) }}
                            ></JoyStickPoint>
                       
                        </JoyStickBox>
                    </Col>
                </Row>
            </Card>


        );
    }
}

export default ServoHV;