import React from "react";
import Checkbox from "./Checkbox";
import { getBaseuri, doFetch, map, mapInt, constrain, inrange } from "./utils"
import RangeCtl from "./RangeCtl";
import { Card, Row, Col } from "./Card"



class JoyStickPoint extends React.Component {
    constructor(props) {
        super(props);
        this.coords = { x: 0, y: 0};
        this.ismove = false;
        this.pointpos = { x: 0, y: 0 };
    }

    handleMouseDown = (e) => {
        this.coords = {
            x: e.pageX,
            y: e.pageY
        }
        document.addEventListener('mousemove', this.handleMouseMove);
        this.ismove = true;
        this.pointpos = this.getPointPos();
    };

    handleMouseUp = () => {
        document.removeEventListener('mousemove', this.handleMouseMove);
        this.coords = {};
        this.ismove = false;
    };

    handleMouseMove = (e) => {
        const { xpos, ypos, x_offs,y_offs, onPositionChange, minx, maxx, miny, maxy, xposmin, xposmax, yposmin, yposmax} = this.getDftProps();
        const xDiff = this.coords.x - e.pageX;
        const yDiff = this.coords.y - e.pageY;

        this.coords.x = e.pageX;
        this.coords.y = e.pageY;
        this.pointpos.x -= xDiff;
        this.pointpos.y -= yDiff;

      
        let resx = this.pointpos.x + x_offs;
        let resy = this.pointpos.y + y_offs;
        if (!inrange(resx, minx, maxx)) {
            this.pointpos.x += xDiff;
        }
        if(!inrange(resy, miny, maxy)) {
            this.pointpos.y += yDiff;
        } 


        resx = mapInt(resx, minx, maxx, xposmin, xposmax);
        resy = mapInt(resy, miny, maxy, yposmin, yposmax);
       
        onPositionChange({
            x: resx,
            y: resy
        });
    };
    getDftProps() {
        const dftlprops = {
            xpos: 0, ypos: 0, onPositionChange: (pos) => { },
            xposmin: 0, xposmax: 180, yposmin: 0, yposmax:180,
            minx: 0, maxx: 300, miny: 0, maxy: 300,
            x_offs: 50, y_offs: 50, radius: 40
        };
        return { ...dftlprops, ...this.props };
    }
    getPointPos() {
        const { xpos, ypos, x_offs, y_offs, radius, minx, maxx, miny, maxy, xposmin, xposmax, yposmin, yposmax } = this.getDftProps();

        const leftval = mapInt(xpos, xposmin, xposmax, minx, maxx) - x_offs;
        const topval = mapInt(ypos, yposmin, yposmax, miny, maxy) - y_offs;
        return { x: leftval, y: topval };
    }
    render() {
       
        const {x_offs, y_offs, radius,} = this.getDftProps();

        const pointpos = this.ismove ? this.pointpos:this.getPointPos();
       
        return (
            <svg height="100" width="100" style={{ position: 'absolute', left: pointpos.x, top: pointpos.y}}>
                <circle cx={x_offs} cy={y_offs} r={radius} stroke="black" strokeWidth="3" fill="red"
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
        width:300,
        height: 300,
        position:'relative',
        alignItems:"center",
        justifyContent: "center"
        //position:'relative'
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
        this.state = { isOn: false, posH:0,posV:0,minH:0,maxH:180,minV:0,maxV:180 };
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
                            minval={this.state.minH}
                            maxval={this.state.maxH}
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
                            minval={this.state.minV}
                            maxval={this.state.maxV}

                            rangevalue={this.state.posV}
                            handleRangeChange={this.onPosChanged}
                        />
                    </Col>
                </Row>
                <Row className="valign-wrapper">
                    <Col num={12} className="valign-wrapper" style={{ alignItems: "center", justifyContent: "center" }}>
                        <JoyStickBox style={{ height: this.jsheight, width: this.jswidth}}>
                            <canvas id="axis" ref={el => this.axis = el} height={this.jsheight} width={this.jswidth}>
                            </canvas>
                            <JoyStickPoint
                                xpos={this.state.posH}
                                ypos={this.state.posV}
                                minx={0}
                                maxx={this.jswidth}
                                miny={0}
                                maxy={this.jsheight}
                                xposmin={this.state.minH}
                                xposmax={this.state.maxH}
                                yposmin={this.state.minV}
                                yposmax={this.state.maxH}
                               
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